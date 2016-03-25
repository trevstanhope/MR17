// CANrx.ino

#include <SPI.h>
#include "mcp_can.h"

const int SPI_CS_PIN = 53;
MCP_CAN CAN(SPI_CS_PIN); // Set CS pin
unsigned char flagRecv = 0;
unsigned char len = 0;
unsigned char buf[8];
char str[20];

void setup() {
    Serial.begin(9600);
    while (CAN_OK != CAN.begin(CAN_500KBPS)) {
        Serial.println("CAN BUS Shield init fail, retrying...");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");
    attachInterrupt(4, MCP2515_ISR, FALLING); // start interrupt

    // set mask, set both the mask to 0x3ff
    CAN.init_Mask(0, 0, 0x3ff); // there are 2 mask in mcp2515, you need to set both of them
    CAN.init_Mask(1, 0, 0x3ff);

    // set filter, we can receive id from 0x04 ~ 0x09
    //CAN.init_Filt(0, 0, 0x04); // there are 6 filter in mcp2515
    //CAN.init_Filt(1, 0, 0x05); // there are 6 filter in mcp2515
    //CAN.init_Filt(2, 0, 0x06); // there are 6 filter in mcp2515
    //CAN.init_Filt(3, 0, 0x07); // there are 6 filter in mcp2515
    //CAN.init_Filt(4, 0, 0x08); // there are 6 filter in mcp2515
    //CAN.init_Filt(5, 0, 0x09); // there are 6 filter in mcp2515
}

void MCP2515_ISR() {
    flagRecv = 1;
}

void loop() {
    if(flagRecv) {
        flagRecv = 0; // clear flag
        CAN.readMsgBuf(&len, buf); // read data, len: data length, buf: data buf
        Serial.println("\r\n------------------------------------------------------------------");
        Serial.print("Get Data From id: ");
        Serial.println(CAN.getCanId());
        for(int i = 0; i<len; i++) {
            Serial.print("0x");
            Serial.print(buf[i], HEX);
            Serial.print("\t");
        }
        Serial.println();
    }
}
