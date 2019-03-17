// Include
#include <SPI.h>
#include <Print.h>

// Constants
const int SREG_S1_PIN = 10;


const int SREG_NUM_BITS = 4;
const int SREG_NUM_REGS = 1;

const bool STEPPER_DIR = 1;
const bool STEPPER_ACTIVE_LEVEL = 1;

// Globals
byte currentStepperState[SREG_NUM_REGS];
bool rotateStepper[SREG_NUM_REGS] = {0};

// Functions

void setRotation(char stepper, bool rotate) {
  rotateStepper[stepper] = rotate;
}

void doStep() {
  for (int i = 0; i < SREG_NUM_REGS; i++) {
    if (rotateStepper[i]) {
      if (STEPPER_DIR) 
        currentStepperState[i] = ((currentStepperState[i]&0x07) << 1) | ((currentStepperState[i]&0x08) >> 3);
      else 
        currentStepperState[i] = ((currentStepperState[i]&0x0E) >> 1) | ((currentStepperState[i]&0x01) << 3);
    }
  }

  // read less often?
//  PORTB = 0b000111;
  digitalWrite(SREG_S1_PIN, HIGH);
  SPI.transfer(0);

//  PORTB = 0b000101;
  digitalWrite(SREG_S1_PIN, LOW);
  byte shift_data[SREG_NUM_REGS];
  for (int i = 0; i < SREG_NUM_REGS; i++) {
      shift_data[i] = SPI.transfer(currentStepperState[i]<<4);
  }
//  PORTB = 0b000000;
  Serial.println(shift_data[0] & 0xff);
}

// Setup
void setup() {
  Serial.begin(230400);
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, LSBFIRST, SPI_MODE0));
  
  pinMode(SREG_S1_PIN, OUTPUT);

  digitalWrite(SREG_S1_PIN, LOW);

  for (int i = 0; i < SREG_NUM_REGS; i++) {
    currentStepperState[i] = STEPPER_ACTIVE_LEVEL ? 0x01 : 0x0E;
    setRotation(i,1);
  }
}

void loop() {
  for (int i = 0; i < 513*4; i++){
    doStep();
    delayMicroseconds(2000);
  }
//  while(1) {;}
  //delay(2);
  //Serial.println(currentStepperState[0], HEX);
}
