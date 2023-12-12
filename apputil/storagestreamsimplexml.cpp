#include "storagestreamsimplexml.h"
#include "base64encoder.h"

namespace sV2 {

//SimpleXML storage
////////////////////
StorageStreamSimpleXML::StorageStreamSimpleXML(StreamMedia* sm)
  :m_streamMedia(sm)
  ,tabsz(1)
{
   buf.resize(4096, '\0');
   strbegin = strend = &buf[0];
   bufBeginOff = 0;
   bufEndOff = 0;
}

StorageStreamSimpleXML::~StorageStreamSimpleXML(){
}

//atomic storage operations
const char* StorageStreamSimpleXML::GetNodeName(){
   return strbegin;
}

bool StorageStreamSimpleXML::nextLine(char** begin, char** end)
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

bool StorageStreamSimpleXML::readMore()
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
   if(m_streamMedia->eos() && r == 0)
     return false;
   else
     return true;
}

int StorageStreamSimpleXML::NextItem(){
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

void StorageStreamSimpleXML::PutStartNode(const char *s){
  bool isVector = !strcmp(s,"vector");
  if(!isVector) {
    char buf[256];
    auto bytes = sprintf_s(buf, "%s<%s>\n", indent.c_str(), s);
    m_streamMedia->write(buf, bytes);

    indent.resize(indent.size() + tabsz, ' ');
  }
}

void StorageStreamSimpleXML::PutEndNode(const char *s){
  bool isVector = !strcmp(s,"vector");
  if(!isVector) {
    indent.resize(indent.size() - tabsz);
    char buf[256];
    auto bytes = sprintf_s(buf, "%s</%s>\n", indent.c_str(), s);
    m_streamMedia->write(buf,bytes);
  }
}

void StorageStreamSimpleXML::GetItem(int* v){
   *v = atoi(strbegin);
}

void StorageStreamSimpleXML::GetItem(float* v){
   *v = std::stof(strbegin);
}

void StorageStreamSimpleXML::GetItem(double* v){
   *v = std::stod(strbegin);
}

void StorageStreamSimpleXML::GetItem(char const** v){
   *v = strbegin;
}

void StorageStreamSimpleXML::GetItem(void const** v, size_t* n){
   // Binary data.
   *v = strbegin;
   *n = strend - strbegin;
}

void StorageStreamSimpleXML::PutItem(int* v){
  char buf [256];
  auto bytes = sprintf_s(buf, "%s%d\n", indent.c_str(), *v);
  m_streamMedia->write(buf,bytes);
}

void StorageStreamSimpleXML::PutItem(float* v){
   char buf [256];
   auto bytes = sprintf_s(buf, "%s%e\n", indent.c_str(), *v);
   m_streamMedia->write(buf,bytes);
}

void StorageStreamSimpleXML::PutItem(double* v){
   char buf [256];
   auto bytes = sprintf_s(buf, "%s%.15e\n", indent.c_str(), *v);
   m_streamMedia->write(buf,bytes);
}

void StorageStreamSimpleXML::PutItem(const char* v){
   char buf [256];
   auto bytes = sprintf_s(buf, "%s%s\n", indent.c_str(), v);
   m_streamMedia->write(buf,bytes);
}

void StorageStreamSimpleXML::PutItem(void const* v, size_t n){
   char buf [1024];
   auto bytes = sprintf_s(buf, "%s<binary>%s</binary>\n", indent.c_str(),
         toBase64((char const*)v, (char const*)v + n).c_str());
   m_streamMedia->write(buf,bytes);
}


}
