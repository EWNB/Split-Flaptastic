// Includes
//#include <Print.h>
#include "RegSplitFlap.h"

// Constants
//const int SREG_SR_PIN = 11;
//const int SREG_CLK_PIN = 13;
//const int SREG_QA_TICK_PIN = 12;
const int ERROR_LED_PIN = 13;

//const int MOSI_PIN = SREG_SR_PIN;
//const int MISO_PIN = SREG_QA_TICK_PIN;
//const int SCLK_PIN = SREG_CLK_PIN;

// Globals
RegSplitFlap disp = RegSplitFlap();


// Functions


// Setup
void setup() 
{
  Serial.begin(230400);

  //pinMode(ERROR_LED_PIN, OUTPUT);

  //digitalWrite(ERROR_LED_PIN, HIGH);

  disp.begin();

}

// loop globals

void loop() 
{

  disp.doStep();
}
