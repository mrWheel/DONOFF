/*
***************************************************************************  
**  Program  : maintenancePage, part of DONOFF
**  Version  : v0.3.6
**
**  Copyright (c) 2019 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

void handleErrorIndex() {                    // HTML error index.html

  String errIndexHTML = "";

  _dThis = true;
  Debugf("FreeHeap [%d]\n", ESP.getFreeHeap() );

  errIndexHTML += "<!DOCTYPE HTML><html lang='en'>\n";
  errIndexHTML += "<head>";
  errIndexHTML += "<meta charset='UTF-8'>";
  errIndexHTML += "<style type='text/css'>";
  errIndexHTML += "  body {background-color: lightgray;}";
  errIndexHTML += "</style>";
  errIndexHTML += "</head>\n";
  errIndexHTML += "<body>\n<h1>DONOFF</h1>\n";
 
  errIndexHTML += "<h2>This page should not look like this</h2>\n";
  errIndexHTML += "<br>missing index.html page<br>\n";
  errIndexHTML += "<hr><h3>Have you uploaded the 'data' dir to the ESP8266? </h3>\n";
  
  errIndexHTML += "<hr>\n";
  errIndexHTML += "<div style='width: 30%'>\n";
  errIndexHTML += "  <form style='float: right;' action='/maintenance' method='POST'>  \n";
  errIndexHTML += "   <input type='submit' class='button' name='SUBMIT' value='Go to Maintenance page'>\n";
  errIndexHTML += "  </form>\n";
  errIndexHTML += "</div>\n";
  errIndexHTML += "<div style='width: 40%'>&nbsp;</div>\n";

  errIndexHTML += "</body></html>\n";

  server.send(200, "text/html", errIndexHTML);
  
} // handleErrorIndex()


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
