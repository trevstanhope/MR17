/*
  OBD.ino
  On-Board Diagnostics
*/

// Libraries
#include <Canbus.h>
#include <ArduinoJson.h>
#include <RunningMedian.h>

// Common Constants
const int BAUD = 9600;
const int OUTPUT_LENGTH = 256;
const int DATA_LENGTH = 256;
const int JSON_LENGTH = 512;
const int INPUT_LENGTH = 256;
const int CANBUS_LENGTH = 8;
unsigned char ESC_ID = 10; // This is the ID we will use to check if the message was for this device.
unsigned char TSC_ID = 11; // This is the ID we will use to check if the message was for this device.
unsigned char VDC_ID = 12; // This is the ID we will use to check if the message was for this device.

// Unique Constants 

// Variables
int chksum; 

// Buffers
char output_buffer[OUTPUT_LENGTH];
char data_buffer[DATA_LENGTH];
unsigned char canbus_rx_buffer[CANBUS_LENGTH];  // Buffer to store the incoming data
unsigned char canbus_tx_buffer[CANBUS_LENGTH];  // Buffer to store the incoming data

// JSON
StaticJsonBuffer<JSON_LENGTH> json_buffer;
JsonObject& root = json_buffer.createObject();

// Setup
void setup() {
  Serial.begin(BAUD);
  delay(10);
  Canbus.init(CANSPEED_500); /* Initialise MCP2515 CAN controller at the specified speed */
  delay(10);
}

// Loop
void loop() {
  
  // Check CANBus
  unsigned int _UID = Canbus.message_rx(canbus_rx_buffer); // Check to see if we have a message on the Bus
  int _ID = canbus_rx_buffer[0];
  Serial.println(_ID);
  
  if (_ID == ESC_ID) { // If we do, check to see if the PID matches this device
    // Create Output JSON with ArduinoJSON;
    root["throttle"] = canbus_rx_buffer[1];
    root["load"] = canbus_rx_buffer[2];
    root["eng_temp"] = canbus_rx_buffer[3];
    root["oil"] = canbus_rx_buffer[4];
    root["lbrake"] = canbus_rx_buffer[5];
    root["rbrake"] = canbus_rx_buffer[6];
    root["bat"] = canbus_rx_buffer[7];
    root["user"] = canbus_rx_buffer[6];
  }
  else if (_ID == VDC_ID) {
    root["ballast"] = canbus_rx_buffer[1];
    root["susp"] = canbus_rx_buffer[2];
  }
  else if (_ID == TSC_ID) {
    root["gear"] = canbus_rx_buffer[1];
    root["cvt_slip"] = canbus_rx_buffer[2];
    root["trans_temp"] = canbus_rx_buffer[3];
    root["cvt"] = canbus_rx_buffer[4];
    root["rpm"] = canbus_rx_buffer[5];
    root["lock"] = canbus_rx_buffer[6];
  }
  root.printTo(data_buffer, sizeof(data_buffer));
  
  // Send JSON to Serial
  int chksum = checksum(data_buffer);
  sprintf(output_buffer, "{\"data\":%s,\"chksum\":%d}", data_buffer, chksum);
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

