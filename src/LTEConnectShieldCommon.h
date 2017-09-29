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

#ifndef _LTE_CONNECT_SHIELD_COMMON_
#define _LTE_CONNECT_SHIELD_COMMON_

//Default GPIO PIN Assignment
#define CON_SHIELD_ON_OFF_PIN 9
#define CON_SHIELD_PWR_IND_PIN 8
#define CON_SHIELD_RTS_PIN 3
#define CON_SHIELD_CTS_PIN 2

#define MAX_AT_COMMAND_BUFFER_SIZE              75   //75 bytes
#define MAX_AT_URC_BUFFER_SIZE                  75   //75 bytes 
#define DEFAULT_AT_COMMAND_DELAY                200   //200 ms
#define DEFAULT_AT_COMMAND_RESPONSE_TIMEOUT     4000  //4000 ms (4s)

#define LTE_CONECT_SHILED_DEFAULT_BAUDRATE 115200

#define MAX_URC_EVENT_LISTENERS 6

const char DEFAULT_AT_COMMAND_RESPONSE_OK[] = {"OK"};
const char LTE_MODULE_SYSLOADING_MESSAGE[] = {"^SYSLOADING"};
const char LTE_MODULE_SYSSTART_MESSAGE[] = {"^SYSSTART"};

/*
 * Enable the debug tracing for logging the debug messages via SoftwareSerial connection
 * Debug levels is bitmap with combinatation of each level which should be enabled
 */

#define LOG_LEVEL_CUST  1<<0
#define LOG_LEVEL_HIGH  1<<1
#define LOG_LEVEL_MED   1<<2
#define LOG_LEVEL_LOW   1<<3

#define LTE_CONNECT_SHIELD_DEBUG (LOG_LEVEL_CUST | LOG_LEVEL_HIGH )

const char LTE_CON_SHIELD_VERSION[] = {"00.001.01"};

#endif
