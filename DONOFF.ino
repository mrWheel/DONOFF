/*
***************************************************************************  
**  Program  : DONOFF
*/
#define  _FW_VERSION  "v0.3.6 (" +String( __DATE__) + ")"
/*
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
**     DONOFF:  Generic ESP8266 Flash Size 1M (128KB SPIFFS) 
**              BuiltIn LED GPIO1
**              PWM-out GPIO3
**               
**     SONOFF:  Generic ESP8266 Flash Size 1M (128KB SPIFFS)
**              BuiltIn LED 13
**              Switch  pin 12            
**               
**     NODEMCU: NodeMCU 1.0 Flash Size 4M (1MB SPIFFS)
**              BuiltIn LED 16
**              PWM-out every free pin available            
**             
**  Arduino-IDE settings for ESP01 (black):
**
**    - Board: "Generic ESP8266 Module"
**    - Flash mode: "DOUT"
**    - Flash size: "1M (128K SPIFFS)"
**    - Debug port: "Disabled"
**    - Debug Level: "None"
**    - IwIP Variant: "v2 Lower Memory"
**    - Reset Method: "nodemcu"   // but will depend on the programmer!
**    - Crystal Frequency: "26 MHz" 
**    - VTables: "Flash"
**    - Flash Frequency: "40MHz"
**    - CPU Frequency: "80 MHz"
**    - Buildin Led: "1"  // "1" for ESP-01, 13 for SONOFF
**    - Upload Speed: "115200"
**    - Erase Flash: "Only Sketch"
**    - Port: "ESP01-DSMR at <-- IP address -->"
**
*/

#include "networkStuff.h"

//  part of ESP8266 Core https://github.com/esp8266/Arduino
#include <ESP8266HTTPClient.h>

//  https://github.com/PaulStoffregen/Time
#include <TimeLib.h>


#define _CONFIG_FILE  "/donoff.cfg"
#define _LED_ON       LOW
#define _LED_OFF      HIGH
#define _MAX_DEVICES  15
#define _HEARTBEAT_INTERVAL  (10 * 60) /* 10 minutes -> _HEARTBEAT is in seconds */
#define _MASTERDONOFF "DONOFF"

typedef struct  {
  String  deviceHostName;
  String  IPaddress;
  String  Label;
  char    Type;
  int8_t  minState;
  int8_t  maxState;
  int8_t  State;
  int8_t  OnOff;
  time_t  lastSeen;
} device_type;

device_type deviceArray[_MAX_DEVICES];

// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer  server(80);
WiFiClient        wifiClient;
// A UDP instance to let us send and receive packets over UDP
WiFiUDP           Udp;

static char *flashMode[]    { "QIO", "QOUT", "DIO", "DOUT", "UnKnown" };

bool      deviceIsMaster      = false;
//String  deviceHostName      = "DONOFF"; // defined in networkStuff.h
String    deviceIPaddress     = "unknown";
String    deviceLabel         = "?";
char      deviceType          = 'D';
int8_t    deviceDefaultState  = 10;
int8_t    deviceMinState      = 10;
int8_t    deviceMaxState      = 90;
bool      deviceFirstStart    = true;
String    masterHostName      = _MASTERDONOFF;
String    masterIPaddress     = "0.0.0.0";
int       localSwitchGPIO     = -1;   // -1 = no Switch
bool      localSwitchToggle   = false;
int       localDeviceGPIO     = LED_BUILTIN;
int       localBuiltInLed     = LED_BUILTIN;
bool      localDeviceInverted = false;
int8_t    webSocketDevNr      = 0;
uint32_t  webSocketHeartbeat  = 0;
uint16_t  localPWMfreq        = 250;   // Hz

//int       device;
int       value;
char      cMsg[100], cMsg2[100], responseURL[100], APname[50], MAChex[13]; //n1n2n3n4n5n6\0
char      inChar;       // TelnetStream input
int       noOfDevices = 0;
bool      statusChanged = true, doUpdateDOM  = false;
bool      doShowHeap = false;
bool      mustReboot = false;
bool      animateDimUpDown  = false;
uint32_t  updateDac, aliveTime, heapInterval;
uint32_t  freeHeap, maxHeap=0, minHeap=99999;
IPAddress ipDNS, ipGateWay, ipSubnet;

// NTP Servers:
static const char ntpServerName[] = "nl.pool.ntp.org";
//static const char ntpServerName[] = "time.nist.gov";
//static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

const int timeZone = 2;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

unsigned int localPort = 8888;  // local port to listen for UDP packets


//===========================================================================================
String macToStr(const uint8_t* mac) {
//===========================================================================================
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;

} // macToStr()


//===========================================================================================
int setLocalDevice() {
//===========================================================================================
  uint8_t State;
  char    tmpType;

    tmpType = deviceArray[0].Type;
    
    if (deviceArray[0].OnOff == 0) { // switched Off!
      State   = 0;
      tmpType = 'S';
    } else {
      State = deviceArray[0].State;
    }
    
    if (tmpType == 'D') { // it's a dimmer
      _dThis = true;
      Debugf("New State local Dimmer! GPIO[%d] => State is now [%d], OnOff is now [%d]\n"
                                                                                        , localDeviceGPIO
                                                                                        , deviceArray[0].State
                                                                                        , deviceArray[0].OnOff);
      if (localDeviceInverted) {
        _dThis = true;
        Debugf("analogWrite(%d, %d)\n", localDeviceGPIO, (100 - State));
        analogWrite(localDeviceGPIO, (100 - State));  // dimmer
      } else {
        _dThis = true;
        Debugf("analogWrite(%d, %d)\n", localDeviceGPIO, State);
        analogWrite(localDeviceGPIO, State);          // dimmer
      }
    } else {  // it's a switch
      _dThis = true;
      Debugf("New State local Switch! GPIO[%d] => State is now [%d], OnOff is now [%d]\n"
                                                                                        , localDeviceGPIO
                                                                                        , deviceArray[0].State
                                                                                        , deviceArray[0].OnOff);
      if (localDeviceInverted) {
        digitalWrite(localDeviceGPIO, !State);        // switch
      } else {
        digitalWrite(localDeviceGPIO,  State);        // switch
      }
    }

} // setLocalDevice()


//===========================================================================================
int isNewSlave(String IPaddress, char Type) {
//===========================================================================================
  for (int i=1; i <= noOfDevices; i++) {
    if (deviceArray[i].IPaddress == IPaddress) return i;
  }

  return -1;
  
} // isNewSlave()


//===========================================================================================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
//===========================================================================================
    String  text = String((char *) &payload[0]);
    char *  textC = (char *) &payload[0];
    String  pOut[5], pDev[5], pVal[5], jsonString;
    int8_t  deviceNr;

    switch(type) {
        case WStype_DISCONNECTED:
            _dThis = true;
            Debugf("[%u] Disconnected!\n", num);
            isConnected = false;
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                if (!isConnected) {
                 _dThis = true;
                 Debugf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                  isConnected = true;
                  webSocket.broadcastTXT("{\"msgType\":\"ConnectState\",\"Value\":\"Connected\"}");
                  sendDevInfo();
                }
        
            }
            break;
        case WStype_TEXT:

            _dThis = true;
            Debugf("[%u] Got message: [%s]\n", num, payload);
            webSocketHeartbeat = millis() + 20000;
            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            if (text.indexOf("getDevInfo") > -1) {
              jsonString  = "{";
              jsonString += "\"msgType\": \"devInfo\"";
              if (deviceIsMaster) {
                jsonString += ", \"devName\": \"" + deviceHostName + " \"";
                jsonString += ", \"devIPaddress\": \"" + WiFi.localIP().toString() + " \"";
                jsonString += ", \"devVersion\": \" [Master " + String(_FW_VERSION) + "]\"";
              } else {
                jsonString += ", \"devName\": \"" + deviceHostName + " \"";
                jsonString += ", \"devIPaddress\": \"" + WiFi.localIP().toString() + " \"";
                jsonString += ", \"devVersion\": \" [Slave " + String(_FW_VERSION) + "]\"";
              }
              jsonString += "}";
              _dThis = true;
              Debugln(jsonString);
              webSocket.broadcastTXT(jsonString);
            } 
            if (text.indexOf("updateDOM") > -1) {
              _dThis = true;
              Debugln("now updateDOM()!");
              updateDOM();
            } 
            if (text.indexOf("DOMloaded") > -1) {
              _dThis = true;
              Debugln("set doUpdateDOM = false;");
              doUpdateDOM = false;
            } 
            if (text.indexOf("dimmer") > -1 || text.indexOf("switch") > -1) {
              splitString(text, ',', pOut, 3);
              _dThis = true;
              Debugf("Found pOut[0] [%s]\n", pOut[0].c_str());
              splitString(pOut[0], '=', pDev, 3);
              _dThis = true;
              Debugf("%s [%s]\n", pDev[0].c_str(), pDev[1].c_str());
              deviceNr        = pDev[1].toInt();
              webSocketDevNr  = pDev[1].toInt();
              splitString(pOut[1], '=', pVal, 3);
              _dThis = true;
              Debugf("%s [%s]\n", pVal[0].c_str(), pVal[1].c_str());
              if (pVal[0] == "OnOff") {
                    deviceArray[deviceNr].OnOff     = pVal[1].toInt();
                    if (text.indexOf("switch") > -1) deviceArray[deviceNr].State     = pVal[1].toInt();
                    _dThis = true;
                    Debugf("deviceArray[%d].OnOff => [%d]\n", deviceNr, deviceArray[deviceNr].OnOff);
              }
              else if (pVal[0] == "sliderVal") {
                    deviceArray[deviceNr].State     = pVal[1].toInt();
                    _dThis = true;
                    Debugf("deviceArray[%d].State => [%d]\n", deviceNr, deviceArray[deviceNr].State);
                    deviceArray[deviceNr].OnOff     = 1;
                    _dThis = true;
                    Debugf("deviceArray[%d].OnOff => [%d]\n", deviceNr, deviceArray[deviceNr].OnOff);
              }
              if (deviceNr == 0) {   // local device
                deviceArray[deviceNr].lastSeen  = now();
                setLocalDevice();
                animateDimUpDown = false;
              }
              handleWebSocketState(deviceNr, payload, lenght);
            }
            break;
                        
    } // switch(type)
    
} // webSocketEvent()


//===========================================================================================
void handleWebSocketState(int16_t deviceNr, uint8_t * webSocketData,  size_t lenght) {
//===========================================================================================
  String    message = "", response;

  _dThis = true;
  Debugf("[%s] \n", webSocketData);

  statusChanged = true;

  _dThis = true;
  Debugf("webSocketDevNr [%d] - [%s] / [%c] => State[%d] / OnOff[%d]\n", deviceNr
                                                                  , deviceArray[deviceNr].IPaddress.c_str()
                                                                  , deviceArray[deviceNr].Type
                                                                  , deviceArray[deviceNr].State
                                                                  , deviceArray[deviceNr].OnOff);
  if (deviceNr == 0) { // local device
    deviceArray[0].lastSeen  = now();
    setLocalDevice();
  }
  
  handleStatus();
  if (deviceArray[deviceNr].Type == 'D')  response = "dimmer";
  else                                    response = "switch";
  response += "," + String(deviceNr);
  response += "," + String(deviceArray[deviceNr].Type);
  response += "," + String(deviceArray[deviceNr].State);
  response += "," + String(deviceArray[deviceNr].OnOff);
  webSocket.broadcastTXT(response);  // tell all other browser-clients

  if (deviceIsMaster)
        sendHTTPrequest(deviceArray[deviceNr].IPaddress.c_str(), deviceNr);
  else  sendHTTPrequest(masterIPaddress.c_str(), deviceNr);

} // handleWebSocketState()


//===========================================================================================
void handleSlaveState() {
//===========================================================================================
  String    message = "", response;
  String    IPaddress, sendIPaddress, Label, Type;
  int16_t   minState, maxState, State, OnOff, deviceNr;
  bool      hasIPaddress  = false, hasType = false, hasLabel = false, hasState = false;
  bool      hasOnOff = false, hasMinS = false, hasMaxS = false;

  _dThis = true;
  Debug("");

  message = "";
  //Debugf(" %s.length(): [%d]\n", message.c_str(), message.length());
  for (uint8_t i = 0; i < server.args(); i++) {
    message += ", " + server.argName(i) + ":'" + server.arg(i) + "'";
  }
  Debugf("[%s] => [%d][%s]\n", server.uri().c_str(), server.args(), message.c_str());

  DebugFlush();
  statusChanged = true;

  if (!deviceIsMaster) {
    if (server.arg("IPaddress") != deviceIPaddress) {
      _dThis = true;
      Debugf("received status update Slave @%s .. but I'm not a Master! Bailout!", server.arg("IPaddress").c_str());
      return;
    }
  }

  if (server.hasArg("IPaddress")) {
    IPaddress = server.arg("IPaddress");
    Debugf("IPaddress [%s] ", IPaddress.c_str());
    hasIPaddress = true;
  }
  if (server.hasArg("Label")) {
    Label = server.arg("Label");
    Debugf(", Label [%s] ", Label.c_str());
    hasLabel  = true;
  } 
  if (server.hasArg("Type")) {
    Type = server.arg("Type");
    Debugf(", Type [%c]", Type[0]);
    hasType = true;
  } 
  if (server.hasArg("State")) {
    State = server.arg("State").toInt();
    Debugf(", State [%d]", State);
    hasState = true;
  }
  if (server.hasArg("OnOff")) {
    OnOff = server.arg("OnOff").toInt();
    Debugf(", OnOff [%d]", OnOff);
    hasOnOff = true;
  }
  if (server.hasArg("minState")) {
    minState = server.arg("minState").toInt();
    Debugf(", minState [%d]", minState);
    hasMinS = true;
  } 
  if (server.hasArg("maxState")) {
    maxState = server.arg("maxState").toInt();
    Debugf(", maxState [%d]", maxState);
    hasMaxS = true;
  } 
  Debugln();
  DebugFlush();

  if (!hasIPaddress || !hasType) {
    message = "handleSlaveState(): no IPaddress or Type!\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for (uint8_t i = 0; i < server.args(); i++) {
      message += ", " + server.argName(i) + ":'" + server.arg(i) + "'";
    }
    message += server.args();
    webSocket.broadcastTXT("{\"msgType\":\"slaveState\", \"Value\":\"Error: " + message + "\"}");
    server.send(404, "text/plain", message);
    _dThis = true;
    Debugln(message);
    return;
  }

  if (deviceIsMaster) {
    deviceNr = isNewSlave(IPaddress, Type[0]);
    _dThis = true;
    Debugf("Found deviceNr [%d]\n", deviceNr);
    sendIPaddress = IPaddress;
    if (deviceNr < 0) { // it's a new device!
      noOfDevices++;
      deviceNr = noOfDevices;
      deviceArray[deviceNr].IPaddress  = IPaddress;
      deviceArray[deviceNr].Label      = Label;
      deviceArray[deviceNr].Type       = Type[0];
      deviceArray[deviceNr].minState   = minState;
      deviceArray[deviceNr].maxState   = maxState;
      deviceArray[deviceNr].State      = State;
      deviceArray[deviceNr].OnOff      = OnOff;
      deviceArray[deviceNr].lastSeen   = now();
      _dThis = true;
      Debugf("new deviceNr [%d] => [%s] [%s] [%c]\n", deviceNr
                                                            , deviceArray[deviceNr].IPaddress.c_str()
                                                            , deviceArray[deviceNr].Label.c_str()
                                                            , deviceArray[deviceNr].Type);
 
      server.send(200, "text/html", server.uri() + server.args());
      handleRoot();
      
    } else {  // update known device
      if (hasState) { deviceArray[deviceNr].State     = State; }
      if (hasOnOff) { deviceArray[deviceNr].OnOff     = OnOff; }
      _dThis = true;
      if (hasLabel && (Label   != deviceArray[deviceNr].Label))     { deviceArray[deviceNr].Label     = Label;    doUpdateDOM = true; }
      _dThis = true;
      if (hasType && (Type[0]  != deviceArray[deviceNr].Type))      { deviceArray[deviceNr].Type      = Type[0];  doUpdateDOM = true; Debugln("new Type"); }
      _dThis = true;
      if (hasMinS && (minState != deviceArray[deviceNr].minState))  { deviceArray[deviceNr].minState  = minState; doUpdateDOM = true; Debugln("new minState"); }
      _dThis = true;
      if (hasMaxS && (maxState != deviceArray[deviceNr].maxState))  { deviceArray[deviceNr].maxState  = maxState; doUpdateDOM = true; Debugln("new maxState"); }
      deviceArray[deviceNr].lastSeen      = now();
      _dThis = true;
      Debugf("Update deviceNr [%d] => [%s] [%s] [%c] [%d] [%d]\n", deviceNr
                                                       , deviceArray[deviceNr].IPaddress.c_str()
                                                       , deviceArray[deviceNr].Label.c_str()
                                                       , deviceArray[deviceNr].Type
                                                       , deviceArray[deviceNr].minState
                                                       , deviceArray[deviceNr].maxState);
    }
    if (doUpdateDOM) {
      updateDOM();
    }
    
  } else {  // Slave, so it must be a local device
    deviceNr = 0;
    sendIPaddress = masterIPaddress;
    if (hasLabel) { deviceArray[0].Label     = Label; }
    if (hasState) { deviceArray[0].State     = State; }
    if (hasOnOff) { deviceArray[0].OnOff     = OnOff; }
    if (deviceArray[0].Type == 'S')
        deviceArray[0].State     = deviceArray[0].OnOff;
    deviceArray[0].lastSeen  = now();
    setLocalDevice();
  }
  
  handleStatus(); 

  server.send(200, "text/html", server.uri() + server.args()); 
  
} // handleSlaveState()


//===========================================================================================
void updateDevices() {
//===========================================================================================
  String updateDevice;
  
  for(int D=0; D <= noOfDevices; D++) {
    if (deviceArray[D].Type == 'D')  updateDevice = "dimmer";
    else                             updateDevice = "switch";
    updateDevice += "," + String(D);
    updateDevice += "," + String(deviceArray[D].Type);
    updateDevice += "," + String(deviceArray[D].State);
    updateDevice += "," + String(deviceArray[D].OnOff);
    _dThis = true;
    Debugf("[%s]\n", updateDevice.c_str());
    webSocket.broadcastTXT(updateDevice);  // tell all other browser-clients

  }

} // updateDevices()


//===========================================================================================
void updateDOM() {
//===========================================================================================
  String message, newDOM, updateDevice;

  message = server.uri();
  for (uint8_t i = 0; i < server.args(); i++) {
    message += ", " + server.argName(i) + ":'" + server.arg(i) + "'";
  }
  _dThis = true;
  Debugf("[%d] [%s]\n", noOfDevices, message.c_str());
  newDOM = "";
  for (int i=0; i<=noOfDevices; i++) {
    if (deviceArray[i].Type == 'D') {
      _dThis = true;
      Debugf("Add Dimmer: device[%d] - [%s]: type[%c] (%d)\n", i
                                                             , deviceArray[i].Label.c_str()
                                                             , deviceArray[i].Type
                                                             , now() - deviceArray[i].lastSeen);
      newDOM += dimmerHTML(i, deviceArray[i].IPaddress, deviceArray[i].Label
                                                      , deviceArray[i].minState
                                                      , deviceArray[i].maxState
                                                      , deviceArray[i].OnOff
                                                      , deviceArray[i].State);
    }
  }
  for (int i=0; i<=noOfDevices; i++) {
    if (deviceArray[i].Type == 'S') {
      _dThis = true;
      Debugf("Add Switch: device[%d] - [%s]: type[%c] (%d)\n", i
                                                             , deviceArray[i].Label.c_str()
                                                             , deviceArray[i].Type
                                                             , now() - deviceArray[i].lastSeen);
      newDOM += switchHTML(i, deviceArray[i].IPaddress, deviceArray[i].Label, deviceArray[i].State);
    }
  }

  doUpdateDOM = false;
  //Debugln(newDOM);
  webSocket.broadcastTXT("updateDOM:" + newDOM);
  updateDevices();
  //server.send(200, "text/html", newDOM);

} // updateDOM()


//===========================================================================================
String deviceToJson(int8_t deviceNr) {
//===========================================================================================
  String jsonDev = "";
  
    jsonDev += "\"Slave\":\"" + String(deviceNr) + "\"";                              // 0
    jsonDev += ",\"IP\":\"" + String(deviceArray[deviceNr].IPaddress) + "\"";         // 1
    jsonDev += ",\"Label\":\"" + String(deviceArray[deviceNr].Label) + "\"";          // 2
    jsonDev += ",\"Type\":\"" + String(deviceArray[deviceNr].Type) + "\"";            // 3
    jsonDev += ",\"minState\":\"" + String(deviceArray[deviceNr].minState) + "\"";    // 4
    jsonDev += ",\"maxState\":\"" + String(deviceArray[deviceNr].maxState) + "\"";    // 5
    jsonDev += ",\"State\":\"" + String(deviceArray[deviceNr].State) + "\"";          // 6
    jsonDev += ",\"OnOff\":\"" + String(deviceArray[deviceNr].OnOff) + "\"";          // 7
    jsonDev += ",\"HB\":\"" + String(now() - deviceArray[deviceNr].lastSeen) + "\"";  // 8

    return jsonDev;

} // deviceToJson()


//===========================================================================================
void handleStatus() {
//===========================================================================================
  String message, jsonString = "";
  int16_t strIndex;
  static uint16_t count = 0;
  bool firstSlave;
  
  message = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += ", " + server.argName(i) + ":'" + server.arg(i) + "'";
  }
  _dThis = true;
  Debugf("[%s] => [%d][%s]\n", server.uri().c_str(), server.args(), message.c_str());
  
  if (!statusChanged) {
    return;
  }

  firstSlave   = true;

  jsonString  = "{";
  jsonString += "  \"msgType\": \"Status\"";
  jsonString += ",\"Slaves\" : [";

  for (int thisDevice = 0; thisDevice <= noOfDevices; thisDevice++) {
    if (deviceArray[thisDevice].Type != 'S') continue;
    if (firstSlave) {
      jsonString += " {";
      firstSlave = false;
    } else {
      jsonString += ",{";
    }
    jsonString += deviceToJson(thisDevice);
    jsonString += "}";
  }
  for (int thisDevice = 0; thisDevice <= noOfDevices; thisDevice++) {
    if (deviceArray[thisDevice].Type != 'D') continue;
    if (firstSlave) {
      jsonString += " {";
      firstSlave = false;
    } else {
      jsonString += ",{";
    }
    jsonString += deviceToJson(thisDevice);
    jsonString += "}";
  }
  jsonString += "]}";

  _dThis = true;
  Debugln("broadcastTXT(status of all devices)");
  webSocket.broadcastTXT(jsonString);
    
} // handleStatus()


//===========================================================================================
void sendDevInfo() {
//===========================================================================================
  String jsonString = "";

    jsonString  = "{";
    jsonString += "\"msgType\": \"devInfo\"";
    if (deviceIsMaster) {
      jsonString += ", \"devName\": \"" + deviceHostName + " \"";
      jsonString += ", \"devVersion\": \" [Master " + String(_FW_VERSION) + "]\"";
    } else {
      jsonString += ", \"devName\": \"" + deviceHostName + " \"";
      jsonString += ", \"devVersion\": \" [Slave " + String(_FW_VERSION) + "]\"";
    }
    jsonString += "}";
    _dThis = true;
    Debugf("broadcastTXT(%s)\n", jsonString.c_str());
    webSocket.broadcastTXT(jsonString);

} // sendDevInfo()


//===========================================================================================
void handleRoot() {
//===========================================================================================

  if (deviceFirstStart) {
    handleAdmin(); 
  }

  _dThis = true;
  Debugln("Send index.html ..");
  //server.send(200, "text/html", indexHTML);
  doUpdateDOM = true;

} // handleRoot()


//===========================================================================================
void handleNotFound() {
//===========================================================================================
  String message = "handleNotFound(): URL not valid!\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += ", " + server.argName(i) + ":'" + server.arg(i) + "'";
  }

  server.send(404, "text/plain", message);
  _dThis = true;
  Debugln(message);

} // handleNotFount()


//===========================================================================================
String listAllSlaves(bool doHtml) {
//===========================================================================================
  String slavesInfo = "";

  MDNS.begin(deviceHostName.c_str());
  int n = MDNS.queryService("dooSlave", "tcp"); // Send out query for esp tcp services

  if (!doHtml) {
    _dThis = true;
    Debugln("sending mDNS query for [DONOFF Slave] (http)");
  }
  if (n == 0) {
    if (doHtml) {
      slavesInfo  = ",\"slaveInfo\": [";
      slavesInfo += "{\"Slave\": \"No Slaves\"";
      slavesInfo += ",\"IP\":\"Found\"}]";
    
    } else {
      _dThis = true;
      Debugln("no Slaves found");
    }
  } else {  // n > 0
    _dThis = true;
    if (!doHtml) {
      Debug(n);
      Debugln(" Slave(s) found:");
    }
    for (int i = 0; i < n; ++i) {
      _dThis = true;
      if (!doHtml) {
        Debug((i + 1));  
        Debug(" "); 
        Debug(MDNS.hostname(i)); 
        Debug("\tIPaddress: ");
        Debugln(MDNS.IP(i).toString());
      
      } else {
        if (i == 0) slavesInfo  = ",\"slaveInfo\": [";
        else        slavesInfo += ",";
        slavesInfo += "{\"Slave\":\"" + MDNS.hostname(i) + ".local\"";
        slavesInfo += ",\"IP\":\"" + MDNS.IP(i).toString() + "\"}";
      }
    }
    slavesInfo += "]";
  }

  return slavesInfo;
  
} // listAllSlaves()


//===========================================================================================
String getServerIP(uint8_t maxTries) {
//===========================================================================================
String first3NumServer, first3NumSlave;
int    n, tryCount = 0;

  _dThis = true;
  Debugln("sending mDNS query for [DONOFF.local] (http)");
  MDNS.update();
  
  first3NumServer = masterIPaddress.substring(0, masterIPaddress.lastIndexOf(".") - 1);
  first3NumSlave  = deviceIPaddress.substring(0, deviceIPaddress.lastIndexOf(".") - 1);
  
  while (tryCount < maxTries) {
    tryCount++;
    n = MDNS.queryService("dooMaster", "tcp"); // Send out query for esp tcp services
    _dThis = true;
    Debugf("[%d] mDNS query done", tryCount);
    if (n == 0) {
      Debugln(" no services found!");
    } else {
      Debug(" service(s) found ");
      for (int i = 0; i < n; ++i) {
        _dThis = true;
        // Print details for each service found
        Debug(i + 1);
        Debug(": ");
        Debug(MDNS.hostname(i));
        Debug(" (");
        Debug(MDNS.IP(i));
        Debug(":");
        Debug(MDNS.port(i));
        Debugln(")");
        return MDNS.IP(i).toString();
      }
    }
  } // tryCount

  //if (first3NumServer == first3NumSlave) return masterIPaddress;
  //return String(masterHostName + ".local");
  return "0.0.0.0";

} // getServerIP()


//===========================================================================================
void checkHeartBeat() {
//===========================================================================================
  static uint32_t checkInterval = millis() + (_HEARTBEAT_INTERVAL * 1000);
  int8_t  d;

  if (deviceIsMaster && millis() > checkInterval) {
    checkInterval += (_HEARTBEAT_INTERVAL * 1000); // _HEARTBEAT is in seconds!  
    deviceArray[0].lastSeen = now();  // local Device
    for(d=1; d <= noOfDevices; d++) {
      _dThis = true;
      Debugf("Now testing [%d] (%s) => now()[%ld] -/- lastSeen[%ld] == [%ld] > [%ld]?\n"
                                      , d, deviceArray[d].Label.c_str()
                                      , now(), deviceArray[d].lastSeen
                                      ,(now() - deviceArray[d].lastSeen)
                                      , (2 * _HEARTBEAT_INTERVAL));  
      if ((now() - deviceArray[d].lastSeen) > (2 * _HEARTBEAT_INTERVAL)) { 
        doUpdateDOM  = true;
        for(int8_t r=d; r < noOfDevices; r++) {
          _dThis = true;
          Debugf("[%d] (%s) := [%d] (%s)\n", r, deviceArray[r].Label.c_str()
                                           , (r+1), deviceArray[r+1].Label.c_str()); 
          deviceArray[r]  = deviceArray[r+1];
        }
        d--;
        noOfDevices--;
      }
    }
    _dThis = true;
    Debugf("number of devices left [%d]\n", (noOfDevices + 1));
  }

  if (doUpdateDOM)  updateDOM();
      
} //  checkHeartBeat()


//===========================================================================================
void sendHTTPrequest(String serverIP, int8_t deviceInx) {
//===========================================================================================
    HTTPClient http; 

    statusChanged = false;

    if (deviceIsMaster && deviceInx == 0) { // master's local device, no need to request
      _dThis = true;
      Debugln("==> master's local device, Skip!");
      return;
    }
    if (serverIP == "0.0.0.0")  {         // not a valid IP address
      masterIPaddress = getServerIP(1);
      if (masterIPaddress == "0.0.0.0") { // still not valid, no use requesting
        _dThis = true;
        Debugln("==> no DONOFF master, Skip! ");
        return;
      }
      writeConfig();                      // save masterIP to config
      serverIP = masterIPaddress;         // use new found masterIP to request
    }

    sprintf(cMsg,"IPaddress=%s&Label=%s&Type=%c&minState=%d&maxState=%d&State=%d&OnOff=%d"
                                                        , deviceArray[deviceInx].IPaddress.c_str()
                                                        , deviceArray[deviceInx].Label.c_str()
                                                        , deviceArray[deviceInx].Type
                                                        , deviceArray[deviceInx].minState
                                                        , deviceArray[deviceInx].maxState
                                                        , deviceArray[deviceInx].State
                                                        , deviceArray[deviceInx].OnOff);
    String URL = "http://" + serverIP + "/slaveState";
    _dThis = true;
    Debug(URL);
    Debug("?");
    Debugln(cMsg);
     
    http.begin(URL);              // Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
 
    int16_t httpCode = http.POST(cMsg);   //Send the request
    String cMsg = http.getString();    //Get the response payload
 
    _dThis = true;
    Debugf("httpCode     [%d]", httpCode);        //Print HTTP return code
    if (httpCode == 200)  { 
      Debugln(" OK");      
      deviceArray[deviceInx].lastSeen = now(); 
    } else { 
      Debugln(" Error!!!"); 
      deviceArray[deviceInx].lastSeen -= 60;
      if (!deviceIsMaster) {
        masterIPaddress = getServerIP(2);
        if (masterIPaddress != "0.0.0.0") { // found a valid master-IP
          _dThis = true;
          Debugln("==> DONOFF master found! Save config .. ");
          writeConfig();                      // save masterIP to config
        }
      }
    }
 
    http.end();  //Close connection    server.send(200, "text/plain", responseURL);

}   // sendHTTPrequest()


//===========================================================================================
void setup() {
//===========================================================================================
  Serial.begin(115200);
  delay(10);

  _dThis = true;
  Debugln("\nStart DONOFF system");
  localDeviceGPIO = LED_BUILTIN;  
  localBuiltInLed = LED_BUILTIN;  

//================ SPIFFS =========================================
  if (!SPIFFS.begin()) {
    _dThis = true;
    Debugln("SPIFFS Mount failed");   // Serious problem with SPIFFS 
    SPIFFSmounted = false;
    
  } else { 
    _dThis = true;
    Debugln("SPIFFS Mount succesfull");
    SPIFFSmounted = true;
  }
 //=============end SPIFFS =========================================

  readConfig();
  noOfDevices = 0;
  deviceArray[0].deviceHostName = deviceHostName;
  deviceArray[0].IPaddress      = deviceIPaddress;
  deviceArray[0].Label          = deviceLabel;
  deviceArray[0].Type           = deviceType;
  deviceArray[0].minState       = deviceMinState;
  deviceArray[0].maxState       = deviceMaxState;
  deviceArray[0].State          = deviceDefaultState;
  deviceArray[0].OnOff          = 0;
  
  //writeConfig();

  if (localSwitchGPIO >= 0) {
    pinMode(localSwitchGPIO, INPUT);
  }
  pinMode(localDeviceGPIO, OUTPUT);
  if (localBuiltInLed >= 0) {
    pinMode(localBuiltInLed, OUTPUT);
  
    for(int I=0; I<3; I++) {
      digitalWrite(localBuiltInLed, !digitalRead(localBuiltInLed));
      delay(2000);
    }
    digitalWrite(localBuiltInLed, _LED_OFF);  // HIGH is OFF
  }
  sprintf(cMsg, "Last reset reason: [%s]", ESP.getResetReason().c_str());
  _dThis = true;
  Debugln(cMsg);
  DebugFlush();
  
  if (localBuiltInLed >= 0) {
    digitalWrite(localBuiltInLed, _LED_ON);
  }

  startWiFi();
  if (localBuiltInLed >= 0) {
    for (int L=0; L < 10; L++) {
      digitalWrite(localBuiltInLed, !digitalRead(localBuiltInLed));
      delay(200);
    }
    digitalWrite(localBuiltInLed, _LED_OFF);
  }
  
  startTelnet();
  startArduinoOTA();

/*  
 *   list all services with the cammand:
 *   dns-sd -B _dooSlave .
 *   dns-sd -B _dooMaster .
*/
  if (MDNS.begin(deviceHostName.c_str())) {              // Start the mDNS responder for thisDevice.local
    _dThis = true;
    Debugf("[1] mDNS responder started as [%s.local]\n", deviceHostName.c_str());
    if (deviceIsMaster) MDNS.addService("dooMaster", "tcp", 8080);
    else                MDNS.addService("dooSlave", "tcp", 8080);
    MDNS.addService("arduino", "tcp", 81);
  } else {
    _dThis = true;
    Debugln("[1] Error setting up MDNS responder!");
  }
  MDNS.port(81);
  MDNS.port(8080);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Save the IP address
  deviceIPaddress = WiFi.localIP().toString();
  if (deviceLabel == "?") {
    deviceLabel = WiFi.localIP().toString();
    deviceArray[0].Label = deviceLabel;
  }
  
  if (deviceIsMaster) {
    _dThis = true;
    Debugln("Starting UDP");
    Udp.begin(localPort);
    _dThis = true;
    Debug("Local port: ");
    Debugln(String(Udp.localPort()));
    bool timeNotSet = true;
    while (timeNotSet) {
      _dThis = true;
      Debugln("waiting for sync on NTP server");
      setSyncProvider(getNtpTime);
      if (now() > 1000) {
        timeNotSet = false;
      }
      _dThis = true;
      Debugf("NTP time: %02d:%02d:%02d\n", hour(), minute(), second());
      setSyncInterval(600);
    }
    deviceArray[0].lastSeen       = now();
  }

  if (deviceIsMaster) {
    // add some dummy Slave's ====================================
    noOfDevices++;
    deviceArray[noOfDevices].IPaddress  = "192.168.12.31";
    deviceArray[noOfDevices].Label      = "Dummy Keuken (S)";
    deviceArray[noOfDevices].Type       = 'S';
    deviceArray[noOfDevices].minState   = 0;
    deviceArray[noOfDevices].maxState   = 1;
    deviceArray[noOfDevices].State      = 1;
    deviceArray[noOfDevices].lastSeen   = now();
    noOfDevices++;
    deviceArray[noOfDevices].IPaddress  = "192.168.12.30";
    deviceArray[noOfDevices].Label      = "Dummy Zolder (D)";
    deviceArray[noOfDevices].Type       = 'D';
    deviceArray[noOfDevices].minState   = 1;
    deviceArray[noOfDevices].maxState   = 99;
    deviceArray[noOfDevices].State      = 1;
    deviceArray[noOfDevices].lastSeen   = now();
  }

  if (SPIFFS.exists(_CONFIG_FILE)) {
    if (SPIFFS.exists("/index.html")) {
      server.serveStatic("/",           SPIFFS, "/index.html");
    } else {
      server.on("/", handleErrorIndex);
    }
  } else {  // no config file found ...
    if (SPIFFS.exists("/admin.html")) { // admin.html found, link to that page
      server.serveStatic("/",        SPIFFS, "/admin.html");
    } else {
      server.on("/", handleMaintenance);  // no config, no index, no admin???
    }
  }
  server.serveStatic("/admin",        SPIFFS, "/admin.html");
  server.serveStatic("/admin.png",    SPIFFS, "/admin_sm.png");
  server.serveStatic("/lightOn.ico",  SPIFFS, "/lightOn.ico");
  server.serveStatic("/lightOff.ico", SPIFFS, "/lightOff.ico");

  server.on("/updateDOM",       updateDOM);
  server.on("/slaveState",      handleSlaveState);

  server.on("/ReBoot", HTTP_POST, handleReBoot);

  server.on("/adminStatus.json",  handleAdminStatus);
//server.on("/admin/status",    handleAdminStatus);
//server.on("/admin/save",      handleAdminSave); // <-- handled in onNotFound()!
  server.on("/maintenance",     handleMaintenance);
  server.on("/fileDelete", HTTP_POST, handleFileDelete);
  server.on("/fileUpload", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  // server.on() does not alway's catch the right URL but if all server.on() fails 
  // it will start the server.onNotFound() function that's why I try to find 
  // what URL was send in onNotFound() and call the appropriate function
  server.onNotFound([]() {
    String message = "[" + server.uri() + "]\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += ", " + server.argName(i) + ":'" + server.arg(i) + "'";
      if (server.arg(i) == "Save Configuration") {
        _dThis = true;
        Debugln("Save new config ...");
        handleAdminSave();
      }
      if (server.arg(i) == "Back to Admin page") {
        _dThis = true;
        Debugln("Back to Admin page ...");
        server.sendHeader("Location", "/admin", true);   // Redirect to the admin web page  
        server.send(302, "text/plane","");    
      }
    }
    _dThis = true;
    Debugf("onNotFound(): %s\n", message.c_str()); 
    if (server.uri().indexOf("save") > 0) {
      _dThis = true;
      Debug("This should endup in the admin/save page .. "); 
      if (server.args() > 0) {
        Debug(" .. why not?"); 
        handleAdminSave();
      }
      Debugln("!");
      _dThis = true;
      Debugln("This should endup in the admin page .. why not?"); 
      server.sendHeader("Location", "/admin", true);   // Redirect to the admin web page  
      server.send(302, "text/plane","");    
    }
    if (server.uri() == "/") {
      statusChanged = true;
      server.sendHeader("Location", "/", true);   // Redirect to our html web page  
      server.send(302, "text/plane","");    
    }
    if (server.uri().indexOf("ReBoot") > 0) {
      handleReBoot();  
    }
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  server.begin();

  _dThis = true;
  Debugln("HTTP server started");

  if (!deviceIsMaster) masterIPaddress = getServerIP(10);

  _dThis = true;
  Debugf("Gataway IP: %s\n", WiFi.gatewayIP().toString().c_str());
 
  if (deviceArray[0].Type == 'D') {
    analogWriteRange(100);
    analogWriteFreq(localPWMfreq);
  } 
  setLocalDevice();

  statusChanged = true;
  updateDac     = millis() + 10000;
  heapInterval  = millis() + 2000;  
  aliveTime     = millis();
  
} // setup()


//===========================================================================================
void loop() {
//===========================================================================================
  
  ArduinoOTA.handle();
  server.handleClient();
  webSocket.loop();
  handleLocalSwitch(masterIPaddress, deviceArray[0], switchHandle(localSwitchGPIO));
  handleAnimate();
  handleKeyInput(); // menu
  checkHeartBeat();

  
  if (doShowHeap && millis() > heapInterval) {
    heapInterval += 5000;
    freeHeap = ESP.getFreeHeap();
    if (freeHeap < minHeap) minHeap = freeHeap;
    if (freeHeap > maxHeap) maxHeap = freeHeap;
    _dThis = true;
    Debugf("FreeHeap min[%ld], max[%ld], now %ld bytes\n", minHeap, maxHeap, freeHeap);
  }

  /******************* send Heartbeat to Master *********************/
  if (!deviceIsMaster && millis() > aliveTime) {
    _dThis = true;
    Debugf("Send Heartbeat to master @%s\n", masterIPaddress.c_str());
    statusChanged = true;
    if (masterIPaddress.indexOf("DONOFF.local") > 0 ) {
      masterIPaddress = getServerIP(10);
      _dThis = true;
      Debugf("deviceHostName: [%s.local] => IP[%s]\n", deviceArray[0].deviceHostName.c_str(), deviceIPaddress.c_str());
    }
    aliveTime += _HEARTBEAT_INTERVAL * 1000; 
    sendHTTPrequest(masterIPaddress, 0);

  }

} // loop()



/***************************************************************************
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the
* following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
* OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* 
***************************************************************************/
