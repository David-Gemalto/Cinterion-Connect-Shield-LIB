/*
This file is a part of the LTE Cinterion Connect Shield library for Arduino.

This is a copy of HardwareSerial implementation from Arduino library
with added modification that allows to use RTS/CTS signal to control the data flow.

There is added new overloaded "begin()" method which takes a GPIO numbers for RTC/CTS signals:
void LTEConnectShieldHwSerial::begin(unsigned long baud, uint8_t config, uint8_t rts, uint8_t cts)

This driver creates a global objects which reflects the serial peripherals 
regards to platform capabilities, in the same way as original version.

Copyright (c) 2017 Gemalto M2M GmbH, a Gemalto Company

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

The latest version of this library can always be found at:
https://github.com/David-Gemalto/Cinterion-Connect-Shield-LIB

*/
#ifndef __LTE_CONNECT_SHIELD_HW_SERIAL__
#define __LTE_CONNECT_SHIELD_HW_SERIAL__

#include <inttypes.h>
#include "Stream.h"
#include "wiring_private.h"

// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which head is the index of the location
// to which to write the next incoming character and tail is the index of the
// location from which to read.
// NOTE: a "power of 2" buffer size is reccomended to dramatically
//       optimize all the modulo operations for ring buffers.
// WARNING: When buffer sizes are increased to > 256, the buffer index
// variables are automatically increased in size, but the extra
// atomicity guards needed for that are not implemented. This will
// often work, but occasionally a race condition can occur that makes
// Serial behave erratically. See https://github.com/arduino/Arduino/issues/2405
#if !defined(SERIAL_TX_BUFFER_SIZE)
#if ((RAMEND - RAMSTART) < 1023)
#define SERIAL_TX_BUFFER_SIZE 16
#else
#define SERIAL_TX_BUFFER_SIZE 64
#endif
#endif
#if !defined(SERIAL_RX_BUFFER_SIZE)
#if ((RAMEND - RAMSTART) < 1023)
#define SERIAL_RX_BUFFER_SIZE 16
#else
#define SERIAL_RX_BUFFER_SIZE 64
#endif
#endif
#if (SERIAL_TX_BUFFER_SIZE>256)
typedef uint16_t tx_buffer_index_t;
#else
typedef uint8_t tx_buffer_index_t;
#endif
#if  (SERIAL_RX_BUFFER_SIZE>256)
typedef uint16_t rx_buffer_index_t;
#else
typedef uint8_t rx_buffer_index_t;
#endif

// Define config for Serial.begin(baud, config);
#define SERIAL_5N1 0x00
#define SERIAL_6N1 0x02
#define SERIAL_7N1 0x04
#define SERIAL_8N1 0x06
#define SERIAL_5N2 0x08
#define SERIAL_6N2 0x0A
#define SERIAL_7N2 0x0C
#define SERIAL_8N2 0x0E
#define SERIAL_5E1 0x20
#define SERIAL_6E1 0x22
#define SERIAL_7E1 0x24
#define SERIAL_8E1 0x26
#define SERIAL_5E2 0x28
#define SERIAL_6E2 0x2A
#define SERIAL_7E2 0x2C
#define SERIAL_8E2 0x2E
#define SERIAL_5O1 0x30
#define SERIAL_6O1 0x32
#define SERIAL_7O1 0x34
#define SERIAL_8O1 0x36
#define SERIAL_5O2 0x38
#define SERIAL_6O2 0x3A
#define SERIAL_7O2 0x3C
#define SERIAL_8O2 0x3E

#define SERIAL_PIN_UNKNOWN 0xFF


// Ensure that the various bit positions we use are available with a 0
// postfix, so we can always use the values for UART0 for all UARTs. The
// alternative, passing the various values for each UART to the
// LTEConnectShieldHwSerial constructor also works, but makes the code bigger and
// slower.
#if !defined(TXC0)
#if defined(TXC)
// Some chips like ATmega8 don't have UPE, only PE. The other bits are
// named as expected.
#if !defined(UPE) && defined(PE)
#define UPE PE
#endif
// On ATmega8, the uart and its bits are not numbered, so there is no TXC0 etc.
#define TXC0 TXC
#define RXEN0 RXEN
#define TXEN0 TXEN
#define RXCIE0 RXCIE
#define UDRIE0 UDRIE
#define U2X0 U2X
#define UPE0 UPE
#define UDRE0 UDRE
#elif defined(TXC1)
// Some devices have uart1 but no uart0
#define TXC0 TXC1
#define RXEN0 RXEN1
#define TXEN0 TXEN1
#define RXCIE0 RXCIE1
#define UDRIE0 UDRIE1
#define U2X0 U2X1
#define UPE0 UPE1
#define UDRE0 UDRE1
#else
#error No UART found in HardwareSerial.cpp
#endif
#endif // !defined TXC0

// Check at compiletime that it is really ok to use the bit positions of
// UART0 for the other UARTs as well, in case these values ever get
// changed for future hardware.
#if defined(TXC1) && (TXC1 != TXC0 || RXEN1 != RXEN0 || RXCIE1 != RXCIE0 || \
		      UDRIE1 != UDRIE0 || U2X1 != U2X0 || UPE1 != UPE0 || \
		      UDRE1 != UDRE0)
#error "Not all bit positions for UART1 are the same as for UART0"
#endif
#if defined(TXC2) && (TXC2 != TXC0 || RXEN2 != RXEN0 || RXCIE2 != RXCIE0 || \
		      UDRIE2 != UDRIE0 || U2X2 != U2X0 || UPE2 != UPE0 || \
		      UDRE2 != UDRE0)
#error "Not all bit positions for UART2 are the same as for UART0"
#endif
#if defined(TXC3) && (TXC3 != TXC0 || RXEN3 != RXEN0 || RXCIE3 != RXCIE0 || \
		      UDRIE3 != UDRIE0 || U3X3 != U3X0 || UPE3 != UPE0 || \
		      UDRE3 != UDRE0)
#error "Not all bit positions for UART3 are the same as for UART0"
#endif

class LTEConnectShieldHwSerial : public Stream
{
  protected:
    volatile uint8_t * const _ubrrh;
    volatile uint8_t * const _ubrrl;
    volatile uint8_t * const _ucsra;
    volatile uint8_t * const _ucsrb;
    volatile uint8_t * const _ucsrc;
    volatile uint8_t * const _udr;
    volatile uint8_t _rts = SERIAL_PIN_UNKNOWN;
    volatile uint8_t _cts = SERIAL_PIN_UNKNOWN;
    // Has any byte been written to the UART since begin()
    bool _written;

    volatile rx_buffer_index_t _rx_buffer_head;
    volatile rx_buffer_index_t _rx_buffer_tail;
    volatile tx_buffer_index_t _tx_buffer_head;
    volatile tx_buffer_index_t _tx_buffer_tail;

    // Don't put any members after these buffers, since only the first
    // 32 bytes of this struct can be accessed quickly using the ldd
    // instruction.
    unsigned char _rx_buffer[SERIAL_RX_BUFFER_SIZE];
    unsigned char _tx_buffer[SERIAL_TX_BUFFER_SIZE];

  public:
    LTEConnectShieldHwSerial(
      volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
      volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
      volatile uint8_t *ucsrc, volatile uint8_t *udr);
    void begin(unsigned long baud) { begin(baud, SERIAL_8N1, SERIAL_PIN_UNKNOWN, SERIAL_PIN_UNKNOWN); }
    void begin(unsigned long baud, uint8_t config){ begin(baud, config, SERIAL_PIN_UNKNOWN, SERIAL_PIN_UNKNOWN);}
    void begin(unsigned long baud, uint8_t config, uint8_t rts, uint8_t cts);
    void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    int availableForWrite(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool() { return true; }

    // Interrupt handlers - Not intended to be called externally
    void _rx_complete_irq(void);
    void _tx_udr_empty_irq(void);
};

#if defined(UBRRH) || defined(UBRR0H)
  extern LTEConnectShieldHwSerial HwSerial;
  #define HWSERIAL0_AVALIABLE
#endif
#if defined(UBRR1H)
  extern LTEConnectShieldHwSerial HwSerial1;
  #define HWSERIAL1_AVALIABLE
#endif
#if defined(UBRR2H)
  extern LTEConnectShieldHwSerial HwSerial2;
  #define HWSERIAL2_AVALIABLE
#endif
#if defined(UBRR3H)
  extern LTEConnectShieldHwSerial HwSerial3;
  #define HWSERIAL3_AVALIABLE
#endif

/* SerialEvent is not supported - will be added in future

extern void serialEventRun(void) __attribute__((weak));
*/

#endif //__LTE_CONNECT_SHIELD_HW_SERIAL__

