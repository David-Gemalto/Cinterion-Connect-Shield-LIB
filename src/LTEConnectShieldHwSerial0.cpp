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

#include "Arduino.h"
#include "LTEConnectShieldHwSerial.h"

// Each LTEConnectShieldHwSerial is defined in its own file, sine the linker pulls
// in the entire file when any element inside is used. --gc-sections can
// additionally cause unused symbols to be dropped, but ISRs have the
// "used" attribute so are never dropped and they keep the
// LTEConnectShieldHwSerial instance in as well. Putting each instance in its own
// file prevents the linker from pulling in any unused instances in the
// first place.

#if defined(HWSERIAL0_AVALIABLE)

#if defined(USART_RX_vect)
  ISR(USART_RX_vect)
#elif defined(USART0_RX_vect)
  ISR(USART0_RX_vect)
#elif defined(USART_RXC_vect)
  ISR(USART_RXC_vect) // ATmega8
#else
  #error "Don't know what the Data Received vector is called for Serial"
#endif
  {
    HwSerial._rx_complete_irq();
  }

#if defined(UART0_UDRE_vect)
ISR(UART0_UDRE_vect)
#elif defined(UART_UDRE_vect)
ISR(UART_UDRE_vect)
#elif defined(USART0_UDRE_vect)
ISR(USART0_UDRE_vect)
#elif defined(USART_UDRE_vect)
ISR(USART_UDRE_vect)
#else
  #error "Don't know what the Data Register Empty vector is called for Serial"
#endif
{
  HwSerial._tx_udr_empty_irq();
}

#if defined(UBRRH) && defined(UBRRL)
  LTEConnectShieldHwSerial HwSerial(&UBRRH, &UBRRL, &UCSRA, &UCSRB, &UCSRC, &UDR);
#else
  LTEConnectShieldHwSerial HwSerial(&UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0);
#endif

/*
// Function that can be weakly referenced by serialEventRun to prevent
// pulling in this file if it's not otherwise used.
bool Serial0_available() {
  return Serial.available();
}
*/

#endif // HAVE_HWSERIAL0

