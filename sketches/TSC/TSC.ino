//

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

// Setup
void setup() {
  Serial.begin(BAUD);
  attachInterrupt(digitalPinToInterrupt(ENGINE_RPM_PIN), increment_engine, RISING);
  attachInterrupt(digitalPinToInterrupt(SHAFT_RPM_PIN), increment_shaft, RISING);
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


