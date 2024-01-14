#ifndef lights_h
#define lights_h
#include "CommonComponents/drawContext.h"
//#include "mathlib/mathutl/mymath.h"
#include "mousectrl.h"
//#include "toolspanel.h"
//#include "eventhnd.h"

class Lights;
class Icon3DLight: public  EditViewObj{  
public:
  int glsel_name = -1;
  float *rot;
  Lights *lights = nullptr;
  virtual void  Draw(DrawCntx *cntx);
  void  TreeScan(TSOCntx *cntx);
};

#ifdef off

class LightsEH: public EvtHandle{
 public:
  Lights *gl;
  int state_drag;

  LightsEH(Lights *v);
  virtual int glName(int id);
  virtual void Handle(EventBall *eventball);
};
#endif

class Lights: public EditViewObj{
 public:
  Lights();

  virtual void AskForData(sV2::Serializer* s);
  virtual void TreeScan(TSOCntx* cntx);
  virtual void Draw(DrawCntx* cntx);
 
  float rot1[16],rot2[16];
  int show1,show2;

  Icon3DLight *g;
  Icon3DLight glic1, glic2;
  

  Ctrl3DRotate mrot;

  int  isfocus(int id);
  void focusctrl(int x, int y);
  void lightrstart(int x, int y);
  void lightrcont(int x, int y);
};


#endif
