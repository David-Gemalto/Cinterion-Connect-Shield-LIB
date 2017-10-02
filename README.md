# Cinterion-Connect-Shield-LIB
Cinterion Connect Shield Library for Arduino Uno

This library simplifies the usage of LTE device available on Cinterion Connect Shield.

Find the device reference on https://developer.gemalto.com/documentation/cinterion%C2%AE-connect-shield-tools-documentation

Feel free to ask the community members all questions about the Cinterion Connect Shield

## Start-up

For default configuration just put the Cinterion Connect Shield directly into Arduino Header without any shifts.
In that configuration pins assignment and usage by library is the following:

Arduino       | Cinterion Connect Shield
------------- | ------------------------
0 RX          | TXD
1 TX          | RXD
2             | RTS
3             | CTS
5 RX (Logger) | none
6 TX (Logger) | none
8             | PWR IND
9             | ON/OFF

Next download whole library and place it into directory where you have other arduino libraries. 
In your sketch add an include:  
__#include <LTECinterionConnectShield.h>__  
then create an object of class:  
__LTEConnectShieldModemc__   
and start using it. 

For more infomration please take a look into examples.

## Limitations
The Cinterion Connect Shield as default uses the main serial connection which is also use for Arduino programing.  
Therefore, if LTE device is enabled during Arduino programing (loading a new software) it may broke transmission and programing fails.  
Please turn off the LTE device in case of programing via ON/OFF button on shield - press for at least 3 second.

## Examples

This library contains a various of examples, please see [examples](https://github.com/David-Gemalto/Cinterion-Connect-Shield-LIB/tree/master/examples "Examples")

Basic example how to use Cinterion Connect Shield an register to network is avilable here:  
[AtCommandUsage.ino](https://github.com/David-Gemalto/Cinterion-Connect-Shield-LIB/blob/master/examples/AtCommandUsage/AtCommandUsage.ino "AtCommandUsage.ino") 

There are also two more examples, which shows how to prepare sketch for publishing and subscribing the MQTT messages:  
[MqttPubExample.ino](https://github.com/David-Gemalto/Cinterion-Connect-Shield-LIB/blob/master/examples/MqttPubExample/MqttPubExample.ino "MqttPubExample.ino")  
[MqttSubExample.ino](https://github.com/David-Gemalto/Cinterion-Connect-Shield-LIB/blob/master/examples/MqttSubExample/MqttSubExample.ino "MqttSubExample.ino")

## License
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
