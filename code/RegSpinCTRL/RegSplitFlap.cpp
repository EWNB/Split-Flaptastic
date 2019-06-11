// Shift register control of a split-flap display
// Elliot Baptist 2018/09/11

// Includes
#include "RegSplitFlap.h"

namespace EWNB_RegSplitFlap
{

  // Constants

  typedef enum
  {
    WAITING_FOR_NOT_HOME,
    SEEN_NOT_HOME,
    HOME_FOUND
  } homing_state_t;


  // Statics
  float FLAP_STEPS_PER_FLAP[SREG_NUM_REGS];
  byte STEPPER_DISABLE_PATTERN[SREG_NUM_REGS];

  byte m_stepperPatterns[8] = {0};
  int m_stepperStepIndex[SREG_NUM_REGS] = {0};
  byte m_stepperCoilState[SREG_NUM_REGS] = {0};
  byte m_stepperReadData[SREG_NUM_REGS] = {0};
  bool m_rotateStepper[SREG_NUM_REGS] = {0};
  homing_state_t m_stepperHomeState[SREG_NUM_REGS] = {0};
  bool m_stepperHomeActiveLastTime[SREG_NUM_REGS] = {0};
  int m_accelCount[SREG_NUM_REGS] = {0};
  int m_accelLimit[SREG_NUM_REGS] = {0};
  int m_stepperCurrentPosition[SREG_NUM_REGS] = {-1};
  int m_stepperTargetStep[SREG_NUM_REGS] = {0};

  // Functions
  void init()
  {
    SPI.begin();
    SPI.beginTransaction(SPISettings(8000000, LSBFIRST, SPI_MODE0));

    pinMode(SREG_nOE_PIN, OUTPUT);

    digitalWrite(SREG_nOE_PIN, HIGH);

    for (int i = 0; i < 8; i++) {
      m_stepperPatterns[i] = (1 << ((i+1)/2)%4)| (1 << i/2);
      if (STEPPER_ACTIVE_LEVEL == 0)
        m_stepperPatterns[i] = ~m_stepperPatterns[i];
    }

    for (int i = 0; i < SREG_NUM_REGS; i++) {
      FLAP_STEPS_PER_FLAP[i] = STEPPER_STEPS_PER_REV / float(FLAP_NUM_FLAPS[i]);
      STEPPER_DISABLE_PATTERN[i] = STEPPER_ACTIVE_LEVEL[i] + 0xFF;
      m_stepperCoilState[i] = m_stepperPatterns[0];
      m_stepperHomeState[i] = WAITING_FOR_NOT_HOME;
      m_accelCount[i] = STEPPER_ACCEL_PERIOD_START;
      m_accelLimit[i] = STEPPER_ACCEL_PERIOD_START;
      m_rotateStepper[i] = 1; //setRotation(i,1);
    }

    // Timer interrupt setup
    noInterrupts(); // global interrupt disable while configuring
    TCCR2A = 0b00000010; // CTC (counter clear) mode
    TCCR2B = 0b00000110; // clk/256
    OCR2A = STEPPER_STEP_PERIOD_US/(256/16); // period
    TCNT2 = 0; // reset count
    TIMSK2 = 0b00000010; // interrupt on OCR2A compare match
    interrupts(); // renable interrupts
  }

  // Timer interrupt handler
  ISR(TIMER2_COMPA_vect) {
    doStep();
  }

  void doStep()
  {
    digitalWrite(SREG_nOE_PIN, HIGH);
    SPI.transfer(0); // clock for a byte to enact load
    digitalWrite(SREG_nOE_PIN, LOW);

    // Check if should rotate
    for (int i = 0; i < SREG_NUM_REGS; i++) {
      // Check if should rotate
      bool homeActive = m_stepperReadData[i] < HOME_ACTIVE_THRESHOLD;
      if (m_stepperHomeState[i] != HOME_FOUND) {
        m_rotateStepper[i] = true;
        //Serial.println(homeActive);
        if (m_stepperHomeState[i] == WAITING_FOR_NOT_HOME && !homeActive) {
          m_stepperHomeState[i] = SEEN_NOT_HOME;
        } else if (m_stepperHomeState[i] == SEEN_NOT_HOME && homeActive) {
          m_stepperHomeState[i] = HOME_FOUND;
          m_stepperCurrentPosition[i] = 0;
          m_rotateStepper[i] = m_stepperTargetStep[i] != 0;
          Serial.println("home found");
        }
      } else {
        if (homeActive && !m_stepperHomeActiveLastTime[i]) {
          if (m_stepperCurrentPosition[i] >= STEPPER_STEPS_PER_REV-FLAP_HOME_TOLERANCE_STEPS
            && m_stepperCurrentPosition[i] <= STEPPER_STEPS_PER_REV+FLAP_HOME_TOLERANCE_STEPS) {
              m_stepperCurrentPosition[i] = 0;
              Serial.println("rehome succeded");
            } else {
              Serial.println("rehome ignored");
            }
        }
        if (m_stepperCurrentPosition[i] > STEPPER_STEPS_PER_REV+FLAP_HOME_TOLERANCE_STEPS) {
          m_stepperHomeState[i] = WAITING_FOR_NOT_HOME;
          Serial.println("home lost");
        }
        m_stepperHomeActiveLastTime[i] = homeActive;
        m_rotateStepper[i] = m_stepperCurrentPosition[i] != m_stepperTargetStep[i];
      }

      // Calculate new coil state
      if (m_rotateStepper[i]) {
        if (m_accelCount[i] <= 0 || !STEPPER_ACCELERATE) {
          if (m_accelLimit[i] > 0) {
            m_accelCount[i] = m_accelLimit[i];
          }
          if (!STEPPER_REVERSE_DIR[i]) {
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
 
        }
        if (m_accelLimit[i] > 0) {
          m_accelCount[i] -= STEPPER_ACCEL_COUNT_REDUCTION;
          m_accelLimit[i] -= STEPPER_ACCEL_PERIOD_REDUCTION;
        }
      } else {
        m_stepperCoilState[i] = STEPPER_DISABLE_PATTERN[i];
        m_accelCount[i] = STEPPER_ACCEL_PERIOD_START;
        m_accelLimit[i] = STEPPER_ACCEL_PERIOD_START;
      }

      // Output to coil drivers and read home sensor
      byte rddata = SPI.transfer(m_stepperCoilState[i]<<2);
      if (i < SREG_NUM_REGS) {
      m_stepperReadData[i] = rddata;
      }
    }
  }

  void setTarget(int targetFlap, int unit)
  {
    Serial.print("target ");
    Serial.println(targetFlap);
    noInterrupts(); // critical section
    m_stepperTargetStep[unit] = (FLAP_HOME_STEP_OFFSET[unit] + (int)(FLAP_STEPS_PER_FLAP[unit]*targetFlap)) % STEPPER_STEPS_PER_REV;
    interrupts(); // critical section
  }

  void setTargets(int targets[], int len, int offset) {
    for (int unit = offset; unit < offset+len; unit++) {
      setTarget(targets[unit], unit);
    }
  }

  bool reachedTarget(int unit) {
    bool result;
    noInterrupts(); // critical section
    result = m_stepperCurrentPosition[unit] == m_stepperTargetStep[unit]
              && m_stepperHomeState[unit] == HOME_FOUND;
    interrupts();
    return result;
  }

  bool reachedTargets() {
    bool result = true;
    //noInterrupts(); // critical section?
    for (int unit = 0; unit < SREG_NUM_REGS; unit++) {
      result &= reachedTarget(unit);
    }
    //interrupts();
    return result;
  }

}
