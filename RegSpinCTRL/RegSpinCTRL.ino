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

// Loop
void loop() 
{
  int targets[] = {0};
  EWNB_RegSplitFlap::setTargets(targets, sizeof(targets)/sizeof(targets[0]));
  while(!EWNB_RegSplitFlap::reachedTargets()) {;}
  delay(1000);
  
  for (int i=0; i<32; i++){
    EWNB_RegSplitFlap::setTarget(i, 0);
    while(!EWNB_RegSplitFlap::reachedTargets()) {;}
    delay(1000);
  }
  
//  EWNB_RegSplitFlap::setTarget(0, 0);
//  while(!EWNB_RegSplitFlap::reachedTargets()) {;}
//  EWNB_RegSplitFlap::setTarget(1, 0);
//  while(!EWNB_RegSplitFlap::reachedTargets()) {;}
}
