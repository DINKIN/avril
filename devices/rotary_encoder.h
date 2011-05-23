// Copyright 2010 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Driver for an external rotary encoder.

#ifndef AVRLIB_DEVICES_ROTARY_ENCODER_H_
#define AVRLIB_DEVICES_ROTARY_ENCODER_H_

#include "avrlib/devices/debounce.h"
#include "avrlib/gpio.h"
#include "avrlib/time.h"

namespace avrlib {

template<typename A, typename B, typename Click, uint16_t debounce_time = 1>
class RotaryEncoder {
 public:
  typedef DebouncedSwitch<A> SwitchA;
  typedef DebouncedSwitch<B> SwitchB;
  typedef DebouncedSwitch<Click> SwitchClick;
 
  RotaryEncoder() { }

  static void Init() {
    SwitchA::Init();
    SwitchB::Init();
    SwitchClick::Init();
    next_readout_ = 0;
  }

  static inline int8_t TimedRead() {
    uint32_t t = milliseconds();
    int8_t increment = 0;
    if (t >= next_readout_) {
      next_readout_ = t + debounce_time;
      increment = Read();
    }
    return increment;
  }

  static inline int8_t Read() {
    int8_t increment = 0;
    uint8_t a = SwitchA::Read();
    uint8_t b = SwitchB::Read();
    if (a == 0x80 && ((b & 0xf0) == 0x00)) {
        increment = 1;
    } else {
      if (b == 0x80 && (a & 0xf0) == 0x00) {
        increment = -1;
      }
    }
    SwitchClick::Read();
    return increment;
  }

  static uint8_t clicked() { return SwitchClick::raised(); }
  static uint8_t immediate_value() { return SwitchClick::immediate_value(); }

 private:
  static uint32_t next_readout_;

  DISALLOW_COPY_AND_ASSIGN(RotaryEncoder);
};

template<typename Encoder>
class RotaryEncoderTracker {
 public:
  RotaryEncoderTracker() { }
  
  static void Init() {
    Encoder::Init();
  }
  
  static inline void Read() {
    if (!increment_) {
      increment_ = Encoder::Read();
    }
    if (!clicked_) {
      clicked_ = Encoder::clicked();
    }
  }
  
  static inline uint8_t clicked() { return clicked_; }
  static inline uint8_t increment() { return increment_; }
  static inline uint8_t immediate_value() { return Encoder::immediate_value(); }
  static void Flush() {
    increment_ = 0;
    clicked_ = 0; 
  }
  
 private:
  static int8_t increment_;
  static uint8_t clicked_;

  DISALLOW_COPY_AND_ASSIGN(RotaryEncoderTracker);
};

/* static */
template<typename A, typename B, typename Click, uint16_t debounce_time>
uint32_t RotaryEncoder<A, B, Click, debounce_time>::next_readout_;

/* static */
template<typename Encoder> uint8_t RotaryEncoderTracker<Encoder>::clicked_;

/* static */
template<typename Encoder> int8_t RotaryEncoderTracker<Encoder>::increment_;

}  // namespace avrlib

#endif  // AVRLIB_DEVICES_SHIFT_REGISTER_H_
