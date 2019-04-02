// Thingspeak
#include <SoftwareSerial.h>
SoftwareSerial EspSerial(6, 7); // Rx,  Tx
#include <stdlib.h>
#define HARDWARE_RESET 8
#define FREEZE_LED 13
#define lpg_gas_sensor 10
#define servomotor 11
#define alarm 12

int lpg_sensor_status;
int spare;
// Variables to be used with timers
long writeTimingSeconds = 17; // ==> Define Sample time in seconds to send data
long readTimingSeconds = 10; // ==> Define Sample time in seconds to receive data
long startReadTiming = 0;
long elapsedReadTime = 0;
long startWriteTiming = 0;
long elapsedWriteTime = 0;
boolean knob;
boolean Alarm;
boolean error;
void setup()
{
  Serial.begin(9600);
  pinMode(HARDWARE_RESET,OUTPUT);
  pinMode(lpg_gas_sensor,INPUT);
  pinMode(servomotor,OUTPUT);
  pinMode(FREEZE_LED,OUTPUT);
  pinMode(alarm,OUTPUT);
  digitalWrite(HARDWARE_RESET, HIGH);
  digitalWrite(FREEZE_LED, LOW);
  EspSerial.begin(9600); // Comunicate with  WiFi module
  EspHardwareReset(); //Reset WiFi module
  startReadTiming = millis();
  startWriteTiming = millis(); // starting the "program clock"
}
void loop()
{
  start: //label
  error=0;

  elapsedWriteTime = millis()-startWriteTiming;
  elapsedReadTime = millis()-startReadTiming;
  if (elapsedReadTime >(readTimingSeconds*1000))
  {
    ESPcheck();
    int command = readWebsiteData();
    Serial.print(command);
    if(command != 9){
      knob = command;
    }
    delay (1000);
    command = readWebsiteData();
    Serial.print(command+"\n");
    if (command != 9){
      Alarm = command;
    }
    takeActions();
    startReadTiming = millis();
  }

  if (elapsedWriteTime >(writeTimingSeconds*1000))
  {
    ESPcheck();
    readSensors();
    writeWebsiteData();
    startWriteTiming = millis();
  }

  if (error==1) //Resend if transmission is not completed
  {
    Serial.println(" <<<< ERROR >>>>\n");
    digitalWrite(FREEZE_LED, HIGH);
    delay (2000);
    goto start; //go to label "start"
  }
}
/********* Read Sensors value *************/
void readSensors(void)
{
  lpg_sensor_status=digitalRead(lpg_gas_sensor);
  Serial.println("Reading Sensor Data:##START");
  Serial.println(lpg_sensor_status);
  Serial.println("Reading Sensor Data:##END");

}
void writeWebsiteData()
{
  startThingSpeakCmd();
  // preparacao da string GET'
  String status="on";
  if(lpg_sensor_status==0){
	  status="off";
  }
  Serial.println("Sending Sensor Data.....");

  String getStr = "GET /sensor_write.php?status="+status+" HTTP/1.1\r\nHost: api.virtualworld.today\r\nConnection: keep-alive\r\n\r\n";
  Serial.println(getStr);

  sendWebsiteGetCmd(getStr);
}
/********* Reset ESP *************/
void EspHardwareReset(void)
{
  Serial.println("Reseting.......");
  digitalWrite(HARDWARE_RESET, LOW);
  delay(500);
  digitalWrite(HARDWARE_RESET, HIGH);
  delay(8000);
  Serial.println("RESET");
}
/********* Start communication with ThingSpeak*************/
void startThingSpeakCmd(void)
{
  EspSerial.flush();

  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "api.virtualworld.today";
  cmd += "\",80";
  EspSerial.println(cmd);
  Serial.print("Start cmd: ");
  Serial.println(cmd);
  if(EspSerial.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }
}
/********* send a GET cmd to ThingSpeak *************/
String sendWebsiteGetCmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  EspSerial.println(cmd);
  Serial.print("lenght cmd: ");
  Serial.println(cmd);
  if(EspSerial.find((char *)">"))
  {
    EspSerial.print(getStr);
    Serial.print("getStr: ");
    Serial.println(getStr);
    delay(500);
    String messageBody = "";
    while (EspSerial.available())
    {
      String line = EspSerial.readStringUntil('\n');
      if (line.length() == 1)
      { //actual content starts after empty line (that has length 1)
        messageBody = EspSerial.readStringUntil('\n');
      }
    }
    Serial.print("MessageBody received: ");
    Serial.println(messageBody);
    return messageBody;
  }
  else
  {
    EspSerial.println("AT+CIPCLOSE");     // alert user
    Serial.println("ESP8266 CIPSEND ERROR: RESENDING"); //Resend...
    spare = spare + 1;
    error=1;
    return "error";
  }
}
int readWebsiteData()
{
  startThingSpeakCmd();
  int command;

  // preparacao da string GET


    String device="knob";
    String getStr = "GET /sensor_read.php HTTP/1.1\r\nHost: api.virtualworld.today\r\nConnection: keep-alive\r\n\r\n";
    String messageDown = sendWebsiteGetCmd(getStr);
    Serial.print("Reading Sensor data From Website ...: ");
    Serial.println(messageDown);


  	Serial.println("##\n###\nReading...Knob..\n###\n###\n");


  device="knob";
  getStr = "GET /action_read.php?on"+device+" HTTP/1.1\r\nHost: api.virtualworld.today\r\nConnection: keep-alive\r\n\r\n";

  messageDown = sendWebsiteGetCmd(getStr);
  Serial.print("Sending ...: ");
  Serial.print("Receive Action status:..,............ ");

  Serial.println(messageDown);
  //TAKING ACTION
 	Serial.println("##\nTaking...Action..knob off\n###\n");


  if(messageDown.indexOf("{STATUS[NOT TAKEN]STATUSEND}")>=0 && messageDown.indexOf("{ACTION[off]ACTIONEND}")>=0){
	  //off the knob and send the data
	  Serial.print("Knob Action received .. ...: ");
	  Serial.print("Taking Action .. ...: ");
	  device="knob";
	  knob=1;
	  getStr = "GET /knob_write.php?status=off HTTP/1.1\r\nHost: api.virtualworld.today\r\nConnection: keep-alive\r\n\r\n";
	  messageDown = sendWebsiteGetCmd(getStr);

	  Serial.print("Taking Taken .. ...: ");

	  Serial.println(messageDown);
  }
    //TAKING ACTION
   	Serial.println("##\n###\nTaking...Action.. knobon\n###\n###\n");

  if(messageDown.indexOf("{STATUS[NOT TAKEN]STATUSEND}")>=0 && messageDown.indexOf("{ACTION[on]ACTIONEND}")>=0){
  	  //off the knob and send the data
  	  Serial.print("Knob Action received .. ...: ");
  	  Serial.print("Taking Action .. ...: ");
  	  device="knob";
  	  knob=1;
  	  getStr = "GET /knob_write.php?status=on HTTP/1.1\r\nHost: api.virtualworld.today\r\nConnection: keep-alive\r\n\r\n";
  	  messageDown = sendWebsiteGetCmd(getStr);

  	  Serial.print("Taking Taken .. ...: ");

  	  Serial.println(messageDown);
    }



  device="alarm";
  getStr = "GET /action_read.php?on"+device+" HTTP/1.1\r\nHost: api.virtualworld.today\r\nConnection: keep-alive\r\n\r\n";
  messageDown = sendWebsiteGetCmd(getStr);
  Serial.print("Sending ...: ");
  Serial.println(messageDown);
  //TAKING ACTION
 	Serial.println("##\n###\nTaking  Action Alarm.. alarm off\n###\n###\n");


  if(messageDown.indexOf("{STATUS[NOT TAKEN]STATUSEND}")>=0 && messageDown.indexOf("{ACTION[off]ACTIONEND}")>=0){
	  //off the knob and send the data
	  Serial.print("Alarm Action received .. ...: ");
	  Serial.print("Taking Action .. ...: ");
	  device="alarm";
	  Alarm=1;
	  getStr = "GET /alarm_write.php?status=off HTTP/1.1\r\nHost: api.virtualworld.today\r\nConnection: keep-alive\r\n\r\n";
	  messageDown = sendWebsiteGetCmd(getStr);

	  Serial.print("Taking Taken .. ...: ");

	  Serial.println(messageDown);
  }
 	  //TAKING ACTION
 	 	Serial.println("##\n###\nTaking  Action Alarm.. alarm on\n###\n###\n");

  if(messageDown.indexOf("{STATUS[NOT TAKEN]STATUSEND}")>=0 && messageDown.indexOf("{ACTION[on]ACTIONEND}")>=0){
	  //off the knob and send the data
	  Serial.print("Alarm Action received .. ...: ");
	  Serial.print("Taking Action .. ...: ");
	  device="alarm";
	  Alarm=1;
	  getStr = "GET /alarm_write.php?status=on HTTP/1.1\r\nHost: api.virtualworld.today\r\nConnection: keep-alive\r\n\r\n";
	  messageDown = sendWebsiteGetCmd(getStr);

	  Serial.print("Taking Taken .. ...: ");

	  Serial.println(messageDown);
  }

  if (messageDown[5] == 49)
  {
    command = messageDown[7]-48;
    Serial.print("Command received: ");
    Serial.println(command);
  }
  else command = 9;
  return command;
}
void takeActions(void)
{
  Serial.print("lpg_sensor_status is ");
  Serial.println(lpg_sensor_status);
  Serial.print("knob: ");
  Serial.println(knob);
  Serial.print("alarm: ");
  Serial.println(Alarm);
  if (knob == 1) digitalWrite(servomotor, HIGH);
  else digitalWrite(servomotor, LOW);
  if (Alarm == 1) digitalWrite(alarm, HIGH);
  else digitalWrite(alarm,LOW);
}
boolean ESPcheck(void)
{
  EspSerial.println("AT"); // Send "AT+" command to module

  if (echoFind("OK"))
  {
    //Serial.println("ESP ok");
    digitalWrite(FREEZE_LED, LOW);
    return true;
  }
  else //Freeze ou Busy
  {
    Serial.println("ESP Freeze *********************************************************************");
    digitalWrite(FREEZE_LED, HIGH);
    EspHardwareReset();
    return false;
  }
}
boolean echoFind(String keyword)
{
 byte current_char = 0;
 byte keyword_length = keyword.length();
 long deadline = millis() + 5000;
 while(millis() < deadline){
  if (EspSerial.available()){
    char ch = EspSerial.read();
    Serial.write(ch);
    if (ch == keyword[current_char])
      if (++current_char == keyword_length){
       Serial.println();
    }
   }
  }
 return false;
}
