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

#ifndef __LTE_CONNECT_SHIELD_LOGGER__
#define __LTE_CONNECT_SHIELD_LOGGER__

#include <Stream.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <LTEConnectShieldCommon.h>

#ifdef LTE_CONNECT_SHIELD_DEBUG

#define LOGGER_MAX_MESSAGE_LENGTH 128

#ifdef LOGGER_RX_PIN
#define _LOGGER_RX_PIN LOGGER_RX_PIN
#else
#define _LOGGER_RX_PIN 5
#endif

#ifdef LOGGER_TX_PIN
#define _LOGGER_TX_PIN LOGGER_TX_PIN
#else
#define _LOGGER_TX_PIN 6
#endif

#ifdef LOGGER_BAUDRATE
#define _LOGGER_BAUDRATE LOGGER_BAUDRATE
#else
#define _LOGGER_BAUDRATE 115200
#endif

class LTEConnectShieldLogger
{
  private:
    bool _traceEnabled;
    SoftwareSerial* _softSerial = NULL;
    
    static LTEConnectShieldLogger* _thisLogger;
    char buf[LOGGER_MAX_MESSAGE_LENGTH];
    char formatBuf[LOGGER_MAX_MESSAGE_LENGTH];

    LTEConnectShieldLogger(){};
        
  public: 
    static LTEConnectShieldLogger* getLogger();
    
    void init();
    void start();
    void stop();

    void printLog(const char *format, ...);
    void printLog(const __FlashStringHelper *format, ...);
};

#define LOGGER LTEConnectShieldLogger::getLogger()

#define LOGGER_INIT() LOGGER->init()
#define LOGGER_START() LOGGER->start()
#define LOGGER_STOP() LOGGER->stop()
#else
#define LOGGER_INIT()
#define LOGGER_START()
#define LOGGER_STOP()
#endif


#if (LTE_CONNECT_SHIELD_DEBUG & LOG_LEVEL_CUST)
#define PRINT_LOG_CUST(str, ...) LOGGER->printLog(F(str), ##__VA_ARGS__)
#else
#define PRINT_LOG_CUST(str, ...)
#endif

#if (LTE_CONNECT_SHIELD_DEBUG & LOG_LEVEL_HIGH )
#define PRINT_LOG_HIGH(str, ...) LOGGER->printLog(F(str), ##__VA_ARGS__)
#else
#define PRINT_LOG_HIGH(str, ...)
#endif

#if (LTE_CONNECT_SHIELD_DEBUG & LOG_LEVEL_MED)
#define PRINT_LOG_MED(str, ...) LOGGER->printLog(F(str), ##__VA_ARGS__)
#else
#define PRINT_LOG_MED(str, ...)
#endif

#if (LTE_CONNECT_SHIELD_DEBUG & LOG_LEVEL_LOW)
#define PRINT_LOG_LOW(str, ...) LOGGER->printLog(F(str), ##__VA_ARGS__)
#else
#define PRINT_LOG_LOW(str, ...)
#endif

#endif
