// Split-Flap display library testing program
// Elliot Baptist 2019/06/16

// Includes
#include <SPI.h>
#include "flaptastic.h"

// Constants
const int N_OE_PIN = 10;

// Functions

// Globals
EWNB::Flaptastic disp;
unsigned long last_active;

// Setup
void setup() {
  // Serial setup
  Serial.begin(230400);
  Serial.println(F("RegFlap EWNB::Flaptastic test program booting"));

  // SPI setup
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, LSBFIRST, SPI_MODE0));

  // Split-flap display global setup
  disp.init({N_OE_PIN}, &SPI);
  // Split-flap display unit setup
  EWNB::Flaptastic::unit_cfg_t unit_cfg;
  unit_cfg.motor_level = 1;
  unit_cfg.home_rising = false;
  unit_cfg.dir = 1;
  unit_cfg.bi = true;
  unit_cfg.shift = 2;
  unit_cfg.thresh = 0b11110000;
  unit_cfg.steps = 2048;
  unit_cfg.offset = 980;
  unit_cfg.flaps = 8;
  unit_cfg.tolerance = 200;
  disp.addUnit(unit_cfg);
  unit_cfg.bi = false;
  unit_cfg.offset = 10;
  unit_cfg.flaps = 16;
  unit_cfg.tolerance = 100;
  disp.addUnit(unit_cfg);
  unit_cfg.flaps = 32;
  disp.addUnit(unit_cfg);
  // Home all units
  while (disp.step()) ;

  last_active = millis();
}

// Loop
void loop() {
  disp.set(0,0);
  while (disp.step()) ;
}

