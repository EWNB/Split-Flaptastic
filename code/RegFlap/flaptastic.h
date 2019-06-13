// Split-Flap display control software.
// Uses 74HC595 shift registers for scalability.
// Elliot Baptist 2019/06/13

#ifndef FLAPTASTIC_H
#define FLAPTASTIC_H

// Includes
#include <SPI.h>

namespace EWNB {

  class Flaptastic {
    // Types
    public:
      struct disp_cfg_t {
        // bool microstep;
        // bool accelerate;
        int num_units;
        int update_us; //
        int n_oe_pin;
      };
      struct unit_cfg_t {
        bool motor_level;
        bool home_edge;
        bool reverse;
        bool bidirect;
        int steps;
        int flaps;
        int offset;
        int thresh;
        int tolerance;
      };
    private:
      typedef enum {
        WAITING_FOR_NOT_HOME,
        SEEN_NOT_HOME,
        HOME_FOUND
      } homing_state_t;
      //
      struct unit_state_t {
        homing_state_t home;
        int msteps_flap; // milli-steps per flap
        int phase;
        int position;
        int target;
      };
      // Constants
      const int NUM_BITS = 8;
      const int MAX_UNITS = 140;
      // Variables
      int _units_configured;
      disp_cfg_t _disp_cfg;
      unit_cfg_t[MAX_UNITS] _unit_cfg;
      volatile unit_state_t[MAX_UNITS] _unit_state;

      bool m_rotateStepper[SREG_NUM_REGS] = {0};
      int m_accelCount[SREG_NUM_REGS] = {0};
      int m_accelLimit[SREG_NUM_REGS] = {0};

      const int STEPPER_ACCEL_PERIOD_START = 100;
      const int STEPPER_ACCEL_PERIOD_REDUCTION = 2 /(1+STEPPER_MICROSTEP);
      const int STEPPER_ACCEL_COUNT_REDUCTION = 20 /(1+STEPPER_MICROSTEP);

    public:
      Flaptastic(disp_cfg_t disp_cfg);
      bool addUnit(unit_cfg_t unit_cfg);
      void setFlap(int unit, int flap);
      // void setTargets(int targets[], int len, int offset=0);
      void doStep();
      bool done(int unit);
      bool allDone();

  };

};

#endif
