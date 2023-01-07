#ifndef editviewobj_h
#define editviewobj_h
//#include "eventlog.h"
#include <iostream>


#define GL3DFIND(name) ((name *)dv->gl3dfind(#name))

class DispView;
class Serializer;
class ViewCtrl;
class CentralWidget;

#define SRLZ_LAYOUT   1
#define SRLZ_PROJECT  2

// Tree Scan Operations TSO
// we need to rebuild this using context rather then type


class TSOCntx{
 public:
  virtual ~TSOCntx();

  static TSOCntx TSO_LayoutStore;
  static TSOCntx TSO_LayoutLoad;

  static TSOCntx TSO_Init;
  static TSOCntx TSO_ProjectStore;
  static TSOCntx TSO_ProjectLoad;
  static TSOCntx TSO_ProjectNew;

   // WARNING: Don't put anything but TCOCntx instances here!
   // If you need a custom context, subclass TCOContext, allocate
   // an instance on the stack, and use dynamic_cast in handlers:
   // if (TSO_CustomContext* ctx = dynamic_cast<TSO_CustomContext*>(context)) {
};

class DrawCntx{
 public:
  DrawCntx(CentralWidget* cw);
  virtual ~DrawCntx();


  ViewCtrl* viewCtrl();
  void setViewCtrl(ViewCtrl* vc);
  void trySetGLName(int& glname);

  CentralWidget* glWidget();

private:
  CentralWidget* m_centralWidget;
  ViewCtrl* m_viewCtrl = nullptr;
  int m_glnamecount = 0;
};

class EditViewObj{
 public:

  //DispView *dv;
 
  virtual ~EditViewObj(){};

  virtual void Draw(DrawCntx *cntx) = 0;
  //virtual void TreeScan(TSOCntx *){};
  //virtual void AskForData(Serializer *s){};

};

#endif
