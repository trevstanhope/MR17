// Libraries
#include <Canbus.h>
#include <ArduinoJson.h>

// Cosntants
unsigned int _ID = 0x0756; // This is the devices ID. The Lower the Number, the Higher its Priority on the CAN Bus. ID 0x0000 would be the highest Priority. (Cant have two with the same ID)
unsigned char _PID = 10;   // This is the ID we will use to check if the message was for this device. If you have more there one UNO with the same PID, they will all accept the message.
unsigned int BAUD = 9600;

// Objects
StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

// Variables
unsigned char RX_Buff[8];
unsigned char JSON_Buff[256];

void setup() {
  Serial.begin(BAUD);
  Serial.println("CAN TX");  /* For debug use */  
  while (!Canbus.init(CANSPEED_250)) {
    Serial.println("CAN Failed! Retrying ...");  /* Initialise MCP2515 CAN controller at the specified speed */
  }
}

void loop() {
  RX_Buff[0] = _PID; // We want this message to be picked up by device with a PID of 10
  RX_Buff[1] = 1 + millis() % 256; // This is the brightness level we want our LED set to on the other device
  RX_Buff[2] = 2 + millis() % 256; // This is the brightness level we want our LED set to on the other device
  RX_Buff[3] = 3 + millis() % 256; // This is the brightness level we want our LED set to on the other device
  RX_Buff[4] = 4 + millis() % 256; // This is the brightness level we want our LED set to on the other device
  RX_Buff[5] = 5 + millis() % 256; // This is the brightness level we want our LED set to on the other device
  RX_Buff[6] = 6 + millis() % 256; // This is the brightness level we want our LED set to on the other device
  RX_Buff[7] = 7 + millis() % 256; // This is the brightness level we want our LED set to on the other device
  Canbus.message_tx(_ID, RX_Buff); // Send the message on the CAN Bus to be picked up by the other devices
  delay(10); 
}
