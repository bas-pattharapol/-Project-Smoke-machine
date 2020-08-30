void Line_Notify(String message1) ;

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <time.h>
#include <Wire.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecureAxTLS.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <SD.h>

#define tmp D2
#define Fog D3
#define St_Water D0
#define St_led D4

#define LINE_TOKEN "ZvLB4BpWE0lK7aecwN7G17bpAzlqr4ovttyyxqWEvET"
#define DBG_OUTPUT_PORT Serial
#define MAXSC 2
#ifndef STASSID
#define STASSID "vivo1723"
#define STAPSK  "00000000"
#endif

String message1 = " \"สัญญาณเตือน!!! เกิดการบุกรุก​ ระบบกำลังทำงาน.​\nWarning!!! Intrusion occurred  Fog​ Shield System is working.\" ";

const char* ssid = STASSID;
const char* password = STAPSK;
const char* host = "esp8266sd";

//*********Server เวลา สำรอง 3 เว็บ****************//
char ntp_server1[20] = "ntp.ku.ac.th";
char ntp_server2[20] = "fw.eng.ku.ac.th";
char ntp_server3[20] = "time.uni.net.th";

//  set Time ประเทศไทย
int timezone = 7;    
int dst = 0;
String data ;
String dd1 ;
String d;
int timeSet = 2;
//--------------------------------------------------

ESP8266WebServer server(80);

//--------------ติดต่อตัวลูก----------
WiFiServer TKDServer(8000);
WiFiClient TKDClient[MAXSC]; // MAXSC << จำนวนลูกที่ต้องการติดต่อ define อยู่ด้านบน
//-----------------------------


static bool hasSD = false;
File uploadFile,myFile;


void returnOK() {
  server.send(200, "text/plain", "");
}

void returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n");
}

bool loadFromSdCard(String path) {
  String dataType = "text/plain";
  if (path.endsWith("/")) {
    path += "index.htm";
  }

  if (path.endsWith(".src")) {
    path = path.substring(0, path.lastIndexOf("."));
  } else if (path.endsWith(".htm")) {
    dataType = "text/html";
  } else if (path.endsWith(".css")) {
    dataType = "text/css";
  } else if (path.endsWith(".js")) {
    dataType = "application/javascript";
  } else if (path.endsWith(".png")) {
    dataType = "image/png";
  } else if (path.endsWith(".gif")) {
    dataType = "image/gif";
  } else if (path.endsWith(".jpg")) {
    dataType = "image/jpeg";
  } else if (path.endsWith(".ico")) {
    dataType = "image/x-icon";
  } else if (path.endsWith(".xml")) {
    dataType = "text/xml";
  } else if (path.endsWith(".pdf")) {
    dataType = "application/pdf";
  } else if (path.endsWith(".zip")) {
    dataType = "application/zip";
  }

  File dataFile = SD.open(path.c_str());
  if (dataFile.isDirectory()) {
    path += "/index.htm";
    dataType = "text/html";
    dataFile = SD.open(path.c_str());
  }

  if (!dataFile) {
    return false;
  }

  if (server.hasArg("download")) {
    dataType = "application/octet-stream";
  }

  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    DBG_OUTPUT_PORT.println("Sent less data than expected!");
  }

  dataFile.close();
  return true;
}

void handleFileUpload() {
  if (server.uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    if (SD.exists((char *)upload.filename.c_str())) {
      SD.remove((char *)upload.filename.c_str());
    }
    uploadFile = SD.open(upload.filename.c_str(), FILE_WRITE);
    DBG_OUTPUT_PORT.print("Upload: START, filename: "); DBG_OUTPUT_PORT.println(upload.filename);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
    DBG_OUTPUT_PORT.print("Upload: WRITE, Bytes: "); DBG_OUTPUT_PORT.println(upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
    }
    DBG_OUTPUT_PORT.print("Upload: END, Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void deleteRecursive(String path) {
  File file = SD.open((char *)path.c_str());
  if (!file.isDirectory()) {
    file.close();
    SD.remove((char *)path.c_str());
    return;
  }

  file.rewindDirectory();
  while (true) {
    File entry = file.openNextFile();
    if (!entry) {
      break;
    }
    String entryPath = path + "/" + entry.name();
    if (entry.isDirectory()) {
      entry.close();
      deleteRecursive(entryPath);
    } else {
      entry.close();
      SD.remove((char *)entryPath.c_str());
    }
    yield();
  }

  SD.rmdir((char *)path.c_str());
  file.close();
}

void handleDelete() {
  if (server.args() == 0) {
    return returnFail("BAD ARGS");
  }
  String path = server.arg(0);
  if (path == "/" || !SD.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }
  deleteRecursive(path);
  returnOK();
}

void handleCreate() {
  if (server.args() == 0) {
    return returnFail("BAD ARGS");
  }
 
  String path = server.arg(0);
  if (path == "/" || SD.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }

  if (path.indexOf('.') > 0) {
    File file = SD.open((char *)path.c_str(), FILE_WRITE);
    if (file) {
      file.write((const char *)0);
      file.close();
    }
  } else {
    SD.mkdir((char *)path.c_str());
  }
  returnOK();
}

void printDirectory() {
  WiFiClient client = server.client();
   String  d = server.arg("index1.htm");

  if(d == "1"){
    digitalWrite(LED_BUILTIN,LOW);
    delay(3000);
    digitalWrite(LED_BUILTIN,HIGH);
  }
  
  

  if ( server.hasArg("index1.htm") ) {
    digitalWrite(LED_BUILTIN,LOW);
    delay(3000);
    digitalWrite(LED_BUILTIN,HIGH);
  } 
  if (!server.hasArg("dir")) {
    return returnFail("BAD ARGS");
  }

  
  String path = server.arg("dir");
  if (path != "/" && !SD.exists((char *)path.c_str())) {
    return returnFail("BAD PATH");
  }
  File dir = SD.open((char *)path.c_str());
  path = String();
  if (!dir.isDirectory()) {
    dir.close();
    return returnFail("NOT DIR");
  }
  dir.rewindDirectory();
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/json", "");


  server.sendContent("[");
  for (int cnt = 0; true; ++cnt) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }

    String output;
    if (cnt > 0) {
      output = ',';
    }

    output += "{\"type\":\"";
    output += (entry.isDirectory()) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += entry.name();
    output += "\"";
    output += "}";
    server.sendContent(output);
    entry.close();
  }
  server.sendContent("]");
  server.sendContent(""); // Terminate the HTTP chunked transmission with a 0-length chunk
  dir.close();
}

void handleNotFound() {
  if (hasSD && loadFromSdCard(server.uri())) {
    return;
  }
  String message = "SDCARD Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  DBG_OUTPUT_PORT.print(message);
}



void setup(void) {
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.setDebugOutput(true);
  DBG_OUTPUT_PORT.print("\n");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  DBG_OUTPUT_PORT.print("Connecting to ");
  DBG_OUTPUT_PORT.println(ssid);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(tmp,OUTPUT);
  pinMode(Fog,OUTPUT);
  pinMode(St_Water,INPUT);
  pinMode(St_led,OUTPUT);

  digitalWrite(tmp,LOW);
  digitalWrite(Fog,LOW);
  // Wait for connection
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) {//wait 10 seconds
    delay(500);
  }
  configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
  time_t now = time(nullptr); 
  TKDServer.begin();

  d = ctime(&now);
  if (i == 21) {
    DBG_OUTPUT_PORT.print("Could not connect to");
    DBG_OUTPUT_PORT.println(ssid);
    while (1) {
      delay(500);
    }
  }
  DBG_OUTPUT_PORT.print("Connected! IP address: ");
  DBG_OUTPUT_PORT.println(WiFi.localIP());

  if (MDNS.begin(host)) {
    MDNS.addService("http", "tcp", 80);
    DBG_OUTPUT_PORT.println("MDNS responder started");
    DBG_OUTPUT_PORT.print("You can now connect to http://");
    DBG_OUTPUT_PORT.print(host);
    DBG_OUTPUT_PORT.println(".local");
  }

  
  
  /*server.on("/list", HTTP_GET, printDirectory);
 
  server.on("/edit", HTTP_DELETE, handleDelete);
  server.on("/edit", HTTP_PUT, handleCreate);
  
  server.on("/edit", HTTP_POST, []() {
    returnOK();
  }, handleFileUpload);
  
  
  */


  server.on("/index.htm", p1); 
  server.on("/Fog_Protection.htm", p2); 

  
  server.onNotFound(handleNotFound);

  server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");

  if (SD.begin(SS)) {
    DBG_OUTPUT_PORT.println("SD Card initialized.");
    hasSD = true;
  }
  digitalWrite(LED_BUILTIN,HIGH);
}

void p1() {
   loadFromSdCard("/index.htm");
  configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
  time_t now = time(nullptr); 

  d = ctime(&now);
  digitalWrite(LED_BUILTIN,LOW);
  led_high();
    myFile = SD.open("test.txt", FILE_WRITE);
    if (myFile) {
      myFile.println(d + String("\t\t-->>> There is an emergency button on the web."));
      myFile.close();
    }
    
    digitalWrite(LED_BUILTIN,HIGH);

 

}

void p2(){
  loadFromSdCard("/Fog_Protection.htm");
  configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
  time_t now = time(nullptr); 
  d = ctime(&now);
   myFile = SD.open("test.txt", FILE_WRITE);
    if (myFile) {
      myFile.println(d + String("\t\t-->>> Have been logged in to the system correctly."));
      myFile.close();
    }
}

void led_high(){
    digitalWrite(tmp,HIGH);
    digitalWrite(Fog,HIGH);
    Line_Notify(message1);
    delay(timeSet*1000);
    digitalWrite(tmp,LOW);
    digitalWrite(Fog,LOW);
    Line_Notify(message1);
    delay(5000);
    Line_Notify(message1);
}

void Clients(){
  if (TKDServer.hasClient())
    {
      for(int i = 0; i < MAXSC; i++)
      {
        //ตรวจสอบ
        if (!TKDClient[i] || !TKDClient[i].connected())
        {
          if(TKDClient[i]) TKDClient[i].stop();
          TKDClient[i] = TKDServer.available();
          Serial.print("New Client : "); Serial.print(String(i+1) + " - ");
          continue;
        }
      }
      //หยุด
      WiFiClient TKDClient = TKDServer.available();
      TKDClient.stop();
    }

    //ตรวจ data
    
    for(int i = 0; i < MAXSC; i++)
    {
      if (TKDClient[i] && TKDClient[i].connected())
      {
          server.handleClient();
          MDNS.update();

        if(TKDClient[i].available())
        {
            server.handleClient();
            MDNS.update();
            configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
            time_t now = time(nullptr); 
            d = ctime(&now);
            

          //ถ้ามีdata ให้อ่าน แล้วเปรียบเทียบ 
          while(TKDClient[i].available()) 
          {
            char a = TKDClient[i].read();
            data = data+a;

            Serial.println(data);

            if(data == "Button_1"){
              led_high();
              configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
              time_t now = time(nullptr); 
              d = ctime(&now);
              myFile = SD.open("test.txt", FILE_WRITE);
              if (myFile) {
                myFile.println(d + String("\t\t-->>> There is an emergency button on the external."));
                myFile.close();
              }
            }
            if(data == "Button_2"){
              led_high();
              configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
              time_t now = time(nullptr); 
              d = ctime(&now);
              myFile = SD.open("test.txt", FILE_WRITE);
              if (myFile) {
                myFile.println(d + String("\t\t-->>> Sensors detected."));
                myFile.close();
              }
            }
            
            
          }
          data = "";
        }
      }
    }
}

void loop(void) {
  
  Clients();
  server.handleClient();
  MDNS.update();
  if(digitalRead(St_Water) == 0){
    digitalWrite(St_led,LOW);
  }else if(digitalRead(St_Water) == 1){
    digitalWrite(St_led,HIGH);
  }

}

void Line_Notify(String message1) {
   axTLS::WiFiClientSecure client;

  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    return;   
  }

  String req = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  req += "Authorization: Bearer " + String(LINE_TOKEN) + "\r\n";
  req += "Cache-Control: no-cache\r\n";
  req += "User-Agent: ESP8266\r\n";
  req += "Connection: close\r\n";
  req += "Content-Type: application/x-www-form-urlencoded\r\n";
  req += "Content-Length: " + String(String("message=" + message1).length()) + "\r\n";
  req += "\r\n";
  req += "message=" + message1;
  // Serial.println(req);
  client.print(req);
    
  delay(20);

  // Serial.println("-------------");
  while(client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    //Serial.println(line);
  }
  // Serial.println("-------------");
}
