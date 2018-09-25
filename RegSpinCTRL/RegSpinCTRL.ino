// Includes
//#include <Print.h>
#include "RegSplitFlap.h"

// Constants

// Functions

// Setup
void setup() 
{
  Serial.begin(230400);
  Serial.println("Booting");

  EWNB_RegSplitFlap::init();
}

// loop globals
int flap = 0;
void loop() 
{
  delay(6000);
  EWNB_RegSplitFlap::setTarget(0, 7); //flap);
  Serial.println("set 7");
  delay(1000);
  EWNB_RegSplitFlap::setTarget(0, 0);
  Serial.println("set 0");
//  flap++;
//  flap = flap % 16;
//  Serial.println(EWNB_RegSplitFlap::STEPPER_DISABLE_PATTERN);
}
