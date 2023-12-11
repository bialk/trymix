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

template<typename T, typename> class CSyncObj;

class Serializer{
public:

  Serializer(StorageStream *s):ss(s){}
  Serializer(Serializer *s):ss(s->ss){}
  
  ~Serializer();

  // general data sychronization
  template<typename T>
  void SyncAs(const char* name, T& data){
    datalist.push_back(*dataset.emplace(name, new CSyncObj((T*)&data)).first);
  }

  //fixed plain arrays syncronization
  template<typename T>
  void SyncAs(const char* name, T* data, int sz){
    datalist.push_back(*dataset.emplace(name, new CSyncObj(data, sz)).first);
  }

  template<typename T>
  void StoreAs(const char* name, T& data){
    ss->PutStartNode(name);
    CSyncObj(&data).Store(this);
    ss->PutEndNode(name);
  }

  template<typename T>
  void LoadAs(const char* name, T& data){
    if(ss->NextItem()==0 && strcmp(name, ss->GetNodeName()) == 0)
        CSyncObj(&data).Load(this);
  }


private:
  StorageStream *ss{nullptr};
  std::map<std::string, SyncDataInterface*> dataset;
  std::list<std::pair<std::string,SyncDataInterface*> > datalist;

  template<typename, typename> friend class CSyncObj;
  template<typename T> friend void LoadListOfItems(Serializer *s, T* obj, size_t sz);
};

// missing object to bypass not found data
class MissingObject{
public:
  void AskForData(Serializer *s){}
};

// synchronisation any custom object with AskForData functions
template<typename T, typename = void> class CSyncObj: public SyncDataInterface{
public:
  T *obj;
  CSyncObj(T *t):obj(t){}

  void Load(Serializer *s) override{
    Serializer srlz(s);
    obj->AskForData(&srlz);

    while(1){
      int type = srlz.ss->NextItem();
      if(type == 1){ //endnode
        return;
      }
      else if(type == 0){ //start node
        auto it=srlz.dataset.find(srlz.ss->GetNodeName());
        if(it!=srlz.dataset.end()){
          it->second->Load(&srlz);
        }
        else{
          MissingObject eo;
          CSyncObj<MissingObject>(&eo).Load(&srlz);
        }
      }
      else if(type==2){ //data node
        // bypass atomic data
      }
    }
  }

  void Store(Serializer *s) override{
    Serializer srlz(s);
    obj->AskForData(&srlz);
    for(auto& it: srlz.datalist){
      srlz.ss->PutStartNode(it.first.c_str());
      it.second->Store(&srlz);
      srlz.ss->PutEndNode(it.first.c_str());
    }
  }
};


// build-in serializable "atomic" types
template<typename T> struct atomic_type { static const bool value = false; };
template<> struct atomic_type<char>     { static const bool value = true;  };
template<> struct atomic_type<unsigned char>     { static const bool value = true;  };
template<> struct atomic_type<unsigned short>  { static const bool value = true;  };
template<> struct atomic_type<unsigned int>  { static const bool value = true;  };
template<> struct atomic_type<unsigned long int>  { static const bool value = true;  };
template<> struct atomic_type<signed char>     { static const bool value = true;  };
template<> struct atomic_type<signed short>  { static const bool value = true;  };
template<> struct atomic_type<signed int>  { static const bool value = true;  };
template<> struct atomic_type<signed long int>  { static const bool value = true;  };
template<> struct atomic_type<double>   { static const bool value = true;  };
template<> struct atomic_type<float>    { static const bool value = true;  };

// mapping build-in serializable (atomic) types to fixed subset of build-in convertible atomic types
template<typename T> struct is_compatible_type { typedef void type;};
template<> struct is_compatible_type<char> { typedef int type;};
template<> struct is_compatible_type<unsigned char> { typedef int type;};
template<> struct is_compatible_type<unsigned short> { typedef int type;};
template<> struct is_compatible_type<unsigned int> { typedef int type;};
template<> struct is_compatible_type<unsigned long int> { typedef int type;};
template<> struct is_compatible_type<signed char> { typedef int type;};
template<> struct is_compatible_type<signed short> { typedef int type;};
template<> struct is_compatible_type<signed int> { typedef int type;};
template<> struct is_compatible_type<signed long int> { typedef int type;};
template<> struct is_compatible_type<float> { typedef float type;};
template<> struct is_compatible_type<double> { typedef double type;};


// synchronisation of atomic types converted to some compatible subset of types
template<typename T> class CSyncObj<T, typename std::enable_if<atomic_type<T>::value>::type>: public SyncDataInterface{
public:
  using R = typename is_compatible_type<T>::type;
  T * const obj;
  CSyncObj(T *t):obj(t){}
  void Load(Serializer *s) override{
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        MissingObject eo;
        CSyncObj<MissingObject>(&eo).Load(s);
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


// syncronizer for plain char* zero-based strings with max length n
// please note that max length n here defined only for protection
// and also used to separate function signature from plain char(one symbol type)
template<>
class CSyncObj<char*>: public SyncDataInterface{
public:
  char * const obj;
  int sz;
  CSyncObj(char *t,int n):obj(t),sz(n){}
  void Load(Serializer *s) override{
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
        MissingObject eo;
        CSyncObj<MissingObject>(&eo).Load(s);
      }
    }
  }

  void Store(Serializer *s) override{
    s->ss->PutItem(obj);
  }
};

// deduce constructor CSyncObj(T*, int) to CSyncObj<T*,void>(T*, void)
template<typename T> CSyncObj(T*, int) -> CSyncObj<T*>;

// syncronizers for std::string type
template<>
class CSyncObj<std::string>: public SyncDataInterface{
public:
  std::string * const obj;
  CSyncObj(std::string *t):obj(t){}
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
      }
      else if(type==0){
        MissingObject eo;
        CSyncObj<MissingObject>(&eo).Load(s);
      }
    }
  }
  void Store(Serializer *s) override{
    s->ss->PutItem(obj->c_str());
  }
};


template<typename T>
void LoadListOfItems(Serializer *s, T* obj, size_t sz)
{
  size_t i = 0;
  for(;;){
    auto type = s->ss->NextItem();
    if(type == 1){
      break;
    }
    else if(type == 0){
      if(strcmp("item",s->ss->GetNodeName())==0 && i < sz){        
        CSyncObj(obj+i).Load(s);
        i++;
      }
      else{
        MissingObject eo;
        CSyncObj<MissingObject>(&eo).Load(s);
      }
    }
    else if(type == 2){
      assert(!"error of stream syncronization");
    }
  }
};


// syncronizers for plain array container for all types
template<typename T>
class CSyncObj<T*, void>:public SyncDataInterface{
public:
  T * const obj;
  int sz;

  CSyncObj(T *t, int n): obj(t), sz(n){}

  void Load(Serializer *s) override{
    int size=0;
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        if(strcmp("size",s->ss->GetNodeName())==0){
          CSyncObj<int>(&size).Load(s);
        }
        else if(strcmp("data",s->ss->GetNodeName())==0){
          LoadListOfItems(s, obj, sz);
        }
        else{
          MissingObject eo;
          CSyncObj<MissingObject>(&eo).Load(s);
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
    s->ss->PutStartNode("data");
    s->ss->PutStartNode("vector");
    for(int i=0;i<sz;i++){
      s->ss->PutStartNode("item");
      CSyncObj<T>(obj+i).Store(s);
      s->ss->PutEndNode("item");
    }
    s->ss->PutEndNode("vector");
    s->ss->PutEndNode("data");
  }
};


// syncronizers for std::map containers for all types (atomic and non atomic)
template<typename T, typename R> class CSyncObj<std::map<T,R>>: public SyncDataInterface{
public:
  std::map<T,R> * const obj;
  CSyncObj(std::map<T,R> *t):obj(t){}
  void Load(Serializer *s) override{
    obj->clear();
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        if(strcmp("item",s->ss->GetNodeName())==0){
          for(;;){
            T key;
            auto type = s->ss->NextItem();
            if(type == 1){
              break;
            }
            else if(type == 0){
              if(strcmp("item",s->ss->GetNodeName())==0){
                CSyncObj<T>(&key).Load(s);
              }
              else{
                MissingObject eo;
                CSyncObj<MissingObject>(&eo).Load(s);
              }
            }

            type = s->ss->NextItem();
            if(type == 1){
              break;
            }
            else if(type == 0){
              if( strcmp("item",s->ss->GetNodeName())==0){
                R value;
                CSyncObj<R>(&value).Load(s);
                obj->insert(std::pair(key,value));
              }
              else{
                MissingObject eo;
                CSyncObj<MissingObject>(&eo).Load(s);
              }
            }
          }
        }
        else{
          MissingObject eo;
          CSyncObj<MissingObject>(&eo).Load(s);
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
          std::pair<T,R> pair;
          s->ss->PutStartNode("item");
          CSyncObj<T>((T*)&it->first).Store(s);
          s->ss->PutEndNode("item");
          s->ss->PutStartNode("item");
          CSyncObj<R>(&it->second).Store(s);
          s->ss->PutEndNode("item");
        s->ss->PutEndNode("vector");
      s->ss->PutEndNode("item");
    }
    s->ss->PutEndNode("vector");
  }
};

// deduce constructor CSyncObj(std::map<T,R>*) to CSyncObj<T*,void>(std::map<T,R>, void)
template<typename T, typename R> CSyncObj(std::map<T,R>* ) -> CSyncObj<std::map<T,R>>;


// there is two variants of vector syncronizations. First variant uses two specializations
// one of each stored atomic data as binary data (efficient way).
// second variant stored data "usual" way as structured nodes
#ifdef off
// syncronizes a std::vector of "atomic" types (char, int, float, ...)
// in storage efficient manner (binary way)
template<typename T> class CSyncObj<std::vector<T>, typename std::enable_if<atomic_type<T>::value>::type>: public SyncDataInterface{
public:
  std::vector<T>* vec;
  CSyncObj(std::vector<T>* v): vec(v){}
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
        *vec=std::vector<T>(begin, begin + n);
      } else {
        MissingObject eo;
        CSyncObj<MissingObject>(&eo).Load(s);
      }
    }
  }
  void Store(Serializer *s) override{
    void const* begin = vec->empty() ? 0 : vec->data();
    s->ss->PutItem(begin, vec->size() * sizeof(T));
  }
};



//std::vector for non atomic types
template<typename T> class CSyncObj<std::vector<T>, typename std::enable_if<!atomic_type<T>::value>::type>: public SyncDataInterface{
public:
  std::vector<T> *obj;
  CSyncObj(std::vector<T> *t):obj(t){}

  void Load(Serializer *s) override{
    int size=0;
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        if(strcmp("size",s->ss->GetNodeName())==0){
          CSyncObj<int>(&size).Load(s);
          obj->resize(size);
        }
        else if(strcmp("data",s->ss->GetNodeName())==0){
          auto it = obj->begin();
          for(;;){
            auto type = s->ss->NextItem();
            if(type == 1){
              break;
            }
            else if(type == 0){
              if(strcmp("item",s->ss->GetNodeName())==0 && it != obj->end()){
                CSyncObj<T>(&*it).Load(s);
                it++;
              }
              else{
                MissingObject eo;
                CSyncObj<MissingObject>(&eo).Load(s);
              }
            }
            else if(type == 2){
              assert(!"error of stream syncronization");
            }
          }
        }
        else{
          MissingObject eo;
          CSyncObj<MissingObject>(&eo).Load(s);
        }
      }
      else if(type==2){
        assert(!"error of stream syncronization");
      }
    }
  }
  void Store(Serializer *s) override{
    int sz = (int)obj->size();
    s->ss->PutStartNode("size");
    s->ss->PutItem(&sz);
    s->ss->PutEndNode("size");

    s->ss->PutStartNode("data");
    s->ss->PutStartNode("vector");
    for(auto it = obj->begin(); it != obj->end(); it++){
      s->ss->PutStartNode("item");
      CSyncObj<T>(&*it).Store(s);
      s->ss->PutEndNode("item");
    }
    s->ss->PutEndNode("vector");
    s->ss->PutEndNode("data");
  }
};

#else

//std::vector for any type (including atomic types)
template<typename T> class CSyncObj<std::vector<T>>: public SyncDataInterface{
public:
  std::vector<T> * const obj;
  CSyncObj(std::vector<T> *t):obj(t){}

  void Load(Serializer *s) override{
    int size=0;
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        if(strcmp("size",s->ss->GetNodeName())==0){
          CSyncObj<int>(&size).Load(s);
          obj->resize(size);
        }
        else if(strcmp("data",s->ss->GetNodeName())==0){
          auto it = obj->begin();
          for(;;){
            auto type = s->ss->NextItem();
            if(type == 1){
              break;
            }
            else if(type == 0){
              if(strcmp("item",s->ss->GetNodeName())==0 && it != obj->end()){
                CSyncObj<T>(&*it).Load(s);
                it++;
              }
              else{
                MissingObject eo;
                CSyncObj<MissingObject>(&eo).Load(s);
              }
            }
            else if(type == 2){
              assert(!"error of stream syncronization");
            }
          }
        }
        else{
          MissingObject eo;
          CSyncObj<MissingObject>(&eo).Load(s);
        }
      }
      else if(type==2){
        assert(!"error of stream syncronization");
      }
    }
  }
  void Store(Serializer *s) override{
    int sz = (int)obj->size();
    s->ss->PutStartNode("size");
    s->ss->PutItem(&sz);
    s->ss->PutEndNode("size");

    s->ss->PutStartNode("data");
    s->ss->PutStartNode("vector");
    for(auto it = obj->begin(); it != obj->end(); it++){
      s->ss->PutStartNode("item");
      CSyncObj<T>(&*it).Store(s);
      s->ss->PutEndNode("item");
    }
    s->ss->PutEndNode("vector");
    s->ss->PutEndNode("data");
  }
};

#endif

template<typename T> class CSyncObj<std::list<T>>: public SyncDataInterface{
public:
  std::list<T> * const obj;
  CSyncObj(std::list<T> *t):obj(t){}
  
  void Load(Serializer *s) override{
    obj->clear();
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        if(strcmp("item",s->ss->GetNodeName())==0){
          T& item = obj->emplace_back();
          CSyncObj<T>(&item).Load(s);
        }
        else{
          MissingObject eo;
          CSyncObj<MissingObject>(&eo).Load(s);
        }
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
      s->ss->PutStartNode("item");
      CSyncObj<T>(&*it).Store(s);
      s->ss->PutEndNode("item");
    }
    s->ss->PutEndNode("vector");
  }
};

template<typename T, typename K> class CSyncObj<std::set<T,K>>: public SyncDataInterface{
public:
  std::set<T,K> * const obj;
  CSyncObj(std::set<T,K> *t):obj(t){}
  
  void Load(Serializer *s) override{
    obj->clear();
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        if(strcmp("item",s->ss->GetNodeName())==0){
          T item;
          CSyncObj<T>(&item).Load(s);
          obj->emplace(item);
        }
        else{
          MissingObject eo;
          CSyncObj<MissingObject>(&eo).Load(s);
        }
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
      s->ss->PutStartNode("item");
      CSyncObj<T>((T*)&*it).Store(s);
      s->ss->PutEndNode("item");
    }
    s->ss->PutEndNode("vector");
  }
};

// this class left for reference it is historically used (in fact it is acting like std::unique_ptr)
template<class T> class CSrlzPtr{
public:
  T *ptr{nullptr};
  CSrlzPtr(T *p=0):ptr(p){}
  ~CSrlzPtr(){ Clear();} // this object should not delete a pointer

  CSrlzPtr<T>(const CSrlzPtr<T>&) = delete;
  CSrlzPtr<T>& operator=(const CSrlzPtr<T>&) = delete;

  CSrlzPtr(CSrlzPtr<T> &&op){
    std::swap(op.ptr, ptr);
  }

  CSrlzPtr<T>& operator=(CSrlzPtr<T> &&op) {
    Clear();
    std::swap(op.ptr, ptr);
    return *this;
  }

  bool operator == (const CSrlzPtr<T> &op) const{
    return op.ptr==ptr;
  }

  void Clear(){
    if(ptr)
      delete ptr;
    ptr=nullptr;
  }
};

// dynamical object pointers created during loading and populated
template<typename T> class CSyncObj<CSrlzPtr<T>>: public SyncDataInterface{
public:
  CSrlzPtr<T> *crlzptr{nullptr};
  CSyncObj(CSrlzPtr<T> *p):crlzptr(p){}
  void Load(Serializer *s) override{
    if(crlzptr->ptr) delete crlzptr->ptr;
    crlzptr->ptr=0;
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        if(strcmp("ClassName",s->ss->GetNodeName())==0){
          char cname[200];
          CSyncObj<char*>(cname,200).Load(s);
          crlzptr->ptr = T::AskForObject(cname); // create object of class cname
        }
        else if(crlzptr && strcmp("ClassData",s->ss->GetNodeName())==0){
          CSyncObj<T>(crlzptr->ptr).Load(s);
        }
        else{
          MissingObject eo;
          CSyncObj<MissingObject>(&eo).Load(s);
        }
      }
      else if(type==2){
        assert(!"error of stream syncronization");
      }
    }
  }

  void Store(Serializer *s) override{
    if(crlzptr){
      s->ss->PutStartNode("ClassName");
      s->ss->PutItem(crlzptr->ptr->AskForClassName());
      s->ss->PutEndNode("ClassName");

      s->ss->PutStartNode("ClassData");
      CSyncObj<T>(crlzptr->ptr).Store(s);
      s->ss->PutEndNode("ClassData");
    }
  }
};


// dynamical object pointers created during loading and populated
template<typename T> class CSyncObj<std::unique_ptr<T>>: public SyncDataInterface{
public:
  std::unique_ptr<T> * const ptr{nullptr};
  CSyncObj(std::unique_ptr<T> *p):ptr(p){}
  void Load(Serializer *s) override{
    while(1){
      int type=s->ss->NextItem();
      if(type==1){
        return;
      }
      else if(type==0){
        if(strcmp("ClassName",s->ss->GetNodeName())==0){
          char cname[200];
          CSyncObj<char*>(cname,200).Load(s);
          ptr->reset(T::AskForObject(cname)); // create object of class cname
        }
        else if(ptr && strcmp("ClassData",s->ss->GetNodeName())==0){
          CSyncObj<T>(ptr->get()).Load(s);
        }
        else{
          MissingObject eo;
          CSyncObj<MissingObject>(&eo).Load(s);
        }
      }
      else if(type==2){
        assert(!"error of stream syncronization");
      }
    }
  }

  void Store(Serializer *s) override{
    s->ss->PutStartNode("ClassName");
    s->ss->PutItem((*ptr)->AskForClassName());
    s->ss->PutEndNode("ClassName");

    s->ss->PutStartNode("ClassData");
    CSyncObj<T>(ptr->get()).Store(s);
    s->ss->PutEndNode("ClassData");
  }
};


};

#endif // SERIALIZERV2_H
