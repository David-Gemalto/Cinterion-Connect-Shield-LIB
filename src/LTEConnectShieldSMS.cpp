/*
This file is a part of the LTE Cinterion Connect Shield library for Arduino.

This is a draft version, full version will be provided soon.

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

#include <LTEConnectShieldSMS.h>

LTEConnectShieldSMS :: LTEConnectShieldSMS(LTEConnectShieldModem * modem)
{
  _modem = modem;
  _modem->registerUrcEventListener(this);
}

void LTEConnectShieldSMS :: begin()
{
  _modem->registerUrcEventListener(this);
}

//to be removed
void LTEConnectShieldSMS :: test()
{
  char resBuf[MAX_AT_COMMAND_BUFFER_SIZE];
  
  int byteRead = 0;

  _modem->sendATcommand("AT+CMGL=4");

  byteRead = _modem->getATcommandReposne(resBuf, MAX_AT_COMMAND_BUFFER_SIZE);
}

void LTEConnectShieldSMS :: processUrcEvent(char * urcData, size_t dataLen)
{
  PRINT_LOG_MED("LTEConnectShieldSMS :: processUrcEvent");
}