/*
***************************************************************************  
**  Program  : adminStuff, part of DONOFF
**  Version  : v0.3.5
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

//File fsUploadFile;                      // Stores de actuele upload

void handleAdmin() {                    // HTML admin
  String masterInfo, slavesInfo;

  _dThis = true;
  Debugf("FreeHeap [%d]\n", ESP.getFreeHeap() );

  if (deviceIsMaster) masterInfo = WiFi.localIP().toString();
  else                masterInfo = getServerIP(5);
  slavesInfo = listAllSlaves(true);
  
  _dThis = true;
  Debugf("[Exit]: FreeHeap [%d]\n", ESP.getFreeHeap() );

} // handleAdmin()


void handleReBoot() {
  String redirectHTML = "";

  redirectHTML += "<!DOCTYPE HTML><html lang='en-US'>";
  redirectHTML += "<head>";
  redirectHTML += "<meta charset='UTF-8'>";
  redirectHTML += "<style type='text/css'>";
  redirectHTML += "body {background-color: lightgray;}";
  redirectHTML += "</style>";
  redirectHTML += "<title>Redirect to LEDdimmer</title>";
  redirectHTML += "</head>";
  redirectHTML += "<body><h1>DONOFF Admin</h1>";
  redirectHTML += "<h3>Rebooting " + deviceHostName + "</h3>";
  redirectHTML += "<br><div style='width: 500px; position: relative; font-size: 25px;'>";
  redirectHTML += "  <div style='float: left;'>Redirect in &nbsp;</div>";
  redirectHTML += "  <div style='float: left;' id='counter'>15</div>";
  redirectHTML += "  <div style='float: left;'>&nbsp; seconds ...</div>";
  redirectHTML += "  <div style='float: right;'>&nbsp;</div>";
  redirectHTML += "</div>";
  redirectHTML += "<!-- Note: don't tell people to `click` the link, just tell them that it is a link. -->";
  redirectHTML += "<br><br><hr>If you are not redirected automatically, click this <a href='/'>" + deviceHostName + "</a>.";
  redirectHTML += "  <script>";
  redirectHTML += "      setInterval(function() {";
  redirectHTML += "          var div = document.querySelector('#counter');";
  redirectHTML += "          var count = div.textContent * 1 - 1;";
  redirectHTML += "          div.textContent = count;";
  redirectHTML += "          if (count <= 0) {";
  redirectHTML += "              window.location.replace('http://" + deviceHostName + ".local/'); ";
  redirectHTML += "          } ";
  redirectHTML += "      }, 1000); ";
  redirectHTML += "  </script> ";
  redirectHTML += "</body></html>\n";
  
  server.send(200, "text/html", redirectHTML);
  
  _dThis = true;
  Debugf("ReBoot %s ..", deviceHostName.c_str());
  DebugFlush();
  delay(1000);
  ESP.reset();
  
} // handleReBoot()


//===========================================================================================
void handleAdminStatus() {
//===========================================================================================
  String message, jsonString, masterInfo, slaveInfo;
  int16_t strIndex;
  static uint8_t count = 0;

  message = "";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += ", " + server.argName(i) + ":'" + server.arg(i) + "'";
  }
  _dThis = true;
  Debugf("[%s]\n", message.c_str());

  if (server.arg(0) == "Status") {
    jsonString = "{";
    jsonString += "  \"msgType\": \"Status\"";
    jsonString += ", \"hostName\":\"" + deviceHostName + "\"";
    jsonString += ", \"Role\": ";
    if (deviceIsMaster) jsonString += "\"M\"";
    else                jsonString += "\"S\"";
    jsonString += ", \"deviceIPaddress\":\"" + deviceIPaddress + "\"";
    jsonString += ", \"Label\":\"" + deviceLabel + "\"";
    jsonString += ", \"deviceType\":\"" + String(deviceType) + "\"";
    jsonString += ", \"defaultState\":\"" + String(deviceDefaultState) + "\"";
    jsonString += ", \"minState\":\"" + String(deviceMinState) + "\"";
    jsonString += ", \"maxState\":\"" + String(deviceMaxState) + "\"";
    jsonString += ", \"masterHostName\":\"" + masterHostName +"\"";
    jsonString += ", \"masterIPaddress\":\"" + masterIPaddress +"\"";
    jsonString += ", \"localBuiltInLed\":\"" + String(localBuiltInLed) + "\"";
    jsonString += ", \"localGPIO\":\"" + String(localDeviceGPIO) + "\"";
    jsonString += ", \"GPIONI\":";
    if (localDeviceInverted)  jsonString += "\"I\"";
    else                      jsonString += "\"N\"";
    jsonString += ", \"localPWMfreq\":\"" + String(localPWMfreq) + "\"";
    jsonString += ", \"localSwitchGPIO\":\"" + String(localSwitchGPIO) + "\"";
    jsonString += ", \"localSwitchToggle\":";
    if (localSwitchToggle)  jsonString += "\"T\"";    // Toggle (On/Off)
    else                    jsonString += "\"P\"";    // Push (momentarely)
    
    jsonString += "}";
  
  } else if (server.arg(0) == "masterInfo") {
    if (deviceIsMaster) masterInfo = WiFi.localIP().toString();
    else                masterInfo = getServerIP(5);
    if (masterInfo.indexOf("local") > 0) {
      jsonString = "{";
      jsonString += "  \"msgType\": \"masterInfo\"";
      jsonString += "}";
      
    } else {
      jsonString = "{";
      jsonString += "  \"msgType\": \"masterInfo\"";
      jsonString += ", \"masterInfo\":\"" + masterInfo + "\"";
      jsonString += "}";
    }
  } else if (server.arg(0) == "slaveInfo") {
    slaveInfo = listAllSlaves(true);
    jsonString = "{";
    jsonString += "  \"msgType\": \"slaveInfo\"";
    jsonString += slaveInfo;
    jsonString += "}";
  
  } else if (server.arg(0) == "FWversion") {
    jsonString = "{";
    jsonString += "  \"msgType\": \"FWversion\"";
    jsonString += ", \"FWversion\":\"";
    jsonString += String(_FW_VERSION);
    jsonString += "\", \"count\":\"";
    jsonString += String(count++);
    jsonString += "\"";
    jsonString += "}";
  } else {
    jsonString  = "{";
    jsonString += "\"msgType\": \"Error\"";
    jsonString += ", \"inReplyOf\":\"";
    jsonString += server.arg(0);
    jsonString += "\"";
    jsonString += "}";
    
  }

  _dThis = true;
  Debugln(jsonString);
  server.send(200, "text/html", jsonString);
    
} // handleAdminStatus()


//===========================================================================================
void handleAdminSave() {
//===========================================================================================
  String message, response, sTmp;

  message = server.uri();
  for (uint8_t i = 0; i < server.args(); i++) {
    message += ", " + server.argName(i) + ":'" + server.arg(i) + "'";
  }
  _dThis = true;
  Debugf("from: %s\n", message.c_str());

  _dThis = true;

  if (server.hasArg("hostName")) {
    deviceHostName = server.arg("hostName");
    Debugf("deviceHostName [%s] ", deviceHostName.c_str());
    mustReboot = true;
  } 
  
  if (server.hasArg("Role")) {
    if (server.arg("Role") == "M") {
      Debug("device Role [Master] ");
      deviceIsMaster = true;
      deviceHostName = "DONOFF";
    } else {
      Debug("device Role [Slave] ");
      deviceIsMaster = false;
      if (deviceHostName == "DONOFF") {
        String lastNumIP  = deviceIPaddress.substring(deviceIPaddress.lastIndexOf(".") + 1);
        deviceHostName += lastNumIP;  
        _dThis = true;
        Debugf("new hostName is [%s]\n", deviceHostName.c_str());
      }
      if (masterIPaddress == deviceIPaddress) {
        masterIPaddress = "0.0.0.0";
      }
    }
    mustReboot = true;
  } 
  
  if (server.hasArg("masterIPaddress")) {
      masterIPaddress = server.arg("masterIPaddress");
      Debug("masterIPaddress [");
      Debug(masterIPaddress);
      Debug("]  ");
  } 

  if (server.hasArg("deviceType")) {
    if (server.arg("deviceType") == "D") {
      Debug("device Type [Dimmer] ");
      deviceType = 'D';
    } else if (server.arg("deviceType") == "S") {
      Debug("device Type [Switch] ");
      deviceType = 'S';
    }
    deviceArray[0].Type = deviceType;
  } 

  if (server.hasArg("localSwitchGPIO")) {
    localSwitchGPIO = server.arg("localSwitchGPIO").toInt();
    if (localSwitchGPIO > 16) localSwitchGPIO = 16;
    if (localSwitchGPIO < -1) localSwitchGPIO = -1;
    Debugf("localSwitchGPIO [%d] ", localSwitchGPIO);
  } 
  
  if (server.hasArg("localSwitchToggle")) {
    if (server.arg("localSwitchToggle") == "T") {
      Debug("switch [Toggle] ");
      localSwitchToggle = true;
    } else {
      Debug("switch [Push] ");
      localSwitchToggle = false;
    }
  } 

  if (server.hasArg("localBuiltInLed")) {
    localBuiltInLed = server.arg("localBuiltInLed").toInt();
    if (localBuiltInLed > 16) localBuiltInLed = 16;
    if (localBuiltInLed < -1) localBuiltInLed = -1;
    Debugf("localBuiltInLed [%d] ", localBuiltInLed);
  } 

  if (server.hasArg("localGPIO")) {
    localDeviceGPIO = server.arg("localGPIO").toInt();
    if (localDeviceGPIO > 16) localDeviceGPIO = 16;
    if (localDeviceGPIO <  0) localDeviceGPIO =  0;
    Debugf("localDeviceGPIO [%d] ", localDeviceGPIO);
  } 
  if (server.hasArg("GPIONI")) {
    if (server.arg("GPIONI") == "I") {
      Debug("device [Inverted] ");
      localDeviceInverted = true;
    } else {
      Debug("device [Non-Inverted] ");
      localDeviceInverted = false;
    }
  } 

  if (server.hasArg("localPWMfreq")) {
    localPWMfreq = server.arg("localPWMfreq").toInt();
    if (localPWMfreq > 900) localPWMfreq = 900;
    if (localPWMfreq < 200) localPWMfreq = 200;
    Debugf("localPWMfreq [%d] ", localPWMfreq);
    analogWriteFreq(localPWMfreq);
  } 

  if (server.hasArg("Label")) {
    deviceLabel = server.arg("Label");
    Debugf("Label [%s] ", deviceLabel.c_str());
  } 

  if (server.hasArg("minState")) {
    deviceMinState = server.arg("minState").toInt();
    if (deviceType == 'S') deviceMinState = 0;
    Debugf("minState [%d] ", deviceMinState);
  } 

  if (server.hasArg("maxState")) {
    deviceMaxState = server.arg("maxState").toInt();
    if (deviceType == 'S') deviceMaxState = 1;
    Debugf("maxState [%d] ", deviceMaxState);
  } 

  if (server.hasArg("defaultState")) {
    deviceDefaultState = server.arg("defaultState").toInt();
    if (deviceDefaultState < 0)  deviceDefaultState = 0;
    if (deviceType == 'S' && deviceDefaultState > 0)  deviceDefaultState = 1;
    Debugf("defaultState [%d] ", deviceDefaultState);
  } 

  Debugln("!");
  DebugFlush();
  writeConfig();

  deviceArray[0].deviceHostName = deviceHostName;
  deviceArray[0].IPaddress      = deviceIPaddress;
  deviceArray[0].Label          = deviceLabel;
  deviceArray[0].Type           = deviceType;
  deviceArray[0].minState       = deviceMinState;
  deviceArray[0].maxState       = deviceMaxState;

  if (!deviceIsMaster)  aliveTime = millis(); 

  if (mustReboot) {
    _dThis = true;
    Debugln("Must reboot ..");
    DebugFlush();
    handleReBoot();
    return;
  }

  server.sendHeader("Location", String("/admin"), true);
  server.send ( 302, "text/plain", "");
  server.send(200, "text/html");
    
} // handleAdminSave()


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
