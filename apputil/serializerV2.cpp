#include "serializerV2.h"

#include <assert.h>

namespace sV2{

static char const zero_ch = '\0';


// Moved here to avoid a warning on strncpy()
void CSync_CStr::Load(Serializer *s)
{
  while(1){
    int type=s->ss->NextItem();
    if(type==1) {
      return;
    } else if(type==2) {
      char const* str;
      s->ss->GetItem(&str);
      // writes str to obj till /0 met or sz-1 reached and fill obj[zs-1] it with zero.
      strncpy_s(obj,sz,str,sz-1);
    } else if(type==0) {
      Serializer(s).Load();
    }
  }
}

//Simple Binary storage
//////////////////////

StorageStreamSimpleBinary::StorageStreamSimpleBinary(StreamMedia *sm)
:  //f(),
  m_streamMedia(sm),
   type(),
   ddata(),
   fdata(),
   idata(),
   shortlen(),
   biglen()
{
}

StorageStreamSimpleBinary::~StorageStreamSimpleBinary(){
}

//atomic storage operations
const char* StorageStreamSimpleBinary::GetNodeName(){
   return &strdata[0];
}

int StorageStreamSimpleBinary::NextItem(){
   //data set parsing
   //fread(&type,1,1,f);
   m_streamMedia->read(&type,1);
   //if(!f || feof(f)) return 1;
   if(m_streamMedia->eos()) return 1;
   switch(type){
      case 0: // start node
      case 2: // string
      case 6: // binary
         if (type == 6) {
           m_streamMedia->read(&biglen, sizeof(biglen));
            shortlen = biglen;
         } else {
           m_streamMedia->read(&shortlen, sizeof(shortlen));
            biglen = shortlen;
         }
         strdata.resize(biglen + 1);
         m_streamMedia->read(strdata.data(), biglen);
         strdata[biglen] = '\0';
         return type == 0 ? 0 : 2;
      case 1: // end node
         return 1;
      case 3: // int
         m_streamMedia->read(&idata, sizeof(idata));
         return 2;
      case 4: // float
         m_streamMedia->read(&fdata, sizeof(fdata));
         return 2;
      case 5: // double
         m_streamMedia->read(&ddata, sizeof(ddata));
         return 2;
   }
   return 2;
}

void StorageStreamSimpleBinary::PutStartNode(const char *s){
  bool isVector = !strcmp(s,"vector");
  if(!isVector) {
    char t = 0;
    auto len_temp = strlen(s);
    assert(len_temp < std::numeric_limits<std::uint16_t>::max());
    std::uint16_t len = std::uint16_t(len_temp);
    m_streamMedia->write(&t, 1);
    m_streamMedia->write(&len, sizeof(len));
    m_streamMedia->write((void*)s, len);
  }
}

void StorageStreamSimpleBinary::PutEndNode(const char *s){
  bool isVector = !strcmp(s,"vector");
  if(!isVector) {
    char t = 1;
    m_streamMedia->write(&t, 1);
  }
}

void StorageStreamSimpleBinary::GetItem(int* v){
   switch(type){
      case 2: //from string
         *v = atoi(&strdata[0]);
         break;
      case 3: //from int
         *v = idata;
         break;
      case 4: //from float
         *v = (int)fdata;
         break;
      case 5: //from double
         *v = (int)ddata;
         break;
      default:
         *v = 0;
         break;
   }
}

void StorageStreamSimpleBinary::GetItem(float* v){
   switch(type){
      case 2: //from string
         *v = (float)atof(&strdata[0]);
         break;
      case 3: //from int
         *v = (float)idata;
         break;
      case 4: //from float
         *v = fdata;
         break;
      case 5: //from double
         *v = (float)ddata;
         break;
      default:
         *v = 0.0f;
         break;
   }
}

void StorageStreamSimpleBinary::GetItem(double* v){
   switch(type){
      case 2: //from string
         *v = atof(&strdata[0]);
         break;
      case 3: //from int
         *v = (double)idata;
         break;
      case 4: //from float
         *v = fdata;
         break;
      case 5: //from double
         *v = ddata;
         break;
      default:
         *v = 0.0;
         break;
   }
}

void StorageStreamSimpleBinary::GetItem(char const** v){
  switch(type){
  case 2: //from string
  case 6: //from binary
    *v = &strdata[0];
    break;
  case 3: //from int
    strdata.resize(32);
    sprintf_s(&strdata[0],strdata.size(), "%d", idata);
    *v = &strdata[0];
    break;
  case 4: // from float
    strdata.resize(32);
    sprintf_s(&strdata[0],strdata.size(), "%e", fdata);
    *v = &strdata[0];
    break;
  case 5: // from double
    strdata.resize(32);
    sprintf_s(&strdata[0],strdata.size(), "%e", ddata);
    *v = &strdata[0];
    break;
  }
}

void StorageStreamSimpleBinary::GetItem(void const** v, size_t* n){
   switch(type){
      case 2: //from string
      case 6: //from binary
         *v = &strdata[0];
         *n = biglen;
      default:
         *v = &zero_ch;
         *n = 0;
   }
}

void StorageStreamSimpleBinary::PutItem(int* v){
   char t = 3;
   m_streamMedia->write(&t, 1);
   m_streamMedia->write(v, sizeof(*v));
}

void StorageStreamSimpleBinary::PutItem(float* v){
   char t = 4;
   m_streamMedia->write(&t, 1);
   m_streamMedia->write(v, sizeof(*v));
}

void StorageStreamSimpleBinary::PutItem(double* v){
   char t = 5;
   m_streamMedia->write(&t, 1);
   m_streamMedia->write(v, sizeof(*v));
}

void StorageStreamSimpleBinary::PutItem(const char* v){
   char t = 2;

   auto len_temp = strlen(v);
   assert(len_temp < std::numeric_limits<std::uint16_t>::max());
   std::uint16_t len = std::uint16_t(len_temp);

   m_streamMedia->write(&t, 1);
   m_streamMedia->write(&len, sizeof(len));
   m_streamMedia->write((void *)v, len);
}

void StorageStreamSimpleBinary::PutItem(void const* v, size_t n)
{
   char t = 6;

   assert(n < std::numeric_limits<std::uint16_t>::max());
   std::uint16_t len = std::uint16_t(n);

   m_streamMedia->write(&t, 1);
   m_streamMedia->write(&len, sizeof(len));
   if (len != 0) { // This check is mandatory.
      m_streamMedia->write((void *)v, len);
   }
}



// StorageStreamSimpleIostream
///////////////////////////////////

//StorageStreamSimpleIostream::StorageStreamSimpleIostream(std::iostream* st)
StorageStreamSimpleIostream::StorageStreamSimpleIostream(StreamMedia* sm)
:  //strm(st),
   strm(sm),
   type(),
   ddata(),
   fdata(),
   idata(),
   shortlen(),
   biglen()
{
}

StorageStreamSimpleIostream::~StorageStreamSimpleIostream(){
}

//atomic storage operations
const char* StorageStreamSimpleIostream::GetNodeName(){
   return &strdata[0];
}

int StorageStreamSimpleIostream::NextItem(){
   //data set parsing
   strm->read(&type, 1);
   if(strm->eos()) return 1;
   switch(type){
      case 0: // start node
      case 2: // string
      case 6: // blob
         if (type == 6) {
            strm->read(&biglen, sizeof(biglen));
            shortlen = biglen;
         } else {
            strm->read((char*)&shortlen, sizeof(shortlen));
            biglen = shortlen;
         }
         strdata.resize(biglen + 1);
         strm->read(&strdata[0], biglen);
         strdata[biglen] = '\0';
         return type == 0 ? 0 : 2;
      case 1: // end node
         return 1;
      case 3: // data node string start node
         strm->read((char*)&idata,sizeof(idata));
         return 2;
      case 4: // data node string start node
         strm->read((char*)&fdata,sizeof(fdata));
         return 2;
      case 5: // data node string start node
         strm->read((char*)&ddata,sizeof(ddata));
         return 2;
   }
   return 2;
}

void StorageStreamSimpleIostream::PutStartNode(const char *s){
  bool isVector = !strcmp(s,"vector");
  if(!isVector) {
    char t = 0;

    auto len_temp = strlen(s);
    assert(len_temp < std::numeric_limits<std::uint16_t>::max());
    std::uint16_t len = std::uint16_t(len_temp);
    strm->write(&t, 1);
    strm->write(&len, sizeof(len));
    strm->write((void*)s, len);
  }
}

void StorageStreamSimpleIostream::PutEndNode(const char *s){
  bool isVector = !strcmp(s,"vector");
  if(!isVector) {
   char t = 1;
   strm->write(&t, 1);
  }
}

void StorageStreamSimpleIostream::GetItem(int* v){
   switch(type){
      case 2: //from string
         *v = atoi(&strdata[0]);
         break;
      case 3: //from int
         *v = idata;
         break;
      case 4: //from float
         *v = (int)fdata;
         break;
      case 5: //from double
         *v = (int)ddata;
         break;
      default:
         *v = 0;
         break;
   }
}

void StorageStreamSimpleIostream::GetItem(float* v){
   switch(type){
      case 2: //from string
         *v = (float)atof(&strdata[0]);
         break;
      case 3: //from int
         *v = (float)idata;
         break;
      case 4: //from float
         *v = fdata;
         break;
      case 5: //from double
         *v = (float)ddata;
         break;
      default:
         *v = 0.0f;
         break;
   }
}

void StorageStreamSimpleIostream::GetItem(double* v){
   switch(type){
      case 2: //from string
         *v = atof(&strdata[0]);
         break;
      case 3: //from int
         *v = (double)idata;
         break;
      case 4: //from float
         *v = fdata;
         break;
      case 5: //from double
         *v = ddata;
         break;
      default:
         *v = 0.0;
         break;
   }
}

void StorageStreamSimpleIostream::GetItem(char const** v){
   switch(type){
      case 2: //from string
      case 6: //from binary
         *v = &strdata[0];
         break;
      case 3: //from int
         strdata.resize(32);
         sprintf_s(strdata.data(),strdata.size(), "%d", idata);
         *v = &strdata[0];
         break;
      case 4: // from float
         strdata.resize(32);
         sprintf_s(strdata.data(),strdata.size(), "%e", fdata);
         *v = &strdata[0];
         break;
      case 5: // from double
         strdata.resize(32);
         sprintf_s(strdata.data(),strdata.size(), "%e", ddata);
         *v = &strdata[0];
         break;
   }
}

void StorageStreamSimpleIostream::GetItem(void const** v, size_t* n){
   switch(type){
      case 2: //from string
      case 6: //from binary
         *v = &strdata[0];
         *n = biglen;
         break;
   }
}

void StorageStreamSimpleIostream::PutItem(int* v){
   char t = 3;
   strm->write(&t, 1);
   strm->write(v, sizeof(*v));
}

void StorageStreamSimpleIostream::PutItem(float* v){
   char t = 4;
   strm->write(&t, 1);
   strm->write(v, sizeof(*v));
}

void StorageStreamSimpleIostream::PutItem(double* v){
   char t = 5;
   strm->write(&t, 1);
   strm->write(v, sizeof(*v));
}

void StorageStreamSimpleIostream::PutItem(const char* v){
   char t = 2;

   auto len_temp = strlen(v);
   assert(len_temp < std::numeric_limits<std::uint16_t>::max());
   std::uint16_t len = std::uint16_t(len_temp);

   strm->write(&t, 1);
   strm->write(&len, sizeof(len));
   strm->write((void *)v, len);
}

void StorageStreamSimpleIostream::PutItem(const void* v, size_t n){
   char t = 6;

   assert(n < std::numeric_limits<std::uint16_t>::max());
   std::uint16_t len = std::uint16_t(n);

   strm->write(&t, 1);
   strm->write(&len, sizeof(len));
   if (len != 0) { // This check is mandatory.
      strm->write((void*)v, len);
   }
}


//Indexed Binary storage
//////////////////////

StorageStreamIndexedBinary::StorageStreamIndexedBinary(StreamMedia *sm, StreamMedia *smi)
:  saveindex(0),
   idxcntr(0),
   m_streamMedia(sm),
   m_streamMediaIndex(smi),
   type(),
   ddata(),
   fdata(),
   idata(),
   shortlen(),
   biglen()
{
}

StorageStreamIndexedBinary::~StorageStreamIndexedBinary(){
   Close();
}

void gets(char*s, int maxCount, StreamMedia* sm){
  int count = 0;
  while(!sm->eos() && count < maxCount-1){
    sm->read(s+count,1);
    ++count;
    if(s[count-1] == '\n')
      break;
  }
  s[count]=0;
}

int StorageStreamIndexedBinary::ReadIndex(){
   int maxidx=0;
   while(!m_streamMediaIndex->eos()){
      char key[256],value[50];
      gets(key,sizeof(key),m_streamMediaIndex);
      gets(value,sizeof(key),m_streamMediaIndex);
      auto keylen = strlen(key);
      auto vallen = strlen(value);
      if(keylen != 0 && vallen !=0){
        key[keylen-1]=value[vallen-1]=0;
        int val=atoi(value);
        str2int[key]=val;
        maxidx= std::max(maxidx,val);
      }
   }
   int2str.resize(maxidx+1);   
   for(auto it=str2int.begin();it!=str2int.end();it++)
      int2str[it->second]=it->first.c_str();
   return 0;
}

//int StorageStreamIndexedBinary::WriteIndex(const char * fname){
int StorageStreamIndexedBinary::WriteIndex(){
   //writting index
   for(auto it=str2int.begin();it!=str2int.end();it++){
     char buf[256];
     auto bytes = sprintf_s(buf,"%s\n",it->first.c_str());
     m_streamMediaIndex->write(buf, bytes);
     bytes = sprintf_s(buf,"%i\n",it->second);
     m_streamMediaIndex->write(buf, bytes);
   }
   return 0;
}

void StorageStreamIndexedBinary::Close(){
   if(saveindex){
     WriteIndex();
     saveindex=0;
   }
   idxcntr=0;
   str2int.clear();
}

//atomic storage operations
const char* StorageStreamIndexedBinary::GetNodeName(){
   return &strdata[0];
}

int StorageStreamIndexedBinary::NextItem(){
  //data set parsing
  m_streamMedia->read(&type,sizeof(type));
  std::uint32_t node_id;
  if(m_streamMedia->eos())
    return 1;
  switch(type){
  case 0: { // start node
    m_streamMedia->read(&node_id,sizeof(node_id));
    char const* str = int2str[node_id];

    biglen = (std::uint32_t)strlen(str);
    assert(std::numeric_limits<std::uint16_t>::max() >= biglen);
    shortlen = std::uint16_t(biglen);

    strdata.resize(biglen + 1);
    memcpy(&strdata[0], str, biglen + 1);
    return 0;
  }
  case 1: // end node
    return 1;
  case 2: // string
  case 6: // binary
    if (type == 6) { //this is for binary: case 6
      m_streamMedia->read(&biglen,sizeof(biglen));
      shortlen = biglen;
    } else { //this is for string: case 2
      m_streamMedia->read(&shortlen,sizeof(shortlen));
      biglen = shortlen;
    }
    strdata.resize(biglen + 1);
    m_streamMedia->read(strdata.data(),biglen);
    strdata[biglen]='\0';
    return 2;
  case 3: // data node int
    m_streamMedia->read(&idata,sizeof(idata));
    return 2;
  case 4: // data node float
    m_streamMedia->read(&fdata,sizeof(fdata));
    return 2;
  case 5: // double
    m_streamMedia->read(&ddata,sizeof(ddata));
    return 2;
  }
  return 2;
}

void StorageStreamIndexedBinary::PutStartNode(const char *s){
  auto it = str2int.insert(std::pair<std::string,int>(s,0));
  if(it.second)
    it.first->second=++idxcntr;

  bool isVector = !strcmp(s,"vector");
  if(!isVector) {
    auto node_id = it.first->second;
    std::uint8_t t = 0;
    m_streamMedia->write(&t, sizeof(t));
    m_streamMedia->write(&node_id, sizeof(node_id));
  }
}

void StorageStreamIndexedBinary::PutEndNode(const char *s){
  bool isVector = !strcmp(s,"vector");
  if(!isVector) {
    std::uint8_t t = 1;
    m_streamMedia->write(&t, sizeof(t));
  }
}

void StorageStreamIndexedBinary::GetItem(int* v){
   switch(type){
      case 2: //from string
         *v = atoi(&strdata[0]);
         break;
      case 3: //from int
         *v = idata;
         break;
      case 4: //from float
         *v = (int)fdata;
         break;
      case 5: //from double
         *v = (int)ddata;
         break;
      default:
         *v = 0;
         break;
   }
}

void StorageStreamIndexedBinary::GetItem(float* v){
   switch(type){
      case 2: //from string
         *v = (float)atof(&strdata[0]);
         break;
      case 3: //from int
         *v = (float)idata;
         break;
      case 4: //from float
         *v = fdata;
         break;
      case 5: //from float
         *v = (float)ddata;
         break;
      default:
         *v = 0.0f;
         break;
   }
}

void StorageStreamIndexedBinary::GetItem(double* v){
   switch(type){
      case 2: //from string
         *v = atof(&strdata[0]);
         break;
      case 3: //from int
         *v = (double)idata;
         break;
      case 4: //from float
         *v = fdata;
         break;
      case 5: //from double
         *v = ddata;
         break;
      default:
         *v = 0.0;
         break;
   }
}

void StorageStreamIndexedBinary::GetItem(char const** v){
   switch(type){
      case 2: //from string
      case 6: //from binary
         *v = &strdata[0];
         break;
      case 3: //from int
         sprintf_s(&strdata[0], strdata.size(), "%d", idata);
         *v = &strdata[0];
         break;
    case 4: // from float
         sprintf_s(&strdata[0], strdata.size(), "%e", fdata);
         *v = &strdata[0];
         break;
      case 5: // from double
         sprintf_s(&strdata[0], strdata.size(), "%e", ddata);
         *v = &strdata[0];
         break;
   }
}

void StorageStreamIndexedBinary::GetItem(void const** v, size_t* n){
   switch(type){
      case 2: //from string
      case 6: //from binary
         *v = &strdata[0];
         *n = biglen;
         break;
      default:
         *v = &zero_ch;
         *n = 0;
         break;
   }
}


void StorageStreamIndexedBinary::PutItem(int* v){
   std::uint8_t t = 3;
   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(v, sizeof(*v));
}

void StorageStreamIndexedBinary::PutItem(float* v){
   std::uint8_t t = 4;
   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(v, sizeof(*v));
}

void StorageStreamIndexedBinary::PutItem(double* v){
   std::uint8_t t = 5;
   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(v, sizeof(*v));
}

void StorageStreamIndexedBinary::PutItem(const char* v){
   std::uint8_t t = 2;
   auto len_temp = strlen(v);
   assert(std::numeric_limits<std::uint16_t>::max() >= len_temp);
   std::uint16_t len = (std::uint16_t)(len_temp);

   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(&len, sizeof(len));
   m_streamMedia->write(v, len);
}

void StorageStreamIndexedBinary::PutItem(void const* v, size_t n){
   std::uint8_t t = 6;

   assert(std::numeric_limits<std::uint32_t>::max() >= n);
   std::uint32_t len = std::uint32_t(n);

   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(&len, sizeof(len));
   m_streamMedia->write(v, len);
}

}
