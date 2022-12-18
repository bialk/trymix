#include "eventhnd.h"


// class EvtHandle
// ===================================================================

EvtHandle::~EvtHandle(){}
void EvtHandle::Handle(EventBall *eventball){}


// class Evthlist
//=====================================================================

  
EvthList::EvthList():modal(0){}

void EvthList::Handle(EventBall *e){
  iterator it, it_next;
  for(it= begin();it!= end();it=it_next){
    it_next=it;  it_next++;
    (*it)->Handle(e);    
  }
}

// class Evthlist
//=====================================================================

  
//EvthStack::EvthStack(){}

void EvthStack::Handle(EventBall *e){
  EvthList::iterator it, it_next;
  if(empty()) return;
  for(it= top().begin();it!= top().end();it=it_next){
    it_next=it; it_next++;
    (*it)->Handle(e);    
  }
}

