// Include
#include <SPI.h>
#include <Print.h>

// Constants
const int SREG_SR_PIN = 11;
const int SREG_CLK_PIN = 13;
const int SREG_S0_PIN = 8; 
const int SREG_S1_PIN = 9;
const int SREG_NOT_OE_PIN = 10;
const int SREG_QA_TICK_PIN = 12;

const int MOSI_PIN = SREG_SR_PIN;
const int MISO_PIN = SREG_QA_TICK_PIN;
const int SCLK_PIN = SREG_CLK_PIN;
//const int SS_PIN = SREG_NOT_OE_PIN;

const int SREG_NUM_BITS = 8;
const int SREG_NUM_REGS = 8;

const bool STEPPER_DIR = 1;
const bool STEPPER_ACTIVE_LEVEL = 0;

// Globals
byte currentStepperState[SREG_NUM_REGS];
bool rotateStepper[SREG_NUM_REGS] = {0};

// Functions

void setRotation(char stepper, bool rotate)
{
  rotateStepper[stepper] = rotate;
}

void doStep()
{
  for (int i = 0; i < SREG_NUM_REGS; i++)
  {
    if (rotateStepper[i])
    {
      if (STEPPER_DIR) 
        currentStepperState[i] = ((currentStepperState[i]&0x07) << 1) | ((currentStepperState[i]&0x08) >> 3);
      else 
        currentStepperState[i] = ((currentStepperState[i]&0x0E) >> 1) | ((currentStepperState[i]&0x01) << 3);
    }
  }

  // faster? https://www.instructables.com/id/Fast-digitalRead-digitalWrite-for-Arduino/
  //digitalWrite(SREG_NOT_OE_PIN, HIGH);

  // read less often?
  PORTB = 0b000111;
  //digitalWrite(SREG_S0_PIN, HIGH);
  //digitalWrite(SREG_S1_PIN, HIGH);
//  digitalWrite(SREG_CLK_PIN, HIGH);
//  digitalWrite(SREG_CLK_PIN, LOW);
  SPI.transfer(0);

  PORTB = 0b000101;
  //digitalWrite(SREG_S0_PIN, HIGH);
  //digitalWrite(SREG_S1_PIN, LOW);
  //digitalWrite(SREG_NOT_OE_PIN, HIGH);
  byte shift_data[SREG_NUM_REGS];
  for (int i = 0; i < SREG_NUM_REGS; i++)
  {
      shift_data[i] = SPI.transfer(currentStepperState[i]);
//      Serial.println(shift_data & 0xF0);
      //delayMicroseconds(500);
      //shiftOut(SREG_SR_PIN, SREG_CLK_PIN, LSBFIRST, shift_data);
//    for (int b = 0; b < SREG_NUM_BITS; b++)
//    {
//      digitalWrite(SREG_SR_PIN, shift_data & 0x01);
//      shift_data >>= 1;
//      //delay(1);
//      digitalWrite(SREG_CLK_PIN, HIGH);
//      //delay(1);
//      digitalWrite(SREG_CLK_PIN, LOW);
//    }
  }
  PORTB = 0b000000;
  //digitalWrite(SREG_NOT_OE_PIN, LOW);
  //digitalWrite(SREG_S0_PIN, LOW);
  //digitalWrite(SREG_S1_PIN, LOW);
  //digitalWrite(SREG_NOT_OE_PIN, LOW);
//  char buf[4];
//  sprintf(buf,"%03d",shift_data[0] & 0xF0);
//  Serial.println(buf);
  Serial.println(bool(shift_data[0] & 0xF0));
}

// Setup
void setup()
{
  Serial.begin(230400);
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, LSBFIRST, SPI_MODE0));
  
//  pinMode(SREG_SR_PIN, OUTPUT);
//  pinMode(SREG_CLK_PIN, OUTPUT);
  pinMode(SREG_S0_PIN, OUTPUT);
  pinMode(SREG_S1_PIN, OUTPUT);
  pinMode(SREG_NOT_OE_PIN, OUTPUT);
//  pinMode(SREG_QA_TICK_PIN, INPUT);
  //pinMode(SS_PIN, OUTPUT);

//  digitalWrite(SREG_SR_PIN, LOW);
//  digitalWrite(SREG_CLK_PIN, LOW);
  digitalWrite(SREG_S0_PIN, LOW);// LOW);
  digitalWrite(SREG_S1_PIN, LOW);
  digitalWrite(SREG_NOT_OE_PIN, HIGH);
  //digitalWrite(SS_PIN, HIGH);

  for (int i = 0; i < SREG_NUM_REGS; i++)
  {
    currentStepperState[i] = STEPPER_ACTIVE_LEVEL ? 0x01 : 0x0E;
    setRotation(i,1);
  }

  //setRotation(0,1);
}

void loop()
{
  for (int i = 0; i < 513*4; i++){
    doStep();
    delayMicroseconds(2000);
  }
  while(1) {;}
  //delay(2);
  //Serial.println(currentStepperState[0], HEX);
}
