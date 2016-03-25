// CANtx.ino

#include <mcp_can.h>
#include <SPI.h>

const int SPI_CS_PIN = 53;
MCP_CAN CAN(SPI_CS_PIN); // Set CS pin

void setup() {
    Serial.begin(9600);
    while (CAN_OK != CAN.begin(CAN_500KBPS)) {
        Serial.println("CAN BUS Shield init fail, retrying...");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");
}

unsigned char stmp[8] = {0, 1, 2, 3, 4, 5, 6, 7};

void loop() {
    for(int id=0; id<10; id++) {
        Serial.println(id);
        memset(stmp, id, sizeof(stmp)); // set id to send data buff
        CAN.sendMsgBuf(id, 0, sizeof(stmp), stmp);
        delay(100);
    }
}
