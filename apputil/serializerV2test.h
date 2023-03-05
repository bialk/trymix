#ifndef SERIALIZERV2TEST
#define SERIALIZERV2TEST
#include "serializerV2.h"
#include "storagestreamjson.h"
#include "storagestreamsimplexml.h"
#include <memory>

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
      s->Item("a",Sync(&a));
      s->Item("b",Sync(&b));
      s->Item("c",Sync(&c));
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
      s->Item("a",Sync(&a));
      s->Item("b",Sync(&b));
      s->Item("c",Sync(&c));
      s->Item("d",Sync(&d));
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
      s->Item("otherint",Sync(&otherint));
      s->Item("otherfloat",Sync(&otherfloat));
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
         eobjA[i].otherint=i;
         eobjA[i].otherfloat=i+0.10f;
      }
   }

   void AskForData(Serializer *s){
      n=100;
      s->Item("otherint",Sync(&otherint));
      s->Item("otherfloat",Sync(&otherfloat));
      s->Item("Vector of EmbedObjectsA",Sync(eobjA,n));
      s->Item("EmbedObjectB",Sync(&eobjB));
      s->Item("EmbedObjectC",Sync(&eobjC));
   }

};

class MainObject{
public:
   // this is a vector of pointer to the base class
   // object class restored dynamically from stream
   std::vector< CSrlzPtr<DynClassBase> > dynobjlist;

   int testint;
   float testfloat;
   std::vector<EmbedObject> stdvect;
   char name[200];
   char chr;
   std::list<int> intlist;
   std::set<int> intset;
   std::set<double> doubleset;

   std::map<int,std::string> map_int_float;


   EmbedObject eobj;

   MainObject(){
      stdvect.resize(1000);
      strcpy(name,"this is a test string");
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


      map_int_float[1]="one";
      map_int_float[2]="two";
      map_int_float[3]="tree";
      map_int_float[4]="four";

   }
   void AskForData(Serializer *s){
      s->Item("dynobjlist", Sync(&dynobjlist));
      //s->SyncAs("dynobjlist", dynobjlist);
      s->Item("map_int_float",Sync(&map_int_float));
      s->Item("intlist",Sync(&intlist));
      s->Item("intset",Sync(&intset));
      s->Item("doubleset",Sync(&doubleset));
      s->Item("TestInt",Sync(&testint));
      s->Item("TestFloat",Sync(&testfloat));
      s->Item("EmbedObject",Sync(&eobj));
      s->Item("stdvector",Sync(&stdvect));
      s->Item("TestChar",Sync(&chr));
      s->Item("TestName",Sync(name,200));
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
    return ::fwrite(buf,1,size,m_f);
  }

  size_t read(void * buf, size_t size) override{
    return ::fread(buf,1,size,m_f);
  }

  bool eos() override {
    return ::feof(m_f)!=0;
  }


private:
  FILE *m_f{nullptr};
};
}

class TestAll{
public:
   TestAll(){

      MainObject mobj;
      mobj.testfloat=123.456F;
      mobj.testint=321;
      mobj.eobj.otherfloat=1.005F;
      mobj.eobj.otherint=22;



      auto makeStorageStream = [](sV2::StreamMediaFile& ssmedia,sV2::StreamMediaFile& ssmediaIndex) -> std::unique_ptr<StorageStream> {

        //return std::unique_ptr<StorageStream>(new StorageStreamSimpleXML(&ssmedia));
        //return std::unique_ptr<StorageStream>(new StorageStreamSimpleIostream(&ssmedia));
        //return std::unique_ptr<StorageStream>(new StorageStreamSimpleBinary(&ssmedia));
        //return std::unique_ptr<StorageStream>(new StorageStreamIndexedBinary(&ssmedia,&ssmediaIndex));
        return std::unique_ptr<StorageStream>(new StorageStreamSimpleJson(&ssmedia));
      };


      {
        sV2::StreamMediaFile ssmedia("test_data.txt",false);
        sV2::StreamMediaFile ssmediaIndex("test_data.txt.idx",false);
        auto ss = makeStorageStream(ssmedia, ssmediaIndex);
        Serializer srlz(ss.get());
        srlz.Item("MainObject", Sync(&mobj));
        srlz.Store();
        if(auto iss = dynamic_cast<StorageStreamIndexedBinary*>(ss.get()))
           iss->WriteIndex();
      }

      if(0){
        sV2::StreamMediaFile ssmedia("test_data.txt", true);
        sV2::StreamMediaFile ssmediaIndex("test_data.txt.idx",true);
        auto ss = makeStorageStream(ssmedia, ssmediaIndex);
        if(auto iss = dynamic_cast<StorageStreamIndexedBinary*>(ss.get()))
           iss->ReadIndex();
        Serializer srlz(ss.get());
        srlz.Item("MainObject", Sync(&mobj));
        srlz.Load();
      }

      if(0){
        sV2::StreamMediaFile ssmedia("test_data1.txt", false);
        sV2::StreamMediaFile ssmediaIndex("test_data1.txt.idx",false);
        auto ss = makeStorageStream(ssmedia, ssmediaIndex);
        Serializer srlz(ss.get());
        srlz.Item("MainObject", Sync(&mobj));
        srlz.Store();
        if(auto iss = dynamic_cast<StorageStreamIndexedBinary*>(ss.get()))
           iss->WriteIndex();
      }


      printf("test finished\n");

      //wait for key
      char buf[200];
      fgets(buf,200,stdin);
   }
};

}

#endif
