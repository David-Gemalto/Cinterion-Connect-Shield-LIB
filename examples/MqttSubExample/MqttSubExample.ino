/*
 Example of MQTT client that subscribe one topic and is waiting for messages using Cinterion Connect Shield.
 As result there is possible to turn ON/OFF the LED on Arduino module.

 This example requires PubSubClient library by Nick O'Leary version 2.6.0

 For turn LED on please send message "1" with topic "LteLed", for turn LED off please send message "0" with the same topic.
 
 Most of Arduino devices have an on-board LED you can control. 
 On the UNO, MEGA and ZERO it is attached to digital pin 13, on MKR1000 on pin 6. 
 LED_BUILTIN is set to the correct LED pin independent of which board is used.

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
#include <PubSubClient.h>
#include <Client.h>

//replace "9999" with your own pin number
#define PIN_NUMBER "9999"

//customize those values according to your use case
const char* apn = "internet";
const char* mqtt_server = "test.mosquitto.org";
uint16_t    mqtt_server_port = 1883;
const char* mqtt_client_id = "LteDevice";
const char* mqtt_client_topic = "LteLed";

/*  Implementation of Client class based on Cinterion LTE modem class
 *  It will be used by MQTT library to connect with server
 */
class LteClient : public Client
{
  private:
  LTEConnectShieldModem* _modem = NULL;
  bool _isConnected = false;

  public:
  LteClient(LTEConnectShieldModem& modem)
  {
    _modem = &modem;
  }
  
  void begin (const char* apn)
  {
    //start the LTE device with default baud rate - 115200
    _modem->begin(LTE_CONECT_SHILED_DEFAULT_BAUDRATE);

    //set SIM PIN if needed
    if(_modem->getSimPinState() != SIM_PIN_READY)
    {
      if(!_modem->setSimPin((char*)PIN_NUMBER))
        PRINT_LOG_CUST("LteClient->PIN Failed");
    }

    //wait a little for sim initialization
    delay(3000);
    
    //register LTE device to network - chosen mode: auto
    _modem->registerToNetwork();
  
    //show the registration state
    PRINT_LOG_CUST("LteClient->registration state:%d", _modem->getRegistrationState());

    //set ip connection type and apn name
    if(!_modem->sendATcommandWithConf("at^sics=1,\"conType\",\"GPRS0\""))
     PRINT_LOG_CUST("LteClient->set conType failed");
      
    if(!_modem->sendATcommandWithConf("at^sics=1,\"apn\",\"%s\"",apn))
     PRINT_LOG_CUST("LteClient->set apn failed");
  }

  int connect(IPAddress ip, uint16_t port)
  {
    char hostName[20];
    memset(hostName,0,20);
    snprintf(hostName,20,"%d.%d.%d.%d:%d",ip[0],ip[1],ip[2],ip[3]);
     
    return connect(hostName,port);
  }
  
  int connect(const char *host, uint16_t port)
  {
    char respData[MAX_AT_COMMAND_BUFFER_SIZE];
    uint8_t conState = 0;
    
    memset(respData,0,MAX_AT_COMMAND_BUFFER_SIZE);
    
    _modem->sendATcommandWithConf("at^siss=1,\"srvType\",\"Socket\"");
    _modem->sendATcommandWithConf("at^siss=1,\"conid\",\"1\"");
    _modem->sendATcommandWithConf("at^siss=1,\"address\",\"socktcp://%s:%d;etx\"",host ,port);

    //check registration again before connecting
    if(_modem->getRegistrationState() != REGISTERED)
    {
      //stop current radio related activity
      _modem->sendATcommandWithConf("at+cops=2");
      //register to network
      if(!_modem->registerToNetwork())
      {
        _isConnected = false;
        return false;
      }
    }

    //open socket connection
    _modem->sendATcommandWithConf("at^siso=1");

    //check if connection are open successfully  
    byte retCount = 0; //retry counter
    while(conState != 4 && retCount <= 10)
    {
      _modem->sendATcommand("at^sisi?");
      _modem->getATcommandReposne(respData,MAX_AT_COMMAND_BUFFER_SIZE);
      
      conState = (*(strstr(respData,"^SISI:")+9))-48;  //get connection state
      
      //if connection is not in "connecting" state then break this loop
      if(conState != 3)
      {
        _isConnected = false;
        break;
      }

      retCount++;
      //wait 2s and check again
      delay(2000);
    }

    if(conState != 4)
    {
      _modem->sendATcommandWithConf("at^sisc=1");
      _isConnected = false;
    }
    else
    {
      _modem->sendATcommand("AT^SIST=1");
      _isConnected = _modem->getATcommandReposneVerify("CONNECT");
      
      //read and discard useless CR and LF characters sent by LTE device after CONNECT
      while(_modem->dataAvailable())
      {
        _modem->read();
      }
    }
    
    return _isConnected;
  }

  size_t write(uint8_t data)
  {
    //not used by pubSubClient
    return 0;
  }
  
  size_t write(const uint8_t *buf, size_t size)
  {
    return _modem->sendData((const char*)buf, size);
  }
  
  int available()
  {
    return _modem->dataAvailable();
  }
  
  int read()
  {
    return _modem->read();
  }
  
  int read(uint8_t *buf, size_t size)
  {
    return _modem->getData((char*)buf, size);
  }
  
  int peek()
  {
    //not used by pubSubClient
    return 0;
  }
  
  void flush()
  {
    _modem->flush();
  }
  
  void stop()
  {
    char plus[]={"+"};

    //exit from transparent connection  
    delay(1500);
    _modem->sendData(plus,1);
    delay(300);
    _modem->sendData(plus,1);
    delay(300);
    _modem->sendData(plus,1);
    delay(1500);

    //close socket
    _modem->sendATcommandWithConf("at^sisc=1");
    _isConnected = false;
      
    return;
  }
  
  uint8_t connected()
  {
    return _isConnected;
  }

  operator bool() 
  {
    return false;
  }
};

LTEConnectShieldModem modem;
LteClient lteClient(modem);
PubSubClient MqttClient;

/**
 * Function used for connection/reconnection with MQTT broker and subscribing one topic.
 */
void reconnect() 
{ 
  // loop until we're reconnected
  while (!MqttClient.connected()) 
  {
    PRINT_LOG_CUST("Attempting MQTT connection...");
    // attempt to connect
    if (MqttClient.connect(mqtt_client_id)) 
    {
      PRINT_LOG_CUST("MQTT connected");
      MqttClient.subscribe(mqtt_client_topic);      
    } 
    else 
    {
      PRINT_LOG_CUST("failed, rc=%d", MqttClient.state());
      PRINT_LOG_CUST("try again in 5 seconds...");
  
      //wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/**
 * Callback function that will be called when proper MQTT message will be received. 
 */
void mqttCallback(char* topic, byte* payload, unsigned int length) 
{
  PRINT_LOG_CUST("Message arrived [%s]:",topic);

  if(length == 1)
  {
    PRINT_LOG_CUST("%c",*payload);
    
    if(*payload == '0')
      digitalWrite(LED_BUILTIN, LOW);
    else
      digitalWrite(LED_BUILTIN, HIGH);
  }
  else //incorrect value
  { 
    for (int i=0;i<length;i++) 
    {
      PRINT_LOG_CUST("%c",(char)payload[i]);
    }
  }
}

void setup() 
{
  //start LTE modem and register to network
  lteClient.begin(apn);

  //configure LED on Arduino
  pinMode(LED_BUILTIN, OUTPUT);
  
  //set MQTT client to use our LteClient for network connection
  MqttClient.setClient(lteClient);

  //set MQTT server name and port number
  MqttClient.setServer(mqtt_server, mqtt_server_port);
  MqttClient.setCallback(mqttCallback);
}

void loop() 
{
  if(!MqttClient.connected()) 
  {
    reconnect();
  }
 
  MqttClient.loop();
  
  //wait 2s.
  delay(2000);
}
