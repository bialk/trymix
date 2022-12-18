#ifndef eventball_h
#define eventball_h
#include <algorithm>

enum EV_TYPE {
  NONE,
  M_DRAG,
  M_UP,
  M_DOWN,
  K_UP,
  K_DOWN,
  Q_ADD,
  Q_DEL
};

#define   KM_SHIFT  0x1
#define   KM_CTRL   0x2
#define   KM_ALT    0x4

#define   EB_RET_NONE   0x0
#define   EB_RET_REDRAW 0x1

class EventBall{
public:

  int bufsz;
  int buf[10];
  int keymod;
  EV_TYPE lastev;
  int cstate,statecnt;
  int msbtn;
  int x,y;
  int gx,gy;  //mouse position in opengl screen coordinates
  int retcode;

 EventBall(): statecnt(0),retcode(0) { Reset();}

  void  Reset(){
    std::fill(buf,buf+10,0);
    bufsz=0;
    lastev=NONE;
    cstate=0;
    msbtn=0;
    keymod=0;
  }

  bool is(const char* s, EV_TYPE v){
    const char *i;
    for(i=s;*i!=0;i++){
      int* it = std::find(buf,buf+bufsz,*i);
      if(it==buf+bufsz) return 0;      
    }
    if(i-s!=bufsz) return 0;
    if(lastev!=v) return 0;

    return 1;
  }
  void addkey(int v){
     if(v){
        int *it=std::find(buf,buf+bufsz,v);
        if(it==buf+bufsz){
           *it=v; bufsz++;
        }
     }
  }
  void rmkey(int v){
      int *it = std::remove(buf,buf+bufsz,v);
      bufsz=int(it-buf);
  }
  bool keys(const char *s){
    const char *i;
    for(i=s;*i!=0;i++){
      int* it = std::find(buf,buf+bufsz,*i);
      if(it==buf+bufsz) return 0;      
    }
    if(i-s!=bufsz) return 0;
    return 1;
  }
  bool key(int v){
      int* it = std::find(buf,buf+bufsz,v);
      if(it==buf+bufsz) return 0;
      return 1;    
  }
  bool kmod(int v){
    return v==keymod;    
  }
  bool event(EV_TYPE v){
    return v==lastev;
  }
  void stop(){
    lastev=NONE;    
  }
  bool mbtn(int v){    
    return v==msbtn;
  }
  bool state(int stat){
    return stat==cstate;
  }
  void setstate(int stat){
    cstate=stat;
  }
  void genstate(int &stat){
    if(stat<=0)
      stat=++statecnt;
    setstate(stat);
  }
};

//extern EventBall *eventball;


#endif 
