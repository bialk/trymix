#include "storagestreamindexedbinary.h"


namespace sV2{

constexpr char const zero_ch = '\0';

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

StorageStreamFormatter::StreamItemType StorageStreamSimpleBinary::NextItem(){
   //data set parsing
   //fread(&type,1,1,f);
   m_streamMedia->read(&type,1);
   //if(!f || feof(f)) return 1;
   if(m_streamMedia->eos()) return EndNode;
   switch(type){
      case StartNode: // start node
      case StringType: // string
      case BinaryType: // binary
         if (type == BinaryType) {
           m_streamMedia->read(&biglen, sizeof(biglen));
            shortlen = biglen;
         } else {
           m_streamMedia->read(&shortlen, sizeof(shortlen));
            biglen = shortlen;
         }
         strdata.resize(biglen + 1);
         m_streamMedia->read(strdata.data(), biglen);
         strdata[biglen] = zero_ch;
         return type == StartNode ? StartNode : StringType;
      case EndNode: // end node
         return EndNode;
      case IntType: // int
         m_streamMedia->read(&idata, sizeof(idata));
         return StringType;
      case FloatType: // float
         m_streamMedia->read(&fdata, sizeof(fdata));
         return StringType;
      case DoubleType: // double
         m_streamMedia->read(&ddata, sizeof(ddata));
         return StringType;
   }
   return StringType;
}

void StorageStreamSimpleBinary::PutStartNode(const char *s){
  bool isVector = !strcmp(s,"vector");
  if(!isVector) {
    char t = StartNode;
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
    char t = EndNode;
    m_streamMedia->write(&t, 1);
  }
}

void StorageStreamSimpleBinary::GetItem(int* v){
   switch(type){
      case StringType: //from string
         *v = atoi(&strdata[0]);
         break;
      case IntType: //from int
         *v = idata;
         break;
      case FloatType: //from float
         *v = (int)fdata;
         break;
      case DoubleType: //from double
         *v = (int)ddata;
         break;
      default:
         *v = 0;
         break;
   }
}

void StorageStreamSimpleBinary::GetItem(float* v){
   switch(type){
      case StringType: //from string
         *v = (float)atof(&strdata[0]);
         break;
      case IntType: //from int
         *v = (float)idata;
         break;
      case FloatType: //from float
         *v = fdata;
         break;
      case DoubleType: //from double
         *v = (float)ddata;
         break;
      default:
         *v = 0.0f;
         break;
   }
}

void StorageStreamSimpleBinary::GetItem(double* v){
   switch(type){
      case StringType: //from string
         *v = atof(&strdata[0]);
         break;
      case IntType: //from int
         *v = (double)idata;
         break;
      case FloatType: //from float
         *v = fdata;
         break;
      case DoubleType: //from double
         *v = ddata;
         break;
      default:
         *v = 0.0;
         break;
   }
}

void StorageStreamSimpleBinary::GetItem(char const** v){
  switch(type){
  case StringType: //from string
  case BinaryType: //from binary
    *v = &strdata[0];
    break;
  case IntType: //from int
    strdata.resize(32);
    sprintf_s(&strdata[0],strdata.size(), "%d", idata);
    *v = &strdata[0];
    break;
  case FloatType: // from float
    strdata.resize(32);
    sprintf_s(&strdata[0],strdata.size(), "%e", fdata);
    *v = &strdata[0];
    break;
  case DoubleType: // from double
    strdata.resize(32);
    sprintf_s(&strdata[0],strdata.size(), "%e", ddata);
    *v = &strdata[0];
    break;
  }
}

void StorageStreamSimpleBinary::GetItem(void const** v, size_t* n){
   switch(type){
      case StringType: //from string
      case BinaryType: //from binary
         *v = &strdata[0];
         *n = biglen;
         break;
      default:
         *v = &zero_ch;
         *n = 0;
   }
}

void StorageStreamSimpleBinary::PutItem(int* v){
   char t = IntType;
   m_streamMedia->write(&t, 1);
   m_streamMedia->write(v, sizeof(*v));
}

void StorageStreamSimpleBinary::PutItem(float* v){
   char t = FloatType;
   m_streamMedia->write(&t, 1);
   m_streamMedia->write(v, sizeof(*v));
}

void StorageStreamSimpleBinary::PutItem(double* v){
   char t = DoubleType;
   m_streamMedia->write(&t, 1);
   m_streamMedia->write(v, sizeof(*v));
}

void StorageStreamSimpleBinary::PutItem(const char* v){
   char t = StringType;

   auto len_temp = strlen(v);
   assert(len_temp < std::numeric_limits<std::uint16_t>::max());
   auto len = std::uint16_t(len_temp);

   m_streamMedia->write(&t, 1);
   m_streamMedia->write(&len, sizeof(len));
   m_streamMedia->write((void *)v, len);
}

void StorageStreamSimpleBinary::PutItem(void const* v, size_t n){
   char t = BinaryType;

   assert(n < std::numeric_limits<std::uint16_t>::max());
   auto len = std::uint32_t(n);

   m_streamMedia->write(&t, 1);
   m_streamMedia->write(&len, sizeof(len));
   if (len != 0) { // This check is mandatory.
      m_streamMedia->write((void *)v, len);
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

StorageStreamFormatter::StreamItemType StorageStreamIndexedBinary::NextItem(){
  //data set parsing
  m_streamMedia->read(&type,sizeof(type));
  std::uint32_t node_id;
  if(m_streamMedia->eos())
    return EndNode;
  switch(type){
  case StartNode: { // start node
    m_streamMedia->read(&node_id,sizeof(node_id));
    char const* str = int2str[node_id];

    biglen = (std::uint32_t)strlen(str);
    assert(std::numeric_limits<std::uint16_t>::max() >= biglen);
    shortlen = std::uint16_t(biglen);

    strdata.resize(biglen + 1);
    memcpy(&strdata[0], str, biglen + 1);
    return StartNode;
  }
  case EndNode: // end node
    return EndNode;
  case StringType: // string
  case BinaryType: // binary
    if (type == 6) { //this is for binary: case 6
      m_streamMedia->read(&biglen,sizeof(biglen));
      shortlen = biglen;
    } else { //this is for string: case 2
      m_streamMedia->read(&shortlen,sizeof(shortlen));
      biglen = shortlen;
    }
    strdata.resize(biglen + 1);
    m_streamMedia->read(strdata.data(),biglen);
    strdata[biglen]=zero_ch;
    return StringType;
  case IntType: // data node int
    m_streamMedia->read(&idata,sizeof(idata));
    return StringType;
  case FloatType: // data node float
    m_streamMedia->read(&fdata,sizeof(fdata));
    return StringType;
  case DoubleType: // double
    m_streamMedia->read(&ddata,sizeof(ddata));
    return StringType;
  }
  return StringType;
}

void StorageStreamIndexedBinary::PutStartNode(const char *s){
  auto it = str2int.insert(std::pair<std::string,int>(s,0));
  if(it.second)
    it.first->second=++idxcntr;

  bool isVector = !strcmp(s,"vector");
  if(!isVector) {
    auto node_id = it.first->second;
    std::uint8_t t = StartNode;
    m_streamMedia->write(&t, sizeof(t));
    m_streamMedia->write(&node_id, sizeof(node_id));
  }
}

void StorageStreamIndexedBinary::PutEndNode(const char *s){
  bool isVector = !strcmp(s,"vector");
  if(!isVector) {
    std::uint8_t t = EndNode;
    m_streamMedia->write(&t, sizeof(t));
  }
}

void StorageStreamIndexedBinary::GetItem(int* v){
   switch(type){
      case StringType: //from string
         *v = atoi(&strdata[0]);
         break;
      case IntType: //from int
         *v = idata;
         break;
      case FloatType: //from float
         *v = (int)fdata;
         break;
      case DoubleType: //from double
         *v = (int)ddata;
         break;
      default:
         *v = 0;
         break;
   }
}

void StorageStreamIndexedBinary::GetItem(float* v){
   switch(type){
      case StringType: //from string
         *v = (float)atof(&strdata[0]);
         break;
      case IntType: //from int
         *v = (float)idata;
         break;
      case FloatType: //from float
         *v = fdata;
         break;
      case DoubleType: //from double
         *v = (float)ddata;
         break;
      default:
         *v = 0.0f;
         break;
   }
}

void StorageStreamIndexedBinary::GetItem(double* v){
   switch(type){
      case StringType: //from string
         *v = atof(&strdata[0]);
         break;
      case IntType: //from int
         *v = (double)idata;
         break;
      case FloatType: //from float
         *v = fdata;
         break;
      case DoubleType: //from double
         *v = ddata;
         break;
      default:
         *v = 0.0;
         break;
   }
}

void StorageStreamIndexedBinary::GetItem(char const** v){
   switch(type){
      case StringType: //from string
      case BinaryType: //from binary
         *v = &strdata[0];
         break;
      case IntType: //from int
         sprintf_s(&strdata[0], strdata.size(), "%d", idata);
         *v = &strdata[0];
         break;
    case FloatType: // from float
         sprintf_s(&strdata[0], strdata.size(), "%e", fdata);
         *v = &strdata[0];
         break;
      case DoubleType: // from double
         sprintf_s(&strdata[0], strdata.size(), "%e", ddata);
         *v = &strdata[0];
         break;
   }
}

void StorageStreamIndexedBinary::GetItem(void const** v, size_t* n){
   switch(type){
      case StringType: //from string
      case BinaryType: //from binary
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
   std::uint8_t t = IntType;
   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(v, sizeof(*v));
}

void StorageStreamIndexedBinary::PutItem(float* v){
   std::uint8_t t = FloatType;
   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(v, sizeof(*v));
}

void StorageStreamIndexedBinary::PutItem(double* v){
   std::uint8_t t = DoubleType;
   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(v, sizeof(*v));
}

void StorageStreamIndexedBinary::PutItem(const char* v){
   std::uint8_t t = StringType;
   auto len_temp = strlen(v);
   assert(std::numeric_limits<std::uint16_t>::max() >= len_temp);
   std::uint16_t len = (std::uint16_t)(len_temp);

   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(&len, sizeof(len));
   m_streamMedia->write(v, len);
}

void StorageStreamIndexedBinary::PutItem(void const* v, size_t n){
   std::uint8_t t = BinaryType;

   assert(std::numeric_limits<std::uint32_t>::max() >= n);
   std::uint32_t len = std::uint32_t(n);

   m_streamMedia->write(&t, sizeof(t));
   m_streamMedia->write(&len, sizeof(len));
   m_streamMedia->write(v, len);
}

}
