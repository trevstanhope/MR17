// Libraries
#include <Canbus.h>

// Constants
unsigned int _ID = 0x0757; // This is the devices ID. The Lower the Number, the Higher its Priority on the CAN Bus. ID 0x0000 would be the highest Priority. (Cant have two with the same ID)
unsigned char _PID = 10;   // This is the ID we will use to check if the message was for this device. If you have more there one UNO with the same PID, they will all accept the message.
unsigned int BAUD = 9600;

// Variables
unsigned char RX_Buff[8];  // Buffer to store the incoming data

void setup() {
  Serial.begin(BAUD); // For debug use
  Serial.println("CAN RX");  
  Canbus.init(CANSPEED_500);  /* Initialise MCP2515 CAN controller at the specified speed */
}

void loop() {
  Serial.println(millis());
  unsigned int ID = Canbus.message_rx(RX_Buff); // Check to see if we have a message on the Bus
  if (RX_Buff[0] == _PID) { // If we do, check to see if the PID matches this device. We are using location (0) to transmit the PID and (1) for the LED's duty cycle
    Serial.println(RX_Buff[1]); // If it does check what the LED's duty cycle should be set to, and set it! We are done! 
  }
}
