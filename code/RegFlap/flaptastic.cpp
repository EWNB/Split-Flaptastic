// Split-Flap display control software.
// Uses 74HC595 shift registers for scalability.
// Elliot Baptist 2019/06/13

// Includes
#include "flaptastic.h"

#include <util/atomic.h>

namespace EWNB {

  Flaptastic::Flaptastic() : _idle(false), _num_units(0) {}

  void Flaptastic::init(disp_cfg_t disp_cfg, SPIClass* spi) {
    // Set display config and SPI pointer
    _disp_cfg = disp_cfg;
    _spi = spi;
    // Setup shift register output enable pin
    pinMode(_disp_cfg.n_oe_pin, OUTPUT);
    digitalWrite(_disp_cfg.n_oe_pin, LOW); // LOW == enable outputs
  }

  bool Flaptastic::addUnit(unit_cfg_t unit_cfg) {
    if (_num_units < MAX_UNITS) {
      _unit_cfg[_num_units].motor_level = unit_cfg.motor_level;
      _unit_cfg[_num_units].home_rising = unit_cfg.home_rising;
      _unit_cfg[_num_units].dir = unit_cfg.dir;
      _unit_cfg[_num_units].bi = unit_cfg.bi;
      _unit_cfg[_num_units].shift = unit_cfg.shift;
      _unit_cfg[_num_units].thresh = unit_cfg.thresh;
      _unit_cfg[_num_units].steps = unit_cfg.steps;
      _unit_cfg[_num_units].offset = unit_cfg.offset;
      _unit_cfg[_num_units].home_start = unit_cfg.steps - unit_cfg.tolerance;
      _unit_cfg[_num_units].home_end = unit_cfg.steps + unit_cfg.tolerance;
      _unit_cfg[_num_units].msteps_flap = unit_cfg.flaps ? (unit_cfg.steps*1000L)/unit_cfg.flaps : unit_cfg.steps*1000L;
      _unit_state[_num_units].homed = false;
      _unit_state[_num_units].prev_home = 2;
      _unit_state[_num_units].phase = 0;
      _unit_state[_num_units].pos = 0;
      _unit_state[_num_units].target = 0;
      _num_units++;
      return true;
    } else {
      return false;
    }
  }

  bool Flaptastic::step() {
    if (!_idle) {
      // Setup
      bool idle = true;
      byte send_data[MAX_UNITS];
      byte recv_data[MAX_UNITS];
      // Calculate motor states
      for (int i = _num_units-1; i >= 0; i--) {
        send_data[i] = _unit_cfg[i].motor_level ? 0x00 : 0xFF;
        if (!_unit_state[i].homed || _unit_state[i].pos != _unit_state[i].target) {
          // TODO: add bidirectional logic
          _unit_state[i].phase = (_unit_cfg[i].dir ? _unit_state[i].phase+1 : _unit_state[i].phase-1) & 0b11;
          send_data[i] ^= 1 << _unit_state[i].phase;
          _unit_state[i].pos++;
          idle = false;
        }
        send_data[i] <<= _unit_cfg[i].shift;
      }
      // Drive clock with shift reg outputs disabled to load sensor measurement
      digitalWrite(_disp_cfg.n_oe_pin, HIGH);
      _spi->transfer(0);
      digitalWrite(_disp_cfg.n_oe_pin, LOW);
      // Send motor coil states and receive sensor measurement data over SPI
      for (int i = _num_units-1; i >= 0; i--) {
        recv_data[i] = _spi->transfer(send_data[i]);
      }
      // Home locating logic
      for (int i = _num_units-1; i >= 0; i--) {
        // Determine current state and detect any edge
        bool home_high = recv_data[i] > _unit_cfg[i].thresh; // TODO: add hysteresis?
        bool found_edge = _unit_state[i].prev_home==0 &&  home_high &&  _unit_cfg[i].home_rising
                       || _unit_state[i].prev_home==1 && !home_high && !_unit_cfg[i].home_rising;
        // Update home state
        bool in_range = _unit_cfg[i].home_start <= _unit_state[i].pos && _unit_state[i].pos <= _unit_cfg[i].home_end;
        if (found_edge && (!_unit_state[i].homed || in_range)) { // homed/rehomed
          _unit_state[i].pos = 0;
          _unit_state[i].homed = true;
        }
        _unit_state[i].homed &= _unit_state[i].pos < _unit_cfg[i].home_end; // lost home
        // Update previous home sensor value state variable
        _unit_state[i].prev_home = home_high;
      }
      // Update idle flag
      _idle = idle;
    }
    return !_idle;
  }

  void Flaptastic::set(int unit, int flap) {
    // Calculate
    int target = (flap * _unit_cfg[unit].msteps_flap) / 1000;
    target =  (target + _unit_cfg[unit].offset) % _unit_cfg[unit].steps;
    // Write
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { // critical section in case using interrupt to call step function
      _unit_state[unit].target = target;
      _idle = false;
    }
  }

  bool Flaptastic::done(int unit) {
    bool running;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { // critical section in case using interrupt to call step function
      running = _unit_state[unit].homed && _unit_state[unit].pos == _unit_state[unit].target;
    }
    return !running;
  }

  bool Flaptastic::allDone() {
    return _idle; // don't need to disable interrupts as byte access is atomic (I think)
  }

  void Flaptastic::reset() {
    for (int i = _num_units-1; i >= 0; i--) {
      _unit_state[i].homed = false; // don't need to disable interrupts as byte access is atomic (I think)
    }
  }

}
