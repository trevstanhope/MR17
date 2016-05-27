/*
  VDC.ino
  Vehicle Dynamics Controller
  * Steering
  * Ballast
  
  DualVNH5019 Motor Shield
  * M1 is Steering
  * M2 is Ballast
*/

/* --- Libraries --- */
#include <Canbus.h>
#include <DualVNH5019MotorShield.h>
#include <RunningMedian.h>
#include <ArduinoJson.h>
#include "MR17_CAN.h"

/* --- Global Constants --- */
const int WHEEL_PIN = A2; // operator wheel
const int STEER_PIN = A4; // steering actuator
const int SUSP_PIN = A5; // suspension ride height
const int STEERING_SAMPLES = 5;
const float STEERING_P_GAIN = 3.0;
const float STEERING_I_GAIN = 0.0;
const float STEERING_D_GAIN = 0.5;
const int STEERING_MIN = 800;
const int STEERING_MAX = 1024;
const int STEERING_MILLIAMP_LIMIT = 25000;
const int STEERING_DEADBAND = 25;
const int BALLAST_MILLIAMP_LIMIT = 30000;
const int BALLAST_DEADBAND = 25;
const float BALLAST_P_GAIN = -5.0;

/* --- Global Variables --- */
// 
RunningMedian steering_array = RunningMedian(STEERING_SAMPLES);
int canbus_status = 0;

// Motor Controller
DualVNH5019MotorShield motors;
int steering_error = 0;
int steering_error_prev = 0;
int steering_power = 0;
int steering_sp = 0;
int steering_pv = 0;
int suspension_pv = 0;
int suspension_sp = 512;
int suspension_error = 0;
int ballast_error = 0;
int ballast_power = 0;
int cart_mode = CART_MODE_MANUAL;
int cart_direction = CART_DIRECTION_OFF;

// JSON
StaticJsonBuffer<JSON_LENGTH> json_buffer;
JsonObject& root = json_buffer.createObject();

// Character Buffers
char output_buffer[OUTPUT_LENGTH];
char data_buffer[DATA_LENGTH];
unsigned char canbus_tx_buffer[CANBUS_LENGTH];
unsigned char canbus_rx_buffer[CANBUS_LENGTH];

// Setup
void setup() {

  // USB
  Serial.begin(BAUD);
  delay(10);
  
  // CANBUS
  int canbus_attempts = 0;
  while (!canbus_status) {
    canbus_status = Canbus.init(CANSPEED_500);
    delay(10);
    canbus_attempts++;
    if (canbus_attempts > 10) {
      break;
    }
  }

  // Pins
  pinMode(WHEEL_PIN, INPUT);
  pinMode(STEER_PIN, INPUT);
  pinMode(SUSP_PIN, INPUT);

  // Motors
  motors.init();
}

// Loop
void loop() {
  
  // CANBus
  if (canbus_status) {
    unsigned int UID = Canbus.message_rx(canbus_rx_buffer);
    int ID = canbus_rx_buffer[0];
    if (ID == ESC_A_ID) {
      cart_mode = canbus_rx_buffer[6];
      cart_direction = canbus_rx_buffer[7];
    }
  }
  
  // Motor Control
  steering_power = set_steering();
  ballast_power = set_ballast();
  
  // CANBus
  if (canbus_status) {
    canbus_tx_buffer[0] = VDC_ID;
    canbus_tx_buffer[1] = map(steering_sp, 0, 1024, 0, 255);
    canbus_tx_buffer[2] = map(steering_pv, 0, 1024, 0, 255);
    canbus_tx_buffer[3] = map(steering_power, -400, 400, 0, 255); // M1 output
    canbus_tx_buffer[4] = suspension_pv;
    canbus_tx_buffer[5] = map(steering_power, -400, 400, 0, 255); // M2 output
    canbus_tx_buffer[6] = 0;
    canbus_tx_buffer[7] = 0;
    Canbus.message_tx(VDC_PID, canbus_tx_buffer);
  }
  
  // Serial Debugger
  if (Serial) {
    root["steeering_sp"] = steering_sp;
    root["steering_pv"] = steering_pv;
    root["steering_pwr"] = steering_power;
    root["suspension_sp"] = suspension_sp;
    root["suspension_pv"] = suspension_pv;
    root["ballast_pwr"] = ballast_power;
    root["cart_mode"] = cart_mode;
    root["cart_direction"] = cart_direction;
    root["canbus"] = canbus_status;
    root.printTo(data_buffer, sizeof(data_buffer));
    int chksum = checksum(data_buffer);
    sprintf(output_buffer, "{\"data\":%s,\"chksum\":%d}", data_buffer, chksum);
    Serial.println(output_buffer);
  }
}

// Functions
int checksum(char* buf) {
  int sum = 0;
  for (int i = 0; i < DATA_LENGTH; i++) {
    sum = sum + buf[i];
  }
  return sum % 256;
}

// Set Steering
int set_steering(void) {
  steering_sp = analogRead(WHEEL_PIN);
  steering_pv = map(analogRead(STEER_PIN), STEERING_MIN, STEERING_MAX, 0, 1024);
  steering_error_prev = steering_error;
  steering_error = steering_pv - steering_sp;
  steering_array.add(steering_sp - steering_pv);
  if (abs(steering_error) < STEERING_DEADBAND) {
    steering_power = 0;
  }
  else {
    int P = STEERING_P_GAIN * (steering_error);
    int I = STEERING_I_GAIN * steering_array.getAverage();
    int D = STEERING_D_GAIN * (steering_error - steering_error_prev);
    steering_power = P + I + D;
    if (steering_power > 400) { steering_power = 400; }
    else if (steering_power < -400) { steering_power = -400; }
    if (motors.getM2CurrentMilliamps() >  STEERING_MILLIAMP_LIMIT) {
      steering_power = 0;
    }
  }
  motors.setM1Speed(steering_power);
  steering_power = map(steering_power, -400, 400, 0, 255);
  return steering_power;
}

// Set Ballast
int set_ballast(void) {
  suspension_pv = analogRead(SUSP_PIN);
  if (cart_mode == CART_MODE_MANUAL) {
    if (cart_direction == CART_DIRECTION_FORWARD) {
      ballast_power = -400;
    }
    else if (cart_direction == CART_DIRECTION_BACKWARD) {
      ballast_power = 400;
    }
    else if (cart_direction == CART_DIRECTION_OFF) {
      ballast_power = 0;
    }
  }
  else if (cart_mode == CART_MODE_AUTO) {
    suspension_error = suspension_sp - suspension_pv;
    ballast_power = BALLAST_P_GAIN * suspension_error;
  }
  if (motors.getM2CurrentMilliamps() > BALLAST_MILLIAMP_LIMIT) {
    ballast_power = 0;
  }
  motors.setM2Speed(ballast_power);
  ballast_power = map(ballast_power, -400, 400, 0, 255);
  return ballast_power;
}

