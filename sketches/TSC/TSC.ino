/*
  TSC.ino
  Transmission System Controller
*/

// Libraries
#include <RunningMedian.h>
#include <Canbus.h>
#include <mcp2515.h>
#include <global.h>
#include <defaults.h>
#include <mcp2515_defs.h>
// #include <DualVNH5019MotorShield.h>

// Constants
const int BAUD = 9600;
const int ENGINE_RPM_PIN = 21;
const int SHAFT_RPM_PIN = 20;
const int ENGINE_BLIPS = 8;
const int SHAFT_BLIPS = 8;
const int INTERVAL = 1000;
const int OUTPUT_LENGTH = 256;
const int DATA_LENGTH = 128;
const char PID[] = {"TSC"};
unsigned int CID = 0x0756; // This is the devices ID. The Lower the Number, the Higher its Priority on the CAN Bus. ID 0x0000 would be the highest Priority. (Cant have two with the same ID)

// Variables
volatile int counter_engine = 0;
volatile int counter_shaft = 0;
int time_a = millis();
int time_b = millis();
int freq_engine; 
int freq_shaft; 
char output_buffer[OUTPUT_LENGTH];
char data_buffer[DATA_LENGTH];
int chksum; 
unsigned char input_buffer[8];
boolean can_status = false;

// Setup
void setup() {
  Serial.begin(BAUD);
  //can_status = Canbus.init(CANSPEED_500);
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
  sprintf(data_buffer, "{\"shaft\":%d,\"engine\":%d}", freq_shaft, freq_engine);
  chksum = checksum(data_buffer);
  sprintf(output_buffer, "{\"data\":%s,\"pid\":\"%s\",\"chksum\":%d}", data_buffer, PID, chksum);
  input_buffer[0] = 10; // We want this message to be picked up by device with a PID of 10
  // Canbus.message_tx(CID, input_buffer);
  Serial.println(output_buffer);
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


