/*
  OBD.ino
  On-Board Diagnostics
*/

// Libraries
#include <Canbus.h>
#include <ArduinoJson.h>
#include <mcp2515.h>
#include <global.h>
#include <defaults.h>
#include <mcp2515_defs.h>
#include <RunningMedian.h>

// Constants
const int BAUD = 9600;
const int OUTPUT_LENGTH = 256;
const int DATA_LENGTH = 256;
const int JSON_LENGTH = 512;
const int INPUT_LENGTH = 256;
const char UID[] = "ODB";

// Variables
char output_buffer[OUTPUT_LENGTH];
char data_buffer[DATA_LENGTH];
char input_buffer[INPUT_LENGTH] = {'\0'};
StaticJsonBuffer<JSON_LENGTH> json_buffer;
JsonObject& root = json_buffer.createObject();
int chksum; 

// Setup
void setup() {
  Serial.begin(BAUD);
}

// Loop
void loop() {

  // Update values
  float cvt_ratio = 0.0;
  int cvt_slip = 0;
  int rpm = 0;
  int throttle = 0;
  int load = 0;
  int eng_rpm = 0;
  int eng_temp = 0;
  int susp = 0;
  float oil = 0;
  int ballast  = 0;
  int lbrake = 0;
  int rbrake = 0;
  float bat = 0;
  int gear = 0;
  int user = 0;
  int lock = 0;
  int trans_temp = 0;
  
  // Create Output JSON with ArduinoJSON;
  root["cvt"] = cvt_ratio;
  root["rpm"] = rpm;
  root["throttle"] = throttle;
  root["load"] = load;
  root["eng_temp"] = eng_temp;
  root["oil"] = oil;
  root["susp"] = susp;
  root["ballast"] = ballast;
  root["lbrake"] = lbrake;
  root["rbrake"] = rbrake;
  root["bat"] = bat;
  root["user"] = user;
  root["lock"] = lock;
  root["gear"] = gear;
  root["cvt_slip"] = cvt_slip;
  root["trans_temp"] = trans_temp;
  root.printTo(data_buffer, sizeof(data_buffer));
  
  // Create Output JSON manually
  chksum = checksum(data_buffer);
  sprintf(output_buffer, "{\"data\":%s,\"pid\":\"%s\",\"chksum\":%d}", data_buffer, UID, chksum);
  Serial.println(output_buffer);
}

// Checksum
int checksum(char* buf) {
  int sum = 0;
  for (int i = 0; i < DATA_LENGTH; i++) {
    sum = sum + buf[i];
  }
  return sum % 256;
}

float float_to_char(float val) {
  return 0.0;
}

