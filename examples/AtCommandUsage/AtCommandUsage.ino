/*
 AT Commands usage example for Cinterion Connect Shield. 

 Cinterion Connect Shield should be connected with Arduino without any modification.
 In that configuration, the PIN 0 and PIN 1 are used for serial communication with LTE device.
 PIN 9 is used to switch on and off the LTE device.

 The Cinterion Connect Shield as default uses the main serial connection which is also use for Arduino programing. 
 Therefore, if LTE device is enabled during Arduino programing (loading a new software) it may broke transmission and programing fails.
 Please turn off the LTE device in case of programing via ON/OFF button on shield - press for at least 3 second. 

 Additionally PIN5 and PIN6 are used for tracing logs via software serial connection. 
 This functionality can be enabled/disabled via: LTE_CONNECT_SHIELD_DEBUG in: 
 libraries\CinterionLTEConnectShield\src\LTEConnectShieldCommon.h
 There is also possible to configure which level of messages should be displayed.

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
#include <LTECinterionConnectShield.h>

//Replace "9999" with your pin number
#define PIN_NUMBER "9999"

LTEConnectShieldModem modem;

/* If you want to perform any action as result of URC 
 * you have to implement the interface class: LTEConnectShieldUrcListener
 * and register for listening of URC via:
 * registerUrcEventListener(),
 * see example below
 */
class CustomUrcListener : public LTEConnectShieldUrcListener
{
  void processUrcEvent(char* urcData, size_t dataLen)
  {
    //add your code here
    PRINT_LOG_CUST("URC revievied: %s", urcData);
  }
};

CustomUrcListener UrcListener;

void setup() 
{
  char buf[MAX_AT_COMMAND_BUFFER_SIZE];
  memset(buf,0,MAX_AT_COMMAND_BUFFER_SIZE);

  //start LTE device with default baud rate - 115200
  modem.begin(LTE_CONECT_SHILED_DEFAULT_BAUDRATE);

  //register listener for URC
  modem.registerUrcEventListener(&UrcListener);

  //right now LTE module is powered and initialized
  //check the module name
  modem.getName(buf, MAX_AT_COMMAND_BUFFER_SIZE);

  //print this name via debug serial interface
  PRINT_LOG_CUST("Module name: %s",buf);

  //check the module software version
  modem.getVersion(buf, MAX_AT_COMMAND_BUFFER_SIZE);
  
  //print this sw version via debug serial interface
  PRINT_LOG_CUST("Module SW version: %s",buf);

  //print library version
  PRINT_LOG_CUST("Cinterion Connect Shield Library Version: %s",modem.getLibVersion());

  //set pin if needed
  if(modem.getSimPinState() != SIM_PIN_READY)
  {
    if(!modem.setSimPin((char*)PIN_NUMBER))
      PRINT_LOG_CUST("PIN Validation Failed");
  }

  //enable URC with registering information
  modem.sendATcommandWithConf("AT+CREG=2");

  //register LTE device to network - auto select
  modem.registerToNetwork();

  //show the registration state
  PRINT_LOG_CUST("Module registration state: %d", modem.getRegistrationState());
}

void loop() 
{
  // put your main code here, to run repeatedly:

  /* Some of library functionality needs to listen and receive the URC messages.
   * Therefore if you want to have fully functional library please invoke this function periodically.
   * It check if any URC are available, read them and process. 
   */
  modem.processUrc();

  //wait 2s.
  delay(2000);
}


