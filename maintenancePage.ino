/*
***************************************************************************  
**  Program  : maintenancePage, part of DONOFF
**  Version  : v0.3.5
**
**  Mostly stolen from https://www.arduinoforum.de/User-Fips
**  See also https://www.arduinoforum.de/arduino-Thread-SPIFFS-DOWNLOAD-UPLOAD-DELETE-Esp8266-NodeMCU
**
***************************************************************************      
*/

File fsUploadFile;                      // Stores de actuele upload

void handleMaintenance() {                    // HTML admin
  FSInfo fs_info;
  SPIFFS.info(fs_info);

  String maintHTML = "";

  _dThis = true;
  Debugf("FreeHeap [%d]\n", ESP.getFreeHeap() );

  maintHTML += "<!DOCTYPE HTML><html lang='en'>\n";
  maintHTML += "<head>";
  maintHTML += "<meta charset='UTF-8'>";
//maintHTML += "<meta name= viewport content='width=device-width, initial-scale=1.0,' user-scalable=yes>";
  maintHTML += "<style type='text/css'>";
  maintHTML += "  body {background-color: lightgray;}";
  maintHTML += "</style>";
  maintHTML += "</head>\n";
  maintHTML += "<body>\n<h1>DONOFF Filesystem Maintenance</h1>\n";
 
  //maintHTML += "<div style='width: 40%'>&nbsp;</div>";

  maintHTML += "<h2>Upload, Download or Delete</h2>\n";
  maintHTML += "<hr><h3>Select File to download:</h3>\n";
  
  if (!SPIFFS.begin())  { _dThis = true; Debugln("SPIFFS failed to mount !\n"); }

  Dir dir = SPIFFS.openDir("/");         // List files on SPIFFS
  while (dir.next())  {
    maintHTML += "<a href ='";
    maintHTML += dir.fileName();
    maintHTML += "?download='>";
    maintHTML += "SPIFFS";
    maintHTML += dir.fileName();
    maintHTML += "</a> ";
    maintHTML += formatBytes(dir.fileSize()).c_str();
    maintHTML += "<br>\n";
  }

  maintHTML += "<p><hr><h3>Move file to delete:</h3>\n";
  maintHTML += "<form action='/fileDelete' method='POST'>To delete move file in here ";
  maintHTML += "<input type='text' style='height:45px; font-size:15px;' name='Delete' placeholder='Move file in here' required>";
  maintHTML += "<input type='submit' class='button' name='SUBMIT' value='Delete'>";
  maintHTML += "</form><p><br>\n";
  
  maintHTML += "<hr><h3>Upload File:</h3>\n";
  maintHTML += "<form method='POST' action='/fileUpload' enctype='multipart/form-data' style='height:35px;'>";
  maintHTML += "<input type='file' name='upload' style='height:35px; font-size:13px;' required>";
  maintHTML += "<input type='submit' value='Upload' class='button'>";
  maintHTML += "</form><p><br>\n";
  maintHTML += "<hr>SPIFFS Info: \n";
  maintHTML += formatBytes(fs_info.totalBytes).c_str();      
  maintHTML += "<br>Of which is used: \n";
  maintHTML += formatBytes(fs_info.usedBytes).c_str();      
  maintHTML += "<p>";


  maintHTML += "<hr>\n";
  maintHTML += "<div style='width: 30%'>\n";
  maintHTML += "  <form style='float: right;' action='/admin' method='POST'>  \n";
  maintHTML += "   <input type='submit' class='button' name='SUBMIT' value='Back to Admin page'>\n";
  maintHTML += "  </form>\n";
  maintHTML += "</div>\n";
  maintHTML += "<div style='width: 40%'>&nbsp;</div>\n";
  
  maintHTML += "</body></html>\n";

  _dThis = true;
  Debugln("Send maintenancePage ..");
  _dThis = true;
  Debugf("[exit] FreeHeap [%d]\n", ESP.getFreeHeap() );

  server.send(200, "text/html", maintHTML);
  
} // handleMaintenance()

String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + " Byte";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + " KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + " MB";
  }
} // formatBytes()

String getContentType(String filename) {
  if (server.hasArg("download"))        return "application/octet-stream";
  else if (filename.endsWith(".htm"))   return "text/html";
  else if (filename.endsWith(".html"))  return "text/html";
  else if (filename.endsWith(".css"))   return "text/css";
  else if (filename.endsWith(".js"))    return "application/javascript";
  else if (filename.endsWith(".png"))   return "image/png";
  else if (filename.endsWith(".gif"))   return "image/gif";
  else if (filename.endsWith(".jpg"))   return "image/jpeg";
  else if (filename.endsWith(".ico"))   return "image/x-icon";
  else if (filename.endsWith(".xml"))   return "text/xml";
  else if (filename.endsWith(".pdf"))   return "application/x-pdf";
  else if (filename.endsWith(".zip"))   return "application/x-zip";
  else if (filename.endsWith(".gz"))    return "application/x-gzip";
  return "text/plain";
  
} // getContentType()


bool handleFileRead(String path) {
  _dThis = true;
  Debugln(path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  _dThis = true;
  Debugln(contentType);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
  
} // handleFileRead()


void handleFileDelete() {  
  String file2Delete, hostNameURL, IPaddressURL;      

  _dThis = true;
  Debugf("FreeHeap [%d]\n", ESP.getFreeHeap() );
               
  String message = server.uri() + "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += "["+ String(i)+"][" + server.argName(i) + ":'" + server.arg(i) +"']\n";
  }
  _dThis = true;
  Debugf("%s\n", message.c_str());
  DebugFlush();
  aliveTime += _HEARTBEAT_INTERVAL * 1000;
  if (server.args() == 0) return handleRoot();
  if (server.hasArg("Delete")) {
    file2Delete = server.arg("Delete");
    file2Delete.toLowerCase();
    Dir dir = SPIFFS.openDir("/");
    while (dir.next())    {
      String path   = dir.fileName();
      path.replace(" ", "%20"); path.replace("ä", "%C3%A4"); path.replace("Ä", "%C3%84"); path.replace("ö", "%C3%B6"); path.replace("Ö", "%C3%96");
      path.replace("ü", "%C3%BC"); path.replace("Ü", "%C3%9C"); path.replace("ß", "%C3%9F"); path.replace("€", "%E2%82%AC");
      hostNameURL   = "http://" + deviceHostName + ".local" + path + "?download=";
      hostNameURL.toLowerCase();
      IPaddressURL  = "http://" + WiFi.localIP().toString() + path + "?download=";
      IPaddressURL.toLowerCase();
      if ( (file2Delete != hostNameURL ) && (file2Delete != IPaddressURL ) ) {
          //Debugf("test [%s] != [%s]\n", file2Delete.c_str(), hostNameURL.c_str());
          //Debugf("test [%s] != [%s]\n", file2Delete.c_str(), IPaddressURL.c_str());
          continue;
      }
      _dThis = true;
      Debugf("File [%s] deleted!\n", dir.fileName().c_str());
      SPIFFS.remove(dir.fileName());
      String header = "HTTP/1.1 303 OK\r\nLocation:";
    //header += server.uri();
      header += "/maintenance";
      header += "\r\nCache-Control: no-cache\r\n\r\n";
      _dThis = true;
      Debugf("sendContent[%s]\n", header.c_str());
      server.sendContent(header);
      return;
    }
    String maintHTML;                                    
    maintHTML += "<!DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'>\n";
    maintHTML += "<meta name= viewport content=width=device-width, initial-scale=1.0, user-scalable=yes>\n";
    maintHTML += "<meta http-equiv='refresh' content='3; URL=";
    maintHTML += server.uri();
    maintHTML += "'><style>body {background-color: powderblue;}</style>\n";
    maintHTML += "</head>\n<body><center><h2>File not found!</h2>Wait for 3 seconds...</center>\n";
    maintHTML += "</body></html>\n";
    server.send(200, "text/html", maintHTML );
  }
  
} // handleFileDelete()


void handleFileUpload() {   
  //Debugf("FreeHeap [%d]\n", ESP.getFreeHeap() );
  //DebugFlush();             
  if (server.uri() != "/fileUpload") return;
  aliveTime += _HEARTBEAT_INTERVAL * 1000;
  HTTPUpload& upload = server.upload();
  yield();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    _dThis = true;
    Debugf("[Start] Name: [%s]\n", filename.c_str());
    DebugFlush();
    yield();
    if (filename.length() > 30) {
      int x = filename.length() - 30;
      filename = filename.substring(x, 30 + x);
    }
    if (!filename.startsWith("/")) filename = "/" + filename;
    _dThis = true;
    Debugf("Name: [%s]\n", filename.c_str());
    DebugFlush();
    fsUploadFile = SPIFFS.open(filename, "w");
    //filename = String();
    filename = "";
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    _dThis = true;
    Debug("[Write] Data: "); Debugln(upload.currentSize);
    yield();
    aliveTime += _HEARTBEAT_INTERVAL * 1000;
    if (fsUploadFile) {
      yield();
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      yield();
      fsUploadFile.close();
    }
    yield();
    _dThis = true;
    Debug("[End] Size: "); Debugln(upload.totalSize);
    DebugFlush();
    String header = "HTTP/1.1 303 OK\r\nLocation:";
    header += "/maintenance";
    header += "\r\nCache-Control: no-cache\r\n\r\n";
    _dThis = true;
    Debugf("sendContent[%s]\n", header.c_str());
    server.sendContent(header);
    handleMaintenance();

  }
  
} // handleFileUpload()


//void formatSpiffs() {       // Format SPIFFS
//  SPIFFS.format();
//  handleAdmin();
//}


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
