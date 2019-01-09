/*
***************************************************************************  
**  Program  : configStuff, part of DONOFF
**  Version  : v0.3.5
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

//=======================================================================
void splitString(String inStrng, char delimiter, String wOut[], uint8_t maxWords) {
//=======================================================================
  int8_t inxS = 0, inxE = 0, wordCount = 0;

  //Debugf("split() : %s\n", inStrng.c_str());
    while(inxE < inStrng.length() && wordCount < maxWords) {
    //Debugf("split() : wordCount[%d], S[%d], E[%d] =", wordCount, inxS, inxE);
      inxE  = inStrng.indexOf(delimiter, inxS);             //finds location of first ,
      wOut[wordCount] = inStrng.substring(inxS, inxE);  //captures first data String
      wOut[wordCount].trim();
    //Debugf("=> %s\n", wOut[wordCount].c_str());
      inxS = inxE;
      inxS++;
      wordCount++;
    }
    if (inxS < inStrng.length()) {
      wOut[wordCount] = inStrng.substring(inxS, inStrng.length());  //captures first data String      
    }
    
} // splitString()


//=======================================================================
void writeConfig() {
//=======================================================================

  _dThis = true;
  Debugf(" ..\n", _CONFIG_FILE);

  File file = SPIFFS.open(_CONFIG_FILE, "w");

  file.print("deviceHostName     = ");  file.println(deviceHostName);
  file.print("deviceIPaddress    = ");  file.println(deviceIPaddress);
  file.print("deviceIsMaster     = "); 
  if (deviceIsMaster) {
    file.println("true");  // is Server
  } else {
    file.println("false");  // is Slave
  }
//file.print("masterHostName     = ");  file.println("DONOFF"); // this will never change!
  file.print("masterIPaddress    = ");  file.println(masterIPaddress);
  file.print("deviceLabel        = ");  file.println(deviceLabel);   
  file.print("localSwitchGPIO    = ");  file.println(localSwitchGPIO);   
  file.print("localSwitchToggle  = "); 
  if (localSwitchToggle) {
    file.println("true");  // is Toggle (On/Off)
  } else {
    file.println("false");  // is Push (momentarely)
  }
  file.print("deviceBuiltInLed   = ");  file.println(localBuiltInLed);   
  file.print("deviceGPIO         = ");  file.println(localDeviceGPIO);   
  file.print("deviceInverted     = "); 
  if (localDeviceInverted) {
    file.println("true");  // is inverted
  } else {
    file.println("false");  // is non-inverted
  }
  file.print("devicePWMfreq      = ");  file.println(localPWMfreq);   
  file.print("deviceType         = ");  file.println(deviceType);   
  file.print("deviceMinState     = ");  file.println(deviceMinState);  
  file.print("deviceMaxState     = ");  file.println(deviceMaxState);  
  file.print("deviceDefaultState = ");  file.println(deviceDefaultState);  

  file.close();  
  
  deviceFirstStart = false;
  
  Debugln(" .. done");

} // writeConfig()


//=======================================================================
void readConfig() {
//=======================================================================
  String sTmp;
  String words[10];
  int p = 0;

  Debugf(" ..\n", _CONFIG_FILE);

  masterHostName          = _MASTERDONOFF;

  if (!SPIFFS.exists(_CONFIG_FILE)) {
    deviceFirstStart        = true;
    deviceHostName          = "DONOFFnew";
    deviceIPaddress         = WiFi.localIP().toString();  // not likely to work .. first read there's no WiFi ..
    deviceIsMaster          = false;
    masterIPaddress         = "0.0.0.0";
    deviceLabel             = "?";
    localSwitchGPIO         = -1;
    localSwitchToggle       = true;
    localBuiltInLed         = 1;  // this is for the PCB with a ESP-01
    localSwitchGPIO         = 2;  // this is pin4 for the PCB with a ESP-01
    localDeviceGPIO         = 3;  // this is pin8 for the PCB with a ESP-01
    localDeviceInverted     = false;
    localPWMfreq            = 500;  // Hz
    deviceType              = 'D';
    deviceDefaultState      = 1;
    Debugln(" .. done (file not found!)");
    return;
  } else {
    deviceFirstStart        = false;
  }

  File file = SPIFFS.open(_CONFIG_FILE, "r");

  _dThis = true;
  while(file.available()) {
    sTmp                = file.readStringUntil('\n');
    sTmp.replace("\r", "");
    Debugf("[%s] \n", sTmp.c_str(), sTmp.length());
    splitString(sTmp, '=', words, 10);
  //Debugln("");
  //Debug(words[0].c_str()); Debug(" = "); Debugln(words[1].c_str());
    words[0].toLowerCase();
    if (words[0] == "devicehostname")     deviceHostName      = words[1];  
    if (words[0] == "deviceipaddress")    deviceIPaddress     = words[1];  
    if (words[0] == "deviceismaster") {
      if (words[1] == "true" || words[1] == "1") 
            deviceIsMaster = true;
      else  deviceIsMaster = false;
    }
  //if (words[0] == "masterhostname")     masterHostName      = words[1];  
    if (words[0] == "masterripaddress")   masterIPaddress     = words[1];  
    if (words[0] == "devicelabel")        deviceLabel         = words[1];  
    if (words[0] == "localswitchgpio")    localSwitchGPIO     = words[1].toInt();  
    if (words[0] == "localswitchtoggle") {
      if (words[1] == "true" || words[1] == "1") 
            localSwitchToggle = true;
      else  localSwitchToggle = false;
    }
    if (words[0] == "devicebuiltinled")   localBuiltInLed     = words[1].toInt();  
    if (words[0] == "devicegpio")         localDeviceGPIO     = words[1].toInt();  
    if (words[0] == "deviceinverted") {
      if (words[1] == "true" || words[1] == "1") 
            localDeviceInverted = true;
      else  localDeviceInverted = false;
    }
    if (words[0] == "devicepwmfreq")      localPWMfreq        = words[1].toInt();  
    if (words[0] == "deviceminstate")     deviceMinState      = words[1].toInt();  
    if (words[0] == "devicemaxstate")     deviceMaxState      = words[1].toInt();  
    if (words[0] == "devicetype") {
      if (words[1] == "D")
            deviceType = 'D';  
      else  deviceType = 'S';  
    }
    if (words[0] == "devicedefaultstate") deviceDefaultState  = words[1].toInt();  
    
  } // while available()

  Debugln();
  Debug("hostName     : "); Debugf("%s.local\n", deviceHostName.c_str());
  Debug("IPaddress    : "); Debugln(deviceIPaddress);
  Debug("Label        : "); Debugln(deviceLabel);
  if (deviceIsMaster) { Debugln("device Role  : Master"); }
  else                { Debugln("device Role  : Slave"); }
  Debug("masterName   : "); Debugf("%s.local\n", masterHostName.c_str());
  Debug("masterIP     : "); Debugln(masterIPaddress);
  if (deviceType == 'D')        { Debugln("Type         : Dimmer"); }
  else if (deviceType == 'S')   { Debugln("Type         : Switch"); }
  else                          { Debugf("Type         : %c\n", deviceType); }
  Debug("Switch pin   : "); Debug(localSwitchGPIO);
  if (localSwitchToggle)  { Debugln(" (Toggle)"); }
  else                    { Debugln(" (Push)");   }
  Debug("Built in Led : "); Debugln(localBuiltInLed);
  Debug("GPIO pin     : "); Debug(localDeviceGPIO);
  if (localDeviceInverted)      { Debugln(" (inverted)"); }
  else                          { Debugln(" (non-inverted)"); }
  Debug("PWM freq.    : "); Debugln(localPWMfreq);
  Debug("min State    : "); Debugln(deviceMinState);
  Debug("max State    : "); Debugln(deviceMaxState);
  Debug("Default State: "); Debugln(deviceDefaultState);
  
  file.close();  
  Debugln(" .. done");
  DebugFlush();

} // readConfig()

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
