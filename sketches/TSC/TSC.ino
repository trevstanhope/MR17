/*
  TSC.ino
  Transmission System Controller
*/

// Libraries
#include <RunningMedian.h>
#include <Canbus.h>
#include <ArduinoJson.h>
#include <DualVNH5019MotorShield.h>

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
const int ENGINE_RPM_PIN = 21;
const int SHAFT_RPM_PIN = 20;
const int ENGINE_BLIPS = 8;
const int SHAFT_BLIPS = 8;
const int INTERVAL = 1000;
unsigned int _PID = 0x0002;

// Variables
volatile int counter_engine = 0;
volatile int counter_shaft = 0;
int time_a = millis();
int time_b = millis();
int freq_engine; 
int freq_shaft; 
int freq_wheel = 0;
int cvt_pos = 0;
int cvt_target = 0;
int trans_gear = 0;
int trans_locked = 0;
int chksum; 
boolean can_status = false;

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
  attachInterrupt(0, increment_engine, RISING);
  attachInterrupt(1, increment_shaft, RISING);
}

// Loop
void loop() {
  counter_engine = 0;
  counter_shaft = 0;
  delay(INTERVAL);
  freq_engine = int(counter_engine * 60 / ENGINE_BLIPS);
  freq_shaft = int(counter_engine * 60 / SHAFT_BLIPS);
  freq_wheel = 0;
  cvt_pos = 0;
  cvt_target = 0;
  trans_gear = 0;
  trans_locked = 0;
  
  // CANBus
  canbus_tx_buffer[0] = TSC_ID;
  canbus_tx_buffer[1] = freq_engine;
  canbus_tx_buffer[2] = freq_shaft;
  canbus_tx_buffer[3] = freq_wheel;
  canbus_tx_buffer[4] = cvt_pos;
  canbus_tx_buffer[5] = cvt_target;
  canbus_tx_buffer[6] = trans_gear;
  canbus_tx_buffer[7] = trans_locked;
  Canbus.message_tx(_PID, canbus_tx_buffer);
  // Canbus.message_rx(_PID, canbus_rx_buffer);
  
  // Serial
  if(Serial) {
    root["shaft"] = freq_shaft;
    root["engine"] = freq_engine;
    root["wheel"] = freq_wheel;
    root["pos"] = cvt_pos;
    root["target"] = cvt_target;
    root["gear"] = trans_gear;
    root["locked"] = trans_locked;
    root.printTo(data_buffer, sizeof(data_buffer));
    chksum = checksum(data_buffer);
    sprintf(output_buffer, "{\"data\":%s,\"chksum\":%d}", data_buffer, chksum);
    Serial.println(output_buffer);
  }
}

// Functions
void increment_engine(void) {
  counter_engine++;
}

void increment_shaft(void) {
  counter_shaft++;
}

int checksum(char* buf) {
  int sum = 0;
  for (int i = 0; i < DATA_LENGTH; i++) {
    sum = sum + buf[i];
  }
  return sum % 256;
}


