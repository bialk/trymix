#include "storagestreamjson.h"


namespace sV2{

StorageStreamSimpleJson::StorageStreamSimpleJson(StreamMedia* sm){}
StorageStreamSimpleJson::~StorageStreamSimpleJson(){}

const char* StorageStreamSimpleJson::GetNodeName(){ return {};};
int StorageStreamSimpleJson::NextItem(){ return {};};
void StorageStreamSimpleJson::GetItem(int* v){};
void StorageStreamSimpleJson::GetItem(float* v){};
void StorageStreamSimpleJson::GetItem(double* v){};
void StorageStreamSimpleJson::GetItem(char const** v){};
void StorageStreamSimpleJson::GetItem(void const** v, size_t* n){};


void StorageStreamSimpleJson::PutStartNode(const char *s){
  auto len = strlen(s);
  m_streamMedia->write("\"",len);
  m_streamMedia->write(s,len);
  m_streamMedia->write("\":",len);
};
void StorageStreamSimpleJson::PutEndNode(const char *s){};

void StorageStreamSimpleJson::PutItem(int* v){};
void StorageStreamSimpleJson::PutItem(float* v){};
void StorageStreamSimpleJson::PutItem(double* v){};
void StorageStreamSimpleJson::PutItem(const char* v){};
void StorageStreamSimpleJson::PutItem(void const* v, size_t n){};

}
