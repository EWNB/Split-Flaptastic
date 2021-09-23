// Split-Flap display library testing program
// Elliot Baptist 2019/06/16

// Includes
#include <SPI.h>
#include "flaptastic.h"

// Constants
const int N_OE_PIN = 10;
const int CE_PIN = 9;
const int STEPPER_STEP_PERIOD_US = 2400;

// Functions
int char_to_flap(char ch) {
  if ('0' <= ch && ch <= '9') {
    return ch - '0';
  }
  ch = toupper(ch);
  if ('A' <= ch && ch <= 'F') {
    return ch - 'A' + 0xA;
  }
  return -1;
}

// Globals
EWNB::Flaptastic disp;
unsigned long last_time;
unsigned long last_active;
char ch, flap;
bool flash;

// Setup
void setup() {
  // Serial setup
  Serial.begin(230400);
  Serial.println(F("RegFlap EWNB::Flaptastic test program booting"));

  // Split-flap display 'chip enable' (CLK pass enable) setup
  pinMode(CE_PIN, OUTPUT);
  digitalWrite(CE_PIN, LOW); // LOW == disable CLK to split-flap display

  // SPI setup
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, LSBFIRST, SPI_MODE0));

  // Split-flap display global setup
  disp.init({N_OE_PIN}, &SPI);
  
  // Split-flap display unit setup
  EWNB::Flaptastic::unit_cfg_t unit_cfg;
  unit_cfg.motor_level = 1;
  unit_cfg.home_rising = false;
  unit_cfg.thresh = 0b11110000;
  unit_cfg.steps = 2048;
  unit_cfg.dir = 1;
  unit_cfg.flaps = 16;
  unit_cfg.offset = unit_cfg.steps / (unit_cfg.flaps * 2) - 5;
  unit_cfg.tolerance = 100;
  disp.addUnit(unit_cfg);

  // Timer interrupt setup
  noInterrupts(); // global interrupt disable while configuring
  TCCR2A = 0b00000010; // CTC (counter clear) mode
  TCCR2B = 0b00000110; // clk/256
  OCR2A = STEPPER_STEP_PERIOD_US/(256/16); // period
  TCNT2 = 0; // reset count
  TIMSK2 = 0b00000010; // interrupt on OCR2A compare match
  interrupts(); // renable interrupts

    // Home all units
  while (!disp.allDone()) ;

  // Initialise time variables
  last_time = millis();
  last_active = millis();
}

// Timer ISR 
ISR(TIMER2_COMPA_vect) {
//  unsigned long start = micros();
  digitalWrite(CE_PIN, HIGH);
  disp.step();
  digitalWrite(CE_PIN, LOW);
//  Serial.println(micros() - start);
}

// Loop
void loop() {
  // Set things for split-flap display to display
  if (1000 < millis() - last_time) {
    flap = -1;
    ch = Serial.read();
    if (ch != -1) {
      // user message
      flap = char_to_flap(ch);
      last_active = millis();
    } else if (last_active + 10000 < millis()) {
      // counting mode
      flap = (millis() / 1000) % 0x10;
    }
    last_time = millis();
    if (flap != -1) {
      disp.setFlap(0, flap);
    }
  }

  // Flash LEDs attached to display unit 0
  if (bool(millis()&(1<<8)) != flash) {
    flash = !flash;
    disp.setOut(0, 0, flash);
    disp.setOut(0, 1, !flash);
  }

  // Enable timer interrupt only when required, 'cause why not
  if (disp.allDone() && TIMSK2 & 0b00000010) {
    noInterrupts(); // global interrupt disable
    TIMSK2 &= ~0b00000010; // disable interrupt
    interrupts(); // renable global interrupts
  } else if (!disp.allDone() && !(TIMSK2 & 0b00000010)) {
    noInterrupts(); // global interrupt disable
    TIMSK2 |= 0b00000010; // enable interrupt
    interrupts(); // renable global interrupts
  }
  
}
