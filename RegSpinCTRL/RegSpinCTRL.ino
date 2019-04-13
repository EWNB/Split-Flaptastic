// Includes
//#include <Print.h>
#include "RegSplitFlap.h"

// Constants

// Functions
int char_to_flap(char ch) {
  int flap = 31;
  ch = toupper(ch);
   if ('A' <= ch && ch <= 'Z') {
    flap = ch-'A';
   } else {
    switch (ch) {
      case '?': flap = 26; break;
      case '!': flap = 27; break;
      case '&': flap = 28; break;
      case '/': flap = 29; break;
      case ',': flap = 30; break;
      case ' ': flap = 31; break;
    }
  }
  return flap;
}

// Globals
char ch, flap;
int demo_index;
unsigned long last_active;
String demo_message = "hi! demo mode, enter msg? ABCDEFGHIJKLMNOPQRSTUVWXYZ?!&/, ";

// Setup
void setup() 
{
  Serial.begin(230400);
  Serial.println("Booting");

  EWNB_RegSplitFlap::init();
  while (!EWNB_RegSplitFlap::reachedTarget(0)) {;}

  last_active = millis();
}

// Loop
void loop() 
{
  flap = -1;
  ch = Serial.read();
  if (ch != -1) { // user message
    flap = char_to_flap(ch);
    last_active = millis();
    demo_index = 0;
  } else if (last_active+10000 < millis()) { // demo mode
    ch = demo_message.charAt(demo_index);
    flap = char_to_flap(ch);
    demo_index = (demo_index + 1) % demo_message.length();
  }
  if (flap != -1) {
     EWNB_RegSplitFlap::setTarget(flap, 0);
     while (!EWNB_RegSplitFlap::reachedTarget(0)) {;}
     delay(1000);
  }
}
