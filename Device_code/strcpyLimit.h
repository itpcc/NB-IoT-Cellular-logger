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