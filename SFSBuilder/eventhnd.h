#ifndef eventhnd_h
#define eventhnd_h
//#include "eventlog.h"
#include "apputil/eventball.h"
#include <list>
#include <algorithm>
#include <string>
#include <stack>


class EvtHandle{
 public:  
  virtual ~EvtHandle();
  virtual void Handle(EventBall *e);
};


// class listitem
// ===================================================================

template <class T> class evth_list: public std::list<T>{
  public:
  typedef typename evth_list::iterator iterator;

  void ins_top(T item){
    remove(item);
    insert(evth_list::begin(),item);    
  }
  void ins_bottom(T item){
    remove(item);
    insert(evth_list::end(),item);    
  }
  void ins_before(T base, T item){
    remove(item);
    iterator base_it = std::find(evth_list::begin(), evth_list::end(), base);
    if(base_it==evth_list::end()){
      err_printf(("Insert_before: base item is not in the list\n"));
      return;
    }
    insert(base_it,item);
  }
  void ins_after(T base, T item){
    remove(item);
    iterator base_it = std::find(evth_list::begin(), evth_list::end(), base);
    if(base_it==evth_list::end()){
      err_printf(("Insert_after: base item is not in the list\n"));
      return;
    }
    base_it++;
    insert(base_it,item);
  }
  void remove(T base){
    iterator base_it = std::find(evth_list::begin(), evth_list::end(), base);
    if(base_it==evth_list::end()){
      //err_printf(("Remove: base item is not in the list\n"));
      return;
    }
    erase(base_it);
  }
  void replace(T base, T item){
    if(base==item) return; // no replace for the same element
    remove(item);
    iterator base_it = std::find(evth_list::begin(), evth_list::end(), base);
    if(base_it==evth_list::end()){
      err_printf(("Replace: base item is not in the list\n"));
      return;
    }
    *base_it=item;
  }
};


class EvthList: public EvtHandle, public evth_list<EvtHandle*>{
 public:
  EvtHandle *modal;
  EvthList();
  void Handle(EventBall *e);
};


class EvthStack: public EvtHandle, public std::stack<EvthList>{
 public:
  void Handle(EventBall *e);
};

#endif
