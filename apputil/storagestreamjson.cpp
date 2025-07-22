#include "storagestreamjson.h"
#include "base64encoder.h"

#include <assert.h>
#include <functional>

namespace sV2{

//JSON storage

StorageStreamSimpleJson::StorageStreamSimpleJson(StreamMedia* sm)
  :m_streamMedia(sm)
  ,tabsz(1)
{
    //prepare ring buffer
    buf.resize(4096, '\x0');
    strbegin = strend = &buf[0];
    bufBeginOff = 0;
    bufEndOff = 0;

    //reset context
    context.push({});

    l1gram = {
        [&](char c){ // InString
            if(c == '\\'){
                pState = InStringSlash;
            }
            else if(c == '"'){
                found_tokens.push_back(token);
                token.clear();
                pState = InSpace;
            }
            else{
                // collect symbol to string
                token.push_back(c);
            }
        },
        [&](char c){ // InStringSlash
            //collect symbol to string
            token.push_back(c);
            pState = InString;
        },
        [&](char c){ // InTocken
            if(specialsymbol.find(c) !=  std::string::npos){
                //collect symbol to string
                found_tokens.push_back(token);
                token.clear();
                found_tokens.push_back({c});
                pState = InSpace;
            }
            else if(whitespace.find(c) !=  std::string::npos ){
                found_tokens.push_back(token);
                token.clear();
                pState = InSpace;
            }
            else{
                //collect symbol to string
                token.push_back(c);
            }
        },
        [&](char c){ // InSpace
            if(whitespace.find(c) !=  std::string::npos ){
                // do nothing to string
            }
            else if(specialsymbol.find(c) !=  std::string::npos){
                found_tokens.push_back({c});
            }
            else if(c == '\"'){
                //collect symbol to string
                pState = InString;
            }
            else {
                token.push_back(c);
                pState = InToken;
            }
        }
    };

    readSymbol = [&](){
        if(bufferCounter == 0){
            if(m_streamMedia->eos())
                return '\x0';
            else{
                bufferCounter = m_streamMedia->read((void*)buffer,1024);
                position = 0;
            }
        }
        bufferCounter--;
        return buffer[position++];
    };
}

StorageStreamSimpleJson::~StorageStreamSimpleJson(){
}

//atomic storage operations
const char* StorageStreamSimpleJson::GetNodeName(){
  return nodename.c_str();
}


std::string StorageStreamSimpleJson::nextTocken(){

  for(;;){
    if(!found_tokens.empty()){
      auto out = found_tokens.front();
      found_tokens.pop_front();
      return out;
    }
    char const nextSymbol = readSymbol();
    if(nextSymbol == 0)
      found_tokens.push_back("}");
    else
      l1gram[pState](nextSymbol);
  }
}

StorageStream::StreamItemType StorageStreamSimpleJson::NextItem(){

  for(;;){
    if(!found_types.empty()){
      auto ret = found_types.front();
      found_types.pop_front();
      return ret;
    }

    auto token = nextTocken();
    symbols.second = symbols.first;
    symbols.first = token[0];

    switch(symbols.first){
    case ':':
      break;
    case ',':
      if(specialsymbol.find(symbols.second) ==  std::string::npos){
        found_types.push_back(StartNode);
        found_types.push_back(StringType);
        found_types.push_back(EndNode);
      }
      break;
    case '{':
      if(symbols.second == ' ') // we get first open bracket
        break;
    case '[':
      if(symbols.second == ':')
        nodename = value;
      else
        nodename = "item";      
      return StartNode; //start node
      break;
    case '}':
    case ']':
      if(specialsymbol.find(symbols.second) ==  std::string::npos){
        found_types.push_back(StartNode);
        found_types.push_back(StringType);
        found_types.push_back(EndNode);
      }
      found_types.push_back(EndNode); // end node
      break;
    default:
      if(symbols.second == ':')
        nodename = value;
      else
        nodename = "item";
      value = token;
      break;
    }

  }
}

void StorageStreamSimpleJson::GetItem(int* v){
  *v = atoi(value.c_str());
}

void StorageStreamSimpleJson::GetItem(float* v){
  *v = (float)atof(value.c_str());
}

void StorageStreamSimpleJson::GetItem(double* v){
  *v = atof(value.c_str());
}

void StorageStreamSimpleJson::GetItem(char const** v){
  *v = value.c_str();
}

void StorageStreamSimpleJson::GetItem(void const** v, size_t* n){
  // Binary data.
  *v=value.c_str();
  *n=value.size();
  auto endofDecoding = decodeBase64InPlace((char*)*v,(char*)*v + (*n));
  *n = (char*)endofDecoding - (char*)*v; // counting number of bytes
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

  context.push({0,s});
  context.top().isVector = isVector;

}

void StorageStreamSimpleJson::PutEndNode(const char *s){
  // context.top().itemCount == 0 only for leaf node - nodes, which does not
  // have children - only have atomic values. Such items no need to be closed
  // also special node with name "vector" also does not have children
  // thus there is no close braket for it.
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

}
