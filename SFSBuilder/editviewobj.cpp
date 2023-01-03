#include "editviewobj.h"
#include "CentralWidget.h"

// class TSOCntx (TreeScanOperCntx)
//================================================

TSOCntx::~TSOCntx(){}

TSOCntx TSOCntx::TSO_LayoutStore;
TSOCntx TSOCntx::TSO_LayoutLoad;


TSOCntx TSOCntx::TSO_Init;
TSOCntx TSOCntx::TSO_ProjectStore;
TSOCntx TSOCntx::TSO_ProjectLoad;
TSOCntx TSOCntx::TSO_ProjectNew;


ViewCtrl*
DrawCntx::viewCtrl(){
  return m_viewCtrl;
}

void
DrawCntx::setViewCtrl(ViewCtrl* vc){
  m_viewCtrl = vc;
}

void DrawCntx::trySetGLName(int& glname){
  if(glname<=0)
    glname = ++m_glnamecount;
}

CentralWidget* DrawCntx::glWidget(){
  return m_centralWidget;
}

DrawCntx::DrawCntx(CentralWidget* cw):
  m_centralWidget(cw){}


DrawCntx::~DrawCntx(){

}
