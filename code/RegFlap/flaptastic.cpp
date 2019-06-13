// Shift register control of a split-flap display
// Elliot Baptist 2018/09/11

// Includes
#include "flaptastic.h"

namespace EWNB {

  Flaptastic::Flaptastic(disp_cfg) : _disp_cfg(disp_cfg) _units_configured(0) {
    SPI.begin();
    SPI.beginTransaction(SPISettings(8000000, LSBFIRST, SPI_MODE0));

    pinMode(_disp_cfg.n_oe_pin, OUTPUT);
    digitalWrite(_disp_cfg.n_oe_pin, HIGH);
  }

  bool Flaptastic::addUnit(unit_cfg) {
    if (_units_configured < MAX_UNITS) {
      _unit_cfg[_units_configured++] = unit_cfg;
      return true;
    }
    return false;
  }

  void Flaptastic::doStep() {

    digitalWrite(_disp_cfg.n_oe_pin, HIGH);
    SPI.transfer(0); // clock for a byte to enact load
    digitalWrite(_disp_cfg.n_oe_pin, LOW);
    for (int i = _disp_cfg.num_units-1; i >= 0; i--) {
      //
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
