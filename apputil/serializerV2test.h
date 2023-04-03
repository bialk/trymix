#ifndef SERIALIZERV2TEST
#define SERIALIZERV2TEST
#include "serializerV2.h"
#include "storagestreamjson.h"
#include "storagestreamsimplexml.h"
#include "storagestreamindexedbinary.h"
#include <memory>
#include <sstream>

namespace sV2 {

//THIS IS TEST CODE SECTION
// (uncomment last line to set debug on)
////////////////////////////////////////

class DynClassBase{
public:
   static int count;
   static  DynClassBase *AskForObject(const char *cname);
   virtual const char *AskForClassName()=0;
   virtual void AskForData(Serializer *s)=0;
};

int DynClassBase::count = 0;

class DynClassABC: public DynClassBase{
public:
   int a,b,c;
   DynClassABC(){
      a=count++;
      b=count++;
      c=count++;
   }
   const char *AskForClassName(){
      return "DynClassABC";
   }
   void AskForData(Serializer *s){
      s->SyncAs("a",a);
      s->SyncAs("b",b);
      s->SyncAs("c",c);
   }
};

class DynClassABCD: public DynClassBase{
public:
   int a,b,c,d;
   DynClassABCD(){
      a=count++;
      b=count++;
      c=count++;
      d=count++;
   }
   const char *AskForClassName(){
      return "DynClassABCD";
   }
   void AskForData(Serializer *s){
     s->SyncAs("a",a);
     s->SyncAs("b",b);
     s->SyncAs("c",c);
     s->SyncAs("d",d);
   }
};

DynClassBase *DynClassBase::AskForObject(const char *cname){
   if(strcmp(cname,"DynClassABC")==0)
      return new DynClassABC;
   else if(strcmp(cname,"DynClassABCD")==0)
      return new DynClassABCD;
   // return 0 if object can not be constructed
   return 0;
}



class EmbedObject2{
public:
   int otherint;
   float otherfloat;

   void AskForData(Serializer *s){
     s->SyncAs("otherint",otherint);
     s->SyncAs("otherfloat",otherfloat);
   }
};

class EmbedObject{
public:
   int otherint;
   float otherfloat;
   int n;

   EmbedObject2 eobjA[1000];
   EmbedObject2 eobjB;
   EmbedObject2 eobjC;

   EmbedObject(){
      int i;
      for(i=0;i<1000;i++){
         eobjA[i].otherint=DynClassBase::count++;
         eobjA[i].otherfloat=DynClassBase::count+0.10f;
      }
      eobjB.otherint = DynClassBase::count++;
      eobjB.otherfloat = DynClassBase::count++;
      eobjC.otherint = DynClassBase::count++;
      eobjC.otherfloat = DynClassBase::count++;
      otherint = DynClassBase::count++;
      otherfloat = DynClassBase::count++;
   }

   void AskForData(Serializer *s){
      n=100;
      s->SyncAs("otherint",otherint);
      s->SyncAs("otherfloat",otherfloat);
      s->SyncAs("Vector of EmbedObjectsA",eobjA,n);
      s->SyncAs("EmbedObjectB",eobjB);
      s->SyncAs("EmbedObjectC",eobjC);
   }
};

class MainObject{
public:
   // this is a vector of pointer to the base class
   // object class restored dynamically from stream
   std::vector< CSrlzPtr<DynClassBase> > dynobjlist;

   std::vector<std::unique_ptr<DynClassBase>> dynobjvector;

   unsigned short int x_float[100];

   int testint = DynClassBase::count++;
   float testfloat = DynClassBase::count++;
   std::vector<EmbedObject> stdvect;
   char name[200];
   char chr = 'X';
   std::vector<int> intvector;
   std::list<int> intlist;
   std::set<int> intset;
   std::set<double> doubleset;

   std::map<int,std::string> map_int_float;
   EmbedObject eobj;

   MainObject(){
      stdvect.resize(1000);

      sprintf_s(name, 200, "this is a test string name %i", DynClassBase::count++);

      int counter = 0;
      for(auto&i: x_float)
        i = counter += 2;

      for(int i=0;i<33; i+=3)
        intvector.push_back(i);

      intlist.push_back(1);
      intlist.push_back(3);
      intlist.push_back(7);
      intlist.push_back(12);

      intset.insert(10);
      intset.insert(7);
      intset.insert(2);
      intset.insert(8);
      intset.insert(4);
      intset.insert(3);

      doubleset.insert(10e20);
      doubleset.insert(7e-20);
      doubleset.insert(2e2);
      doubleset.insert(8e0);
      doubleset.insert(4);
      doubleset.insert(3);

      dynobjlist.push_back(CSrlzPtr<DynClassBase>(new DynClassABC));
      dynobjlist.push_back(CSrlzPtr<DynClassBase>(new DynClassABC));
      dynobjlist.push_back(CSrlzPtr<DynClassBase>(new DynClassABC));
      dynobjlist.push_back(CSrlzPtr<DynClassBase>(new DynClassABCD));
      dynobjlist.push_back(CSrlzPtr<DynClassBase>(new DynClassABCD));

      dynobjvector.emplace_back(new DynClassABC);
      dynobjvector.emplace_back(new DynClassABC);
      dynobjvector.emplace_back(new DynClassABC);
      dynobjvector.emplace_back(new DynClassABCD);
      dynobjvector.emplace_back(new DynClassABCD);

      map_int_float[1]="one";
      map_int_float[2]="two";
      map_int_float[3]="tree";
      map_int_float[4]="four";

   }
   void AskForData(Serializer *s){
      s->SyncAs("dynobjlist", dynobjlist);
      s->SyncAs("dynobjvector", dynobjvector);
      s->SyncAs("map_int_float",map_int_float);
      s->SyncAs("intvector",intvector);
      s->SyncAs("intlist",intlist);
      s->SyncAs("intset",intset);
      s->SyncAs("doubleset",doubleset);
      s->SyncAs("TestInt",testint);
      s->SyncAs("TestFloat",testfloat);
      s->SyncAs("EmbedObject",eobj);
      s->SyncAs("stdvector",stdvect);
      s->SyncAs("TestChar",chr);
      s->SyncAs("TestName",name,200);
      s->SyncAs("x_float",x_float,100);
   }

};




namespace sV2{
class StreamMediaFile: public StreamMedia{
public:

  StreamMediaFile(const char *fname, bool readonly = true){
    if(readonly)
      OpenForRead(fname);
    else
      OpenForWrite(fname);
  }
  ~StreamMediaFile(){
    Close();
  }

  int  OpenForWrite(const char * fname){
    Close();
    auto err = fopen_s(&m_f,fname,"w+b");
    if(err){
      char buf[256];
      std::cout << "Serializer error to open stream:" << strerror_s(buf, sizeof(buf), errno) << '\n';
    }
    return !m_f;
  }
  int  OpenForRead(const char * fname){
    Close();
    auto err = fopen_s(&m_f, fname,"rb");
    if(err){
      char buf[256];
      std::cout << "Serializer error to open stream:" << strerror_s(buf, sizeof(buf), errno) << '\n';
    }
    return !m_f;
  }
  void Close(){
    if(m_f){
       fclose(m_f);
       m_f=nullptr;
    }
  }

  size_t write(void const * buf, size_t size) override{
    auto n = ::fwrite(buf,1,size,m_f);
    positionCounter+=n;
    fflush(m_f);
    return n;
  }

  size_t read(void * buf, size_t size) override{
    auto n = ::fread(buf,1,size,m_f);
    positionCounter+=n;
    return n;
  }

  bool eos() override {
    return ::feof(m_f)!=0;
  }


private:
  size_t positionCounter = 0;
  FILE *m_f{nullptr};
};
}

class TestAll{
public:
   TestAll(){

      MainObject mobj;

      if(0){
        sV2::StreamMediaFile ssmediaIndex("test_data.txt",true);
        auto jsonStream = std::unique_ptr<StorageStreamSimpleJson>(new StorageStreamSimpleJson(&ssmediaIndex));

        for(;;){
          auto t = jsonStream->nextTocken();
          std::cout << t << std::endl;
        }
      }

      auto makeStorageStream = [](int st, sV2::StreamMediaFile& ssmedia,sV2::StreamMediaFile& ssmediaIndex) -> std::unique_ptr<StorageStream> {
        switch(st){
        case 0: return std::unique_ptr<StorageStream>(new StorageStreamSimpleXML(&ssmedia));
        case 1: return std::unique_ptr<StorageStream>(new StorageStreamSimpleIostream(&ssmedia));
        case 2: return std::unique_ptr<StorageStream>(new StorageStreamSimpleBinary(&ssmedia));
        case 3: return std::unique_ptr<StorageStream>(new StorageStreamIndexedBinary(&ssmedia,&ssmediaIndex));
        case 4: return std::unique_ptr<StorageStream>(new StorageStreamSimpleJson(&ssmedia));
        }
        return nullptr;
      };

      for(int i = 0; i<5; ++i){
        std::stringstream test_data_a; test_data_a << "test_data_a" << std::to_string(i) << ".txt";
        std::stringstream test_data_a_idx; test_data_a_idx << "test_data_a" << std::to_string(i) << ".txt.idx";
        std::stringstream test_data_b; test_data_b << "test_data_b" << std::to_string(i) << ".txt";
        std::stringstream test_data_b_idx; test_data_b_idx << "test_data_b" << std::to_string(i) << ".txt.idx";

        {
          sV2::StreamMediaFile ssmedia(test_data_a.str().c_str(),false);
          sV2::StreamMediaFile ssmediaIndex(test_data_a_idx.str().c_str(),false);
          auto ss = makeStorageStream(i, ssmedia, ssmediaIndex);

          Serializer(ss.get()).StoreAs("MainObject", mobj);

          if(auto iss = dynamic_cast<StorageStreamIndexedBinary*>(ss.get()))
            iss->WriteIndex();
        }

        MainObject mobj2;
        if(1){
          sV2::StreamMediaFile ssmedia(test_data_a.str().c_str(),true);
          sV2::StreamMediaFile ssmediaIndex(test_data_a_idx.str().c_str(),true);
          auto ss = makeStorageStream(i, ssmedia, ssmediaIndex);
          if(auto iss = dynamic_cast<StorageStreamIndexedBinary*>(ss.get()))
            iss->ReadIndex();

          Serializer(ss.get()).LoadAs("MainObject", mobj2);
        }

        if(1){
          sV2::StreamMediaFile ssmedia(test_data_b.str().c_str(),false);
          sV2::StreamMediaFile ssmediaIndex(test_data_b_idx.str().c_str(),false);
          auto ss = makeStorageStream(i, ssmedia, ssmediaIndex);

          Serializer(ss.get()).StoreAs("MainObject", mobj2);

          if(auto iss = dynamic_cast<StorageStreamIndexedBinary*>(ss.get()))
            iss->WriteIndex();
        }

        printf("test finished\n");
      }
      std::cout << "for i in 0 1 2 3 4; do diff test_data_a${i}.txt test_data_b${i}.txt; done" << std::endl;
      std::system("bash -c \"for i in 0 1 2 3 4; do diff test_data_a${i}.txt test_data_b${i}.txt; done; echo done; read\"");
   }
};

}

#endif
