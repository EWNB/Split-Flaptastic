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

  EBaptist_RegSplitFlap::init();
}

// loop globals
int flap = 0;
void loop() 
{
  delay(6000);
  EBaptist_RegSplitFlap::setTarget(0, flap);
  flap++;
  flap = flap % 16;
}
