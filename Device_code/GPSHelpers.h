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