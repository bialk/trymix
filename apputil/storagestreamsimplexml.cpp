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
   strbegin = strend = buf.data();
   bufBeginOffset = 0;
   bufEndOffset = 0;
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
      char* nl = (char*)memchr(buf.data() + bufBeginOffset, '\n', bufEndOffset - bufBeginOffset);
      if (nl) {
         *begin = buf.data() + bufBeginOffset;
         *end = nl;
     if (nl != buf.data() && nl[-1] == '\r') {
      --*end;
     }
         **end = '\0';

         bufBeginOffset = (nl - buf.data()) + 1;
         if (bufBeginOffset == bufEndOffset) {
            bufBeginOffset = bufEndOffset = 0;
         }

         return true;
      }
   } while (readMore());

   // So, we haven't found a newline symbol.
   // We are still going to set *begin and *end, putting
   // a null character to **end.
   if (bufEndOffset != buf.size()) {
      buf[bufEndOffset] = '\0';
   } else {
      buf.push_back('\0');
   }
   *begin = buf.data() + bufBeginOffset;
   *end = buf.data() + bufEndOffset;

   if (bufBeginOffset != bufEndOffset) {
      // We have something in the buffer, but it's not a newline.
      // Let's pretend it does end with a newline.
      bufBeginOffset = bufEndOffset = 0;
      return true;
   }

   return false;
}

bool StorageStreamSimpleXML::readMore()
{
  if(m_streamMedia->eos())
    return false;

   if (buf.size() == bufEndOffset) {
      if (bufEndOffset - bufBeginOffset < buf.size() / 2) {
         // Move the data to the beginning of the buffer.
         memmove(buf.data(), buf.data() + bufBeginOffset, bufEndOffset - bufBeginOffset);
         bufEndOffset -= bufBeginOffset;
         bufBeginOffset = 0;
      } else {
         buf.resize(buf.size() * 2);
      }
   }

   auto r = m_streamMedia->read(buf.data()+bufEndOffset, buf.size()-bufEndOffset);
   bufEndOffset += r;
   return  r != 0;
}

StorageStreamFormatter::StreamItemType StorageStreamSimpleXML::NextItem(){
   if (!nextLine(&strbegin, &strend)) {
      return EndNode; // end node
   }

   for (; *strbegin == ' '; ++strbegin) {
      // skip initial whitespace
   }

   if (*strbegin != '<' || strend[-1] != '>') {
      return StringType; // data node
   }

   ++strbegin; // skip the '<' character
   --strend; // trim the '>' character
   *strend = '\0';

   if (*strbegin == '/') {
      ++strbegin;
      return EndNode; // end node
   }

   if (strncmp(strbegin, "binary>", 7) != 0) {
      return StartNode; // start node
   }

   if ((strend - strbegin) < 8 || strcmp(strend - 8, "</binary") != 0) {
      return StartNode; // start node
   }
   strbegin += 7; // strlen("binary>");
   strend -= 8; // strlen("</binary");

   strend = decodeBase64InPlace(strbegin, strend);
   return StringType; // data node
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
