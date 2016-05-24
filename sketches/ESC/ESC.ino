/*
  Engine Safety Controller (ESC)
  McGill ASABE Engineering Team

  Board: Arduino Mega 2560
  Shields: Dual VNH5019 Motor Shield
*/

/* --- LIBRARIES --- */
#include "DualMC33926MotorShield.h"
#include "DallasTemperature.h"
#include "OneWire.h"
#include <RunningMedian.h>
#include <Canbus.h>
#include <ArduinoJson.h>
#include "MR17_CAN.h"

/* --- Global Constants --- */
// Print Flags
const bool PRINT_MAIN_INFO = true;
const bool PRINT_THROTTLE_INFO = false;
const bool PRINT_VDC_INFO = false;
const bool PRINT_TSC_INFO = false;
const bool PRINT_BRAKE_INFO = false;

/// Digital Pins
// D21 is reserved for CANBUS
// D14 Reserved for Serial3 RFID
// D15 Reserved for Serial3 RFID
const int LPH_SENSOR_PIN = 18;  // interrupt (#5) required

// Failsafe Digital Input
const int HITCH_KILL_PIN = 22;
const int HITCH_KILL_POUT = 24;
const int SEAT_KILL_PIN = 26;
const int SEAT_KILL_POUT = 28;
const int BUTTON_KILL_PIN = 30;
const int BUTTON_KILL_POUT = 32;

// Joystick (Digital)
const int CART_BACKWARD_PIN = 23;
const int THROTTLE_HIGH_PIN = 25; // correct
const int CART_MODE_PIN = 31;
const int PULL_MODE_PIN = 29;
const int IGNITION_PIN = 35;
const int DISPLAY_MODE_PIN = 33;
const int TRIGGER_KILL_PIN = 27;
const int THROTTLE_UP_PIN = 39;
const int THROTTLE_DOWN_PIN = 41; // correct
const int CART_FORWARD_PIN = 43; // correct
const int THROTTLE_LOW_PIN = 47; // correct

// Relay Pins
const int STOP_RELAY_PIN = 38;
const int REGULATOR_RELAY_PIN = 40;
const int STARTER_RELAY_PIN = 42;
const int REBOOT_RELAY_PIN = 44;

// Additional Sensors
const int TEMP_SENSOR_PIN = 46; // TODO: find actual pin number

/// Analog Input Pins
// A0 Reserved for DualMC33936
// A1 Reserved for DualMC33936
const int THROTTLE_POS_PIN = A8;
const int THROTTLE_POS_MIN_PIN = A9;
const int THROTTLE_POS_MAX_PIN = A10;
const int LEFT_BRAKE_PIN = A11;
const int JOYSTICK_Y_PIN = A12;
const int JOYSTICK_X_PIN = A13;
const int PSI_SENSOR_PIN = A14;
const int RIGHT_BRAKE_PIN = A15;

/* --- Global Settings --- */
/// RFID
const bool USE_RFID_AUTH = true;
const int RFID_BAUD = 9600;
const int RFID_SEARCH = 0x01;
const int RFID_READ = 0x02;

/// Time Delays
const int KILL_WAIT = 500;
const int IGNITION_WAIT = 200;
const int MOTORS_WAIT = 100;
const int STANDBY_WAIT = 10;
const int REBOOT_WAIT = 1000;

/// Brake variables
const int BRAKES_THRESHOLD = 20;
const int BRAKES_MILLIAMP_THRESH = 30000;
const int BRAKES_DEADBAND = 50;
const int BRAKES_MAX = 512;

/// Throttle
const int THROTTLE_POS_MIN = 274;
const int THROTTLE_POS_MAX = 980;
const int THROTTLE_MIN = 0;
const int THROTTLE_MAX = 1024;
const int THROTTLE_MILLIAMP_THRESHOLD = 15000;
const int THROTTLE_P = 10 ;
const int THROTTLE_I = 1;
const int THROTTLE_D = 2;
const int THROTTLE_STEP = 64;

/// Joystick
const int JOYSTICK_MAX = 430;
const int JOYSTICK_ZERO = 705;
const int JOYSTICK_MIN = 1024;
const int JOYSTICK_DEADBAND = 50;

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
int cart_forward = 0;
int cart_backward = 0;
int cart_mode = 0;
int throttle_high = 0;
int throttle_low = 0;
int throttle_up = 0;
int throttle_down = 0;
int throttle_sp = 1024; // initialize to the minima
int throttle_pv = 0; // initialize to the minima
int cvt_sp = 0;
float lph = 0;
float psi = 0;
float temperature = 0;
int canbus_status = 0;

// Volatile values (Asynchronous variables)
volatile int LPH_COUNTER = 0;
volatile long LPH_TIME_A = millis();
volatile long LPH_TIME_B = millis();

/// String output
/* --- Global Objects --- */
/// Dual Motor Controller (M1 vs. M2)
DualMC33926MotorShield motors;

/// Temperature probe
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature temperature_sensor(&oneWire);

// Fuel flow and Oil pressure
RunningMedian LPH_HIST = RunningMedian(LPH_SAMPLESIZE);
RunningMedian PSI_HIST = RunningMedian(PSI_SAMPLESIZE);
RunningMedian TEMP_HIST = RunningMedian(TEMP_SAMPLESIZE);

// JSON
StaticJsonBuffer<JSON_LENGTH> json_output;
JsonObject& output = json_output.createObject();
StaticJsonBuffer<JSON_LENGTH> json_input;
JsonObject& input = json_input.createObject();

// Character Buffers
char temp_buffer[DIGITS + PRECISION];
char lph_buffer[DIGITS + PRECISION];
char psi_buffer[DIGITS + PRECISION];
char output_buffer[OUTPUT_LENGTH];
char data_buffer[DATA_LENGTH];
char input_buffer[INPUT_LENGTH];
unsigned char canbus_tx_buffer[CANBUS_LENGTH];
unsigned char canbus_rx_buffer[CANBUS_LENGTH];

/* --- SETUP --- */
void setup() {

  // Relays
  pinMode(STOP_RELAY_PIN, OUTPUT); digitalWrite(STOP_RELAY_PIN, HIGH);
  pinMode(REGULATOR_RELAY_PIN, OUTPUT); digitalWrite(REGULATOR_RELAY_PIN, HIGH);
  pinMode(STARTER_RELAY_PIN, OUTPUT); digitalWrite(STARTER_RELAY_PIN, HIGH);
  pinMode(REBOOT_RELAY_PIN, OUTPUT); digitalWrite(REBOOT_RELAY_PIN, HIGH);

  // Initialize RFID authentication device
  Serial3.begin(RFID_BAUD); // Pins 14 and 15
  delay(10);
  Serial3.write(RFID_READ);
  delay(10);

  // CANBUS
  int canbus_attempts = 0;
  while (!canbus_status) {
    canbus_status = Canbus.init(CANSPEED_500);
    delay(10);
    canbus_attempts++;
    if (canbus_attempts > 10) {
      break;
    }
  }

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

  // CVT
  pinMode(JOYSTICK_Y_PIN, INPUT);
  pinMode(JOYSTICK_X_PIN, INPUT);

  // Brakes
  pinMode(RIGHT_BRAKE_PIN, INPUT);
  digitalWrite(RIGHT_BRAKE_PIN, LOW);
  pinMode(LEFT_BRAKE_PIN, INPUT);
  digitalWrite(LEFT_BRAKE_PIN, LOW);

  // DualMC33926MotorShield Motor Controller
  motors.init();
  motors.setM1Speed(0);
  motors.setM2Speed(0);
  delay(REBOOT_WAIT);

  // Engine temperature sensor DS18B20
  temperature_sensor.begin();

  // Fuel Sensor
  attachInterrupt(digitalPinToInterrupt(LPH_SENSOR_PIN), lph_counter, RISING);

  // Begin serial communication
  Serial.begin(BAUD);
}

/* --- LOOP --- */
void loop() {

  // Check Failsafe Switches (Default to 1 if disconnected)
  hitch_kill = check_switch(HITCH_KILL_PIN);
  button_kill = check_switch(BUTTON_KILL_PIN);

  // Check Seat
  seat_kill = check_seat();

  // Check Regular Switches (Default to 0 if disconnected)
  trigger_kill  = check_switch(TRIGGER_KILL_PIN);
  ignition = check_switch(IGNITION_PIN);
  display_mode = check_switch(DISPLAY_MODE_PIN);
  cart_forward = check_switch(CART_FORWARD_PIN);
  cart_backward = check_switch(CART_BACKWARD_PIN);
  throttle_up = check_switch(THROTTLE_UP_PIN);
  throttle_down = check_switch(THROTTLE_DOWN_PIN);
  throttle_high = check_switch(THROTTLE_HIGH_PIN);
  throttle_low = check_switch(THROTTLE_LOW_PIN);

  // Change VDC and TSC modes
  if (check_switch(CART_MODE_PIN)) {
    if (cart_mode == 1) {
      cart_mode = 0;
    }
    else {
      cart_mode = 1;
    }
  }
  if (check_switch(PULL_MODE_PIN)) {
    if (pull_mode == 1) {
      pull_mode = 0;
    }
    else {
      pull_mode = 1;
    }
  }

  // RFID
  if (rfid_auth == 0) {
    rfid_auth = check_rfid();
  }

  // Brakes
  left_brake = check_brake(LEFT_BRAKE_PIN);
  right_brake = check_brake(RIGHT_BRAKE_PIN);
  set_brakes(right_brake, left_brake);

  // CVT
  cvt_sp = check_joystick(JOYSTICK_Y_PIN);

  //  // Adjust throttle limit, either HIGH/LOW, or increment
  if (throttle_high  && !throttle_low) {
    throttle_sp = THROTTLE_MAX;
  }
  else if (throttle_low && !throttle_high) {
    throttle_sp = THROTTLE_MIN;
  }
  else if (throttle_up && !throttle_down) {
    throttle_sp = THROTTLE + THROTTLE_STEP;
  }
  else if (throttle_down && !throttle_up) {
    throttle_sp = THROTTLE - THROTTLE_STEP;
  }
  else {
    if (throttle_sp > THROTTLE_MAX) {
      throttle_sp = THROTTLE_MAX;
    }
    else if (throttle_sp < THROTTLE_MIN) {
      throttle_sp = THROTTLE_MIN;
    }
    else {
      0; // do nothing
    }
  }

  // Ignore throttle setpoint if trigger not engaged
  if (trigger_kill) {
    throttle_pv = set_throttle(throttle_sp);
  }
  else {
    throttle_pv = set_throttle(0); // DISABLE THROTTLE IF OPERATOR RELEASES TRIGGER
  }

  // (0) If OFF
  if (run_mode == 0) {
    if (rfid_auth != 0 && !seat_kill) {
      standby();
    }
  }
  // (1) If STANDBY
  else if (run_mode == 1) {
    if ( seat_kill || hitch_kill ) {
      kill(); // kill engine
    }
    else if (ignition && left_brake && right_brake) {
      start(); // execute ignition sequence
    }
    else {
      standby(); // remain in standby (run_mode 1)
    }
  }
  // (2) If DRIVE
  else if (run_mode == 2) {
    if (seat_kill || hitch_kill ) {
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

  // Check engine sensors
  temperature = get_engine_temp();
  lph = get_fuel_lph();
  psi = get_oil_psi();
  dtostrf(lph, DIGITS, PRECISION, lph_buffer);
  dtostrf(temperature, DIGITS, PRECISION, temp_buffer);
  dtostrf(psi, DIGITS, PRECISION, psi_buffer);

  // Main Info
  if (Serial) {
    if (PRINT_MAIN_INFO) {
      sprintf(data_buffer, "{'run_mode':%d,'disp_mode':%d,'rfid_auth':%d,'seat_kill':%d,'hitch_kill':%d,'canbus':%d}", run_mode, display_mode, rfid_auth, seat_kill, hitch_kill, canbus_status);
      sprintf(output_buffer, "{\"data\":%s,\"chksum\":%d}", data_buffer, checksum(data_buffer));
      Serial.println(output_buffer);
      Serial.flush();
    }
  
    // Throttle Info
    if (PRINT_THROTTLE_INFO) {
      sprintf(data_buffer, "{'ignition':%d,'throttle_sp':%d,'throttle_pv':%d,'trigger':%d,'throttle_low':%d,'throttle_high':%d,'throttle_down':%d,'throttle_up':%d}", ignition, throttle_sp, throttle_pv, trigger_kill, throttle_low, throttle_high, throttle_down, throttle_up);
      sprintf(output_buffer, "{\"data\":%s,\"chksum\":%d}", data_buffer, checksum(data_buffer));
      Serial.println(output_buffer);
      Serial.flush();
    }
  
    // Ballast Info
    if (PRINT_VDC_INFO) {
      sprintf(data_buffer, "{'cart_mode':%d,'cart_forward':%d,'cart_backward':%d}", cart_mode, cart_forward, cart_backward);
      sprintf(output_buffer, "{\"data\":%s,\"chksum\":%d}", data_buffer, checksum(data_buffer));
      Serial.println(output_buffer);
      Serial.flush();
    }
  
    // CVT Info
    if (PRINT_TSC_INFO) {
      sprintf(data_buffer, "{'cvt_sp':%d,'pull_mode':%d}", cvt_sp, pull_mode);
      sprintf(output_buffer, "{\"data\":%s,\"chksum\":%d}", data_buffer, checksum(data_buffer));
      Serial.println(output_buffer);
      Serial.flush();
    }
  
    // Brake Info
    if (PRINT_BRAKE_INFO) {
      sprintf(data_buffer, "{'right_brake':%d,'left_brake':%d}", right_brake, left_brake);
      sprintf(output_buffer, "{\"data\":%s,\"chksum\":%d}", data_buffer, checksum(data_buffer));
      Serial.println(output_buffer);
      Serial.flush();
    }
  }

  // CANBus
  if (canbus_status) {
    
    // Check if messages
    unsigned char UID = Canbus.message_rx(canbus_rx_buffer);

    // Send Message A
    canbus_tx_buffer[0] = ESC_A_ID;
    canbus_tx_buffer[1] = freq_engine;
    canbus_tx_buffer[2] = freq_shaft;
    canbus_tx_buffer[3] = freq_wheel;
    canbus_tx_buffer[4] = cvt_pos;
    canbus_tx_buffer[5] = cvt_target;
    canbus_tx_buffer[6] = trans_gear;
    canbus_tx_buffer[7] = trans_locked;
    Canbus.message_tx(ESC_A_PID, canbus_tx_buffer);
  }

}

/* --- SYNCHRONOUS TASKS --- */
/// Set Throttle
int set_throttle(int sp) {

  // Read position and compute PID
  int pv = analogRead(THROTTLE_POS_PIN); // the current throttle position, i.e. the process value (PV)
  int throttle_in = map(pv, THROTTLE_POS_MIN, THROTTLE_POS_MAX, 900, 0); // get the position feedback from the linear actuator
  int error = throttle_in - sp;
  int throttle_out = -1 * THROTTLE_P * error;
  if (throttle_out > 400) {
    throttle_out = 400;
  }
  if (throttle_out < -400) {
    throttle_out = -400;
  }

  // Engage throttle actuator
  if (motors.getM2CurrentMilliamps() >  THROTTLE_MILLIAMP_THRESHOLD) {
    motors.setM2Speed(0); // disable if over-amp
  }
  else {
    motors.setM2Speed(throttle_out);
  }

  return pv;
}

/// Check Joystick
int check_joystick(int pin) {
  int val = analogRead(pin); // read signal
  int percent;
  if (val < (JOYSTICK_ZERO - JOYSTICK_DEADBAND)) {
    percent = map(val, JOYSTICK_ZERO - JOYSTICK_DEADBAND, JOYSTICK_MAX, 0, 100);
  }
  else {
    percent = 0;
  }
  return percent;
}

/// Check Brake
// Engage the brakes linearly
int check_brake(int pin) {
  int val = analogRead(pin); // read brake signal
  return val;
}

/// Set brakes
// Returns true if the brake interlock is engaged
int set_brakes(int right_brake, int left_brake) {

  // Map the brake values for DualVNH5019 output
  int output; // (left_brake + right_brake) / 2;
  if (left_brake > BRAKES_DEADBAND) {
    output = 400;
  }
  else {
    output = -400;
  }

  // Engage brakes
  brakes_milliamp = motors.getM1CurrentMilliamps();
  motors.setM1Speed(output);
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

/// Check RFID
int check_rfid(void) {
  Serial3.write(RFID_READ);
  int a = Serial3.read();
  int b = Serial3.read();
  int c = Serial3.read();
  int d = Serial3.read();
  if ((a >= 0) || (b >= 0) || (c >= 0) || (d >= 0)) {
    return a + b + c + d; // the user's key number
  }
  else {
    return 0;
  }
}

/// Checksum
int checksum(char* buf) {
  int sum = 0;
  for (int i = 0; i < DATA_LENGTH; i++) {
    sum += buf[i];
  }
  int val = sum % 256;
  return val;
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

/// Kill
void kill(void) {
  digitalWrite(STOP_RELAY_PIN, HIGH);
  digitalWrite(REGULATOR_RELAY_PIN, HIGH);
  digitalWrite(STARTER_RELAY_PIN, HIGH);
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
  float tmp = temperature_sensor.getTempCByIndex(0);
  temperature_sensor.requestTemperatures();
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
