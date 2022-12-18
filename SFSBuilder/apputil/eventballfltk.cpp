#include "apputil/eventballfltk.h"
#include <Fl/Fl.h>

void EventBallFltk::Handle(int evt){    
  int key;

  x=Fl::event_x();
  y=Fl::event_y();

  if(evt==FL_LEAVE){
    Reset();
    return;
  }

  keymod=0;
  if(Fl::event_state(FL_SHIFT))
    keymod|=KM_SHIFT;
  if(Fl::event_state(FL_CTRL))
    keymod|=KM_CTRL;
  if(Fl::event_state(FL_ALT))
    keymod|=KM_ALT;

  switch( evt){
  case FL_KEYDOWN:
    key=Fl::event_key();
    if(key!=65505 &&  // shift 
       key!=65507 &&  // ctrl
       key!=65513)    // alt
      addkey(key);
    lastev=K_DOWN;
    break;
  case FL_KEYUP:
    rmkey(Fl::event_key());
    lastev=K_UP;
    break;        
  case FL_PUSH:
    lastev=M_DOWN;
    key = Fl::event_button();
    msbtn |= (0x1<<(key-1));
    break;
  case FL_RELEASE:
    lastev=M_UP;
    key = Fl::event_button();
    msbtn &= !(0x1<<(key-1));
    break;
  case FL_DRAG:
    lastev=M_DRAG;
    break;
  default:
    lastev=NONE;
    break;
  }
}

