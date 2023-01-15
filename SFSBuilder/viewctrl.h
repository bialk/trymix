#ifndef viewctrl_h
#define viewctrl_h

#include "editviewobj.h"
#include "mousectrl.h"

class ViewCtrl: public EditViewObj{
 public:

  // view point controlling methods
  int opercode;
  MouseMoveAndShift mssh;
  float zoom, zoom0, zoomy;
  float InitialModelMatrix[16];
  float PrjMtrxRender[16];
  float PrjMtrxSelect[16]; 

  // backward matrix for controling object space
  float InvMVMat[16];
  void SetProjectionMatrix();
  void BackScreenTransform(float &x, float &y, float &z);

  long play;

  //void PsvMOper(int x, int y);
  void Zoom(float r=1.0);

  int background; // 0 - black, 1 - white
  int prjtype;    // 1 - orthogonal, 2 - perspecive
  int drawsimple; // 0 - draw full  scene during view control 
                  // 1 - draw simpl scene during view control
  
  int glroshow;  // 0/1 (hide/show) controls


  ViewCtrl();
  ~ViewCtrl();
  virtual void AskForData(Serializer *s);
  virtual void TreeScan(TSOCntx *cntx);
  virtual void Draw(DrawCntx *cntx);

  void hitscene(float &x, float &y, float &z);
  void movestart(int oper, int x, int y);
  void movecont(int x, int y);
  void movestop(int x, int y);
  void reset();

  void fplay();
  int play_method;

};



#endif

