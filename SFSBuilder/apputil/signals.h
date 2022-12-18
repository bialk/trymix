#ifndef mir_signals_h
#define mir_signals_h

#include <list>

#ifdef off

namespace mir {
   class connector_base{
   public:
      int type; //0x0 - normal behaviour; 0x1 - marked as deleted
      connector_base():type(0){}
      virtual ~connector_base(){};
      virtual void call()=0;
   };

   template<class T> class connector: public connector_base{
   public:
      T *ptr;
      void (T::*func)();
      connector(T *p, void (T::*f)()):ptr(p),func(f){}
      virtual void call(){
         if(type==0x1) return; // do not process if marked ad deleted
         (ptr->*func)();     
      }
   };

   class signal{
   public:
      std::list<connector_base*> receiptlist;

      ~signal(){ clear(); }
      void clear(){
         std::list<connector_base*>::iterator it;
         for(it=receiptlist.begin();it!=receiptlist.end();it++) 
            delete (*it);
         receiptlist.clear();
      }
   private:
      void clear_marked(){
         std::list<connector_base*>::iterator it;
         for(it=receiptlist.begin();it!=receiptlist.end();) {
            if( (*it)->type == 0x1 ){
               delete (*it);
               it=receiptlist.erase(it);
            }
            else
               it++;
         }
      }
   public:
      template<class T> void connect(T *p,  void (T::*f)() ){
         std::list<connector_base*>::iterator it;
         for(it=receiptlist.begin();it!=receiptlist.end();it++) {
            connector<T> *ptr = dynamic_cast< connector<T>* >(*it);
            if(ptr && ptr->ptr==p && ptr->func==f)
               return; //connector already in the list
         }
         receiptlist.push_back(new mir::connector<T>(p, f));
      }
      template<typename T> void disconnect(T *p,  void (T::*f)() ){
         std::list<connector_base*>::iterator it;
         for(it=receiptlist.begin();it!=receiptlist.end();it++) {
            connector<T> *ptr = dynamic_cast< connector<T>* >(*it);
            if(ptr && ptr->ptr==p && ptr->func==f)
               (*it)->type |= 0x1; //mark connector as deleted
         }
      }
      void operator()(){
         std::list<connector_base*>::iterator it;
         for(it=receiptlist.begin();it!=receiptlist.end();it++) {
            (*it)->call();
            if(it==receiptlist.end()) return;
         }
         clear_marked(); // delete all connectors marked ad deleted
      }
   };

}
#endif //off

#endif
