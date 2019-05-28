// Shift register control of a split-flap display
// Elliot Baptist 2018/09/11

#ifndef REGSPLITFLAP_H
#define REGSPLITFLAP_H

// Includes
#include <SPI.h>

namespace EWNB_RegSplitFlap
{

  // Constants
  const int SREG_nOE_PIN = 10;

  const int HOME_ACTIVE_THRESHOLD = 150;

  const int SREG_NUM_BITS = 8;
  const int SREG_NUM_REGS = 1;

  const bool STEPPER_REVERSE_DIR = 1;
  const bool STEPPER_ACTIVE_LEVEL = 1;
  const byte STEPPER_DISABLE_PATTERN = STEPPER_ACTIVE_LEVEL + 0xFF;
  const bool STEPPER_MICROSTEP = 1;
  const bool STEPPER_ACCELERATE = 1;
  const int STEPPER_STEPS_PER_REV = 2048 * (1+STEPPER_MICROSTEP);
  const int STEPPER_STEP_PERIOD_US = 1500 /(1+STEPPER_MICROSTEP); //1700
  const int STEPPER_ACCEL_PERIOD_START = 100;
  const int STEPPER_ACCEL_PERIOD_REDUCTION = 2 /(1+STEPPER_MICROSTEP);
  const int STEPPER_ACCEL_COUNT_REDUCTION = 20 /(1+STEPPER_MICROSTEP);

  const int FLAP_NUM_FLAPS = 8;
  const int FLAP_HOME_STEP_OFFSET[SREG_NUM_REGS] = {980 * (1+STEPPER_MICROSTEP)}; //970 1050 1020
  const int FLAP_HOME_TOLERANCE_STEPS = 10;

  void init();
  void doStep();
  void setTarget(int target, int unit);
  void setTargets(int targets[], int len, int offset=0);
  bool reachedTarget(int unit);
  bool reachedTargets();

};

#endif
