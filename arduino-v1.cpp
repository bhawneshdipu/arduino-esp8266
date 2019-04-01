#include <SoftwareSerial.h>

SoftwareSerial esp8266 (6,7);

String ssid = "NIPU1";
String password = "nipu123456";

String server = "api.virtualworld.today";

const int timeout = 2000;

void setup() {
  Serial.begin(9600);
  esp8266.begin(115200); // Should match ESP's current baudrate
  
  setupESP8266();
  connectToWiFi();

  //startTCPConnection();
  //sendDataRequest();
  atGetCommand();
  
  sendStatusRequest();
  closeTCPConnection();
}

void loop() {

  //startTCPConnection();
  //sendDataRequest();
  atGetCommand();
  sendStatusRequest();
  closeTCPConnection();
  
}

String atCommand(String command, int timeout,char resp[]="OK"){
  String response = "";
  esp8266.println(command);

  long int time = millis();
  int i=0;
  Serial.println(command);

  while( (time+timeout) > millis() ) {
    if(esp8266.find(resp)){
       Serial.println(resp);
      while(esp8266.available()){
          String line = esp8266.readStringUntil('\n');
          Serial.println("Line:"+String(i)+"::"+line);
          response+=line;
          i++;
      }

    }else{
    while(esp8266.available()){
        String line = esp8266.readStringUntil('\n');
        Serial.println("Line:"+String(i)+"::"+line);

        response+=line;
        i++;
    }

    }
  }
  Serial.println(response);
  return response;
}
String atGetCommand(){
  String response = "";
  String connect = "AT+CIPSTART=\"TCP\",\"" + server + "\",80";
  esp8266.println(connect);
  String send_path = "/receive.php?name="+getData();

  String request = "GET " + send_path + " HTTP/1.1\r\n" + "Host: " + server + "\r\n" + "Connection: keep-alive\r\n\r\n";
  String requestLength = String(request.length());

  String send = "AT+CIPSEND=" + requestLength;
  esp8266.println(send);
  long int time = millis();
  int i=0;
  while( (time+5000) > millis() ) {
   Serial.println("Waiting to send data...");
  if(esp8266.find((char *)">"))
  {
    Serial.println("Now sending data...");
    esp8266.print(request);
    Serial.print("request: ");
    Serial.println(request);
    delay(500);
    String messageBody = "";
    while (esp8266.available()) 
    {
      String line = esp8266.readStringUntil('\n');
      if (line.length() == 1) 
      { //actual content starts after empty line (that has length 1)
        messageBody = esp8266.readStringUntil('\n');
      }
    }
    Serial.print("MessageBody received: ");
    Serial.println(messageBody);
    response = messageBody;
    break;
  }else{
    Serial.println("Waiting..");
  }
  }
  
  Serial.println(response);
  return response;
}

void setupESP8266() {
  atCommand("AT+RST", timeout);
  atCommand("AT+CWMODE=1", timeout);
}

void connectToWiFi() {
  String connect = "AT+CWJAP=\"" +ssid+"\",\"" + password + "\"";
  atCommand(connect, 6000);
  atCommand("AT+CIFSR", timeout);
}

void startTCPConnection() {
  String connect = "AT+CIPSTART=\"TCP\",\"" + server + "\",80";
  atCommand(connect, timeout);
}

void closeTCPConnection() {
  atCommand("AT+CIPCLOSE", timeout);
}

String getData(){
  return "arduino"+String(millis());
}
String sendDataRequest() {
  String send_path = "/receive.php?name="+getData();

  String request = "GET " + send_path + " HTTP/1.1\r\n" + "Host: " + server + "\r\n" + "Connection: keep-alive\r\n\r\n";
  String requestLength = String(request.length());

  atCommand("AT+CIPSEND=" + requestLength, timeout,"SEND OK");
  String response = atCommand(request, 6000);
  return response;
}
String sendStatusRequest() {
String status_path = "/status.php";

  String request = "GET " + status_path + " HTTP/1.1\r\n" + "Host: " + server + "\r\n" + "Connection: keep-alive\r\n\r\n";
  String requestLength = String(request.length());
  atCommand("AT+CIPSEND=" + requestLength, timeout);
  String response = atCommand(request, 6000,"SEND OK");
  return response;
}