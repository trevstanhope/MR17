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
const int DATA_LENGTH = 128;
const int JSON_LENGTH = 256;
const int INPUT_LENGTH = 256;
const char UID[] = "ODB";

// Variables
char output_buffer[OUTPUT_LENGTH];
char data_buffer[DATA_LENGTH];
char input_buffer[INPUT_LENGTH] = {'\0'};
StaticJsonBuffer<JSON_LENGTH> json_buffer;
int chksum; 

// Setup
void setup() {
  Serial.begin(BAUD);
}

// Loop
void loop() {
  sprintf(data_buffer, "{\"wheel\":%d,\"steer\":%d,\"susp\":%d}", 0, 0, 0);
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
