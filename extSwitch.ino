/*
***************************************************************************  
**  Program  : extSwitch, part of DONOFF
**  Version  : v0.3.6
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/


enum {
  NOTPRESSED,
  INTRANSITION,
  SHORTPRESSED,
  SHORTRELEASED,
  LONGPRESSED,
  LONGRELEASED
};

uint32_t  switchTime, resetTimer;
uint8_t   switchState = NOTPRESSED;
int8_t    upDown = 5;


uint8_t switchHandle(int extSwitchGPIO) {

  if (extSwitchGPIO < 0) {
    return NOTPRESSED;
  }
  
  bool pressed = !digitalRead(extSwitchGPIO);
  
  if (!pressed) { // switch not pressed
    switchTime = 0;
    resetTimer = 0;
    if (switchState != NOTPRESSED) {
      if (switchState == SHORTPRESSED) {
        _dThis = true;
        Debugln("switch SHORT release");
        switchState = NOTPRESSED;
        return SHORTRELEASED;
      }
      if (switchState == LONGPRESSED) {
        _dThis = true;
        Debugln("switch LONG release");
        switchState = NOTPRESSED;
        return LONGRELEASED;
      }
    }
    switchState = NOTPRESSED;
    return NOTPRESSED;
  }
  /* from here on switched is pressed */
  if (switchState == NOTPRESSED) {
     switchTime   = millis();
     resetTimer   = 0;
     switchState  = INTRANSITION;
  }

  switch(switchState) {
      case INTRANSITION:  if ((millis() - switchTime) > 10) {
                            //Debugln("Short press ..");
                            switchState = SHORTPRESSED;
                            return SHORTPRESSED;
                          }
                          break;
      case SHORTPRESSED:  if ((millis() - switchTime) > 1000) {
                            _dThis = true;
                            Debugln("Long Pressed ..");
                            switchState = LONGPRESSED;
                            switchTime  = millis();
                            resetTimer  = millis();
                            return LONGPRESSED;
                          }
                          break;
      case LONGPRESSED:   if ((millis() - switchTime) > 500) {
                            _dThis = true;
                            Debugln("Long Pressed ..");
                            switchState = LONGPRESSED;
                            switchTime  = millis();
                            if ((resetTimer > 0) && (millis() - resetTimer) > 35000) {  // reset na 40 seconden!
                              _dThis = true;
                              Debugf("Reset device!! [%d]\n", (millis() - resetTimer));
                              bool State = false;
                              digitalWrite(localDeviceGPIO, LOW);        // switch
                              for (int i = 0; i< 6; i++) {
                                digitalWrite(localDeviceGPIO, !digitalRead(localDeviceGPIO));        // switch
                                delay(1000);
                              }
                              _dThis = true;
                              Debugln("\n\n=========================================================================");
                              Debugf("Butten pressed for [%d] seconds .. Reset device!!\n", (millis() - resetTimer));
                              Debugln("=========================================================================\n");
                              DebugFlush();
                              delay(500);
                              ESP.reset();
                            }
                            return LONGPRESSED;
                          }
  } // switch()
  DebugFlush();

  return NOTPRESSED;
  
} // switchHandle()


//===========================================================================================
void handleLocalSwitch(String IPaddress, device_type &localDevice, int16_t swState) {
//===========================================================================================
  String response;
  char xMsg[10];
  
  switch (swState) {
    case SHORTRELEASED: if (localDevice.OnOff == 0)  localDevice.OnOff = 1;
                        else                         localDevice.OnOff = 0;
                        setLocalDevice();
                        handleStatus();
                        if (localDevice.Type == 'D')  response = "dimmer";
                        else                          response = "switch";
                        response += "," + String(0);
                        response += "," + String(localDevice.Type);
                        response += "," + String(localDevice.State);
                        response += "," + String(localDevice.OnOff);
                        webSocket.broadcastTXT(response);  // tell all other browser-clients
                        // continue with LONGRELEASED                    
                            
    case LONGRELEASED:  upDown = 0 - upDown; _dThis = true; Debug("upDown: "); Debugln(upDown); 
                        if (!deviceIsMaster) {
                          sendHTTPrequest(IPaddress.c_str(), 0);
                        }
                        animateDimUpDown = false;
                        break;

    case LONGPRESSED:   localDevice.State += upDown; 
                        if (localDevice.State < localDevice.minState) localDevice.State = localDevice.minState;
                        if (localDevice.State > localDevice.maxState) localDevice.State = localDevice.maxState;
                        localDevice.OnOff = 1;
                        _dThis = true;
                        Debug("Dimlevel: ");
                        Debugln(localDevice.State);
                        setLocalDevice();
                        handleStatus();
                        if (localDevice.Type == 'D')  response = "dimmer";
                        else                          response = "switch";
                        response += "," + String(0);
                        response += "," + String(localDevice.Type);
                        response += "," + String(localDevice.State);
                        response += "," + String(localDevice.OnOff);
                        webSocket.broadcastTXT(response);  // tell all other browser-clients
                        animateDimUpDown = false;
                        break;
  } // switch(handle)
  
} // handleLocalSwitch()


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
