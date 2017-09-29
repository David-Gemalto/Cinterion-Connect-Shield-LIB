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

#ifndef __LTE_CONNECT_SHIELD_MODEM__
#define __LTE_CONNECT_SHIELD_MODEM__

#include <Stream.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <LTEConnectShieldCommon.h>
#include <LTEConnectShieldInterface.h>
#include <LTEConnectShieldUrcListener.h>
#include <LTEConnectShieldLogger.h>
#include <LTEConnectShieldHwSerial.h>


enum eSimPinState
{
  SIM_PIN_READY   = 0,
  SIM_PIN_NEED    = 1,
  SIM_PIN2_NEED   = 2,
  SIM_PUK_NEED    = 3,
  SIM_PUK2_NEED   = 4,
  SIM_PIN_ERROR   = 5
};

enum eRegState
{
  NOT_REGISTERED  = 0,  //Not registered, ME is currently not searching
  REGISTERED      = 1,  //Registered to home network
  SEARCHING       = 2,  //Not registered, but ME is currently searching
  DENIED          = 3,  //Registration denied
  UNKNOW          = 4,  //Unknown (not used)
  ROAMING         = 5,  //Registered, roaming
  ERROR           = 6
};


class LTEConnectShieldModem : private LTEConnectShieldUrcListener
{			
  private:
    Stream* _streamCon = NULL;
    LTEConnectShieldHwSerial* _hSerialCon = NULL;
    SoftwareSerial* _sSerialCon = NULL;
    LTEConnectShieldInterface* _serialInterface = NULL;
    uint8_t _onOffPin;
    uint8_t _rts = SERIAL_PIN_UNKNOWN;
    uint8_t _cts = SERIAL_PIN_UNKNOWN;
    bool _isAtRunning = false; 
    LTEConnectShieldUrcListener* _urcEventListeners[MAX_URC_EVENT_LISTENERS];
    bool _isUrcProcEnabled = false;

    //buffer for AT command communication
    char cmdBuf[MAX_AT_COMMAND_BUFFER_SIZE];
    char urcBuf[MAX_AT_URC_BUFFER_SIZE ];

    
    /** Implementation of virtual method from LTEConnectShieldUrcListener.
        This function will be run when URC message has been received from LTE device.
        It will work only when object of LTEConnectShieldUrcListener class will be registered 
        in LTEConnectShieldModem via registerUrcEventListener().

        @param urcData  pointer to URC string
        @param dataLen  length of URC 
     */
    virtual void processUrcEvent(char* urcData, size_t dataLen);

    /** Read all available data from LTE device via serial connection and discard them.
     */
    void readUrcDiscard();

    /** Turn on the LTE device by pressing a proper button 
     */
    void turnON();

    /** Turn off the LTE device. 
     */
    void turnOFF();
    
  public:
    /**  Constructors  */
   
    /** Default constructor for "LTEConnectShieldModem" class. 
        It will set the "HwSerial" object to communicate with LTE device with enabled flow control, GPIO pins usage:
        pin 0 - TX
        pin 1 - RX
        pin 2 - CTS
        pin 3 - RTS
        from Arduino shield.
        It also uses the pin number: 9 form Arduino connector to turn on/off the LTE device.
		 */
    LTEConnectShieldModem();

    /** Overloaded constructor for "LTEConnectShieldModem" class. 
        It uses the pointer to LTEConnectShieldHwSerial object to communicate with LTE device.
        It also uses the onOffPin to turn on/off the LTE device.

        @param serialCon  LTEConnectShieldHwSerial object which will be used to communicate with LTE device.
        @param onOffPin   Pin number which will be used to turn on/off the LTE device
        @param rts        Pin number which will be used as RTS line in serial connection
        @param cts        Pin number which will be used as CTS line in serial connection 
		 */
    LTEConnectShieldModem(LTEConnectShieldHwSerial& serialCon, 
                         uint8_t onOffPin, 
                         uint8_t rts = SERIAL_PIN_UNKNOWN, 
                         uint8_t cts = SERIAL_PIN_UNKNOWN);

    /** Overloaded constructor for "LTEConnectShieldModem" class. 
        It uses the pointer to SoftwareSerial object to communicate with LTE device.
        It also uses the onOffPin to turn on/off the LTE device.

        @param serialCon  SoftwareSerial object which will be used to communicate with LTE device
        @param onOffPin   Pin number which will be used to turn on/off the LTE device 
		 */
    LTEConnectShieldModem(SoftwareSerial& serialCon, uint8_t onOffPin);

    /** Overloaded constructor for "LTEConnectShieldModem" class. 
        It uses the pointer to "LTEConnectShieldInterface" object to communicate with LTE device.
        It also uses the onOffPin to turn on/off the LTE device.

        @param serialInterface LTEConnectShieldInterface object which will be used communicate with LTE device
        @param onOffPin     Pin number which will be used to turn on/off the LTE device
		 */
    LTEConnectShieldModem(LTEConnectShieldInterface& serialInterface, uint8_t onOffPin);

    /** Initializing the LTE device for working. It sets the given baud rate for serial communication with device.
        It must be called in order to use the LTE device. 

        @param baudrate   Baud rate used with LTE device communication
        @return           true if LTE device has been initialized properly 
		 */
	  bool begin(long baudrate);

    /** Send AT command to LTE device

        @param format  Variable arguments containing AT command string.
                       It must be a null terminated string.
                       Max length is defined in MAX_AT_COMMAND_BUFFER_SIZE - 1
                       (for CR character at the end of string).
        @return        true if AT command has been sent.
		 */
    bool sendATcommand(const char *format, ...);

    /** Send AT command to LTE device and wait.

        @param delayTime  time in ms for wait after AT command has been sent
        @param format     Variable arguments containing AT command string.
                          It must be a null terminated string.
                          Max length is defined in MAX_AT_COMMAND_BUFFER_SIZE - 1.
                          (for CR character at the end of string).
        @return           true if AT command has been sent.
		 */
    bool sendATcommand(int delayTime, const char *format, ...);

    /** Send AT command to LTE device and wait.

        @param delayTime  time in ms for wait after AT command has been sent
        @param cmd        Pointer to char string with AT command.
                          It must be a null terminated string.
                          Max length is defined in MAX_AT_COMMAND_BUFFER_SIZE -1 
                          (for CR character at the end of string).
        @return           true if AT command has been sent.
		 */
    bool sendATcommand(int delayTime, char *cmd);

    /** Send AT command to LTE device and receive response. 
        It returns true in case of "OK" in AT command response.

        @param format     Variable arguments containing AT command string.
                          It must be a null terminated string.
                          Max length is defined in MAX_AT_COMMAND_BUFFER_SIZE - 1.
                          (for CR character at the end of string).
        @return           true if AT command response contain "OK"
		 */
    bool sendATcommandWithConf(const char *format, ...);

    /** Send AT command to LTE device, wait for some time and receive response. 
        It returns true in case when response contain an "OK" in response. 

        @param delayTime  Time in ms for wait after AT command has been sent
        @param format     Variable arguments containing AT command string.
                          It must be a null terminated string.
                          Max length is defined in MAX_AT_COMMAND_BUFFER_SIZE - 1.
                          (for CR character at the end of string).
        @return           true if AT command response contain "OK"
		 */
    bool sendATcommandWithConf(int delayTime, const char *format, ...);

    /** Send AT command to LTE device and receive response. 
        It returns true in case when response contain a expResp. 

        @param expRes     Pointer to char string with expected response.
                          It must be a null terminated string.
                          Max length is defined in MAX_AT_COMMAND_BUFFER_SIZE - 1.
                          (for CR character at the end of string).
        @param format     Variable arguments containing AT command string.
                          It must be a null terminated string.
                          Max length is defined in MAX_AT_COMMAND_BUFFER_SIZE - 1.
                          (for CR character at the end of string).
        @return           true if AT command response contain a expResp
		 */
    bool sendATcommandVerifyRes(const char* expResp, const char *format, ...);

    /** Get AT command response from LTE device.
        It read bytes only when previously AT command has been sent via sendATcommand().

        @param resp  pointer to data preapred for response
        @param len   length of data for response
        @return      number of bytes writen in resp
     */
    size_t getATcommandReposne(char* resp, size_t len);

    /** Get AT command response from LTE device and check if it contain an expected response.
        It read bytes only when previously AT command has been sent via sendATcommand().

        @param format     Variable arguments containing a string with expected command response
                          It must be a null terminated string.
                          Max length is defined in MAX_AT_COMMAND_RESPONSE_BUFFER_SIZE - 1
                          (for CR character at the end of string).
        @return           true if AT command response contain an expected response.
     */
    bool getATcommandReposneVerify(const char *format, ...);

    /** Send data to LTE device

        @param data    pointer to data
        @param len     length of data
        @return        the number of written bytes
     */
    size_t sendData(const char* data, size_t len);

    /** Check if there are data to be read. 

        @return  number of bytes to be read
     */
    int dataAvailable();

    /** Read data from LTE device

        @param data    pointer to data
        @param len     length of allocated space for data to be read
        @return        number of read data
        
     */
    size_t getData(char* data, size_t len);

    /** Read one byte from LTE device

        @return        read data
        
     */
    int read();

    /** Flush data from LTE device serial connection
      */
    void flush();

    /** This function must be periodically invokes in order to check if there is a URC to be read and processed. 
        The "loop()" function is the best place to call this function. 
      */
    void processUrc();

    /** Register the URC listener.

        @param urcListener  pointer to object of LTEConnectShieldUrcListener class
     */
    void registerUrcEventListener(LTEConnectShieldUrcListener* urcListener);

    /** Unregister the URC listener.

        @param urcListener  pointer to object of LTEConnectShieldUrcListener class
     */
    void unRegisterUrcEventListener(LTEConnectShieldUrcListener* urcListener);

    /** Gets module product and manufacturer name

        @param name    pointer to data prepared for name
        @param len     length of allocated data
        @return        number of read data
     */
    size_t getName(char* name, size_t len);

    /** Gets module version

        @param version  pointer to data prepared for version string
        @param len      length of allocated data
        @return         number of read data
     */
    size_t getVersion(char *version, size_t len);

    /** Gets library version

        @return  pointer to null terminated string with library version
     */
    const char* getLibVersion();

    /** Get the registration state of LTE device

        @return  eRegState which reflects to LTE device registration state
      */
    eRegState getRegistrationState();

    /** Register LTE device to network.
        It register automatically to home operator without in case of no specifying the MCC and MNC number.
        For manual registration, please provide string containing MCC and MNC number, e.g. 
        26002

        @param mccMnc  string with MCC and MNC number for manual registration 
                       Mak length is 5 or 6(in case 3 digit MNC) chars
        @return        true if registration ends successfully
        
    */
    bool registerToNetwork(const char * mccMnc = NULL);

     /** Get Sim Pin state.

        @return       eSimPinState which reflect the status of SIM PIN.

    */
    eSimPinState getSimPinState();

    /** Set SIM PIN number

        @param pin   pointer to char string with SIM PIN. Only 4 chars length is allowed.
        @return      true if entering SIM PIN ends with success
      */
    bool setSimPin(char* pin);
};

#endif
