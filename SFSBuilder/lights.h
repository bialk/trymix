#ifndef lights_h
#define lights_h
#include "editviewobj.h"
#include "mathlib/mathutl/mymath.h"
#include "mousectrl.h"
//#include "toolspanel.h"
#include "eventhnd.h"


class Icon3DLight: public  EditViewObj{  
public:
  int glsel_name;
  float *rot;
  virtual void  Draw(DrawCntx *cntx);
  void  TreeScan(TSOCntx *cntx);
};


class Lights;
class LightsEH: public EvtHandle{
 public:
  Lights *gl;
  int state_drag;

  LightsEH(Lights *v);
  virtual int glName(int id);
  virtual void Handle(EventBall *eventball);
};

class Lights: public EditViewObj{
 public:
  Lights();

  virtual void AskForData(Serializer* s);
  virtual void TreeScan(TSOCntx* cntx);
  virtual void Draw(DrawCntx* cntx);
 
  void    reselect();
  void    select();

  float rot1[16],rot2[16];
  int show1,show2;
  int selectid;

  Icon3DLight *g;
  Icon3DLight glic1, glic2;
  

  Ctrl3DRotate mrot;

  int  isfocus();
  void focusctrl(int x, int y);
  void lightrstart(int x, int y);
  void lightrcont(int x, int y);
  void lightrstop();

  LightsEH  lighteh;
};


#endif
