#include "storagestreamjson.h"

#include <assert.h>

namespace sV2{

//SimpleXML storage
////////////////////
StorageStreamSimpleJson::StorageStreamSimpleJson(StreamMedia* sm)
  :m_streamMedia(sm)
  ,tabsz(1)
{
   buf.resize(4096, '\0');
   strbegin = strend = &buf[0];
   bufBeginOff = 0;
   bufEndOff = 0;
   context.push({});
}

StorageStreamSimpleJson::~StorageStreamSimpleJson(){
}

//atomic storage operations
const char* StorageStreamSimpleJson::GetNodeName(){
  return strbegin;
}

bool StorageStreamSimpleJson::nextLine(char** begin, char** end)
{
  do {
    char* nl = (char*)memchr(&buf[0] + bufBeginOff, '\n', bufEndOff - bufBeginOff);
    if (nl) {
      *begin = &buf[0] + bufBeginOff;
      *end = nl;
      if (nl != &buf[0] && nl[-1] == '\r') {
        --*end;
      }
      **end = '\0';

      bufBeginOff = (nl - &buf[0]) + 1;
      if (bufBeginOff == bufEndOff) {
        bufBeginOff = bufEndOff = 0;
      }
      return true;
    }
  } while (readMore());

  // So, we haven't found a newline symbol.
  // We are still going to set *begin and *end, putting
  // a null character to **end.
  if (bufEndOff != buf.size()) {
    buf[bufEndOff] = '\0';
  } else {
    buf.push_back('\0');
  }
  *begin = &buf[0] + bufBeginOff;
  *end = &buf[0] + bufEndOff;

  if (bufBeginOff != bufEndOff) {
    // We have something in the buffer, but it's not a newline.
    // Let's pretend it does end with a newline.
    bufBeginOff = bufEndOff = 0;
    return true;
  }

  return false;
}

bool StorageStreamSimpleJson::readMore()
{
  if (buf.size() == bufEndOff) {
    if (bufEndOff - bufBeginOff < buf.size() / 2) {
      // Move the data to the beginning of the buffer.
      memmove(&buf[0], &buf[0] + bufBeginOff, bufEndOff - bufBeginOff);
      bufEndOff -= bufBeginOff;
      bufBeginOff = 0;
    } else {
      buf.resize(buf.size() * 2);
    }
  }

  // FIX IT!
  //int r = fread(&buf[0] + bufEndOff, 1, buf.size() - bufEndOff, f);
  //   if (r <= 0) {
  //      return false;
  //   }
  //   bufEndOff += r;

  auto r = m_streamMedia->read(buf.data()+bufEndOff, buf.size()-bufEndOff);
  bufEndOff += r;
  if(m_streamMedia->eos())
    return false;
  else
    return true;
}

int StorageStreamSimpleJson::NextItem(){

  bool withinQuote = false;
  bool withinSlash = false;
  bool startingTrim = true;

  std::string tockenCollector;

  for(;;){
    const char in = 'd'; //= readSymbol();
    if(withinQuote){
      if(withinSlash){
        withinSlash = false;
        tockenCollector.push_back(in);
        continue;
      }
      else if(in == '\\'){
        withinSlash = false;
        continue;
      }
      else if(in == '\"'){
        withinQuote = false;
        return 0; // tocken in collector
      }
      else{
        tockenCollector.push_back(in);
        continue;
      }
    }
    else if(in == '\"'){
      withinQuote = true;
      continue;
    }
    else if(in == '{'){
      tockenCollector.push_back(in);
      return 0; // new node;
    }
    else if(in == '[' ){
      tockenCollector.push_back(in);
      return 0; // new node;
    }
    else if(in == ']' ){
      tockenCollector.push_back(in);
      return 0; // end node;
    }
    else if(in == ']' ){
      tockenCollector.push_back(in);
      return 0; // end node;
    }
    else if(in == ',' ){
      tockenCollector.push_back(in);
      return 0; // end node;
    }
    else if(startingTrim == true && std::string("notwitespacesymbols").find(in) == std::string::npos){
      continue; //trim symbols at start
    }
    else if(std::string("number").find(in) != std::string::npos){
      startingTrim = false;
      tockenCollector.push_back(in);
      continue;
    }
    else {
      return 0; // non quotted string;
    }

  }





  if (!nextLine(&strbegin, &strend)) {
    return 1; // end node
  }

  for (; *strbegin == ' '; ++strbegin) {
    // skip initial whitespace
  }

  if (*strbegin != '<' || strend[-1] != '>') {
    return 2; // data node
  }

  ++strbegin; // skip the '<' character
  --strend; // trim the '>' character
  *strend = '\0';

  if (*strbegin == '/') {
    ++strbegin;
    return 1; // end node
  }

  if (strncmp(strbegin, "binary>", 7) != 0) {
    return 0; // start node
  }

  if ((strend - strbegin) < 8 || strcmp(strend - 8, "</binary") != 0) {
    return 0; // start node
  }
  strbegin += 7; // strlen("binary>");
  strend -= 8; // strlen("</binary");

  strend = decodeBase64InPlace(strbegin, strend);
  return 2; // data node
}

void StorageStreamSimpleJson::GetItem(int* v){
  *v = atoi(strbegin);
}

void StorageStreamSimpleJson::GetItem(float* v){
  *v = (float)atof(strbegin);
}

void StorageStreamSimpleJson::GetItem(double* v){
  *v = atof(strbegin);
}

void StorageStreamSimpleJson::GetItem(char const** v){
  *v = strbegin;
}

void StorageStreamSimpleJson::GetItem(void const** v, size_t* n){
  // Binary data.
  *v = strbegin;
  *n = strend - strbegin;
}

void StorageStreamSimpleJson::PutStartNode(const char *s){

  bool isVector = !strcmp(s,"vector");

  char buf[256];

  if (!isVector){
    if(context.top().itemCount == 0)
    {
      if(context.top().isVector){
        auto bytes = sprintf_s(buf, "[\n%s ", indent.c_str());
        m_streamMedia->write(buf, bytes);
      }
      else{
        auto bytes = sprintf_s(buf, "{\n%s \"%s\": ", indent.c_str(), s);
        m_streamMedia->write(buf, bytes);
      }
    }
    else{
      if(context.top().isVector){
        auto bytes = sprintf_s(buf, ",\n%s ", indent.c_str());
        m_streamMedia->write(buf, bytes);
      }
      else{
        auto bytes = sprintf_s(buf, ",\n%s \"%s\": ", indent.c_str(), s);
        m_streamMedia->write(buf, bytes);
      }
    }
    context.top().itemCount++;
    indent.insert(indent.size(), tabsz, ' ');
  }

  context.push({0,s,});
  context.top().isVector = isVector;

}

void StorageStreamSimpleJson::PutEndNode(const char *s){
  // context.top().itemCount == 0 only for leave node - nodes, which does not
  // have children - only have atomic values. Such items no need to be closed
  if(context.top().itemCount>0){
    if(context.top().isVector){
      char buf[256];
      auto bytes = sprintf_s(buf, "\n%s]", indent.c_str());
      m_streamMedia->write(buf,bytes);
    }
    else{
      char buf[256];
      auto bytes = sprintf_s(buf, "\n%s}", indent.c_str());
      m_streamMedia->write(buf,bytes);
    }
  }

  if(!context.top().isVector){
    indent.resize(indent.size() - tabsz);
  }
  context.pop();

  if(context.size()==1){ // last node shall be closed
    char buf[256];
    auto bytes = sprintf_s(buf, "\n%s}\n", indent.c_str());
    m_streamMedia->write(buf,bytes);
  }

}

void StorageStreamSimpleJson::PutItem(int* v){
  char buf [256];
  auto bytes = sprintf_s(buf, "%d", *v);
  m_streamMedia->write(buf,bytes);
}

void StorageStreamSimpleJson::PutItem(float* v){
  char buf [256];
  auto bytes = sprintf_s(buf, "%e", *v);
  m_streamMedia->write(buf,bytes);
}

void StorageStreamSimpleJson::PutItem(double* v){
  char buf [256];
  auto bytes = sprintf_s(buf, "%e", *v);
  m_streamMedia->write(buf,bytes);
}

void StorageStreamSimpleJson::PutItem(const char* v){
  char buf [256];
  auto bytes = sprintf_s(buf, "\"%s\"", v);
  m_streamMedia->write(buf,bytes);
}

void StorageStreamSimpleJson::PutItem(void const* v, size_t n){
  char buf [1024];
  auto bytes = sprintf_s(buf, "\"%s\"", toBase64((char const*)v, (char const*)v + n).c_str());
  m_streamMedia->write(buf,bytes);
}

char* StorageStreamSimpleJson::decodeBase64InPlace(char* begin, char* end){
  int c1, c2, c3, c4;
  char const* g = begin; // get ptr
  char* p = begin; // put ptr
  while(g != end) {
    do {
      c1 = m_base64DecodeChars[*g++ & 0xff];
    } while(g != end && c1 == -1);
    if (c1 == -1) {
      break;
    }

    do {
      c2 = m_base64DecodeChars[*g++ & 0xff];
    } while(g != end && c2 == -1);
    if (c2 == -1) {
      break;
    }

    *p++ = char((c1 << 2) | (c2 & 0x30) >> 4);

    do {
      if (*g == '=') {
        goto done;
      }
      c3 = m_base64DecodeChars[*g++ & 0xff];
    } while(g != end && c3 == -1);
    if (c3 == -1) {
      break;
    }

    *p++ = char(((c2 & 0x0F) << 4) | ((c3 & 0x3C) >> 2));

    do {
      if (*g == '=') {
        goto done;
      }
      c4 = m_base64DecodeChars[*g++ & 0xff];
    } while(g != end && c4 == -1);
    if (c4 == -1) {
      break;
    }

    *p++ = char(((c3 & 0x03) << 6) | c4);
  }

done:
  assert(p <= end);
  *p = '\0';
  return p;
}

std::string StorageStreamSimpleJson::toBase64(char const* begin, char const* end){
  std::string out;

  char const* g = begin;
  while (g != end) {
    int c1 = *g++ & 0xff;
    if (g == end) {
      out += m_base64EncodeChars[c1 >> 2];
      out += m_base64EncodeChars[(c1 & 0x03) << 4];
      out += '=';
      out += '=';
      break;
    }

    int c2 = *g++ & 0xff;
    if (g == end) {
      out += m_base64EncodeChars[c1 >> 2];
      out += m_base64EncodeChars[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
      out += m_base64EncodeChars[(c2 & 0x0F) << 2];
      out += '=';
      break;
    }

    int c3 = *g++ & 0xff;
    out += m_base64EncodeChars[c1 >> 2];
    out += m_base64EncodeChars[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
    out += m_base64EncodeChars[((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6)];
    out += m_base64EncodeChars[c3 & 0x3F];
  }

  return out;
}

char const StorageStreamSimpleJson::m_base64EncodeChars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

signed char const StorageStreamSimpleJson::m_base64DecodeChars[] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

}
