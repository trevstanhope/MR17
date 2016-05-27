// Buffer Sizes for TX/RX
extern const unsigned int BAUD = 38400;
extern const unsigned int OUTPUT_LENGTH = 256;
extern const unsigned int DATA_LENGTH = 256;
extern const unsigned int JSON_LENGTH = 512;
extern const unsigned int INPUT_LENGTH = 256;
extern const unsigned int CANBUS_LENGTH = 8;
extern const unsigned int CANBUS_BITS = 255;

// CAN IDs for message types
extern const unsigned int OBD_ID = 8;
extern const unsigned char ESC_A_ID = 9;
extern const unsigned char ESC_B_ID = 10;
extern const unsigned char TSC_ID = 11;
extern const unsigned char VDC_ID = 12;

// CAN Priorities
extern const unsigned int OBD_PID = 0x0001;
extern const unsigned int ESC_A_PID = 0x0002;
extern const unsigned int ESC_B_PID = 0x0003;
extern const unsigned int TSC_PID = 0x0004;
extern const unsigned int VDC_PID = 0x0005;

// VDC
extern const unsigned int CART_MODE_MANUAL = 0;
extern const unsigned int CART_MODE_AUTO = 1;
extern const unsigned int CART_DIRECTION_OFF = 0;
extern const unsigned int CART_DIRECTION_FORWARD = 1;
extern const unsigned int CART_DIRECTION_BACKWARD = 2;
