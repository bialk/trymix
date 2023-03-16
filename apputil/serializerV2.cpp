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
      Serializer(s).Load();
    }
  }
}


Serializer::~Serializer(){
  //delete all syncronizers
  std::list<std::pair<std::string,SyncDataInterface*> >::iterator it;
  for(it=datalist.begin();it!=datalist.end();it++){
    delete (it->second);
  }
}

// call back to build index of element for node
void Serializer::Item(const char* name, SyncDataInterface* sdi){
  dataset[name]=sdi;
  datalist.push_back(std::pair<std::string,SyncDataInterface*>(std::string(name),sdi));
};

//two technique for load/storing data from/into storage
void Serializer::Load(){  
  while(1){
    int type = ss->NextItem();
    if(type == 1){ //endnode
      return;
    }
    else if(type == 0){ //start node
      auto it=dataset.find(ss->GetNodeName());
      if(it!=dataset.end()){
        it->second->Load(this);
      }
      else{
        Serializer(this).Load(); //bypass if item is not found
      }
    }
    else if(type==2){ //data node
      //assert(!"error of stream syncronization");
    }
  }
};

void Serializer::Store(){
  std::list<std::pair<std::string,SyncDataInterface*> >::iterator it;
  for(it=datalist.begin();it!=datalist.end();it++){
    ss->PutStartNode(it->first.c_str());
    it->second->Store(this);
    ss->PutEndNode(it->first.c_str());
  }
};


}
