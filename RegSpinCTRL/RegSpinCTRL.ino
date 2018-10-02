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
  int targets[] = {0};
  EWNB_RegSplitFlap::setTargets(targets, sizeof(targets)/sizeof(targets[0]));
  while(!EWNB_RegSplitFlap::reachedTargets()) {;}
  delay(1000);
  EWNB_RegSplitFlap::setTarget(1, 0);
  while(!EWNB_RegSplitFlap::reachedTargets()) {;}
  delay(1000);
  EWNB_RegSplitFlap::setTarget(2, 0);
  while(!EWNB_RegSplitFlap::reachedTargets()) {;}
  delay(1000);
//  flap++;
//  flap = flap % 16;
}
