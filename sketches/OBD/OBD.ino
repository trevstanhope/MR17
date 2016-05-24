/*
  OBD.ino
  On-Board Diagnostics
*/

/* --- Libraries --- */
#include <Canbus.h>
#include <ArduinoJson.h>
#include <RunningMedian.h>
#include "MR17_CAN.h"

/* --- Global Constants --- */

/* --- Global Variables --- */
// Variables
int chksum;
int canbus_status = 0;

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

  // Initialize USB
  Serial.begin(BAUD);
  delay(10);
  
  // Initialise MCP2515 CAN controller at the specified speed
  int canbus_attempts = 0;
  while (!canbus_status) {
    canbus_status = Canbus.init(CANSPEED_500);
    delay(10);
    canbus_attempts++;
    if (canbus_attempts > 10) {
      break;
    }
  }
}

// Loop
void loop() {
  
  // Check CANBus
  unsigned int UID = Canbus.message_rx(canbus_rx_buffer); // Check to see if we have a message on the Bus
  int ID = canbus_rx_buffer[0];
  
  // Check to see if the ID matches a known device on CAN
  if (ID == ESC_A_ID) { 
    root["stat"] = canbus_rx_buffer[1]; // 0 is off, 1 is standby, 2 is running, 3 is ignition
    root["thro"] = map(canbus_rx_buffer[2], 0, 255, 0, 100);
    root["kill"] = canbus_rx_buffer[3]; // 0 is none, 1 is seat, 2 is hitch
    root["int"] = canbus_rx_buffer[4];
    root["lbpw"] = map(canbus_rx_buffer[5], 0, 255, 0, 100); // 0 to 256 bits is 0 to 100 %
    root["rbpw"] = map(canbus_rx_buffer[6], 0, 255, 0, 100); // 0 to 256 bits is 0 to 100 %
  }
  if (ID == ESC_B_ID) { 
    root["user"] = canbus_rx_buffer[1]; // 0 to 256 bits is 0 to 25 V
    root["temp"] = canbus_rx_buffer[2];
    root["oilp"] = canbus_rx_buffer[3];
    root["temp"] = canbus_rx_buffer[4];
    root["lbse"] = canbus_rx_buffer[5]; // 0 to 100 %
    root["rbse"] = canbus_rx_buffer[6]; // 0 to 100 %
    root["batt"] = mapfloat(canbus_rx_buffer[7], 0, 255, 0, 25); // 0 to 25 V
  }
  else if (ID == VDC_ID) {
    root["wheel"] = map(canbus_rx_buffer[1], 0, 255, -100, 100); // -100 to 100 %
    root["steer"] = map(canbus_rx_buffer[2], 0, 255, -100, 100); // -100 to 100 %
    root["mot1"] = mapfloat(canbus_rx_buffer[3], 0, 255, -100, 100) + 1; // -100 to 100 %
    root["susp"] = mapfloat(canbus_rx_buffer[4], 0, 255, 0, 100); // 0 to 100 %
    root["mot2"] = mapfloat(canbus_rx_buffer[5], 0, 255, -100, 100) + 1; // 0 to 100 %
  }
  else if (ID == TSC_ID) {
    root["gear"] = canbus_rx_buffer[1]; // 0,1,2,3,4 is neutral, 1st, 2nd, 3rd and reverse, respectively
    root["slip"] = map(canbus_rx_buffer[2], 0, 255, 0, 100); // 0 to 100 %
    root["temp"] = map(canbus_rx_buffer[3], 0, 255, -100, 100); //-100 to 100 degrees Celcius
    root["cvtp"] = map(canbus_rx_buffer[4], 0, 255, 0, 100);
    root["erpm"] = map(canbus_rx_buffer[5], 0, 255, 0, 3600); // 0 to 3600 rpm
    root["drpm"] = map(canbus_rx_buffer[6], 0, 255, 0, 3600); // 0 to 3600 rpm
    root["lock"] = canbus_rx_buffer[7]; // 0 is unlocked, 1 is locked
  }
  root.printTo(data_buffer, sizeof(data_buffer));
  int chksum = checksum(data_buffer);
  sprintf(output_buffer, "{\"data\":%s,\"chksum\":%d,\"id\":%d,\"canbus\":%d}", data_buffer, chksum, ID, canbus_status);
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

float mapfloat(long x, long in_min, long in_max, long out_min, long out_max) {
 return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

