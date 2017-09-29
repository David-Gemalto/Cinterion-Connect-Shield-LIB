/*
This file is a part of the LTE Cinterion Connect Shield library for Arduino.

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

#include <LTEConnectShieldLogger.h>

#ifdef LTE_CONNECT_SHIELD_DEBUG

LTEConnectShieldLogger* LTEConnectShieldLogger :: _thisLogger = NULL;

LTEConnectShieldLogger* LTEConnectShieldLogger :: getLogger()
{
  if(_thisLogger == NULL)
  	_thisLogger = new LTEConnectShieldLogger();
  return _thisLogger;
}

void LTEConnectShieldLogger :: init()
{
  //enable serial connection for tracing
  _softSerial = new SoftwareSerial(_LOGGER_RX_PIN, _LOGGER_TX_PIN);
}

void LTEConnectShieldLogger :: start()
{
  if(_softSerial != NULL)
  {
	_traceEnabled = true;
    _softSerial->begin(_LOGGER_BAUDRATE);
  }
}

void LTEConnectShieldLogger :: stop()
{
  if(_softSerial != NULL)
  {
    _traceEnabled = false;
    _softSerial->end();
  }
}

void LTEConnectShieldLogger :: printLog(const char *format, ...)
{
  if((_softSerial != NULL) && (_traceEnabled == true))
  {
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, LOGGER_MAX_MESSAGE_LENGTH, format, ap);
    va_end(ap);
	
    _softSerial->println(buf);
  }
}

void LTEConnectShieldLogger :: printLog(const __FlashStringHelper *format, ...)
{
  if((_softSerial != NULL) && (_traceEnabled == true))
  {
    strcpy_P(formatBuf, (PGM_P)format);

    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, LOGGER_MAX_MESSAGE_LENGTH, formatBuf, ap);
    va_end(ap);
  
    _softSerial->println(buf);
  }
}

#endif 

