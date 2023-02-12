#include "drawContext.h"
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


void DrawCntx::trySetGLName(int& glname){
  if(glname<=0)
    glname = ++m_glnamecount;
}

int DrawCntx::w(){
  return m_centralWidget->width();
}

int DrawCntx::h(){
  return m_centralWidget->height();
}

void DrawCntx::setEventContext(EventContext3D* eventContext){
  m_eventContext = eventContext;
}

EventContext3D& DrawCntx::eventContext(){
  return *m_eventContext;
}

void DrawCntx::update(){
  m_centralWidget->update();
}

QOpenGLWidget* DrawCntx::glWidget(){
  return m_centralWidget;
}

DrawCntx::DrawCntx(CentralWidget* cw):
  m_centralWidget(cw){}


DrawCntx::~DrawCntx(){

}
