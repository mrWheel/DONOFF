/*
***************************************************************************  
**  Program  : menuStuff, part of DONOFF
**  Version  : v0.3.5
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/
//===========================================================================================
int32_t freeSpace() {
//===========================================================================================
  int32_t space;
  
  SPIFFS.info(SPIFFSinfo);

  space = (int32_t)(SPIFFSinfo.totalBytes - SPIFFSinfo.usedBytes);

  return space;
  
} // freeSpace()

//===========================================================================================
void listSPIFFS() {
//===========================================================================================
  Dir dir = SPIFFS.openDir("/");

  _dThis = true;
  Debugln();
  while (dir.next()) {
    File f = dir.openFile("r");
    Debugf("%-15s %ld \r\n", dir.fileName().c_str(), f.size());
    yield();
  }
  TelnetStream.flush();

  SPIFFS.info(SPIFFSinfo);

  int32_t space = (int32_t)(SPIFFSinfo.totalBytes - SPIFFSinfo.usedBytes);
  Debugln("\r");
  Debugf("Available SPIFFS space [%ld]bytes\r\n", freeSpace());
  Debugf("           SPIFFS Size [%ld]kB\r\n", (SPIFFSinfo.totalBytes / 1024));
  Debugf("     SPIFFS block Size [%ld]bytes\r\n", SPIFFSinfo.blockSize);
  Debugf("      SPIFFS page Size [%ld]bytes\r\n", SPIFFSinfo.pageSize);
  Debugf(" SPIFFS max.Open Files [%ld]\r\n\n", SPIFFSinfo.maxOpenFiles);


} // listSPIFFS()


//=======================================================================
void showStatus() {
//=======================================================================
  char cMsg[150];
  Debug("\r\n===========================================================================================\r");
  Debug(" \r\n               (c)2019 by [Willem Aandewiel");
  Debug("]\r\n         Firmware Version [");  Debug( _FW_VERSION );
  Debug("]\r\n                 Compiled [");  Debug( __DATE__ ); 
                                              Debug( "  " );
                                              Debug( __TIME__ );
  Debug("]\r\n                 FreeHeap [");  Debug( String(ESP.getFreeHeap()).c_str() );
  Debug("]\r\n                 Hostname [");  Debug( deviceHostName.c_str() );
  Debug("]\r\n               IP address [");  Debug( deviceIPaddress.c_str() );
  if (deviceIsMaster) 
       { Debug("]\r\n              device Role [Master"); }
  else { Debug("]\r\n              device Role [Slave"); }
  Debug("]\r\n        local light Label [");  Debug( deviceLabel.c_str() );
  Debug("]\r\n        local device Type [");  
  if (deviceType == 'D')      Debug( "Dimmer" );
  else if (deviceType == 'S') Debug( "Switch" );
  else                        Debug( "UnKnown" );
  if (localSwitchGPIO >= 0) {
    Debug("]\r\n             local Switch [GPIO");  Debug( localSwitchGPIO );
    if (localSwitchToggle)  { Debug( " (toggle)" );     }
    else                    { Debug( " (Pushbutton)" ); }
  }
  Debug("]\r\n              builtin Led [GPIO");  Debug( localBuiltInLed );
  Debug("]\r\n           device PWM out [GPIO");  Debug( localDeviceGPIO );
  Debug("]\r\n            PWM frequency [");  Debug( localPWMfreq );      Debug(" Hz");
  Debug("]\r\n          local Min State [");  Debug( deviceMinState );    Debug("%");
  Debug("]\r\n          local Max State [");  Debug( deviceMaxState );    Debug("%");
  Debug("]\r\n      local Default State [");  Debug( deviceDefaultState ); Debug("%");
  if (!deviceIsMaster) {
    Debug("]\r\n          master HostName [");  Debug( masterHostName.c_str() );
    Debug("]\r\n         master IPaddress [");  Debug( masterIPaddress.c_str() );
  } else {
    for (int d=1; d <= noOfDevices; d++) {
      Debug("]\r\n  slave device IPaddress [");  Debug( deviceArray[d].IPaddress.c_str() );
                                                  Debug("], Label ["); Debug(deviceArray[d].Label.c_str());
                                                  Debug("], Type [");  Debug( deviceArray[d].Type);
                                                  Debug("], min [");   Debug( deviceArray[d].minState);
                                                  Debug("], max [");   Debug( deviceArray[d].maxState);
                                                  Debug("], State ["); Debug(deviceArray[d].State);
    }
  }

  Debug("]\r\n");

  Debug("===========================================================================================\r\n");
  sprintf(cMsg, "%08X", String( ESP.getChipId(), HEX ).c_str() );

  Debug( "\r\n                  Chip ID [");  Debug( cMsg );
  Debug("]\r\n             Core Version [");  Debug( String( ESP.getCoreVersion() ).c_str() );
  Debug("]\r\n              SDK Version [");  Debug( String( ESP.getSdkVersion() ).c_str() );
  Debug("]\r\n           CPU Freq (MHz) [");  Debug( String( ESP.getCpuFreqMHz() ).c_str() );
  Debug("]\r\n         Sketch Size (kB) [");  Debug( String( ESP.getSketchSize() / 1024.0 ).c_str() );
  Debug("]\r\n   Free Sketch Space (kB) [");  Debug( String( ESP.getFreeSketchSpace() / 1024.0 ).c_str() );

  if ((ESP.getFlashChipId() & 0x000000ff) == 0x85) 
        sprintf(cMsg, "%08X (PUYA)", ESP.getFlashChipId());
  else  sprintf(cMsg, "%08X", ESP.getFlashChipId());
  Debug("]\r\n            Flash Chip ID [");  Debug( cMsg );
  Debug("]\r\n     Flash Chip Size (kB) [");  Debug( String( ESP.getFlashChipSize() / 1024 ).c_str() );
  Debug("]\r\nFlash Chip Real Size (kB) [");  Debug( String( ESP.getFlashChipRealSize() / 1024 ).c_str() );
  Debug("]\r\n    Flash Chip Speed (MHz)[");  Debug( String( ESP.getFlashChipSpeed() / 1000 / 1000 ).c_str() );
  FlashMode_t ideMode = ESP.getFlashChipMode();
  Debug("]\r\n          Flash Chip Mode [");  Debug( flashMode[ideMode] );

  Debug("]\r\n");

  Debug("==================================================================\r\n");
  Debug(" \r\n               Board type [");
#ifdef ARDUINO_ESP8266_NODEMCU
    Debug("ESP8266_NODEMCU");
#endif
#ifdef ARDUINO_ESP8266_GENERIC
    Debug("ESP8266_GENERIC");
#endif
#ifdef ESP8266_ESP01
    Debug("ESP8266_ESP01");
#endif
#ifdef ESP8266_ESP12
    Debug("ESP8266_ESP12");
#endif
  Debug("]\r\n                     SSID [");  Debug( WiFi.SSID() );
  Debug("]\r\n                  PSK key [");  Debug( WiFi.psk() );
  Debug("]\r\n               IP Address [");  Debug( WiFi.localIP().toString() );
//Debug("]\r\n                   upTime [");  Debug( uptime() );
  Debug("]\r\n");
  Debug("==================================================================\r\n");
  DebugFlush();
  
} // showStatus()


//=======================================================================
void handleAnimate() {
//=======================================================================
  String response;
  static uint32_t nextStep = millis();

  if (!animateDimUpDown || nextStep > millis()) {  // not yet time to make a next step
    return;
  }
  nextStep += 500;
  
  if (deviceArray[0].Type != 'D') {
    animateDimUpDown = false;
    return;
  }

  deviceArray[0].State += upDown; 
  if (deviceArray[0].State <= 0) {
    deviceArray[0].State = 0;
    upDown = 0 - upDown;
  }
  if (deviceArray[0].State >= 100) {
    deviceArray[0].State = 100;
    upDown = 0 - upDown;
  }
  deviceArray[0].OnOff = 1;
  _dThis = true;
  Debug("Dimlevel: ");
  Debugln(deviceArray[0].State);
  setLocalDevice();
  handleStatus();
  response = "dimmer";
  response += "," + String(0);
  response += "," + String(deviceArray[0].Type);
  response += "," + String(deviceArray[0].State);
  response += "," + String(deviceArray[0].OnOff);
  webSocket.broadcastTXT(response);  // tell all other browser-clients
  
} // handleAnimate()


//=======================================================================
void handleKeyInput() {
//=======================================================================
  String slavesInfo;  
  
  while (TelnetStream.available() > 0) {
    yield();
    inChar = (char)TelnetStream.read();
    while (TelnetStream.available() > 0) {
       yield();
       (char)TelnetStream.read();
    }

    switch(inChar) {
      case 'b':
      case 'B':     showStatus();
                    break;
      case 'c':
      case 'C':     slavesInfo = listAllSlaves(false);
                    Debugln(slavesInfo);
                    break;
      case 'd':
      case 'D':     if (animateDimUpDown) {
                      animateDimUpDown = false;
                    } else {
                      animateDimUpDown = true;
                    }
                    break;
      case 'h':
      case 'H':     doShowHeap = !doShowHeap;
                    break;
      case 'l':
      case 'L':     readConfig();
                    break;
      case 'R':     ESP.reset();
                    break;
      case 's':
      case 'S':     listSPIFFS();
                    break;
      case 'W':     { WiFiManager manageWiFi;
                      _dThis = true;
                      Debugln("**============================**");
                      Debugln("** RESETTING WIFI CREDENTIALS **");
                      Debugln("**============================**");
                      DebugFlush();
                      manageWiFi.resetSettings();   
                      ESP.reset();
                    }
                    break;
      default:      Debugln("\nCommandos are:\n");
                    Debugln("   B - Board, Build info & System Status");
                    Debugln("   C - List All Slaves");
                    if (animateDimUpDown) {
                      Debugln("   D - Stop dim-Up/Down");
                    } else {
                      Debugln("   D - Start dim-Up/Down");
                    }                    
                    Debug("   H - Toggle Heap Trace "); 
                    if (doShowHeap) { Debugln("OFF"); }
                    else            { Debugln("ON");  }
                    Debugln("   L - Load donoff.cfg from SPIFFS");
                    Debugln("  *R - Reboot");
                    Debugln("   S - list SPIFFS");
                    Debugln("  *W - reset WiFi credentials");
                    Debugln(" ");

    } // switch()
  }
  
} // handleKeyInput()

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
