/*
***************************************************************************  
**  Program  : otherHTML, part of DONOFF
**  Version  : v0.3.5
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

String dimmerHTML(uint8_t vlgNr, String IPaddress, String Label, int8_t minS, int8_t maxS, int8_t OnOff, int16_t State) {
    String devHtml;

    devHtml  = "<div id='_wrapper'>\n";
    devHtml += "  <div id='label" + String(vlgNr) + "' tooltip='"+IPaddress+" ("+minS+"%-"+maxS+"%)'>" + Label + "</div>\n";
    devHtml += "  <div id='switch' style='text-align:left;'>\n";
    devHtml += "    <input type='hidden' name='dimmer' value='" + String(vlgNr) + "' id='dimmer" + String(vlgNr) + "' />\n";

    devHtml += "    <input type='hidden' name='donoff' value='"+ String(vlgNr) + "' id='donoff" + String(vlgNr) + "' />\n";
    devHtml += "    <input type='hidden' name='action' value='" + String(OnOff) + "' id='action" + String(vlgNr) + "' />\n";
    devHtml += "    <button";
    devHtml += "      onclick=\"sendDimmerOnOff(document.getElementById('donoff" + String(vlgNr) + "').value,";
    devHtml += "        document.getElementById('action" + String(vlgNr) + "').value);\"/>";
    devHtml += "        <span id='buttonLabel" + String(vlgNr) + "'>Dimmer <b>Off</b></span>";
    devHtml += "    </button>\n";
    devHtml += "  </div>\n";
    
    devHtml += "  <div id='slider' style='text-align:left;'>\n";
  //devHtml += "    <span style='font-size:small; color:gray;'>["+String(minS)+"%]</span> \n";
    devHtml += "    <input type='range' style='_slider'";
    devHtml += "         id='slider" + String(vlgNr) + "'";
    devHtml += "         min='" + String(minS) + "'";
    devHtml += "         max='" + String(maxS) + "'";
    devHtml += "         value='" + String(State) + "'";
    devHtml += "         step='1'";
    devHtml += "         onchange=\"sendDimmerValue(document.getElementById('dimmer" + String(vlgNr) + "').value,";
    devHtml += "         document.getElementById('slider" + String(vlgNr) + "').value);\"/>";
  //devHtml += "    <span style='font-size:small;color:gray;'>["+String(maxS)+"%]</span> \n";
    devHtml += "  </div>\n";
    
    devHtml += "  <div id='icon'>";
    devHtml += "    <span id='deviceIcon" + String(vlgNr) + "'><img src='/lightOff.ico' alt='Off'></span>\n";
    devHtml += "  </div>\n";

    devHtml += "  <div id='state'>\n";
    devHtml += "    <span id='dimValue" + String(vlgNr) + "'>-</span>%";
    devHtml += "    <span id='hartBeat" + String(vlgNr) + "'>-</span>";
    devHtml += "  </div>\n";
    devHtml += "</div>\n";

    return devHtml;

} // dimmerHTML()


String switchHTML(uint8_t vlgNr, String IPaddress, String Label, int16_t State) {
  String devHtml;
  
    devHtml  = "<div id='_wrapper'>\n";
    devHtml += "  <div id='label" + String(vlgNr) + "' tooltip='"+IPaddress+"'>" + Label + "</div>\n";
    devHtml += "  <div id='switch' style='text-align:left;'>\n";
    devHtml += "    <input type='hidden' name='switch' value='"+ String(vlgNr) + "' id='switch" + String(vlgNr) + "' />\n";
    devHtml += "    <input type='hidden' name='action' value='" + String(State) + "' id='action" + String(vlgNr) + "' />\n";
    devHtml += "    <button ";
    devHtml += "      onclick=\"sendButton(document.getElementById('switch" + String(vlgNr) + "').value,";
    devHtml += "        document.getElementById('action" + String(vlgNr) + "').value);\"/>";
    devHtml += "        <span id='buttonLabel" + String(vlgNr) + "'>Switch <b>Off</b></span>";
    devHtml += "    </button>\n";
    devHtml += "  </div>\n";
    devHtml += "  <div id='slider' style='text-align:left;'>\n";
    devHtml += "    <span style='_slider'> </span>\n";
    devHtml += "  </div>\n";
    
    devHtml += "  <div id='icon'>";
    devHtml += "    <span id='deviceIcon" + String(vlgNr) + "'><img src='/lightOff.ico' alt='Off'></span>\n";
    devHtml += "  </div>\n";
    
    devHtml += "  <div id='state'>\n";
    devHtml += "    <span id='hartBeat" + String(vlgNr) + "'>-</span>";
    devHtml += "  </div>\n";
    devHtml += "</div>\n";

    return devHtml;

} // swtitchHTML()

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
