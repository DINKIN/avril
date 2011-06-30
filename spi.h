// Copyright 2009 Olivier Gillet.
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
// Fast SPI communication (using the hardware implementation). This will take
// ownership of the pins 11 (data output),  12 (data input) and 13 (clock), +
// a user-definable pin for slave selection. Pin 10 should be kept as an output
// pin, since the SPI master/slave mode is based upon the value of this pin.
//
// This is a fairly basic implementation:
// - nothing is buffered, since the overhead of managing a circular buffer is
//   around 15 cycles (not including the interrupt prelude/postlude), which is
//   close to the transmission time at the fastest speed.
// - the atmega is always configured as a master.

#ifndef AVRLIB_SPI_H_
#define AVRLIB_SPI_H_

#include "avrlib/avrlib.h"
#include "avrlib/gpio.h"

namespace avrlib {

IORegister(SPSR);
typedef BitInRegister<SPSRRegister, SPI2X> DoubleSpeed;
typedef BitInRegister<SPSRRegister, SPIF> TransferComplete;

template<typename SlaveSelect,
         DataOrder order = MSB_FIRST,
         uint8_t speed = 4>
class SpiMaster {
 public:
  enum {
    buffer_size = 0,
    data_size = 8
  };

  static void Init() {
    SpiSCK::set_mode(DIGITAL_OUTPUT);
    SpiMOSI::set_mode(DIGITAL_OUTPUT);
    SpiMISO::set_mode(DIGITAL_INPUT);
    SpiSS::set_mode(DIGITAL_OUTPUT);  // I'm a master!
    SpiSS::High();
    SlaveSelect::set_mode(DIGITAL_OUTPUT);
    SlaveSelect::High();

    // SPI enabled, configured as master.
    uint8_t configuration = _BV(SPE) | _BV(MSTR);
    if (order == LSB_FIRST) {
      configuration |= _BV(DORD);
    }
    switch (speed) {
      case 2:
        DoubleSpeed::set();
      case 4:
        break;
      case 8:
        DoubleSpeed::set();
      case 16:
        configuration |= _BV(SPR0);
        break;
      case 32:
        DoubleSpeed::set();
      case 64:
        configuration |= _BV(SPR1);
        break;
      case 128:
        configuration |= _BV(SPR0);
        configuration |= _BV(SPR1);
        break;
    }
    SPCR = configuration;
  }
  
  static inline void Begin() {
    SlaveSelect::Low();
  }

  static inline void End() {
    SlaveSelect::High();
  }
  
  static inline void Strobe() {
    SlaveSelect::High();
    SlaveSelect::Low();
  }

  static inline void Write(uint8_t v) {
    Begin();
    Send(v);
    End();
  }
  
  static inline void Send(uint8_t v) {
    Overwrite(v);
    Wait();
  }
  
  static inline uint8_t Receive() {
    Send(0xff);
    return ImmediateRead();
  }
  
  static inline uint8_t ImmediateRead() {
    return SPDR;
  }
  
  static inline void Wait() {
    while (!TransferComplete::value());
  }
  
  static inline void OptimisticWait() {
    Wait();
  }
  
  static inline void Overwrite(uint8_t v) {
    SPDR = v;
  }

  static inline void WriteWord(uint8_t a, uint8_t b) {
    Begin();
    Send(a);
    Send(b);
    End();
  }
};

template<typename SlaveSelect,
         DataOrder order = MSB_FIRST,
         bool enable_interrupt = false>
class SpiSlave {
 public:
  enum {
    buffer_size = 128,
    data_size = 8
  };

  static void Init() {
    SpiSCK::set_mode(DIGITAL_INPUT);
    SpiMOSI::set_mode(DIGITAL_INPUT);
    SpiMISO::set_mode(DIGITAL_OUTPUT);
    SpiSS::set_mode(DIGITAL_INPUT);  // Ohhh mistress, ohhhh!
    SlaveSelect::set_mode(DIGITAL_INPUT);

    // SPI enabled, configured as master.
    uint8_t configuration = _BV(SPE);
    if (order == LSB_FIRST) {
      configuration |= _BV(DORD);
    }
    if (enable_interrupt) {
      configuration |= _BV(SPIE);
    }
    SPCR = configuration;
  }

  static inline uint8_t Read(uint8_t v) {
    while (!TransferComplete::value());
    return SPDR;
  }
};


template<typename SlaveSelect,
         DataOrder order = MSB_FIRST,
         uint8_t speed = 2>
class UartSpiMaster {
 public:
  enum {
    buffer_size = 0,
    data_size = 8
  };

  static void Init() {
    SlaveSelect::set_mode(DIGITAL_OUTPUT);
    SlaveSelect::High();

    UBRR0 = 0;

    UartSpiXCK::set_mode(DIGITAL_OUTPUT);
    UartSpiTX::set_mode(DIGITAL_OUTPUT);
    UartSpiRX::set_mode(DIGITAL_INPUT);
    
    // Set UART to SPI Master mode.
    UCSR0C = _BV(UMSEL01) | _BV(UMSEL00);
    
    // Enable TX and RX
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    
    UBRR0 = (speed / 2) - 1;
  }

  static inline void Begin() {
    SlaveSelect::Low();
  }

  static inline void End() {
    SlaveSelect::High();
  }
  
  static inline void Strobe() {
    SlaveSelect::High();
    SlaveSelect::Low();
  }

  static inline void Write(uint8_t v) {
    Begin();
    Send(v);
    End();
  }
  
  static inline void Send(uint8_t v) {
    Overwrite(v);
    Wait();
  }

  static inline void Wait() {
    while (!UCSR0A & _BV(UDRE0));
  }
  
  static inline void OptimisticWait() { }
  
  static inline void Overwrite(uint8_t v) {
    UDR0 = v;
  }
  
  static inline void WriteWord(uint8_t a, uint8_t b) {
    Begin();
    Send(a);
    Send(b);
    End();
  }
};


#define SPI_RECEIVE ISR(SPI_STC_vect)

}  // namespace avrlib

#endif AVRLIB_SPI_H_
