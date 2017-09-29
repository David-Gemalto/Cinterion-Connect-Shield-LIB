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

#ifndef __LTE_CONNECT_SHIELD_SMS__
#define __LTE_CONNECT_SHIELD_SMS__

#include <Arduino.h>
#include <LTEConnectShieldCommon.h>
#include <LTEConnectShieldModem.h>
#include <LTEConnectShieldUrcListener.h>


class LTEConnectShieldSMS : public LTEConnectShieldUrcListener
{
  private:
    LTEConnectShieldModem * _modem;
        
  public:  
    LTEConnectShieldSMS(LTEConnectShieldModem * modem);

    void begin();  
    void processUrcEvent(char * urcData, size_t dataLen); 

    void test();
};

#endif 
