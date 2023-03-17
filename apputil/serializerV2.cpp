#include "serializerV2.h"

#include <assert.h>

namespace sV2{

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
      EmptyObject eo;
      CSyncObj<EmptyObject>(&eo).Load(s);
    }
  }
}


Serializer::~Serializer(){
  //delete all syncronizers
  for(auto& it: datalist)
    delete it.second;
}

// call back to build index of element for node
void Serializer::Item(const char* name, SyncDataInterface* sdi){
  dataset[name]=sdi;  
  datalist.emplace_back(std::string(name),sdi);
};

}
