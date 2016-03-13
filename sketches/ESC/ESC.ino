/*
  Engine Safety Controller (ESC)
  McGill ASABE Engineering Team
 
 Board: Arduino Mega 2560
 Shields: Dual VNH5019 Motor Shield
 */

/* --- LIBRARIES --- */

#include "DualVNH5019MotorShield.h"
#include "DallasTemperature.h"
#include "OneWire.h"
#include <RunningMedian.h>
#include <Canbus.h>
#include <ArduinoJson.h>

/* --- Global Constants --- */
/// Digital Pins
// D14 Reserved for Serial2 RFID
// D15 Reserved for Serial2 RFID
const int LPH_SENSOR_PIN = 18;  // interrupt (#5) required

// Failsafe Digital Input
const int BUTTON_KILL_POUT = 28;
const int HITCH_KILL_POUT = 30;
const int SEAT_KILL_POUT = 32;
const int SEAT_KILL_PIN = 26;
const int HITCH_KILL_PIN = 24;
const int BUTTON_KILL_PIN = 22;

// Joystick (Digital) 
const int IGNITION_PIN = 23; // white
const int TRIGGER_KILL_PIN = 25; // pink
const int PULL_MODE_PIN = 27; // purple
// D29 is a Blocked pin
const int CART_FORWARD_PIN = 31; // brown
const int CART_BACKWARD_PIN = 33; // light blue
const int CART_MODE_PIN = 35; // dark blue
// D37 is a Blocked pin
const int THROTTLE_HIGH_PIN = 39; // yellow
const int THROTTLE_LOW_PIN = 41; // green
const int THROTTLE_UP_PIN = 43; // red
// D45 is a Blocked pin
const int THROTTLE_DOWN_PIN = 47; // grey
const int DISPLAY_MODE_PIN = 49; // tan

// Relay Pins 
const int STOP_RELAY_PIN = 38;
const int REGULATOR_RELAY_PIN = 40;
const int STARTER_RELAY_PIN = 42;
const int REBOOT_RELAY_PIN = 44;

// Additional Sensors
const int TEMP_SENSOR_PIN = 46; // TODO: find actual pin number

/// Analog Input Pins
// A0 Reserved for DualVNH5019
// A1 Reserved for DualVNH5019
const int THROTTLE_POS_PIN = A8;
const int THROTTLE_POS_MIN_PIN = A9;
const int THROTTLE_POS_MAX_PIN = A10;
const int PSI_SENSOR_PIN = A11;
const int CVT_GUARD_POUT = A12;
const int CVT_GUARD_PIN = A13;
const int LEFT_BRAKE_PIN = A2;
const int RIGHT_BRAKE_PIN = A3;

/* --- Global Settings --- */ 
/// CMQ
const int BAUD = 9600;
const int OUTPUT_SIZE = 512;
const int DATA_SIZE = 256;

/// RFID
const int RFID_BAUD = 9600;
const int RFID_READ = 0x02;

/// Time Delays
const int KILL_WAIT = 5000;
const int IGNITION_WAIT = 200;
const int MOTORS_WAIT = 100;
const int STANDBY_WAIT = 10;
const int REBOOT_WAIT = 1000;

/// Brake variables
const int BRAKES_THRESHOLD = 20;
const int BRAKES_MILLIAMP_THRESH = 30000;
const int BRAKES_MIN = 0;
const int BRAKES_MAX = 512;

/// Throttle
int THROTTLE_POS_MIN = 274;
int THROTTLE_POS_MAX = 980;
const int THROTTLE_MIN = 0;
const int THROTTLE_MAX = 1024;
const int THROTTLE_MILLIAMP_THRESHOLD = 15000;
const int THROTTLE_P =10 ;
const int THROTTLE_I = 1;
const int THROTTLE_D = 2;
const int THROTTLE_STEP = 64;

/// CVT Guard
const int CVT_GUARD_THRESHOLD = 300;

// Seat
const int SEAT_LIMIT = 3;

// Engine sensors
const int LPH_SAMPLESIZE = 20;
const int PSI_SAMPLESIZE = 5;
const int TEMP_SAMPLESIZE = 3;
const int DIGITS = 4;
const int PRECISION = 2;

/* --- GLOBAL VARIABLES --- */
/// Safety switches, rfid, and Joystick button variables
int brakes_milliamp = 0;
int seat_counter = 0;
int seat_kill = 0;
int hitch_kill = 0;
int trigger_kill  = 0;
int button_kill = 0;
int ignition = 0;
int pull_mode = 0;
int cvt_guard = 0;
int run_mode = 0;
int left_brake = 0;
int right_brake = 0;
int rfid_auth = 0;
int display_mode = 0; // the desired display mode on the HUD
int CART_FORWARD = 0;
int CART_BACKWARD = 0;
int CART_MODE = 0;
int THROTTLE_HIGH = 0;
int THROTTLE_LOW = 0;
int THROTTLE_UP = 0;
int THROTTLE_DOWN = 0;
int THROTTLE = THROTTLE_MIN;
float lph = 0;
float psi = 0;
float TEMP = 0;

// Volatile values (Asynchronous variables)
volatile int LPH_COUNTER = 0;
volatile long LPH_TIME_A = millis();
volatile long LPH_TIME_B = millis();

/// String output
char data_buffer[DATA_SIZE];
char output_buffer[OUTPUT_SIZE];
char temp_buffer[DIGITS + PRECISION];
char lph_buffer[DIGITS + PRECISION];
char psi_buffer[DIGITS + PRECISION];

/// Throttle PID values
int THROTTLE_SET;
int THROTTLE_IN;
int THROTTLE_OUT;

/* --- Global Objects --- */

/// Dual Motor Controller (M1 vs. M2)
DualVNH5019MotorShield motors;

/// Temperature probe
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature TEMP_SENSOR(&oneWire);

// Fuel flow and Oil pressure
RunningMedian LPH_HIST = RunningMedian(LPH_SAMPLESIZE);
RunningMedian PSI_HIST = RunningMedian(PSI_SAMPLESIZE);
RunningMedian TEMP_HIST = RunningMedian(TEMP_SAMPLESIZE);

/* --- SETUP --- */
void setup() {
   // Relays
  pinMode(STOP_RELAY_PIN, OUTPUT); digitalWrite(STOP_RELAY_PIN, HIGH);
  pinMode(REGULATOR_RELAY_PIN, OUTPUT); digitalWrite(REGULATOR_RELAY_PIN, HIGH);
  pinMode(STARTER_RELAY_PIN, OUTPUT); digitalWrite(STARTER_RELAY_PIN, HIGH);
  pinMode(REBOOT_RELAY_PIN, OUTPUT); digitalWrite(REBOOT_RELAY_PIN, HIGH);
  
  // Initialize RFID authentication device
  Serial3.begin(RFID_BAUD); // Pins 14 and 15
  Serial3.write(RFID_READ);

  // Failsafe Seat
  pinMode(SEAT_KILL_PIN, INPUT);
  digitalWrite(SEAT_KILL_PIN, HIGH);
  pinMode(SEAT_KILL_POUT, OUTPUT);
  digitalWrite(SEAT_KILL_POUT, LOW);
  
  // Failsafe Hitch
  pinMode(HITCH_KILL_PIN, INPUT);
  digitalWrite(HITCH_KILL_PIN, HIGH);
  pinMode(HITCH_KILL_POUT, OUTPUT);
  digitalWrite(HITCH_KILL_POUT, LOW);
  
  // Failsafe Button
  pinMode(BUTTON_KILL_PIN, INPUT);
  digitalWrite(BUTTON_KILL_PIN, HIGH);
  pinMode(BUTTON_KILL_POUT, OUTPUT);
  digitalWrite(BUTTON_KILL_POUT, LOW);

  // Joystick Digital Input
  pinMode(IGNITION_PIN, INPUT);
  pinMode(TRIGGER_KILL_PIN, INPUT);
  pinMode(PULL_MODE_PIN, INPUT);
  pinMode(CART_FORWARD_PIN, INPUT);
  pinMode(CART_BACKWARD_PIN, INPUT);
  pinMode(CART_MODE_PIN, INPUT);
  pinMode(THROTTLE_HIGH_PIN, INPUT);
  pinMode(THROTTLE_LOW_PIN, INPUT);
  pinMode(THROTTLE_UP_PIN, INPUT);
  pinMode(THROTTLE_DOWN_PIN, INPUT);
  pinMode(DISPLAY_MODE_PIN, INPUT);
  
  // Throttle
  pinMode(THROTTLE_POS_PIN, INPUT);
  pinMode(THROTTLE_POS_MIN_PIN, OUTPUT);
  digitalWrite(THROTTLE_POS_MIN_PIN, LOW);
  pinMode(THROTTLE_POS_MAX_PIN, OUTPUT);
  digitalWrite(THROTTLE_POS_MAX_PIN, HIGH);
  
  // Brakes
  pinMode(RIGHT_BRAKE_PIN, INPUT); digitalWrite(RIGHT_BRAKE_PIN, LOW);
  pinMode(LEFT_BRAKE_PIN, INPUT); digitalWrite(LEFT_BRAKE_PIN, LOW);
  
  // CVT Guard
  pinMode(CVT_GUARD_POUT, OUTPUT);
  digitalWrite(CVT_GUARD_POUT, LOW);
  pinMode(CVT_GUARD_PIN, INPUT);
  digitalWrite(CVT_GUARD_PIN, HIGH);

  // DualVNH5019 Motor Controller
  motors.init(); 

  // Engine temperature sensor DS18B20
  TEMP_SENSOR.begin();
  
  // Fuel Sensor
  attachInterrupt(LPH_SENSOR_PIN, lph_counter, RISING);
  
  // Reboot host
  reboot();
  
  // Begin serial communication
  Serial.begin(BAUD);
}

/* --- LOOP --- */
void loop() {

  // Check Failsafe Switches (Default to 1 if disconnected)
  hitch_kill = check_failsafe_switch(HITCH_KILL_PIN);
  button_kill = check_failsafe_switch(BUTTON_KILL_PIN);
  
  // Check Seat
  seat_kill = check_seat();
  
  // Check Regular Switches (Default to 0 if disconnected)
  trigger_kill  = check_switch(TRIGGER_KILL_PIN);
  ignition = check_switch(IGNITION_PIN);
  display_mode = check_switch(DISPLAY_MODE_PIN);
  CART_FORWARD = check_switch(CART_FORWARD_PIN);
  CART_BACKWARD = check_switch(CART_BACKWARD_PIN);
  THROTTLE_UP = check_switch(THROTTLE_UP_PIN);
  THROTTLE_DOWN = check_switch(THROTTLE_DOWN_PIN);
  THROTTLE_HIGH = check_switch(THROTTLE_HIGH_PIN);
  THROTTLE_LOW = check_switch(THROTTLE_LOW_PIN);
  
  // Change VDC and TCS modes
  if (check_switch(CART_MODE_PIN)) {
    if (CART_MODE == 1) { CART_MODE = 0; }
    else { CART_MODE = 1; }
  }
  if (check_switch(PULL_MODE_PIN)) {
     if (pull_mode == 1) { pull_mode = 0; }
    else { pull_mode = 1; }
  }
  
  // Check non-switches
  cvt_guard = check_guard();
  if (rfid_auth == 0) {
    rfid_auth = check_rfid();
  }
  left_brake = check_brake(LEFT_BRAKE_PIN);
  right_brake = check_brake(RIGHT_BRAKE_PIN);
  
  // Check engine condition
  TEMP = get_engine_temp();
  lph = get_fuel_lph();
  psi = get_oil_psi();
  
  // Set brakes ALWAYS
  set_brakes(right_brake, left_brake);
  
  //  // Adjust throttle limit, either HIGH/LOW, or increment
  if (THROTTLE_HIGH  && !THROTTLE_LOW) {
    THROTTLE = THROTTLE_MAX;
  }
  else if (THROTTLE_LOW && !THROTTLE_HIGH) {
    THROTTLE = THROTTLE_MIN;
  }
  else if (THROTTLE_UP && !THROTTLE_DOWN) {
    THROTTLE = THROTTLE + THROTTLE_STEP;
  }
  else if (THROTTLE_DOWN && !THROTTLE_UP) {
    THROTTLE = THROTTLE - THROTTLE_STEP;
  }
  else {
    if (THROTTLE > THROTTLE_MAX) {
      THROTTLE = THROTTLE_MAX;
    }
    else if (THROTTLE < THROTTLE_MIN) {
      THROTTLE = THROTTLE_MIN;
    }
  }
  if (trigger_kill ) {
    set_throttle(THROTTLE);
  }
  else {
    set_throttle(0); // DISABLE THROTTLE IF OPERATOR RELEASES TRIGGER
  }
    
  // (0) If OFF
  if (run_mode == 0) {
    if (rfid_auth != 0 && !seat_kill) {
      standby();
    }
  }
  // (1) If STANDBY
  else if (run_mode == 1) {
    if ( seat_kill || hitch_kill || button_kill ) {
      kill(); // kill engine
    }
    else if (ignition && left_brake && right_brake && !cvt_guard) {
      start(); // execute ignition sequence
    }
    else {
      standby(); // remain in standby (run_mode 1)
    }
  }
  // (2) If DRIVE
  else if (run_mode == 2) {
    if (seat_kill || hitch_kill || button_kill ) {
      kill(); // kill engine
      standby();
    }
    if (ignition) {
      start();
    }
    else {
      /* STUFF PERTAINING TO run_mode 2 */
    }
  }
  // If ??? (UNKNOWN)
  else {
    kill();
  }

  // Format float to string
  dtostrf(lph, DIGITS, PRECISION, lph_buffer);
  dtostrf(TEMP, DIGITS, PRECISION, temp_buffer);
  dtostrf(psi, DIGITS, PRECISION, psi_buffer);
  
  // Output to USB Serial
  sprintf(data_buffer, "{'run_mode':%d,'display_mode':%d,'right_brake':%d,'left_brake':%d,'cvt_guard':%d,'button':%d,'seat':%d,'hitch':%d,'ignition':%d,'rfid':%d,'cart_mode':%d,'cart_fwd':%d,'cart_bwd':%d,'throttle':%d,'trigger':%d,'pull_mode':%d}", run_mode, display_mode, right_brake, left_brake, cvt_guard, button_kill, seat_kill, hitch_kill, ignition, rfid_auth, CART_MODE, CART_FORWARD, CART_BACKWARD, THROTTLE, trigger_kill, pull_mode);
  sprintf(output_buffer, "{\"data\":%s,\"chksum\":%d}", data_buffer, checksum(data_buffer));
  Serial.println(output_buffer);
  Serial.flush();
}

/* --- SYNCHRONOUS TASKS --- */
/// Set Throttle
int set_throttle(int val) {
  THROTTLE_SET = val;
  int throttle_pos = analogRead(THROTTLE_POS_PIN);
  
  // Auto-calibrate
  if (throttle_pos > THROTTLE_POS_MAX) {THROTTLE_POS_MAX = throttle_pos;}
  if (throttle_pos < THROTTLE_POS_MIN) {THROTTLE_POS_MIN = throttle_pos;}
  
  THROTTLE_IN = map(throttle_pos, THROTTLE_POS_MIN, THROTTLE_POS_MAX, 900, 0); // get the position feedback from the linear actuator
  int error = THROTTLE_IN - THROTTLE_SET;
  THROTTLE_OUT = -1 * THROTTLE_P * error;
  if (THROTTLE_OUT > 400) { THROTTLE_OUT = 400; }
  if (THROTTLE_OUT < -400) { THROTTLE_OUT = -400; }
  
  // Engage throttle actuator
  if (motors.getM1CurrentMilliamps() >  THROTTLE_MILLIAMP_THRESHOLD) {
    motors.setM1Speed(0); // disable if over-amp
  }
  else {
    motors.setM1Speed(THROTTLE_OUT);
  }
}

/// Check Brake
// Engage the brakes linearly
int check_brake(int pin) {
  int val = analogRead(pin); // read left brake signal
  if (val > BRAKES_THRESHOLD) {
    return 400;
  }
  else {
    return 0;
  }
}

/// Set brakes
// Returns true if the brake interlock is engaged
int set_brakes(int left_brake, int right_brake) {
  
  // Map the brake values for DualVNH5019 output
  int output = (left_brake + right_brake) / 2;

  // Engage brakes
  brakes_milliamp = motors.getM2CurrentMilliamps();
  motors.setM2Speed(output);
}

/// Check Switch
int check_switch(int pin_num) {
  if  (digitalRead(pin_num)) {
    if (digitalRead(pin_num)) {
      return 1;
    }
    else {
      return 0;
    }
  }
  else {
    return 0;
  }
}

/// Check Switch
int check_failsafe_switch(int pin_num) {
  if  (digitalRead(pin_num)) {
    if (digitalRead(pin_num)) {
      return 0;
    }
    else {
      return 1;
    }
  }
  else {
    return 1;
  }
}

/// Check RFID
int check_rfid(void) {
  Serial3.write(RFID_READ);
  delay(2);
  int a = Serial3.read();
  int b = Serial3.read();
  int c = Serial3.read();
  int d = Serial3.read();
  if ((a > 0) && (b > 0) && (c > 0) && (d > 0)) {
    return a + b + c + d; // the user's key number
  }
  else {
    return 0;
  }
}

/// Checksum
int checksum(char* buf) {
  int sum = 0;
  for (int i = 0; i < DATA_SIZE; i++) {
    sum += buf[i];
  }
  int val = sum % 256;
  return val;
}

/// Check Guard
// Returns true if guard is open
int check_guard(void) {
  int val = analogRead(CVT_GUARD_PIN);
  if (val <= CVT_GUARD_THRESHOLD) {
      return 1;
    }
    else {
      return 0;
    }
}

/// Check Seat
// Returns 1 if seat is empty, checks for SEAT_LIMIT iterations before activating
int check_seat(void) {
  if (!digitalRead(SEAT_KILL_PIN)) {
    seat_counter++;
  } 
  else {
    seat_counter = 0;
  }
  if (seat_counter >= SEAT_LIMIT) {
    return 1;
  }
  else {
    return 0;
  }
}

/// Reboot the Atom
void reboot(void) {
  digitalWrite(REBOOT_RELAY_PIN, HIGH);
  delay(REBOOT_WAIT);
  digitalWrite(REBOOT_RELAY_PIN, LOW);
  delay(REBOOT_WAIT);
  digitalWrite(REBOOT_RELAY_PIN, HIGH);
}

/// Kill
void kill(void) {
  digitalWrite(STOP_RELAY_PIN, HIGH);
  //digitalWrite(REGULATOR_RELAY_PIN, HIGH);
  //digitalWrite(STARTER_RELAY_PIN, HIGH);
  delay(KILL_WAIT);
  run_mode = 0;
}

/// Standby
void standby(void) {
  digitalWrite(STOP_RELAY_PIN, LOW);
  digitalWrite(REGULATOR_RELAY_PIN, LOW);
  digitalWrite(STARTER_RELAY_PIN, HIGH);
  delay(STANDBY_WAIT);
  run_mode = 1;
}

/// Ignition
void start(void) {
  motors.setM1Speed(0);
  motors.setM2Speed(0);
  delay(MOTORS_WAIT);
  while (check_switch(IGNITION_PIN)) {
    digitalWrite(STOP_RELAY_PIN, LOW);
    digitalWrite(REGULATOR_RELAY_PIN, LOW);
    digitalWrite(STARTER_RELAY_PIN, LOW);
    delay(IGNITION_WAIT);
  }
  digitalWrite(STOP_RELAY_PIN, LOW);
  digitalWrite(REGULATOR_RELAY_PIN, LOW);
  digitalWrite(STARTER_RELAY_PIN, HIGH);
  delay(STANDBY_WAIT);
  run_mode = 2;
}

/// Get Fuel Rate
float get_fuel_lph(void) {
  LPH_TIME_A = millis();
  float lph = (float(LPH_COUNTER) * 0.00038 * 3600.0) / (float(LPH_TIME_B - LPH_TIME_A) / 1000.0);
  LPH_HIST.add(lph);
  LPH_COUNTER = 0;
  LPH_TIME_B = millis();
  return LPH_HIST.getAverage();
}

/// Get Engine Temperature
float get_engine_temp(void) {
  float tmp = TEMP_SENSOR.getTempCByIndex(0);
  TEMP_SENSOR.requestTemperatures();
  if (isnan(tmp)) {
    return TEMP_HIST.getAverage();
  }
  else {
    TEMP_HIST.add(tmp);
  }
  return TEMP_HIST.getAverage();
}

/// Get the Engine Oil Pressure
float get_oil_psi(void) {
  int ohms = analogRead(PSI_SENSOR_PIN);
  float psi = 0.0226 * ohms * ohms - 4.3316 * ohms + 240;
  PSI_HIST.add(psi);
  return PSI_HIST.getAverage();
}

/* --- ASYNCHRONOUS TASKS --- */
/// Increment the LPH counter
void lph_counter(void) {
  LPH_COUNTER++;
}
