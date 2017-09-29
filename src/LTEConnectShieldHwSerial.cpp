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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"

#include "LTEConnectShieldHwSerial.h"

// this next line disables the entire LTEConnectShieldHwSerial.cpp, 
// this is so I can support Attiny series and any other chip without a uart
#if defined(HWSERIAL0_AVALIABLE) || defined(HWSERIAL1_AVALIABLE) || defined(HWSERIAL2_AVALIABLE) || defined(HWSERIAL3_AVALIABLE)

/*
 * SerialEvent is not supported - will be added in future

// SerialEvent functions are weak, so when the user doesn't define them,
// the linker just sets their address to 0 (which is checked below).
// The Serialx_available is just a wrapper around Serialx.available(),
// but we can refer to it weakly so we don't pull in the entire
// HardwareSerial instance if the user doesn't also refer to it.

#if defined(HAVE_HWSERIAL0)
  void serialEvent() __attribute__((weak));
  bool Serial0_available() __attribute__((weak));
#endif

#if defined(HAVE_HWSERIAL1)
  void serialEvent1() __attribute__((weak));
  bool Serial1_available() __attribute__((weak));
#endif

#if defined(HAVE_HWSERIAL2)
  void serialEvent2() __attribute__((weak));
  bool Serial2_available() __attribute__((weak));
#endif

#if defined(HAVE_HWSERIAL3)
  void serialEvent3() __attribute__((weak));
  bool Serial3_available() __attribute__((weak));
#endif

void serialEventRun(void)
{
#if defined(HAVE_HWSERIAL0)
  if (Serial0_available && serialEvent && Serial0_available()) serialEvent();
#endif
#if defined(HAVE_HWSERIAL1)
  if (Serial1_available && serialEvent1 && Serial1_available()) serialEvent1();
#endif
#if defined(HAVE_HWSERIAL2)
  if (Serial2_available && serialEvent2 && Serial2_available()) serialEvent2();
#endif
#if defined(HAVE_HWSERIAL3)
  if (Serial3_available && serialEvent3 && Serial3_available()) serialEvent3();
#endif
}
*/


static inline void SET_RTS_ON(uint8_t rts)
{ 
  if(rts != SERIAL_PIN_UNKNOWN)
    digitalWrite(rts, 0);
}

static inline void SET_RTS_OFF(uint8_t rts) 
{
  if(rts != SERIAL_PIN_UNKNOWN)
    digitalWrite(rts, 1);
}

static inline bool IS_CTS_SET(uint8_t cts)  
{ 
  if(cts != SERIAL_PIN_UNKNOWN)
  	return !digitalRead(cts);
  else
  	return 1;
}

// Constructors ////////////////////////////////////////////////////////////////

LTEConnectShieldHwSerial::LTEConnectShieldHwSerial(
  volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
  volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
  volatile uint8_t *ucsrc, volatile uint8_t *udr) :
    _ubrrh(ubrrh), _ubrrl(ubrrl),
    _ucsra(ucsra), _ucsrb(ucsrb), _ucsrc(ucsrc),
    _udr(udr),
    _rx_buffer_head(0), _rx_buffer_tail(0),
    _tx_buffer_head(0), _tx_buffer_tail(0)
{
}

// Actual interrupt handlers //////////////////////////////////////////////////////////////

void LTEConnectShieldHwSerial::_rx_complete_irq(void)
{
  if (bit_is_clear(*_ucsra, UPE0)) {
    // No Parity error, read byte and store it in the buffer if there is
    // room
    unsigned char c = *_udr;
    rx_buffer_index_t i = (unsigned int)(_rx_buffer_head + 1) % SERIAL_RX_BUFFER_SIZE;

    // if we should be storing the received character into the location
    // just before the tail (meaning that the head would advance to the
    // current location of the tail), we're about to overflow the buffer
    // and so we don't write the character or advance the head.
    if (i != _rx_buffer_tail) {
      _rx_buffer[_rx_buffer_head] = c;
      _rx_buffer_head = i;
	  
	  //check if we have spece in buffer for more bytes 
	  if(((_rx_buffer_head + 2) % SERIAL_RX_BUFFER_SIZE ) == _rx_buffer_tail)
	  {
	    SET_RTS_OFF(_rts);
	  }
    }
  } else {
    // Parity error, read byte but discard it
    *_udr;
  };
}

void LTEConnectShieldHwSerial::_tx_udr_empty_irq(void)
{
  // If interrupts are enabled, there must be more data in the output
  // buffer. Send the next byte
  if(IS_CTS_SET(_cts))
  {
  unsigned char c = _tx_buffer[_tx_buffer_tail];
  _tx_buffer_tail = (_tx_buffer_tail + 1) % SERIAL_TX_BUFFER_SIZE;

  *_udr = c;

  // clear the TXC bit -- "can be cleared by writing a one to its bit
  // location". This makes sure flush() won't return until the bytes
  // actually got written
  sbi(*_ucsra, TXC0);

  if (_tx_buffer_head == _tx_buffer_tail) {
    // Buffer empty, so disable interrupts
    cbi(*_ucsrb, UDRIE0);
  }
  }
}

// Public Methods //////////////////////////////////////////////////////////////

void LTEConnectShieldHwSerial::begin(unsigned long baud, uint8_t config, uint8_t rts, uint8_t cts)
{
  _rts = rts;
  _cts = cts;
  // Try u2x mode first
  uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;
  *_ucsra = 1 << U2X0;

  // hardcoded exception for 57600 for compatibility with the bootloader
  // shipped with the Duemilanove and previous boards and the firmware
  // on the 8U2 on the Uno and Mega 2560. Also, The baud_setting cannot
  // be > 4095, so switch back to non-u2x mode if the baud rate is too
  // low.
  if (((F_CPU == 16000000UL) && (baud == 57600)) || (baud_setting >4095))
  {
    *_ucsra = 0;
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }

  // assign the baud_setting, a.k.a. ubrr (USART Baud Rate Register)
  *_ubrrh = baud_setting >> 8;
  *_ubrrl = baud_setting;

  _written = false;

  //set the data bits, parity, and stop bits
#if defined(__AVR_ATmega8__)
  config |= 0x80; // select UCSRC register (shared with UBRRH)
#endif
  *_ucsrc = config;
  
  sbi(*_ucsrb, RXEN0);
  sbi(*_ucsrb, TXEN0);
  sbi(*_ucsrb, RXCIE0);
  cbi(*_ucsrb, UDRIE0);

  if(_rts != SERIAL_PIN_UNKNOWN)
    pinMode(_rts, OUTPUT);
  
  if(_cts != SERIAL_PIN_UNKNOWN)
    pinMode(_cts, INPUT_PULLUP);

  SET_RTS_ON(_rts);
  
}

void LTEConnectShieldHwSerial::end()
{
  // wait for transmission of outgoing data
  flush();

  SET_RTS_OFF(_rts);

  cbi(*_ucsrb, RXEN0);
  cbi(*_ucsrb, TXEN0);
  cbi(*_ucsrb, RXCIE0);
  cbi(*_ucsrb, UDRIE0);
  
  // clear any received data
  _rx_buffer_head = _rx_buffer_tail;
}

int LTEConnectShieldHwSerial::available(void)
{
  return ((unsigned int)(SERIAL_RX_BUFFER_SIZE + _rx_buffer_head - _rx_buffer_tail)) % SERIAL_RX_BUFFER_SIZE;
}

int LTEConnectShieldHwSerial::peek(void)
{
  if (_rx_buffer_head == _rx_buffer_tail) {
    return -1;
  } else {
    return _rx_buffer[_rx_buffer_tail];
  }
}

int LTEConnectShieldHwSerial::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if (_rx_buffer_head == _rx_buffer_tail) {
    return -1;
  } else {
    unsigned char c = _rx_buffer[_rx_buffer_tail];
    _rx_buffer_tail = (rx_buffer_index_t)(_rx_buffer_tail + 1) % SERIAL_RX_BUFFER_SIZE;

    //resume receiving when 50% of buffer will be free
    if(abs(_rx_buffer_head - _rx_buffer_tail )>= (SERIAL_RX_BUFFER_SIZE/2))
    {
      SET_RTS_ON(_rts);
    }
	
    return c;
  }
}

int LTEConnectShieldHwSerial::availableForWrite(void)
{
#if (SERIAL_TX_BUFFER_SIZE>256)
  uint8_t oldSREG = SREG;
  cli();
#endif
  tx_buffer_index_t head = _tx_buffer_head;
  tx_buffer_index_t tail = _tx_buffer_tail;
#if (SERIAL_TX_BUFFER_SIZE>256)
  SREG = oldSREG;
#endif
  if (head >= tail) return SERIAL_TX_BUFFER_SIZE - 1 - head + tail;
  return tail - head - 1;
}

void LTEConnectShieldHwSerial::flush()
{
  // If we have never written a byte, no need to flush. This special
  // case is needed since there is no way to force the TXC (transmit
  // complete) bit to 1 during initialization
  if (!_written)
    return;

  while (bit_is_set(*_ucsrb, UDRIE0) || bit_is_clear(*_ucsra, TXC0)) {
    if (bit_is_clear(SREG, SREG_I) && bit_is_set(*_ucsrb, UDRIE0))
	// Interrupts are globally disabled, but the DR empty
	// interrupt should be enabled, so poll the DR empty flag to
	// prevent deadlock
	if (bit_is_set(*_ucsra, UDRE0))
	  _tx_udr_empty_irq();
  }
  // If we get here, nothing is queued anymore (DRIE is disabled) and
  // the hardware finished tranmission (TXC is set).
}

size_t LTEConnectShieldHwSerial::write(uint8_t c)
{
  _written = true;
  // If the buffer and the data register is empty, just write the byte
  // to the data register and be done. This shortcut helps
  // significantly improve the effective datarate at high (>
  // 500kbit/s) bitrates, where interrupt overhead becomes a slowdown.

  if(IS_CTS_SET(_cts))
  {
    if (_tx_buffer_head == _tx_buffer_tail && bit_is_set(*_ucsra, UDRE0))
	{
      *_udr = c;
      sbi(*_ucsra, TXC0);
      return 1;
    }
  }
  tx_buffer_index_t i = (_tx_buffer_head + 1) % SERIAL_TX_BUFFER_SIZE;
	
  // If the output buffer is full, there's nothing for it other than to 
  // wait for the interrupt handler to empty it a bit
  while (i == _tx_buffer_tail) {
    if (bit_is_clear(SREG, SREG_I)) {
      // Interrupts are disabled, so we'll have to poll the data
      // register empty flag ourselves. If it is set, pretend an
      // interrupt has happened and call the handler to free up
      // space for us.
      if(bit_is_set(*_ucsra, UDRE0))
	_tx_udr_empty_irq();
    } else {
      // nop, the interrupt handler will free up space for us
    }
  }

  _tx_buffer[_tx_buffer_head] = c;
  _tx_buffer_head = i;
	
  sbi(*_ucsrb, UDRIE0);
  
  return 1;
}

#endif // whole file

