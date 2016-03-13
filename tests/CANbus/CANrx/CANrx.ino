// Libraries
#include <Canbus.h>
#include <ArduinoJson.h>

// Constants
unsigned int _ID = 0x0757; // This is the devices ID. The Lower the Number, the Higher its Priority on the CAN Bus. ID 0x0000 would be the highest Priority. (Cant have two with the same ID)
unsigned char _PID = 10;   // This is the ID we will use to check if the message was for this device. If you have more there one UNO with the same PID, they will all accept the message.
unsigned int BAUD = 9600;

// Objects
StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

// Variables
unsigned char RX_Buff[8];  // Buffer to store the incoming data
unsigned char JSON_Buff[256];

void setup() {
  Serial.begin(BAUD); // For debug use
  Serial.println("CAN RX");  
  Canbus.init(CANSPEED_500);  /* Initialise MCP2515 CAN controller at the specified speed */
}

void loop() {
  unsigned int ID = Canbus.message_rx(RX_Buff); // Check to see if we have a message on the Bus
  if (RX_Buff[0] == _PID) { // If we do, check to see if the PID matches this device
    root["a"] = RX_Buff[1];
    root["b"] = RX_Buff[2];
    root["c"] = RX_Buff[3];
    root["d"] = RX_Buff[4];
    root["e"] = RX_Buff[5];
    root["f"] = RX_Buff[6];
    root["g"] = RX_Buff[7];
    root.printTo(Serial); 
    Serial.println();   
  }
}
