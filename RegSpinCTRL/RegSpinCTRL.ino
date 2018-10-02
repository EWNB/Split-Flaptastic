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
  while(!EWNB_RegSplitFlap::reachedTargets()) {;}
  delay(2000);
  int targets[] = {7};
  EWNB_RegSplitFlap::setTargets(targets, sizeof(targets)/sizeof(targets[0]));
  Serial.println("set 7");
  while(!EWNB_RegSplitFlap::reachedTargets()) {;}
  EWNB_RegSplitFlap::setTarget(0, 0);
  Serial.println("set 0");
//  flap++;
//  flap = flap % 16;
}
