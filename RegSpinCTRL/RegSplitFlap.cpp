// Shift register control of a split-flap display
// Elliot Baptist 2018/09/11

// Includes
#include "RegSplitFlap.h"

namespace EBaptist_RegSplitFlap
{

  // Constants
  const float FLAP_STEPS_PER_FLAP = STEPPER_STEPS_PER_REV / float(FLAP_NUM_FLAPS);
  const int FLAP_HOME_STEP_OFFSET[SREG_NUM_REGS] = {970 * (1+STEPPER_MICROSTEP)};

  // Statics
  byte m_stepperPatterns[8];
  int m_stepperStepIndex[SREG_NUM_REGS];
  byte m_stepperCoilState[SREG_NUM_REGS];
  byte m_stepperReadData[SREG_NUM_REGS];
  bool m_rotateStepper[SREG_NUM_REGS];
  homing_state_t m_stepperHomeState[SREG_NUM_REGS]; 
  unsigned long m_timeNextStep;
  int m_accelCount[SREG_NUM_REGS];
  int m_accelLimit[SREG_NUM_REGS];
  int m_stepperCurrentPosition[SREG_NUM_REGS];
  int m_stepperTargetFlap[SREG_NUM_REGS];

  // Function Prototypes
  //void setRotation(int stepper, bool rotate);
  //void startHoming(int stepper);

  //RegSplitFlap *instance;
  
  //// Constructors
  //RegSplitFlap::RegSplitFlap()
  //{
  //
  //}

  
  // Functions
  void init()
  {
    SPI.begin();
    SPI.beginTransaction(SPISettings(8000000, LSBFIRST, SPI_MODE0));
    
    pinMode(SREG_S0_PIN, OUTPUT);
    pinMode(SREG_S1_PIN, OUTPUT);
    pinMode(SREG_NOT_OE_PIN, OUTPUT);
    
    digitalWrite(SREG_S0_PIN, LOW);
    digitalWrite(SREG_S1_PIN, LOW);
    digitalWrite(SREG_NOT_OE_PIN, HIGH);
  
    for (int i = 0; i < 8; i++) {
      m_stepperPatterns[i] = (1 << ((i+1)/2)%4)| (1 << i/2);
      if (STEPPER_ACTIVE_LEVEL == 0) 
        m_stepperPatterns[i] = ~m_stepperPatterns[i];
    }
  
    for (int i = 0; i < SREG_NUM_REGS; i++) {
      //m_stepperCoilState[i] = STEPPER_ACTIVE_LEVEL ? 0x01 : 0x0E;
      m_stepperCoilState[i] = m_stepperPatterns[0];
      m_stepperHomeState[i] = WAITING_FOR_NOT_HOME;
      m_accelCount[i] = STEPPER_ACCEL_PERIOD_START;
      m_accelLimit[i] = STEPPER_ACCEL_PERIOD_START;
      m_rotateStepper[i] = 1; //setRotation(i,1);
    }
  
    // Interrupt handler setup
    //RegSplitFlap::instance = this;
  
    // Timer interrupt setup
    noInterrupts(); // global interrupt disable while configuring
    TCCR2A = 0b00000010; // CTC (counter clear) mode
    TCCR2B = 0b00000110; // clk/256
    OCR2A = STEPPER_STEP_PERIOD_US/(256/16); // period
    TCNT2 = 0; // reset count
    TIMSK2 = 0b00000010; // interrupt on OCR2A compare match
//    // TIFR2 // interrupt status reg
    interrupts(); // renable interrupts
  }
  
  // Timer interrupt handler
  ISR(TIMER2_COMPA_vect) {
    //Serial.println("!");
    doStep();
  }
    
//  void setRotation(int stepper, bool rotate)
//  {
//    m_rotateStepper[stepper] = rotate;
//  }
    
//  void startHoming(int stepper)
//  {
//    m_stepperHomeState[stepper] = WAITING_FOR_NOT_HOME;
//  }
  
  void doStep()
  {
  ////  if (m_timeNextStep != 0 && m_timeNextStep < micros()) 
  ////    Serial.println("t!");//digitalWrite(ERROR_LED_PIN, HIGH);
  //  Serial.println(m_timeNextStep - micros());
  //  while (m_timeNextStep > micros()) ;
  //  if (m_timeNextStep == 0) 
  //    m_timeNextStep = micros();
  //  m_timeNextStep += STEPPER_STEP_PERIOD_US;
  
    // Check if should rotate
    for (int i = 0; i < SREG_NUM_REGS; i++) {
      // Check if should rotate
      if (m_stepperHomeState[i] != HOME_FOUND) {
        m_rotateStepper[i] = true;
        bool homeActive = ((bool)(m_stepperReadData[i] & SREG_HOME_BITMASK)) == HOME_ACTIVE_LEVEL;
        //Serial.println(homeActive);
        if (m_stepperHomeState[i] == WAITING_FOR_NOT_HOME && !homeActive) {
          m_stepperHomeState[i] = SEEN_NOT_HOME;
        } else if (m_stepperHomeState[i] == SEEN_NOT_HOME && homeActive) {
          m_stepperHomeState[i] = HOME_FOUND;
          m_stepperCurrentPosition[i] = FLAP_HOME_STEP_OFFSET[i];
        }
      }
      if (m_stepperHomeState[i] == HOME_FOUND) {
        if ((m_stepperCurrentPosition[i]/FLAP_STEPS_PER_FLAP) != m_stepperTargetFlap[i]) {
          m_rotateStepper[i] = true;
        } else {
          m_rotateStepper[i] = false;
        }
      }
  //    Serial.println(m_stepperCurrentPosition[i]/FLAP_STEPS_PER_FLAP);
  //    Serial.println(m_stepperTargetFlap[i]);
  //    Serial.println(m_stepperCurrentPosition[i]);
      
      // Calculate new coil state
      if (m_rotateStepper[i]) {
        if (m_accelCount[i] <= 0 || !STEPPER_ACCELERATE) {
          if (m_accelLimit[i] > 0) {
            m_accelCount[i] = m_accelLimit[i];
          }
          if (!STEPPER_REVERSE_DIR) {
            if (m_stepperStepIndex[i] == 7 - !STEPPER_MICROSTEP) m_stepperStepIndex[i] = 0;
            else m_stepperStepIndex[i] += 1 + !STEPPER_MICROSTEP;
            //m_stepperCoilState[i] = ((m_stepperCoilState[i]&0x7F) << 1) | ((m_stepperCoilState[i]&0x08) >> 7);
          } else {
            if (m_stepperStepIndex[i] == 0) m_stepperStepIndex[i] = 7 - !STEPPER_MICROSTEP;
            else m_stepperStepIndex[i] -= 1 + !STEPPER_MICROSTEP;
            //m_stepperCoilState[i] = ((m_stepperCoilState[i]&0xFE) >> 1) | ((m_stepperCoilState[i]&0x01) << 7);
          }
          m_stepperCoilState[i] = m_stepperPatterns[m_stepperStepIndex[i]];
          m_stepperCurrentPosition[i]++;
          if (m_stepperCurrentPosition[i] >= STEPPER_STEPS_PER_REV) m_stepperCurrentPosition[i] = 0;
        } 
        if (m_accelLimit[i] > 0) {
          m_accelCount[i] -= STEPPER_ACCEL_COUNT_REDUCTION;
          m_accelLimit[i] -= STEPPER_ACCEL_PERIOD_REDUCTION;
        }
      } else {
        m_stepperCoilState[i] = STEPPER_DISABLE_PATTERN;
        m_accelCount[i] = STEPPER_ACCEL_PERIOD_START;
        m_accelLimit[i] = STEPPER_ACCEL_PERIOD_START;
      }
    }
  
    // https://www.instructables.com/id/Fast-digitalRead-digitalWrite-for-Arduino/
    PORTB = 0b000111; // load mode
    SPI.transfer(0); // clock to enact parallel load
    PORTB = 0b000101; // shift right mode
    
    for (int i = 0; i < SREG_NUM_REGS; i++) {  
      // Output to coil drivers and read home sensor
      m_stepperReadData[i] = SPI.transfer(m_stepperCoilState[i]);
    }
    
    PORTB = 0b000000;
    //Serial.println((m_stepperReadData[0] & 0xF0));
  }

  void setTarget(int unit, int target)
  {
    m_stepperTargetFlap[unit] = target;
  }

}

