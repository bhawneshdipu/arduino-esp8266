#include <SoftwareSerial.h>
SoftwareSerial EspSerial(6, 7); // Rx,  Tx
#include <stdlib.h>
// pin variable declaration


#define HARDWARE_RESET 8
#define FREEZE_LED 13
#define lpg_gas_sensor 10
#define servomotor 11
#define alarm 12

//needed variable declaration

boolean lpg_sensor_status;
int spare;
long writeTimingSeconds = 17;
long readTimingSeconds = 10;
long startReadTiming = 0;
long elapsedReadTime = 0;
long startWriteTiming = 0;
long elapsedWriteTime = 0;
boolean knob;
boolean Alarm;
boolean error;
void setup()
{

  // setup code to run once 



  Serial.begin(9600);
  pinMode(HARDWARE_RESET,OUTPUT);
  pinMode(lpg_gas_sensor,INPUT);
  pinMode(servomotor,OUTPUT);
  pinMode(FREEZE_LED,OUTPUT);
  pinMode(alarm,OUTPUT);
  digitalWrite(HARDWARE_RESET, HIGH);
  digitalWrite(FREEZE_LED, LOW);
  EspSerial.begin(9600);
  EspHardwareReset();
  startReadTiming = millis();
  startWriteTiming = millis();
}
void loop()
{
  start:
  error=0;
  elapsedWriteTime = millis()-startWriteTiming;
  elapsedReadTime = millis()-startReadTiming;
// reading sensors data and writing the data on the website api.virtualworld.today

    ESPcheck();
    readSensors();
    writeWebsiteData("sensor_write",lpg_sensor_status);
  if (error==1)
  {
    Serial.println(" <<<< ERROR >>>>\n");
    digitalWrite(FREEZE_LED, HIGH);
    delay (2000);
    goto start;
  }

// reading the data from the website from table knob action status where android app writes the data for regulators and alarm
    ESPcheck();
    int command = readWebsiteData("knob");
    knob = command;
    Serial.print("Knob:=>"+command);
    // taking actions after reading data from the website table action_status


    takeActions();

   // writes the data on the website table knob and alarm status after executing the particular actions

    writeWebsiteData("knob_write",knob);
    command=0;
    delay (1000);
    command = readWebsiteData("alarm");
    Serial.print(command+"\n");
    Alarm = command;
    takeActions();
    writeWebsiteData("alarm_write",Alarm);  
    command=0;
    startReadTiming = millis();

   
}

// function to read sensors data

void readSensors(void)
{
  lpg_sensor_status=digitalRead(lpg_gas_sensor);
  Serial.println("Reading Sensor Data:##START");
  Serial.println(lpg_sensor_status);
  Serial.println("Reading Sensor Data:##END");
}


// function to write data on the website


void writeWebsiteData(String write,boolean device)
{
  startCmd();
  String status;
  if(device==1){
    status="on";
  }
  else{
    status="off";
  }
  Serial.println("Sending Sensor Data.....");
  String getStr = "GET /"+write+".php?status="+status+" HTTP/1.1\r\nHost: api.virtualworld.today\r\nConnection: keep-alive\r\n\r\n";
  Serial.println(getStr);
  sendWebsiteGetCmd(getStr);
}
void EspHardwareReset(void)
{
  Serial.println("Reseting.......");
  digitalWrite(HARDWARE_RESET, LOW);
  delay(500);
  digitalWrite(HARDWARE_RESET, HIGH);
  delay(8000);
  Serial.println("RESET");
}

// start serial monitor command to send or receive data to website

void startCmd(void)
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
String sendWebsiteGetCmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  EspSerial.println(cmd);
  Serial.print("lenght cmd: ");
  Serial.println(cmd);
  if(EspSerial.find(">"))
  {
    EspSerial.print(getStr);
    Serial.print("REQUEST==>: ");
    Serial.println(getStr);
    delay(500);
    String messageBody = "";
    while (EspSerial.available())
    {
      String line = EspSerial.readStringUntil('$');
      messageBody+=line;
      messageBody+="\n";
      if (line.length() == 1)
      { 
        messageBody += EspSerial.readStringUntil('$');
        messageBody+="\n";
      }else{
        messageBody += EspSerial.readStringUntil('$');
        messageBody+="\n";
      }
    }
    delay(1000);
    while (EspSerial.available())
    {
      String line = EspSerial.readStringUntil('$');
      if (line.length() == 1)
      { 
        messageBody += EspSerial.readStringUntil('$');
        messageBody+="\n";
      }else{
        messageBody += line;
        messageBody+="\n";
      }
    }
    Serial.print("MessageBody received: ==============>START");
    Serial.println(messageBody);
    Serial.print("MessageBody received: ==============>END");

    return messageBody;
  }
  else
  {
    EspSerial.println("AT+CIPCLOSE");
    Serial.println("ESP8266 CIPSEND ERROR: RESENDING");
    spare = spare + 1;
    error=1;
    return "error";
  }
}

// function to read data from the website 

int readWebsiteData(String device_name)
{
  startCmd();
  String getStr = "GET /action_read.php?on="+device_name+" HTTP/1.1\r\nHost: api.virtualworld.today\r\nConnection: keep-alive\r\n\r\n";
  String messageDown = sendWebsiteGetCmd(getStr);
  Serial.print("Sending ...: ");
  Serial.print("reading :"+device_name);
  Serial.println(messageDown);
  if(device_name=="knob"  && messageDown.indexOf("{KNOB NOT TAKEN OFF}")>=0){
    Serial.println("##\nTaking...Action.."+device_name+" off\n###\n");
    Serial.print(device_name+"Action received .. ...: ");
    return 0;
  }
  if(device_name=="knob"  && messageDown.indexOf("{KNOB NOT TAKEN ON}")>=0){
    Serial.println("##\nTaking...Action.."+device_name+" on\n###\n");
    Serial.print(device_name+"Action received .. ...: ");
    return 1;
  }

  if(device_name=="alarm" && messageDown.indexOf("{ALARM NOT TAKEN OFF}")>=0){
    Serial.println("##\nTaking...Action.."+device_name+" off\n###\n");
    Serial.print(device_name+"Action received .. ...: ");
    return 0;
  }
  if(device_name=="alarm" && messageDown.indexOf("{ALARM NOT TAKEN ON}")>=0){
    Serial.println("##\nTaking...Action.."+device_name+" on\n###\n");
    Serial.print(device_name+"Action received .. ...: ");
    return 0;
  }
  
  return 9;
}

//function to perform actions on regulator or alarm

void takeActions(void)
{
  Serial.print("lpg_sensor_status is :");
  Serial.println(lpg_sensor_status);
  Serial.print("knob: ");
  Serial.println(knob);
  Serial.print("alarm: ");
  Serial.println(Alarm);
  if(knob == 1){
   digitalWrite(servomotor, HIGH);
   Serial.print("knob action performed");
 }else {
  digitalWrite(servomotor, LOW);
 }
 if (Alarm == 1){
   digitalWrite(alarm, HIGH);
   Serial.print("alarm action performed");
 }else{
    digitalWrite(alarm,LOW);
 }
}
boolean ESPcheck(void)
{
  EspSerial.println("AT");

  if (echoFind("OK"))
  {
    digitalWrite(FREEZE_LED, LOW);
    return true;
  }
  else 
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
