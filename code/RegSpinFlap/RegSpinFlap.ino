// Include
#include <SPI.h>
#include <Print.h>

// Constants
const int SREG_nOE_PIN = 10;


const int SREG_NUM_BITS = 8;
const int SREG_NUM_REGS = 3;

const bool STEPPER_LEVEL[SREG_NUM_REGS] = {1,1,1};
const bool STEPPER_DIR[SREG_NUM_REGS] = {0,1,0};

// Globals
byte currentStepperState[SREG_NUM_REGS];
bool rotateStepper[SREG_NUM_REGS] = {0,0,0};

// Functions

void setRotation(int stepper, bool rotate) {
  rotateStepper[stepper] = rotate;
}

void doStep() {
  for (int i = 0; i < SREG_NUM_REGS; i++) {
    if (rotateStepper[i]) {
      if (STEPPER_DIR[i])
        currentStepperState[i] = ((currentStepperState[i]&0x07) << 1) | ((currentStepperState[i]&0x08) >> 3);
      else
        currentStepperState[i] = ((currentStepperState[i]&0x0E) >> 1) | ((currentStepperState[i]&0x01) << 3);
    }
  }

  byte shift_data[SREG_NUM_REGS];
  digitalWrite(SREG_nOE_PIN, HIGH);
  //delay(1);
  //shift_data[0] = SPI.transfer(0);
  SPI.transfer(0);

  digitalWrite(SREG_nOE_PIN, LOW);
  for (int i=0; i<SREG_NUM_REGS; i++) {
    byte read_val = SPI.transfer(currentStepperState[i]<<2);
    //if (i<SREG_NUM_REGS-1) shift_data[i+1] = read_val;
    shift_data[i] = read_val;

  }
  for (int i=0; i<SREG_NUM_REGS; i++) {
    Serial.print(shift_data[i], BIN);
    if (i<SREG_NUM_REGS-1) Serial.print(',');
  }
  Serial.println();
}

// Setup
void setup() {
  Serial.begin(230400);
  Serial.println("Booting RegSpinFlap");

  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, LSBFIRST, SPI_MODE0));

  pinMode(SREG_nOE_PIN, OUTPUT);

  digitalWrite(SREG_nOE_PIN, LOW);

  for (int i = 0; i < SREG_NUM_REGS; i++) {
    currentStepperState[i] = STEPPER_LEVEL[i] ? 0x01 : 0x0E;
    setRotation(i,1);
  }
}

void loop() {
  for (int i = 0; i < 513*4; i++){
    doStep();
    delayMicroseconds(2000);
  }
}
