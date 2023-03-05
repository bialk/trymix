#ifndef SERIALIZERV2_H
#define SERIALIZERV2_H

#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <assert.h>

namespace sV2{
/*
Index based serialization. We want to avoid version control in standart serialization technique.
During data reading we dynamically build index for each node and looking for full syncronization by names.
The functionality:
  1. Registering Data For sycnronization
  2. Objects -> Stream
  3. Stream -> Objects
*/

class Serializer;

class StreamMedia{
public:
  virtual size_t write(void const* buf, size_t size) = 0;
  virtual size_t read(void* buf, size_t size) = 0;
  virtual bool eos() = 0;
};

class SyncDataInterface{
public:
  virtual ~SyncDataInterface(){}
  virtual void Load(Serializer *s){};
  virtual void Store(Serializer *s){};
};

class StorageStream{
public:
  virtual ~StorageStream(){};
  virtual int NextItem()=0;

  virtual const char* GetNodeName()=0;

  virtual void GetItem(int* v)=0;
  virtual void GetItem(float* v)=0;
  virtual void GetItem(double* v)=0;
  virtual void GetItem(char const** v)=0;
  virtual void GetItem(void const** v, size_t* n)=0;


  virtual void PutStartNode(const char *s)=0;
  virtual void PutEndNode(const char *s)=0;

  virtual void PutItem(int* v)=0;
  virtual void PutItem(float* v)=0;
  virtual void PutItem(double* v)=0;
  virtual void PutItem(const char* v)=0;
  virtual void PutItem(void const* v, size_t n)=0;
};


class StorageStreamSimpleBinary:public StorageStream{
public:
  StorageStreamSimpleBinary(StreamMedia* sm);
  ~StorageStreamSimpleBinary();
  //   int  OpenForWrite(const char * fname);
  //   int  OpenForRead(const char * fname);
  //   void Close();
protected:
  const char* GetNodeName() override;
  int NextItem() override;
  void GetItem(int* v) override;
  void GetItem(float* v) override;
  void GetItem(double* v) override;
  void GetItem(char const** v) override;
  void GetItem(void const** v, size_t* n) override;
  
  
  void PutStartNode(const char *s) override;
  void PutEndNode(const char *s) override;
  
  void PutItem(int* v) override;
  void PutItem(float* v) override;
  void PutItem(double* v) override;
  void PutItem(const char* v) override;
  void PutItem(void const* v, size_t n) override;
  
private:
  //FILE *f;
  StreamMedia* m_streamMedia{nullptr};
  
  // aux data type to keep current item
  char type; // 0 - start node, 1 - end node,
  // 2 - data node string, 3 - data node int, 4 - data node float,
  // 5 - data node double, 6 - data node blob
  double ddata;
  float fdata;
  int   idata;
  
  // These two are synchronized.
  unsigned short shortlen;
  unsigned int biglen;
  
  std::vector<char> strdata;
};

/**
 * This one is binary compatible with StorageStreamSimpleBinary.
 * It works with std::iostream subclasses, which makes it possible
 * to use it both with files (std::fstream) and with memory buffers
 * (std::stringstream).
 */
class StorageStreamSimpleIostream:public StorageStream{
public:
  /**
    * Construct a storage stream from an std::iostream derivative,
  * which must already be opened in appropriate mode.
  */
  //StorageStreamSimpleIostream(std::iostream* strm);
  StorageStreamSimpleIostream(StreamMedia* sm);
  ~StorageStreamSimpleIostream();
protected:
  const char* GetNodeName() override;
  int NextItem() override;
  void GetItem(int* v) override;
  void GetItem(float* v) override;
  void GetItem(double* v) override;
  void GetItem(char const** v) override;
  void GetItem(void const** v, size_t *n) override;
  
  
  void PutStartNode(const char *s) override;
  void PutEndNode(const char *s) override;
  
  void PutItem(int* v) override;
  void PutItem(float* v) override;
  void PutItem(double* v) override;
  void PutItem(const char* v) override;
  void PutItem(void const* v, size_t n) override;
  
private:
  //std::iostream* strm;
  StreamMedia* strm{nullptr};
  
  // aux data type to keep current item
  char type; // 0 - start node, 1 - end node,
  // 2 - data node string, 3 - data node int, 4 - data node float,
  // 5 - data node double, 6 - data node blob
  double ddata;
  float fdata;
  int   idata;
  
  // These two are syncrhonized.
  unsigned short shortlen;
  unsigned biglen;
  
  std::vector<char> strdata;
};

class StorageStreamIndexedBinary:public StorageStream{
public:
  StorageStreamIndexedBinary(StreamMedia* sm, StreamMedia* smi);
  ~StorageStreamIndexedBinary();
  
  int  ReadIndex();
  int  WriteIndex();
  
private:
  //index support structure
  int saveindex{0};
  std::string indexfilename;
  int idxcntr;
  StreamMedia* m_streamMedia{nullptr};
  StreamMedia* m_streamMediaIndex{nullptr};
  
  std::map<std::string, std::uint32_t> str2int;
  std::vector<const char*> int2str;
  
  // aux data type to keep current item
  std::uint8_t  type; // 0 - start node, 1 - end node,
  // 2 - data node string, 3 - data node int, 4 - data node float,
  // 5 - data node double, 6 - data node binary
  double ddata;
  float fdata;
  int   idata;
  
  // These two are synchronized.
  std::uint16_t shortlen;
  std::uint32_t biglen;
  
  std::vector<char> strdata;
  
  //public:
  //   int  OpenForWrite(const char * fname);
  //   int  OpenForRead(const char * fname);
  //   int  ReadIndex(const char * fname);
  //   int  WriteIndex(const char * fname);
  void Close();
  
protected:
  const char* GetNodeName() override;
  int NextItem() override;
  void GetItem(int* v) override;
  void GetItem(float* v) override;
  void GetItem(double* v) override;
  void GetItem(char const** v) override;
  void GetItem(void const** v, size_t *n) override;
  
  
  void PutStartNode(const char *s) override;
  void PutEndNode(const char *s) override;
  
  void PutItem(int* v) override;
  void PutItem(float* v) override;
  void PutItem(double* v) override;
  void PutItem(const char* v) override;
  void PutItem(void const* v, size_t n) override;
};

class Serializer{
public:
  
  StorageStream *ss;
  std::map<std::string, SyncDataInterface*> dataset;
  std::list<std::pair<std::string,SyncDataInterface*> > datalist;
  
  Serializer(StorageStream *s):ss(s){}
  Serializer(Serializer *s):ss(s->ss){}
  
  ~Serializer(){
    //delete all syncronizers
    std::list<std::pair<std::string,SyncDataInterface*> >::iterator it;
    for(it=datalist.begin();it!=datalist.end();it++){
      delete (it->second);
    }
  }

  template<typename T>
  void SyncAs(const char* name, T& data){
    auto sdi = Sync(&data);
    dataset[name]=sdi;
    datalist.push_back(std::pair<std::string,SyncDataInterface*>(std::string(name),sdi));
  }

  // call back to build index of element for node
  void Item(const char* name, SyncDataInterface* sdi){
    dataset[name]=sdi;
    datalist.push_back(std::pair<std::string,SyncDataInterface*>(std::string(name),sdi));
  };
  
  //two technique for load/storing data from/into storage
  void Load(){
    std::map<std::string, SyncDataInterface*>::iterator it;
    while(1){
      int type = ss->NextItem();
      if(type == 1){ //endnode
        return;
      }
      else if(type == 0){ //start node
        it=dataset.find(ss->GetNodeName());
        if(it!=dataset.end()){
          it->second->Load(this);
        }
        else{
          Serializer(this).Load();
        }
      }
      else if(type==2){ //data node
        //assert(!"error of stream syncronization");
      }
    }
    
  };
  
  void Store(){
    std::list<std::pair<std::string,SyncDataInterface*> >::iterator it;
    for(it=datalist.begin();it!=datalist.end();it++){
      ss->PutStartNode(it->first.c_str());
      it->second->Store(this);
      ss->PutEndNode(it->first.c_str());
    }
  };
  
  //atomic storage operations
};


//syncronizers for objects
template<class T> class CSyncObj: public SyncDataInterface{
public:
  T *obj;
  CSyncObj(T *t):obj(t){}
  
  void Load(Serializer *s) override{
    Serializer srlz(s);
    obj->AskForData(&srlz);
    srlz.Load();
  }
  void Store(Serializer *s) override{
    Serializer srlz(s);
    obj->AskForData(&srlz);
    srlz.Store();
  }
};

template<class T> inline SyncDataInterface* Sync(T *t){
  return  new CSyncObj<T>(t);
}


// syncronizers for scalar variables
template<class T> class CSyncVar: public SyncDataInterface{
public:
  T *obj;
  CSyncVar(T *t):obj(t){}
  void Load(Serializer *s) override{
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        Serializer(s).Load();
      }
      else if(type==2){
        s->ss->GetItem(obj);
      }
    }
  }
  void Store(Serializer *s) override{
    s->ss->PutItem(obj);
  }
};

template<class T, class R> class CSyncVarAsOtherType: public SyncDataInterface{
public:
  T *obj;
  CSyncVarAsOtherType(T *t):obj(t){}
  void Load(Serializer *s) override{
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        Serializer(s).Load();
      }
      else if(type==2){
        R i;
        s->ss->GetItem(&i);
        *obj=T(i);
      }
    }
  }
  void Store(Serializer *s) override{
    R i=R(*obj);
    s->ss->PutItem(&i);
  }
};

template<> inline SyncDataInterface* Sync<double>(double *t){
  return new CSyncVar<double>(t);
}

template<> inline SyncDataInterface* Sync<float>(float *t){
  return new CSyncVar<float>(t);
}

template<> inline SyncDataInterface* Sync<int>(int *t){
  return new CSyncVar<int>(t);
}

//note: this is only one symbol as int not a char array
template<> inline SyncDataInterface* Sync<char>(char *t){
  return new CSyncVarAsOtherType<char,int>(t);
}


// syncronizer for char string* arrays with fixed length
class CSync_CStr: public SyncDataInterface{
public:
  char *obj;
  int sz;
  CSync_CStr(char *t,int n):obj(t),sz(n){}
  void Load(Serializer *s) override;
  void Store(Serializer *s) override{
    s->ss->PutItem(obj);
  }
};

inline SyncDataInterface* Sync(char *t, int sz){
  return new CSync_CStr(t,sz);
}


// syncronizers for std::string type
class CSync_STLStr: public SyncDataInterface{
public:
  std::string *obj;
  CSync_STLStr(std::string *t):obj(t){}
  void Load(Serializer *s) override{
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==2){
        char const* str;
        s->ss->GetItem(&str);
        *obj = str;
        //strncpy(obj,str,sz);
      }
      else if(type==0){
        Serializer(s).Load();
      }
    }
  }
  void Store(Serializer *s) override{
    s->ss->PutItem(obj->c_str());
  }
};

inline SyncDataInterface* Sync(std::string *t){
  return new CSync_STLStr(t);
}


// syncronizes a sequence of primitive types (char, int, float, ...)
// in storage efficient manner.
template<class T> class CSyncVarVector:public SyncDataInterface{
public:
  std::vector<T>* vec;
  CSyncVarVector(std::vector<T>* v): vec(v){}
  void Load(Serializer *s) override{
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      if(type==2){
        void const* v;
        size_t n;
        s->ss->GetItem(&v, &n);
        T const* begin = static_cast<T const*>(v);
        n /= sizeof(T);
        std::vector<T>(begin, begin + n).swap(*vec);
      } else {
        Serializer(s).Load();
      }
    }
  }
  void Store(Serializer *s) override{
    void const* begin = vec->empty() ? 0 : &(*vec)[0];
    s->ss->PutItem(begin, vec->size() * sizeof(T));
  }
};

inline SyncDataInterface* SyncPacked(std::vector<char> *t){
  return new CSyncVarVector<char>(t);
}

inline SyncDataInterface* SyncPacked(std::vector<int> *t){
  return new CSyncVarVector<int>(t);
}

inline SyncDataInterface* SyncPacked(std::vector<float> *t){
  return new CSyncVarVector<float>(t);
}

inline SyncDataInterface* SyncPacked(std::vector<double> *t){
  return new CSyncVarVector<double>(t);
}

// syncronizers for plain vector container
template<class T> class CSyncVector:public SyncDataInterface{
public:
  T *obj;
  int sz;
  CSyncVector(T *t, int n): obj(t), sz(n){}
  void Load(Serializer *s) override{
    int size=0, i=0;
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){        
        if(strcmp("size",s->ss->GetNodeName())==0 && i == 0){
          SyncDataInterface *sync=Sync(&size);
          sync->Load(s);
          delete sync;
          //obj->resize(size);
        }
        else if(strcmp("data",s->ss->GetNodeName())==0 && size!=0 && i<sz){
          Serializer srlz(s);
          s->Item("item",Sync(obj+i));
          s->Load();
          i++;
        }
        else{
          Serializer(s).Load();
        }
      }
      else if(type==2){
        assert(!"error of stream syncronization");
      }
    }
  }
  void Store(Serializer *s) override{
    s->ss->PutStartNode("size");
    s->ss->PutItem(&sz);
    s->ss->PutEndNode("size");
    int i;
    s->ss->PutStartNode("data");
    s->ss->PutStartNode("vector");
    for(i=0;i<sz;i++){
      Serializer srlz(s);
      srlz.Item("item",Sync(obj+i));
      srlz.Store();
    }
    s->ss->PutEndNode("vector");
    s->ss->PutEndNode("data");
  }
};

template<class T> inline SyncDataInterface* Sync(T *t, int n){
  return new CSyncVector<T>(t,n);
}

template<typename T, int N> inline SyncDataInterface* Sync(T (&arr)[N]){
  return new CSyncVector<T>(&arr[0], N);
}

template<typename T, int N> inline SyncDataInterface* Sync(T (*arr)[N]){
  return new CSyncVector<T>(&(*arr)[0], N);
}



// syncronizers for stl containers

template<typename T, typename R> class CStlPair:public std::pair<T,R>{
public:
  CStlPair(){}
  void AskForData(Serializer *s){
    s->Item("key",Sync(& this->first));
    s->Item("value",Sync(& this->second));
  }
};

template<typename T, typename R> class CSync_StlMap: public SyncDataInterface{
public:
  std::map<T,R> *obj;
  CSync_StlMap(std::map<T,R> *t):obj(t){}
  void Load(Serializer *s) override{
    obj->clear();
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        if(strcmp("item",s->ss->GetNodeName())==0){
          CStlPair<T,R> pair;
          {
            Serializer srlz(s);
            srlz.Item("item",Sync(&pair.first));
            srlz.Load();
          }
          {
            Serializer srlz(s);
            srlz.Item("item",Sync(&pair.second));
            srlz.Load();
          }
          obj->insert(pair);
        }
        else{
          Serializer(s).Load();
        }
      }
      else if(type==2){
        assert(!"error of stream syncronization");
      }
    }
  }
  void Store(Serializer *s) override{
    s->ss->PutStartNode("vector");
    for(auto it = obj->begin(); it != obj->end(); it++){
      s->ss->PutStartNode("item");
      s->ss->PutStartNode("vector");
      CStlPair<T,R> pair;
      {
        Serializer srlz(s);
        srlz.Item("item",Sync((T*)&it->first));
        srlz.Store();
      }
      {
        Serializer srlz(s);
        srlz.Item("item",Sync(&it->second));
        srlz.Store();
      }
      s->ss->PutEndNode("vector");
      s->ss->PutEndNode("item");
    }
    s->ss->PutEndNode("vector");
  }
};

template<typename T,typename R> inline SyncDataInterface* Sync(std::map<T,R> *t){
  return new CSync_StlMap<T,R>(t);
}


template<typename T> class CSync_StlVector: public SyncDataInterface{
public:
  std::vector<T> *obj;
  CSync_StlVector(std::vector<T> *t):obj(t){}
  
  void Load(Serializer *s) override{
    int size=0;    
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        if(strcmp("size",s->ss->GetNodeName())==0){
          SyncDataInterface *sync=Sync(&size);
          sync->Load(s);
          delete sync;
          obj->resize(size);          
        }
        else if(strcmp("data",s->ss->GetNodeName())==0 && size!=0){
          for(auto it = obj->begin(); it != obj->end(); ++it){
            auto type = s->ss->NextItem();
            if(type == 1){
              break;
            }
            else if(type == 0){
              Serializer srlz(s);
              srlz.Item("item",Sync(&*it));
              srlz.Load();
            }
            else if(type == 2){
              assert(!"error of stream syncronization");
            }
          }
        }
        else{
          Serializer(s).Load();
        }
      }
      else if(type==2){
        assert(!"error of stream syncronization");
      }
    }
  }
  void Store(Serializer *s) override{
    int sz = (int)obj->size();
//    s->ss->PutStartNode("size");
//    s->ss->PutItem(&sz);
//    s->ss->PutEndNode("size");
    Serializer srlz(s);
    srlz.Item("size",Sync(&sz));
    srlz.Store();

    s->ss->PutStartNode("data");
    s->ss->PutStartNode("vector");
    for(auto it = obj->begin(); it != obj->end(); it++){
      Serializer srlz(s);
      srlz.Item("item",Sync(&*it));
      srlz.Store();
    }
    s->ss->PutEndNode("vector");
    s->ss->PutEndNode("data");
  }
};

template<typename T> inline SyncDataInterface* Sync(std::vector<T> *t){
  return new CSync_StlVector<T>(t);
}

template<typename T> class CSync_StlList: public SyncDataInterface{
public:
  std::list<T> *obj;
  CSync_StlList(std::list<T> *t):obj(t){}
  
  void Load(Serializer *s) override{
    obj->clear();
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        Serializer srlz(s);
        T item;
        srlz.Item("item",Sync(&item));
        srlz.Load();
        obj->push_back(item);
      }
      else if(type==2){
        assert(!"error of stream synchronisation");
      }
    }
  }
  void Store(Serializer *s) override{
    typename std::list<T>::iterator it;
    s->ss->PutStartNode("vector");
    for(it = obj->begin(); it != obj->end(); it++){
      Serializer srlz(s);
      srlz.Item("item",Sync(&*it));
      srlz.Store();
    }
    s->ss->PutEndNode("vector");
  }
};

template<typename T> inline SyncDataInterface* Sync(std::list<T> *t){
  return new CSync_StlList<T>(t);
}


template<typename T, typename K> class CSync_StlSet: public SyncDataInterface{
public:
  std::set<T,K> *obj;
  CSync_StlSet(std::set<T,K> *t):obj(t){}
  
  void Load(Serializer *s) override{
    obj->clear();
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        Serializer srlz(s);
        T item;
        srlz.Item("item",Sync(&item));
        srlz.Load();
        obj->insert(item);
      }
      else if(type==2){
        assert(!"error of stream syncronization");
      }
    }
  }
  void Store(Serializer *s) override{
    typename std::set<T,K>::iterator it;
    s->ss->PutStartNode("vector");
    for(it = obj->begin(); it != obj->end(); it++){
      Serializer srlz(s);
      srlz.Item("item",Sync((T*)&*it));
      srlz.Store();
    }
    s->ss->PutEndNode("vector");
  }
};

template<typename T,typename K> inline SyncDataInterface* Sync(std::set<T,K> *t){
  return new CSync_StlSet<T,K>(t);
}

// dynamical object pointers created during loading and populated

template<class T> class CSyncPtr: public SyncDataInterface{
public:
  T *&ptr;
  CSyncPtr(T *&p):ptr(p){}
  void Load(Serializer *s) override{
    if(ptr) delete ptr;
    ptr=0;
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        if(strcmp("ClassName",s->ss->GetNodeName())==0){
          char cname[200];
          SyncDataInterface *sync=Sync(cname,200);
          sync->Load(s);
          delete sync;
          ptr = T::AskForObject(cname); // create object of class cname
        }
        else if(ptr && strcmp("ClassData",s->ss->GetNodeName())==0){
          SyncDataInterface *sync=Sync(ptr);
          sync->Load(s);
          delete sync;
        }
        else{
          Serializer(s).Load();
        }
      }
      else if(type==2){
        assert(!"error of stream syncronization");
      }
    }
  }
  void Store(Serializer *s) override{
    if(ptr){
      s->ss->PutStartNode("ClassName");
      s->ss->PutItem(ptr->AskForClassName());
      s->ss->PutEndNode("ClassName");
      
      SyncDataInterface *sync=Sync(ptr);
      s->ss->PutStartNode("ClassData");
      sync->Store(s);
      s->ss->PutEndNode("ClassData");
      delete sync;
    }
  }
};

//Syncronizer Dynamical Pointer
template<class T> inline SyncDataInterface* SyncPtr(T *&t){
  return  new CSyncPtr<T>(t);
}


template<class T> class CSrlzPtr{
public:
  T *ptr;
  CSrlzPtr(T *p=0):ptr(p){}
  //~CSrlzPtr(){ Clear();} // this object should not delete a pointer
  
  void AskForData(Serializer *s){
    s->Item("CSrlzPtr",SyncPtr(ptr));
  }
  bool operator == (const CSrlzPtr<T> &op){
    return op.ptr==ptr;
  }
  void Clear(){ if(ptr) delete ptr; ptr=0; }
};

};

#endif // SERIALIZERV2_H
