#include "serializerV2.h"

#include <assert.h>

namespace sV2{


Serializer::~Serializer(){
  //delete all syncronizers
  for(auto& it: datalist)
    delete it.second;
}

}
