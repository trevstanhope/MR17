// Libraries
#include <Canbus.h>

// Cosntants
unsigned int _ID = 0x0756; // This is the devices ID. The Lower the Number, the Higher its Priority on the CAN Bus. ID 0x0000 would be the highest Priority. (Cant have two with the same ID)
unsigned int BAUD = 9600;

// Variables
unsigned char _PID = 1; // This is the ID we will use to check if the message was for this device.
unsigned char RX_Buff[8];

void setup() {
  Serial.begin(BAUD);
  Serial.println("CAN TX");  /* For debug use */  
  Canbus.init(CANSPEED_500); /* Initialise MCP2515 CAN controller at the specified speed */
}

void loop() {
  RX_Buff[0] = 10; // We want this message to be picked up by device with a PID of 10
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
