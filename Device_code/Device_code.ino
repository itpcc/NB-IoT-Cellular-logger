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

char* strcpyLimit(char* dest, const char* src, size_t maxLen){
  return strcpyLimit(dest, (char*)src, maxLen);
}

char* strcpyLimit(char* dest, char* src, size_t maxLen){
  size_t srcLen;
  srcLen = strlen(src);

  dest[maxLen] = '\0';
  
  if(srcLen >= maxLen){
    return strncpy(dest, src, maxLen);
  }
  
  memset(dest, ' ', maxLen - srcLen);
  strncpy(dest+(maxLen - srcLen), src, srcLen);
  return dest;
}

/***** GPS Helper function *****/
char* getGPGGA(){
  unsigned long startTime = millis();
  char ch = '\0';
  char *newline;
  size_t len;
  
  memset(strBuffer, '\0', 121);
  do{
    while(
      gpsSerial.available() && 
      ch != '$' && 
      millis() - startTime < MAX_WAIT_TIME
    ){
      ch = gpsSerial.read();
    }
    
    if(ch == '$'){
      *strBuffer = ch;
      currBuffer = strBuffer+1;
      len = 1;
      
      while(
        millis() - startTime < MAX_WAIT_TIME &&
        len < 120 && 
        ch != '\n'
      ){
        if(gpsSerial.available()){
          ch = gpsSerial.read();
          *(currBuffer) = ch;
          ++currBuffer;
          ++len;
        }
      }
      
      currBuffer = strstr(strBuffer ,"$GPGGA");
      if(currBuffer != NULL){
        if((newline = strchr(strBuffer, '\n'))!=NULL) 
          *newline = '\0';
        if((newline = strchr(strBuffer, '\r'))!=NULL) 
          *newline = '\0';
          
        return currBuffer;
      }
    }
  }while(
    millis() - startTime < MAX_WAIT_TIME
  );
  
  return NULL;
}

bool parseGPGGA(){
  char *GPGGAStr, *pch;
  GPGGAStr = getGPGGA();
  Serial.print("GPGGA>"); Serial.println(GPGGAStr);
  if(strlen(GPGGAStr) == 0 || strstr(GPGGAStr, ",,,,,0") != NULL)
    return false;
  strtok (GPGGAStr,",");
  strcpy(UTCTime, strtok (NULL,","));
  strcpy(Lat, strtok (NULL,","));
  strcat(Lat, strtok (NULL,","));
  strcpy(Lng, strtok (NULL,","));
  strcat(Lng, strtok (NULL,","));
  return true;  
}

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





