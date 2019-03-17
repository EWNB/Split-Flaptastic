// Shift register control of a split-flap display
// Elliot Baptist 2018/09/11

#ifndef REGSPLITFLAP_H
#define REGSPLITFLAP_H

// Includes
#include <SPI.h>

namespace EWNB_RegSplitFlap
{
  
  // Constants
  const int SREG_S1_PIN = 10;
  
  const int SREG_HOME_BITMASK = 0b1000;
  const bool HOME_ACTIVE_LEVEL = 1;
  
  const int SREG_NUM_BITS = 4;
  const int SREG_NUM_REGS = 1;
  
  const bool STEPPER_REVERSE_DIR = 0;
  const bool STEPPER_ACTIVE_LEVEL = 1;
  const byte STEPPER_DISABLE_PATTERN = STEPPER_ACTIVE_LEVEL + 0xFF;
  const bool STEPPER_MICROSTEP = 1;
  const bool STEPPER_ACCELERATE = 1;
  const int STEPPER_STEPS_PER_REV = 2048 * (1+STEPPER_MICROSTEP);
  const int STEPPER_STEP_PERIOD_US = 2000 /(1+STEPPER_MICROSTEP); //1700
  const int STEPPER_ACCEL_PERIOD_START = 100;
  const int STEPPER_ACCEL_PERIOD_REDUCTION = 2 /(1+STEPPER_MICROSTEP);
  const int STEPPER_ACCEL_COUNT_REDUCTION = 20 /(1+STEPPER_MICROSTEP);
   
  const int FLAP_NUM_FLAPS = 32;
  const int FLAP_HOME_STEP_OFFSET[SREG_NUM_REGS] = {10 * (1+STEPPER_MICROSTEP)}; //970 1050 1020
  const int FLAP_HOME_TOLERANCE_STEPS = 10;

  
 
  
  // Class declaration
  //class RegSplitFlap {
    //public:
      //RegSplitFlap();
      
      void init();
      void doStep();
      void setTarget(int target, int unit);
      void setTargets(int targets[], int len, int offset=0);
      bool reachedTarget(int unit);
      bool reachedTargets();
  
    //private:
//      void setRotation(int stepper, bool rotate);
//      void startHoming(int stepper);
  
      //static RegSplitFlap * instance;
      
//      byte m_stepperPatterns[8];
//      int m_stepperStepIndex[SREG_NUM_REGS];
//      byte m_stepperCoilState[SREG_NUM_REGS];
//      byte m_stepperReadData[SREG_NUM_REGS];
//      bool m_rotateStepper[SREG_NUM_REGS];
//      homing_state_t m_stepperHomeState[SREG_NUM_REGS]; 
//      unsigned long m_timeNextStep;
//      int m_accelCount[SREG_NUM_REGS];
//      int m_accelLimit[SREG_NUM_REGS];
//      int m_stepperCurrentPosition[SREG_NUM_REGS];
//      int m_stepperTargetFlap[SREG_NUM_REGS];
  
};

#endif
