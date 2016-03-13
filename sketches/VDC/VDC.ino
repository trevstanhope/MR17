/*
  VDC.ino
  Vehicle Dynamics Controller
  * Steering
  * Ballast
  
  DualVNH5019 Motor Shield
  * M1 is Steering
  * M2 is Ballast
*/

// Libraries
#include <Canbus.h>
#include <DualVNH5019MotorShield.h>
#include <RunningMedian.h>
#include <ArduinoJson.h>

// Common Constants
const int BAUD = 9600;
const int OUTPUT_LENGTH = 256;
const int DATA_LENGTH = 256;
const int JSON_LENGTH = 512;
const int INPUT_LENGTH = 256;
const int CANBUS_LENGTH = 8;
unsigned char ESC_ID = 10;
unsigned char TSC_ID = 11;
unsigned char VDC_ID = 12;

// Unique Constants
const int WHEEL_PIN = A2;
const int STEER_PIN = A3;
const int SUSP_PIN = A4;
const int STEERING_SAMPLES = 5;
const float STEERING_P_GAIN = 1;
const float STEERING_I_GAIN = 0.0;
const float STEERING_D_GAIN = 0.1;
unsigned int _PID = 0x0003;

// Variables
RunningMedian steering_error = RunningMedian(STEERING_SAMPLES);

// Motor Controller
DualVNH5019MotorShield motors;

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
  Serial.begin(BAUD);
  delay(10);
  Canbus.init(CANSPEED_500);
  delay(10);
  pinMode(WHEEL_PIN, INPUT);
  pinMode(STEER_PIN, INPUT);
  pinMode(SUSP_PIN, INPUT);
  //motors.init();
}

// Loop
void loop() {
  int wheel = analogRead(WHEEL_PIN);
  int susp = analogRead(SUSP_PIN);
  int steer = analogRead(STEER_PIN);
  steering_error.add(steer-wheel);
  int P = STEERING_P_GAIN * steer-wheel;
  int I = STEERING_I_GAIN * steering_error.getAverage();
  int D = STEERING_D_GAIN * (steering_error.getHighest() - steering_error.getLowest());
  //motors.setM1Speed(P+I+D);
  
  // CANBus
  canbus_tx_buffer[0] = VDC_ID;
  canbus_tx_buffer[1] = wheel;
  canbus_tx_buffer[2] = susp;
  canbus_tx_buffer[3] = steer;
  canbus_tx_buffer[4] = 0;
  canbus_tx_buffer[5] = 0;
  canbus_tx_buffer[6] = 0;
  canbus_tx_buffer[7] = 0;
  Canbus.message_tx(_PID, canbus_tx_buffer);
  // Canbus.message_rx(_PID, canbus_rx_buffer);
  
  // Serial Debugger
  if (Serial) {
    root["wheel"] = wheel;
    root["steer"] = steer;
    root["susp"] = susp;
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
