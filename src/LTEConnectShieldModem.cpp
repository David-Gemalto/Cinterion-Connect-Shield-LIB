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

#include <LTEConnectShieldModem.h>
#include <Arduino.h>

#define DIGITAL_WRITE_DELAY(pin, val, wait) {\
	digitalWrite(pin, val);\
	delay(wait);\
}

LTEConnectShieldModem :: LTEConnectShieldModem()
{
  _hSerialCon = &HwSerial;
  _onOffPin = CON_SHIELD_ON_OFF_PIN;
  _rts = CON_SHIELD_RTS_PIN;
  _cts = CON_SHIELD_CTS_PIN;
}

LTEConnectShieldModem :: LTEConnectShieldModem(LTEConnectShieldHwSerial& serialCon, uint8_t onOffPin, uint8_t rts, uint8_t cts)
{
  _hSerialCon = &serialCon;
  _onOffPin = onOffPin;
  _rts = rts;
  _cts = cts;
}

LTEConnectShieldModem :: LTEConnectShieldModem(SoftwareSerial& serialCon, uint8_t onOffPin)
{
  _sSerialCon = &serialCon;
  _onOffPin = onOffPin;
}

LTEConnectShieldModem :: LTEConnectShieldModem(LTEConnectShieldInterface& serialInterface, uint8_t onOffPin)
{
  _serialInterface = &serialInterface;
  _onOffPin = onOffPin;
}

bool LTEConnectShieldModem :: begin(long baudrate)
{
  bool retValue = false;

  //enable serial port for logger
  LOGGER_INIT();

  //start logging
  LOGGER_START();

  //configure serial connection with LTE modem

  if(_hSerialCon != NULL)
  {
    _hSerialCon->begin(baudrate, SERIAL_8N1, _rts, _cts);
    _hSerialCon->setTimeout(DEFAULT_AT_COMMAND_RESPONSE_TIMEOUT);
    _streamCon = (Stream *) _hSerialCon;
  }
  else if(_sSerialCon != NULL)
  {
    _sSerialCon->begin(baudrate);
    _sSerialCon->setTimeout(DEFAULT_AT_COMMAND_RESPONSE_TIMEOUT);
    _streamCon = (Stream *) _sSerialCon;
  }
  else if(_serialInterface != NULL)
  {
    _serialInterface->begin(baudrate);
    _streamCon->setTimeout(DEFAULT_AT_COMMAND_RESPONSE_TIMEOUT);
    _streamCon = (Stream *) _serialInterface;
  }
  else
  {
    return false;
  }

  _isUrcProcEnabled = false;

  for(byte i=0; i<MAX_URC_EVENT_LISTENERS; i++)
    _urcEventListeners[i] = NULL;

  registerUrcEventListener(this);

  //set on/off pin to output
  pinMode(_onOffPin, OUTPUT);

  //turn LTE device on
  turnON();

  //wait for ^SYSLOADING message
  //Simulate running AT command in order to get first URC as command response for verification
  _isAtRunning = true;  
  retValue = getATcommandReposneVerify((char*)LTE_MODULE_SYSLOADING_MESSAGE);
  if(retValue != true)
  {
    PRINT_LOG_LOW("didn't get: ^SYSLOADING !");
	//turn LTE device off
    turnOFF();
  }
  else
  {
    //wait for ^SYSSTART message
    //Simulate running AT command in order to get URC as command response for verification
    _isAtRunning = true;  
    retValue = getATcommandReposneVerify((char*)LTE_MODULE_SYSSTART_MESSAGE);
    if(retValue != true)
    {
      PRINT_LOG_LOW("didn't get: ^SYSSTART !");
	  //turn LTE device off
      turnOFF();
    }
    else 
    {
      PRINT_LOG_HIGH("Successful");
    }
  }

  //disable echo
  if(retValue)
    retValue = sendATcommandWithConf("ATE0");

  //enable flow control
  if(retValue && (_rts != SERIAL_PIN_UNKNOWN || _cts != SERIAL_PIN_UNKNOWN))
    retValue = sendATcommandWithConf("AT\\Q2");

  return retValue;
}


void LTEConnectShieldModem :: turnON()
{
  PRINT_LOG_HIGH("LTE modem on...");
  DIGITAL_WRITE_DELAY(_onOffPin, LOW, 1000);
  DIGITAL_WRITE_DELAY(_onOffPin, HIGH, 500);
}

void LTEConnectShieldModem :: turnOFF()
{
  PRINT_LOG_HIGH("LTE modem off...");
  DIGITAL_WRITE_DELAY(_onOffPin, LOW, 3500);
  DIGITAL_WRITE_DELAY(_onOffPin, HIGH, 500)
}


bool LTEConnectShieldModem :: sendATcommand(const char *format, ...)
{
  va_list ap;

  memset(cmdBuf, 0, MAX_AT_COMMAND_BUFFER_SIZE);

  va_start(ap, format);
  vsnprintf(cmdBuf, MAX_AT_COMMAND_BUFFER_SIZE-1, format, ap);
  va_end(ap);

  return sendATcommand(DEFAULT_AT_COMMAND_DELAY, cmdBuf);
}

bool LTEConnectShieldModem :: sendATcommand(int delayTime, const char *format, ...)
{
  va_list ap;

  memset(cmdBuf, 0, MAX_AT_COMMAND_BUFFER_SIZE);

  va_start(ap, format);
  vsnprintf(cmdBuf, MAX_AT_COMMAND_BUFFER_SIZE-1, format, ap);
  va_end(ap);

  return sendATcommand(delayTime, cmdBuf);
}

bool LTEConnectShieldModem :: sendATcommand(int delayTime, char *cmd)
{
  if(_isAtRunning == true || _streamCon == NULL || cmd == NULL || (strlen(cmd) >= (MAX_AT_COMMAND_BUFFER_SIZE - 1)))
    return false;

  strcat(cmd, "\r");

  _isAtRunning = true;

  //check if there is any data waiting to be read from LTE device
  if(_isUrcProcEnabled) 
  {
    //process URC if it is any at waiting
    processUrc();
  }
  else
  {
    //read all data and discard it
    readUrcDiscard();
  }

  //send AT command to modem
  _streamCon->print(cmd);

  PRINT_LOG_MED("AT-> %s", cmd);
  delay(delayTime);

  return true;
}

bool LTEConnectShieldModem :: sendATcommandWithConf(const char *format, ...)
{
  memset(cmdBuf, 0, MAX_AT_COMMAND_BUFFER_SIZE );

  va_list ap;
  va_start(ap, format);
  vsnprintf(cmdBuf, MAX_AT_COMMAND_BUFFER_SIZE - 1, format, ap);
  va_end(ap);

  if(!sendATcommand(DEFAULT_AT_COMMAND_DELAY, cmdBuf))
  	return false;

  return getATcommandReposneVerify(DEFAULT_AT_COMMAND_RESPONSE_OK);
}

bool LTEConnectShieldModem :: sendATcommandWithConf(int delayTime, const char *format, ...)
{
  memset(cmdBuf, 0, MAX_AT_COMMAND_BUFFER_SIZE);

  va_list ap;
  va_start(ap, format);
  vsnprintf(cmdBuf, MAX_AT_COMMAND_BUFFER_SIZE - 1, format, ap);
  va_end(ap);

  if(!sendATcommand(delayTime, cmdBuf))
  	return false;

  return getATcommandReposneVerify(DEFAULT_AT_COMMAND_RESPONSE_OK);
}

bool LTEConnectShieldModem :: sendATcommandVerifyRes(const char* expResp, const char *format, ...)
{
  if(_isAtRunning == true || _streamCon == NULL || expResp == NULL || format == NULL)
    return false;

  memset(cmdBuf, 0, MAX_AT_COMMAND_BUFFER_SIZE);

  va_list ap;
  va_start(ap, format);
  vsnprintf(cmdBuf, MAX_AT_COMMAND_BUFFER_SIZE - 1, format, ap);
  va_end(ap);

  if(!sendATcommand(DEFAULT_AT_COMMAND_DELAY, cmdBuf))
    return false;

  return getATcommandReposneVerify(expResp);
}

size_t LTEConnectShieldModem :: getATcommandReposne(char* resp, size_t len)
{
  if(_isAtRunning == false || _streamCon == NULL || resp == NULL)
  	return 0;

  size_t retBytes = 0;

  //get AT command response
  //Serial.readBytesUntil('\r', resp, MAX_AT_COMMAND_RESPONSE_BUFFER_SIZE);
  retBytes = _streamCon->readBytes(resp, len);

  _isAtRunning = false;

  PRINT_LOG_MED("AT<- len:%d, str:%s", retBytes, resp);

  return retBytes;
}

bool LTEConnectShieldModem :: getATcommandReposneVerify(const char *format, ...)
{
  if(_isAtRunning == false || _streamCon == NULL || format == NULL)
  	return 0;

  memset(cmdBuf, 0, MAX_AT_COMMAND_BUFFER_SIZE);

  bool retValue= false;

  va_list ap;
  va_start(ap, format);
  vsnprintf(cmdBuf, MAX_AT_COMMAND_BUFFER_SIZE - 1, format, ap);
  va_end(ap);

  //get AT command response
  retValue = _streamCon->find(cmdBuf);

  _isAtRunning = false;

  PRINT_LOG_MED("AT<- ver:%d, exp:%s", retValue, cmdBuf);

  return retValue;
}

size_t LTEConnectShieldModem :: sendData(const char* data, size_t len)
{
  if(data == NULL)
  	return 0;
  
  return _streamCon->write(data, len);
}

int LTEConnectShieldModem :: dataAvailable()
{
  return _streamCon->available();
}

size_t LTEConnectShieldModem :: getData(char* data, size_t len)
{
  if(data == NULL)
  	return 0;
  
  return _streamCon->readBytes(data, len);
}

int LTEConnectShieldModem :: read()
{
  return _streamCon->read();
}

void LTEConnectShieldModem :: flush()
{
  return _streamCon->flush();
}

void LTEConnectShieldModem :: registerUrcEventListener(LTEConnectShieldUrcListener* provider)
{
  for(int i=0;i<MAX_URC_EVENT_LISTENERS;i++)
  {
    if(_urcEventListeners[i] == NULL)
    {
      _urcEventListeners[i] = provider;
      break;
    }
  }
}

void LTEConnectShieldModem :: unRegisterUrcEventListener(LTEConnectShieldUrcListener* provider)
{
  for(int i=0;i<MAX_URC_EVENT_LISTENERS;i++)
  {
    if(_urcEventListeners[i] == provider)
    {
      _urcEventListeners[i] = NULL;
      break;
    }
  }
}

void LTEConnectShieldModem :: readUrcDiscard()
{
  memset(urcBuf, 0, MAX_AT_URC_BUFFER_SIZE);

  while(_streamCon->available())
  {
    _streamCon->readBytes(urcBuf, MAX_AT_URC_BUFFER_SIZE);
	PRINT_LOG_LOW( "disc data:%s", urcBuf);
  }
}

void LTEConnectShieldModem :: processUrc()
{
  size_t retBytes = 0;

  memset(urcBuf, 0, MAX_AT_URC_BUFFER_SIZE);

  //first usage of processURC() enables the URC processing
  _isUrcProcEnabled = true;
  
  while(_streamCon->available())
  {
    retBytes = _streamCon->readBytesUntil('\n', urcBuf, MAX_AT_URC_BUFFER_SIZE);

	PRINT_LOG_LOW("URC-> len:%d, data:%s", retBytes, urcBuf);

	if(retBytes > 1 && (urcBuf[0] == '+' || urcBuf[0] == '^'))
    {
      PRINT_LOG_LOW("Send URC for further processing");

      //add null termination to URC string
      urcBuf[retBytes-1] = 0;
	  
      for(int i=0; i<MAX_URC_EVENT_LISTENERS; i++)
      {
        if(_urcEventListeners[i] != NULL)
        {
          _urcEventListeners[i]->processUrcEvent(urcBuf, retBytes);
	    }
      }
    }
  }
}

void LTEConnectShieldModem :: processUrcEvent(char * urcData, size_t dataLen)
{
  PRINT_LOG_LOW("modem class process URC");
  //put code which wait for URC event here 
}

size_t LTEConnectShieldModem :: getName(char *name, size_t len)
{
  char* strPtr = NULL;

  sendATcommand("ATI");

  getATcommandReposne(name, len);

  //cut response before REVISION information
  strPtr = strstr(name, "REVISION");

  //terminate string here 
  *strPtr = 0;

  //return the name lenght
  return strPtr - name;
}

size_t LTEConnectShieldModem :: getVersion(char *version, size_t len)
{
  char* strPtr = NULL;

  sendATcommand("ATI");

  getATcommandReposne(version, len);

  //find REVISION string in response
  strPtr = strstr(version, "REVISION");

  //find end of Revision information and terminate string there
  *(strchr(strPtr, '\r')) = 0;

  //copy Revision info at the begining of version table
  strcpy(version, strPtr);

  //return the version length
  return strlen(version);
}

const char * LTEConnectShieldModem :: getLibVersion()
{
  return LTE_CON_SHIELD_VERSION;
}

bool LTEConnectShieldModem :: registerToNetwork(const char * mccMnc)
{
  bool retValue = false;
  eRegState regState = NOT_REGISTERED;
  char respData[25];
  byte retCount = 0;

  //check if SIM pin is entered
  if(SIM_PIN_READY != getSimPinState())
  {
    PRINT_LOG_MED("SIM PIN state is not READY");
    return false;
  }

  if(mccMnc == NULL)
  {
    retValue = sendATcommand("AT+COPS=0");
  }
  else if (strlen(mccMnc) == 5 || strlen(mccMnc) == 6)
  {
	retValue = sendATcommand("AT+COPS=1,2,\"%s\"", mccMnc);
  }
  //incorrect parameters, retValue stays false

  if(!retValue)
  {
    return retValue;
  }

  //wait for OK or ERROR
  retCount = 0; //retry counter
  while(true)
  {
    //allow to call getATcommandReposne many times
    _isAtRunning = true;
	memset(respData, 0, 25);
    getATcommandReposne(respData, 25);
	retCount ++;

	PRINT_LOG_MED("Reg data:%s", respData);

	if(strstr(respData, "OK") != NULL)
	{
	  break;
	}
	else if (strstr(respData, "ERROR") != NULL)
	{
      return false;
	}
	else if(retCount >= 50)
    {
      //abort command and return false
      respData[0] = '\r';
	  sendData(respData,1);
      return false;
    }
  }

  //wait for registration 
  retCount = 0; //retry counter
  memset(respData, 0, 25);

  while(regState != REGISTERED && retCount <= 30)
  {
    sendATcommand("AT+CREG?");
	
    getATcommandReposne(respData, 25);
	
    regState =(eRegState) ((*(strstr(respData,"+CREG")+9))-48); //get connection state
    
    PRINT_LOG_LOW("Reg state:%d", regState);

    retCount++;
	
    //wait 1s and check again
    delay(1000);
  }

  if(regState != REGISTERED)
  	retValue = false;

  return retValue;
}

eRegState LTEConnectShieldModem :: getRegistrationState()
{
  eRegState regState = ERROR;
  char* strptr = NULL;

  memset(cmdBuf, 0, MAX_AT_COMMAND_BUFFER_SIZE + 1);

  if(!sendATcommand("AT+CREG?"))
    return ERROR;

  if(getATcommandReposne(cmdBuf, MAX_AT_COMMAND_BUFFER_SIZE) <= 10)
    return ERROR;

  strptr = strstr(cmdBuf, "+CREG");
  if(strptr != NULL)
    return (eRegState)(strptr[9] - 48);
}

eSimPinState LTEConnectShieldModem :: getSimPinState()
{
  char* strptr = NULL;
  eSimPinState pinState = SIM_PIN_ERROR;

  memset(cmdBuf, 0, MAX_AT_COMMAND_BUFFER_SIZE + 1);

  if(!sendATcommand("AT+CPIN?"))
    return SIM_PIN_ERROR;

  if(getATcommandReposne(cmdBuf, MAX_AT_COMMAND_BUFFER_SIZE) == 0)
    return SIM_PIN_ERROR;

  if(strstr(cmdBuf, "READY") != NULL)
  {
    pinState = SIM_PIN_READY;
  }
  else if(strstr(cmdBuf, "PIN") != NULL)
  {
    pinState = SIM_PIN_NEED;
  }
  else if(strstr(cmdBuf, "PUK") != NULL)
  {
    pinState = SIM_PUK_NEED;
  }
  else if(strstr(cmdBuf, "PIN2") != NULL)
  {
    pinState = SIM_PIN2_NEED;
  }
  else if(strstr(cmdBuf, "PUK2") != NULL)
  {
    pinState = SIM_PUK2_NEED;
  }

  return pinState;
}

bool LTEConnectShieldModem :: setSimPin(char* pin)
{
  if(pin == NULL || (strlen(pin) > 4))
    return false;

  return sendATcommandWithConf("AT+CPIN=\"%s\"",pin);
}

