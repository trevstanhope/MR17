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
#include <mcp2515.h>
#include <global.h>
#include <defaults.h>
#include <mcp2515_defs.h>
#include <DualVNH5019MotorShield.h>
#include <RunningMedian.h>

// Constants
const int WHEEL_PIN = A2;
const int STEER_PIN = A3;
const int SUSP_PIN = A4;
const int BAUD = 9600;
const int OUTPUT_LENGTH = 256;
const int DATA_LENGTH = 128;
const char PID[] = {"VDC"};
const int STEERING_SAMPLES = 5;
const float STEERING_P_GAIN = 1;
const float STEERING_I_GAIN = 0.0;
const float STEERING_D_GAIN = 0.1;

// Variables
int wheel;
int steer;
int susp;
char output_buffer[OUTPUT_LENGTH];
char data_buffer[DATA_LENGTH];
int chksum; 
DualVNH5019MotorShield motors;
RunningMedian steering_error = RunningMedian(STEERING_SAMPLES);

// Setup
void setup() {
  Serial.begin(BAUD);
  pinMode(WHEEL_PIN, INPUT);
  pinMode(STEER_PIN, INPUT);
  pinMode(SUSP_PIN, INPUT);
  motors.init();
}

// Loop
void loop() {
  wheel = analogRead(WHEEL_PIN);
  susp = analogRead(SUSP_PIN);
  steer = analogRead(STEER_PIN);
  steering_error.add(steer-wheel);
  int P = STEERING_P_GAIN * steer-wheel;
  int I = STEERING_I_GAIN * steering_error.getAverage();
  int D = STEERING_D_GAIN * (steering_error.getHighest() - steering_error.getLowest());
  motors.setM1Speed(P+I+D);
  sprintf(data_buffer, "{\"wheel\":%d,\"steer\":%d,\"susp\":%d}", wheel, steer, susp);
  chksum = checksum(data_buffer);
  sprintf(output_buffer, "{\"data\":%s,\"pid\":\"%s\",\"chksum\":%d}", data_buffer, PID, chksum);
  Serial.println(output_buffer);
}

// Functions
int checksum(char* buf) {
  int sum = 0;
  for (int i = 0; i < DATA_LENGTH; i++) {
    sum = sum + buf[i];
  }
  return sum % 256;
}
