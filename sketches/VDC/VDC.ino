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

/* --- Global Variables --- */
// 
RunningMedian steering_array = RunningMedian(STEERING_SAMPLES);
int canbus_status = 0;

// Motor Controller
DualVNH5019MotorShield motors;
int steering_error = 0;
int steering_error_prev = 0;
int steering_power = 0;

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

  // Steering
  int wheel = analogRead(WHEEL_PIN);
  int steer = map(analogRead(STEER_PIN), STEERING_MIN, STEERING_MAX, 0, 1024);
  steering_error_prev = steering_error;
  steering_error = steer - wheel;
  steering_array.add(steer - wheel);
  if (abs(steering_error) < STEERING_DEADBAND) {
    steering_power = 0;
  }
  else {
    int P = STEERING_P_GAIN * (steering_error);
    int I = STEERING_I_GAIN * steering_array.getAverage();
    int D = STEERING_D_GAIN * (steering_error - steering_error_prev);
    steering_power = P + I + D;
    if (motors.getM2CurrentMilliamps() >  STEERING_MILLIAMP_LIMIT) {
      steering_power = 0;
    }
  }
  motors.setM1Speed(steering_power);

  // Ballast
  int susp = analogRead(SUSP_PIN);
  int ballast_power = 0;
  motors.setM2Speed(ballast_power);
  
  // CANBus
  canbus_tx_buffer[0] = VDC_ID;
  canbus_tx_buffer[1] = map(wheel, 0, 1024, 0, 255);
  canbus_tx_buffer[2] = map(steer, 0, 1024, 0, 255);
  canbus_tx_buffer[3] = map(steering_power, -400, 400, 0, 255); // M1 output
  canbus_tx_buffer[4] = map(susp, 0, 1024, 0, 255);
  canbus_tx_buffer[5] = map(steering_power, -400, 400, 0, 255); // M2 output
  canbus_tx_buffer[6] = 0;
  canbus_tx_buffer[7] = 0;
  Canbus.message_tx(VDC_PID, canbus_tx_buffer);
  Canbus.message_rx(canbus_rx_buffer);
  
  // Serial Debugger
  if (Serial) {
    root["wheel"] = wheel;
    root["steer"] = steer;
    root["steer_pwr"] = steering_power;
    root["susp"] = susp;
    root["ballast_pwr"] = ballast_power;
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
