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
const int ENGINE_RPM_PIN = 19;
const int SHAFT_RPM_PIN = 18;
// 21 is reserved for CANBUS
const int ENGINE_BLIPS = 8;
const int SHAFT_BLIPS = 8;
const int CVT_POSITION_PIN = 3;
const int GEAR_POSITION_PIN = 4;
const int CVT_POSITION_MIN = 980; // reading when fully retracted
const int CVT_POSITION_MAX = 280; // reading when fully extended
const int CVT_AMP_LIMIT = 30000; // mA
const int CVT_SPEED_MIN = 30;
const int SAMPLES = 3;
const int INTERVAL = 50;
unsigned int _PID = 0x0002;
const float P_COEF = -3.0;
const float I_COEF = -3.0;
const float D_COEF = 0.0;

// Variables
volatile int engine_a = 0;
volatile int engine_b = 0;
volatile int shaft_a = 0;
volatile int shaft_b = 0;
int time_a = millis();
int time_b = millis();
int freq_engine; 
int freq_shaft; 
int freq_wheel = 0;
int cvt_pos = 0;
int cvt_pos_last = 0;
int cvt_target = 0;
int trans_gear = 0;
int trans_locked = 0;
int chksum; 
boolean canbus_ok = false;
RunningMedian error = RunningMedian(SAMPLES);

// Motor Controller
DualVNH5019MotorShield motors;

// JSON
StaticJsonBuffer<JSON_LENGTH> json_output;
JsonObject& output = json_output.createObject();
StaticJsonBuffer<JSON_LENGTH> json_input;
JsonObject& input = json_input.createObject();

// Character Buffers
char output_buffer[OUTPUT_LENGTH];
char data_buffer[DATA_LENGTH];
char input_buffer[INPUT_LENGTH];
unsigned char canbus_tx_buffer[CANBUS_LENGTH];
unsigned char canbus_rx_buffer[CANBUS_LENGTH];

// Setup
void setup() {
  Serial.begin(BAUD);
  delay(10);
  canbus_ok = Canbus.init(CANSPEED_500);
  delay(10);
  attachInterrupt(digitalPinToInterrupt(ENGINE_RPM_PIN), increment_engine, RISING);
  attachInterrupt(digitalPinToInterrupt(SHAFT_RPM_PIN), increment_shaft, RISING);
}

// Loop
void loop() {

  // Read Sensors
  delay(INTERVAL);
  freq_engine = int(60000 / ( ENGINE_BLIPS * (engine_b - engine_a)));
  freq_shaft = int(60000 / (SHAFT_BLIPS * (shaft_b - shaft_a)));
  freq_wheel = 0;
  cvt_pos_last = cvt_pos;
  cvt_pos = map(analogRead(CVT_POSITION_PIN), CVT_POSITION_MIN, CVT_POSITION_MAX, 0, 255);
  trans_gear = analogRead(GEAR_POSITION_PIN);
  
  // Run Motor
  error.add(cvt_pos - cvt_target);
  if (!motors.getM1Fault() && motors.getM1CurrentMilliamps() < CVT_AMP_LIMIT) {
    int P = P_COEF * (cvt_pos - cvt_target);
    int I = I_COEF * error.getAverage();
    int D = D_COEF * ((cvt_pos - cvt_pos_last) - cvt_target);
    int speed = P + I + D;
    motors.setM1Speed(speed);
  }
  else {
    Serial.println("MOTOR FAULT");
  }
  
  // CANBus
  if (canbus_ok) {
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
  }
  
  // Serial
  if(Serial) {
    if (Serial.available() > 0) {
      int i = 0;
      char c = ' ';
      while (c != '}') {
        c = Serial.read();
        input_buffer[i] = c;
        i++;
      }
      JsonObject& input = json_input.parseObject(input_buffer);
      while (Serial.available() > 0) {
        Serial.read();
      }
      if (input.success()) {
        cvt_target = input["target"];
      }
    }
    output["shaft"] = freq_shaft;
    output["engine"] = freq_engine;
    output["wheel"] = freq_wheel;
    output["pos"] = cvt_pos;
    output["target"] = cvt_target;
    output["gear"] = trans_gear;
    output["locked"] = trans_locked;
    output.printTo(data_buffer, sizeof(data_buffer));
    chksum = checksum(data_buffer);
    sprintf(output_buffer, "{\"data\":%s,\"chksum\":%d}", data_buffer, chksum);
    Serial.println(output_buffer);
  }
}

// Functions
void increment_engine(void) {
  engine_a = engine_b;
  engine_b = millis();
}

void increment_shaft(void) {
  shaft_a = shaft_b;
  shaft_b = millis();
}

int checksum(char* buf) {
  int sum = 0;
  for (int i = 0; i < DATA_LENGTH; i++) {
    sum = sum + buf[i];
  }
  return sum % 256;
}


