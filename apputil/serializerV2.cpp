#include "serializerV2.h"

//#include "system.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

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
   //Close();
}

////stream initialization operation
//int StorageStreamSimpleBinary::OpenForWrite(const char * fname){
//   Close();
//   f=fopen(fname,"w+b");
//   return !f;
//}
//int StorageStreamSimpleBinary::OpenForRead(const char * fname){
//   Close();
//   f=fopen(fname,"rb");
//   return !f;
//}
//void StorageStreamSimpleBinary::Close(){
//   if(f!=NULL){
//      fclose(f);
//      f=NULL;
//   }
//}

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
            //fread(&biglen, 1, sizeof(biglen), f);
           m_streamMedia->read(&biglen, sizeof(biglen));
            shortlen = biglen;
         } else {
            //fread(&shortlen, 1, sizeof(shortlen), f);
           m_streamMedia->read(&shortlen, sizeof(shortlen));
            biglen = shortlen;
         }
         strdata.resize(biglen + 1);
         //fread(&strdata[0], 1, biglen, f);
         m_streamMedia->read(strdata.data(), biglen);
         strdata[biglen] = '\0';
         return type == 0 ? 0 : 2;
      case 1: // end node
         return 1;
      case 3: // int
         //fread(&idata,sizeof(idata),1,f);
         m_streamMedia->read(&idata, sizeof(idata));
         return 2;
      case 4: // float
         //fread(&fdata,sizeof(fdata),1,f);
         m_streamMedia->read(&fdata, sizeof(fdata));
         return 2;
      case 5: // double
         //fread(&ddata,sizeof(ddata),1,f);
         m_streamMedia->read(&ddata, sizeof(ddata));
         return 2;
   }
   return 2;
}

void StorageStreamSimpleBinary::PutStartNode(const char *s){
   char t = 0;

   auto len_temp = strlen(s);
   assert(len_temp < std::numeric_limits<std::uint16_t>::max());
   std::uint16_t len = std::uint16_t(len_temp);
//   fwrite(&t,1,1,f);
//   fwrite(&len,sizeof(len),1,f);
//   fwrite(s,1,len,f);
   m_streamMedia->write(&t, 1);
   m_streamMedia->write(&len, sizeof(len));
   m_streamMedia->write((void*)s, len);
}
void StorageStreamSimpleBinary::PutEndNode(const char *s){
   char t = 1;
   //fwrite(&t,1,1,f);
   m_streamMedia->write(&t, 1);
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
//   fwrite(&t,1,1,f);
//   fwrite(v,sizeof(int),1,f);
   m_streamMedia->write(&t, 1);
   m_streamMedia->write(v, sizeof(*v));
}
void StorageStreamSimpleBinary::PutItem(float* v){
   char t = 4;
//   fwrite(&t,1,1,f);
//   fwrite(v,sizeof(float),1,f);
   m_streamMedia->write(&t, 1);
   m_streamMedia->write(v, sizeof(*v));
}
void StorageStreamSimpleBinary::PutItem(double* v){
   char t = 5;
//   fwrite(&t,1,1,f);
//   fwrite(v,sizeof(double),1,f);
   m_streamMedia->write(&t, 1);
   m_streamMedia->write(v, sizeof(*v));
}
void StorageStreamSimpleBinary::PutItem(const char* v){
   char t = 2;

   auto len_temp = strlen(v);
   assert(len_temp < std::numeric_limits<std::uint16_t>::max());
   std::uint16_t len = std::uint16_t(len_temp);

//   fwrite(&t,1,1,f);
//   fwrite(&len,sizeof(len),1,f);
//   fwrite(v,1,len,f);
   m_streamMedia->write(&t, 1);
   m_streamMedia->write(&len, sizeof(len));
   m_streamMedia->write((void *)v, len);
}
void StorageStreamSimpleBinary::PutItem(void const* v, size_t n)
{
   char t = 6;

   assert(n < std::numeric_limits<std::uint16_t>::max());
   std::uint16_t len = std::uint16_t(n);

   //   fwrite(&t,1,1,f);
//   fwrite(&len,sizeof(len),1,f);
//   fwrite(v,1,len,f);
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
   //if(!*strm) return 1;
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
   char t = 0;

   auto len_temp = strlen(s);
   assert(len_temp < std::numeric_limits<std::uint16_t>::max());
   std::uint16_t len = std::uint16_t(len_temp);
//   strm->write((char const*)&t, 1);
//   strm->write((char const*)&len, sizeof(len));
//   strm->write((char const*)s, len);
   strm->write(&t, 1);
   strm->write(&len, sizeof(len));
   strm->write((void*)s, len);
}
void StorageStreamSimpleIostream::PutEndNode(const char *s){
   char t = 1;
   //strm->write((char const*)&t, 1);
   strm->write(&t, 1);
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
//   strm->write((char const*)&t, 1);
//   strm->write((char const*)v, sizeof(*v));
   strm->write(&t, 1);
   strm->write(v, sizeof(*v));
}
void StorageStreamSimpleIostream::PutItem(float* v){
   char t = 4;
//   strm->write((char const*)&t, 1);
//   strm->write((char const*)v, sizeof(*v));
   strm->write(&t, 1);
   strm->write(v, sizeof(*v));
}
void StorageStreamSimpleIostream::PutItem(double* v){
   char t = 5;
//   strm->write((char const*)&t, 1);
//   strm->write((char const*)v, sizeof(*v));
   strm->write(&t, 1);
   strm->write(v, sizeof(*v));
}
void StorageStreamSimpleIostream::PutItem(const char* v){
   char t = 2;

   auto len_temp = strlen(v);
   assert(len_temp < std::numeric_limits<std::uint16_t>::max());
   std::uint16_t len = std::uint16_t(len_temp);

//   strm->write((char const*)&t, 1);
//   strm->write((char const*)&len, sizeof(len));
//   strm->write(v, len);
   strm->write(&t, 1);
   strm->write(&len, sizeof(len));
   strm->write((void *)v, len);
}
void StorageStreamSimpleIostream::PutItem(const void* v, size_t n){
   char t = 6;

   assert(n < std::numeric_limits<std::uint16_t>::max());
   std::uint16_t len = std::uint16_t(n);

//   strm->write((char const*)&t, 1);
//   strm->write((char const*)&len, sizeof(len));
//   if (len != 0) { // This check is mandatory.
//      strm->write((char const*)v, len);
//   }
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
   //f(),
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

////stream initialization operation
//int StorageStreamIndexedBinary::OpenForWrite(const char * fname){
//   Close();
//   f=fopen(fname,"w+b");
//   if(!f) return 1;
//   saveindex=1;
//   indexfilename=fname;
//   indexfilename+=".idx";
//   return 0;
//}

//int StorageStreamIndexedBinary::OpenForRead(const char * fname){
//   Close();
//   if(ReadIndex((std::string(fname)+".idx").c_str())) return 1;
//   f=fopen(fname,"rb");
//   if(!f) return 1;
//   return 0;
//}

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

//int StorageStreamIndexedBinary::ReadIndex(const char * fname){
int StorageStreamIndexedBinary::ReadIndex(){
   //reading index
//   FILE *fidx=fopen(fname,"r+b");
//   if(!fidx) return 1;
   int maxidx=0;
   //while(fidx && !feof(fidx)){
   while(!m_streamMediaIndex->eos()){
      char key[256],value[50];
//      fgets(key,200,fidx);
//      fgets(value,200,fidx);
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
   //fclose(fidx);
   return 0;
}

//int StorageStreamIndexedBinary::WriteIndex(const char * fname){
int StorageStreamIndexedBinary::WriteIndex(){
   //writting index
   //FILE *fidx=fopen(fname,"w+b");
   //if(!fidx) return 1;
   for(auto it=str2int.begin();it!=str2int.end();it++){
     char buf[256];
     auto bytes = sprintf_s(buf,"%s\n",it->first.c_str());
     m_streamMediaIndex->write(buf, bytes);
     bytes = sprintf_s(buf,"%i\n",it->second);
     m_streamMediaIndex->write(buf, bytes);
   }
   //fclose(fidx);
   return 0;
}


void StorageStreamIndexedBinary::Close(){
//   if(f!=NULL){
//      fclose(f);
//      f=NULL;
//   }
   if(saveindex){
     //WriteIndex(indexfilename.c_str());
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
  //fread(&type,1,1,f);
  m_streamMedia->read(&type,sizeof(type));
  std::uint32_t node_id;
  //if(!f || feof(f)) return 1;
  if(m_streamMedia->eos())
    return 1;
  switch(type){
  case 0: { // start node
    //fread(&node_id,sizeof(int),1,f);
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
      //fread(&biglen, 1, sizeof(biglen), f);
      m_streamMedia->read(&biglen,sizeof(biglen));
      shortlen = biglen;
    } else { //this is for string: case 2
      // fread(&shortlen, 1, sizeof(shortlen), f);
      m_streamMedia->read(&shortlen,sizeof(shortlen));
      biglen = shortlen;
    }
    strdata.resize(biglen + 1);
    //fread(&strdata[0],1,biglen,f);
    m_streamMedia->read(strdata.data(),biglen);
    strdata[biglen]='\0';
    return 2;
  case 3: // data node int
    //fread(&idata,sizeof(idata),1,f);
    m_streamMedia->read(&idata,sizeof(idata));
    return 2;
  case 4: // data node float
    //fread(&fdata,sizeof(fdata),1,f);
    m_streamMedia->read(&fdata,sizeof(fdata));
    return 2;
  case 5: // double
    //fread(&ddata,sizeof(ddata),1,f);
    m_streamMedia->read(&ddata,sizeof(ddata));
    return 2;
  }
  return 2;
}

void StorageStreamIndexedBinary::PutStartNode(const char *s){
   auto it = str2int.insert(std::pair<std::string,int>(s,0));
   if(it.second)
      it.first->second=++idxcntr;

   auto node_id = it.first->second;
   std::uint8_t t = 0;

   //fwrite(&node_id,sizeof(int),1,f);
   //fwrite(&t,1,1,f);
   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(&node_id, sizeof(node_id));
}

void StorageStreamIndexedBinary::PutEndNode(const char *s){
   std::uint8_t t = 1;
   //fwrite(&t,1,1,f);
   m_streamMedia->write(&t, sizeof(t));
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
//   fwrite(&t,1,1,f);
//   fwrite(v,sizeof(int),1,f);
   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(v, sizeof(*v));
}
void StorageStreamIndexedBinary::PutItem(float* v){
   std::uint8_t t = 4;
   //fwrite(&t,1,1,f);
   //fwrite(v,sizeof(float),1,f);
   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(v, sizeof(*v));
}
void StorageStreamIndexedBinary::PutItem(double* v){
   std::uint8_t t = 5;
   //fwrite(&t,1,1,f);
   //fwrite(v,sizeof(double),1,f);
   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(v, sizeof(*v));
}
void StorageStreamIndexedBinary::PutItem(const char* v){
   std::uint8_t t = 2;
   auto len_temp = strlen(v);
   assert(std::numeric_limits<std::uint16_t>::max() >= len_temp);
   std::uint16_t len = (std::uint16_t)(len_temp);

   //fwrite(&t,1,1,f);
   //fwrite(&len,sizeof(len),1,f);
   //fwrite(v,1,len,f);
   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(&len, sizeof(len));
   m_streamMedia->write(v, len);
}
void StorageStreamIndexedBinary::PutItem(void const* v, size_t n){
   std::uint8_t t = 6;

   assert(std::numeric_limits<std::uint32_t>::max() >= n);
   std::uint32_t len = std::uint32_t(n);

   //fwrite(&t,1,1,f);
   //fwrite(&len,sizeof(len),1,f);
   //fwrite(v,1,len,f);
   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(&len, sizeof(len));
   m_streamMedia->write(v, len);
}


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
   if(m_streamMedia->eos())
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
  char buf[256];
   auto bytes = sprintf_s(buf, "%s<%s>\n", indent.c_str(), s);
   m_streamMedia->write(buf, bytes);

   indent.resize(indent.size() + tabsz, ' ');
}
void StorageStreamSimpleXML::PutEndNode(const char *s){
   indent.resize(indent.size() - tabsz);
   char buf[256];
   auto bytes = sprintf_s(buf, "%s</%s>\n", indent.c_str(), s);
   m_streamMedia->write(buf,bytes);

}
void StorageStreamSimpleXML::GetItem(int* v){
   *v = atoi(strbegin);
}
void StorageStreamSimpleXML::GetItem(float* v){
   *v = (float)atof(strbegin);
}
void StorageStreamSimpleXML::GetItem(double* v){
   *v = atof(strbegin);
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
   //fprintf(f, "%s%e\n", indent.c_str(), *v);
   char buf [256];
   auto bytes = sprintf_s(buf, "%s%e\n", indent.c_str(), *v);
   m_streamMedia->write(buf,bytes);
}
void StorageStreamSimpleXML::PutItem(double* v){
   //fprintf(f, "%s%e\n", indent.c_str(), *v);
   char buf [256];
   auto bytes = sprintf_s(buf, "%s%e\n", indent.c_str(), *v);
   m_streamMedia->write(buf,bytes);
}
void StorageStreamSimpleXML::PutItem(const char* v){
   //fprintf(f, "%s%s\n", indent.c_str(), v);
   char buf [256];
   auto bytes = sprintf_s(buf, "%s%s\n", indent.c_str(), v);
   m_streamMedia->write(buf,bytes);
}
void StorageStreamSimpleXML::PutItem(void const* v, size_t n)
{
//   fprintf(f, "%s<binary>%s</binary>\n", indent.c_str(),
//      toBase64((char const*)v, (char const*)v + n).c_str());
   char buf [1024];
   auto bytes = sprintf_s(buf, "%s<binary>%s</binary>\n", indent.c_str(),
         toBase64((char const*)v, (char const*)v + n).c_str());
   m_streamMedia->write(buf,bytes);
}

char* StorageStreamSimpleXML::decodeBase64InPlace(char* begin, char* end)
{
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

std::string StorageStreamSimpleXML::toBase64(char const* begin, char const* end)
{
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

char const StorageStreamSimpleXML::m_base64EncodeChars[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

signed char const StorageStreamSimpleXML::m_base64DecodeChars[] = {
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
