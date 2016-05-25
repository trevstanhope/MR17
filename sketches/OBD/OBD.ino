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

/* --- Setup --- */
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

/* --- Loop --- */
void loop() {
  
  // Check CANBus
  unsigned int UID = Canbus.message_rx(canbus_rx_buffer); // Check to see if we have a message on the Bus
  int ID = canbus_rx_buffer[0];

  // Create empty JSON buffer
  StaticJsonBuffer<JSON_LENGTH> json_buffer;
  JsonObject& root = json_buffer.createObject();
  
  // Check to see if the ID matches a known device on CAN
  if (ID == ESC_A_ID) { 
    root["run"] = canbus_rx_buffer[1];
    root["cart"] = canbus_rx_buffer[2];
    root["pull"] = canbus_rx_buffer[3];
    root["cvt"] = canbus_rx_buffer[4];
    root["throttle"] = canbus_rx_buffer[5];
    root["right_brake"] = canbus_rx_buffer[6]; 
    root["left_brake"] = canbus_rx_buffer[7];
  }
  if (ID == ESC_B_ID) { 
    root["rfid_auth"] = canbus_rx_buffer[1];
    root["engine_temp"] = canbus_rx_buffer[2];
    root["oil_pressure"] = canbus_rx_buffer[3];
    root["cvt_temp"] = canbus_rx_buffer[4];
    root["lbse"] = canbus_rx_buffer[5];
    root["rbse"] = canbus_rx_buffer[6];
    root["voltage"] = mapfloat(canbus_rx_buffer[7], 0, 255, 0, 25); // 0 to 25 V
  }
  else if (ID == VDC_ID) {
    root["wheel"] = map(canbus_rx_buffer[1], 0, 255, -100, 100);
    root["steer"] = map(canbus_rx_buffer[2], 0, 255, -100, 100); 
    root["mot1"] = map(canbus_rx_buffer[3], 0, 255, -100, 100);
    root["susp"] = map(canbus_rx_buffer[4], 0, 255, 0, 100);
    root["mot2"] = map(canbus_rx_buffer[5], 0, 255, -100, 100);
    root["slot6"] = canbus_rx_buffer[6];
    root["slot7"] = canbus_rx_buffer[7];
  }
  else if (ID == TSC_ID) {
    root["erpm"] = canbus_rx_buffer[1]; // 0,1,2,3,4 is neutral, 1st, 2nd, 3rd and reverse, respectively
    root["drpm"] = map(canbus_rx_buffer[2], 0, 255, 0, 100); // 0 to 100 %
    root["slot3"] = map(canbus_rx_buffer[3], 0, 255, -100, 100); //-100 to 100 degrees Celcius
    root["cvtp"] = map(canbus_rx_buffer[4], 0, 255, 0, 100);
    root["cvtt"] = map(canbus_rx_buffer[5], 0, 255, 0, 100); // 0 to 3600 rpm
    root["gear"] = canbus_rx_buffer[6]; // 0 to 3600 rpm
    root["lock"] = canbus_rx_buffer[7]; // 0 is unlocked, 1 is locked
  }
  root.printTo(data_buffer, sizeof(data_buffer));
  int chksum = checksum(data_buffer);
  sprintf(output_buffer, "{\"data\":%s,\"chksum\":%d,\"id\":%d,\"canbus\":%d}", data_buffer, chksum, ID, canbus_status);
  Serial.println(output_buffer);
}

/* --- Functions --- */
// Checksum
int checksum(char* buf) {
  int sum = 0;
  for (int i = 0; i < DATA_LENGTH; i++) {
    sum = sum + buf[i];
  }
  return sum % 256;
}

// Float to Char
float float_to_char(float val) {
  return 0.0;
}

// Map function adapted to floating points numbers
float mapfloat(long x, long in_min, long in_max, long out_min, long out_max) {
 return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

