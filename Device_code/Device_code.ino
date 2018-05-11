#include "AIS_NB_BC95.h"
#include <string.h>
#include <stdlib.h>

/***** AIS NB-IoT Variable *****/
AIS_NB_BC95 AISnb;
bool debug = true;
String apnName = "devkit.nb";

String serverIP = "<UDP_SERVER_IP>"; // Your Server IP
String serverPort = "<UDP_SERVER_PORT>"; // Your Server Port

AIS_NB_BC95_CEREG networkInfo;
signal networkSignalQuality;
String networkCode = "";
String ip1;

/***** GPS Variable *****/
#include <NeoSWSerial.h>

NeoSWSerial gpsSerial(11, 12); // RX, TX

char strBuffer[121];
char *currBuffer;
#define MAX_WAIT_TIME 5000

char UTCTime[12], Lat[16], Lng[16];

/***** General Variable *****/
const long interval = 20000;  //millisecond
unsigned long previousMillis = 0;
unsigned long seq = 0;

void debugEPS(){
  Serial.print(F("EPS: ")); Serial.print(networkInfo.EPS_status, DEC);
  Serial.print(F(", TAC: ")); Serial.print(networkInfo.TAC, HEX);
  Serial.print(F(", ECI: ")); Serial.println(networkInfo.ECI, HEX);
}

/******** AIS Helper functions ******/
void getNetworkStatus(){
  do{
    if(!networkCode.length())
      networkCode = AISnb.getPLMN();
    networkInfo = AISnb.getEPSNetworkStatus();
    if(debug) debugEPS();
    delay(2000);
  }while(networkInfo.EPS_status != 1 || !networkCode.length());
}
void getDeviceIP(){
  bool attemp = false;
  do{
    ip1 = AISnb.getDeviceIP();
    delay(2000);
    attemp = !attemp;
  }while(!ip1.length() || ip1 == "0"); 
}

#include "strcpyLimit.h"

#include "GPSHelpers.h"

void pingTest(){  
  unsigned char i;
  pingRESP pingR;
  do{
    for(i = 1; i <= 3; ++i){
      pingR = AISnb.pingIP(serverIP);
      if(pingR.status){
        break;
      }
    }

    if(!pingR.status){
      AISnb.reset();
      AISnb.attachNB(serverPort);
      AISnb.createUDPSocket(serverPort);
    }
  }while(!pingR.status);
}

void setup(){ 
  AISnb.debug = debug;
  
  Serial.begin(9600);
  AISnb.setupDevice(serverPort);
  ip1 = AISnb.getDeviceIP();

  pingTest();
  
  previousMillis = millis() - interval;
  
  gpsSerial.begin(9600);
  delay(1000);
}

void loop(){
  UDPSend udp;
  unsigned long currentMillis = millis();
  String data = "";
  char numberBuffer[20];
  bool isGPSGet;
  
  if (currentMillis - previousMillis >= interval){
      getNetworkStatus();
      networkSignalQuality = AISnb.getSignal();
      isGPSGet = parseGPGGA();
      
      seq++;
      /****** Format Data 
       *
       * Because the module are only allow to send no more than 32 characters (as my experiment found),
       * I have to use base 36 (the maximum base natively allow to convert both AVR and Python) number
       * for all possible number.
       *
       * And, because it just a simple code for testing outdoor usage. So, I'm lazy to do any big-shot 
       * complex data encoding :/.
       * 
       ******/
      data  = "C,";
      data += ultoa(seq, numberBuffer, 36);
      data += ','; data += String(ultoa(strtoul(networkCode.c_str(), (char **)NULL, 10), numberBuffer, 36));
      data += ","; data += String(utoa (networkInfo.TAC,                                 numberBuffer, 36));
      data += ","; data += String(ultoa(networkInfo.ECI,                                 numberBuffer, 36));
      data += ','; data += String(utoa (
        strtoul(networkSignalQuality.csq.c_str(), (char **)NULL, 10), 
        numberBuffer, 
        36
      ));
      /****** /Format Data *****/
      Serial.print("\r\n* Data -> "); Serial.println(data);
       
      // UDPSend udp = AISnb.sendUDPmsg(serverIP, serverPort, strlen(data), data,MODE_STRING);
      // udp = AISnb.sendUDPmsgStr(serverIP, serverPort, "Test,52003,0Xa3c3,0X9c802a3");
      
      udp = AISnb.sendUDPmsgStr(serverIP, serverPort, data);

      /****** Format GPS Data *****/
      if(isGPSGet){
        data = "G,";
        data += ultoa(seq, numberBuffer, 36); data += ',';
        data += Lat; data += ',';
        data += Lng;
        Serial.print("\r\n* GPS  -> "); Serial.println(data);
        udp = AISnb.sendUDPmsgStr(serverIP, serverPort, data);
      }
      
      previousMillis = currentMillis;
      if(seq % 10 == 9)
        pingTest();
  
  }
  UDPReceive resp = AISnb.waitResponse();
}
