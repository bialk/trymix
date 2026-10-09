#include "drawContext.h"
#include "CentralWidget.h"
#include "ModernOpenGLRenderer.h"

// class TSOCntx (TreeScanOperCntx)
//================================================

TSOCntx::~TSOCntx(){}

TSOCntx TSOCntx::TSO_LayoutStore;
TSOCntx TSOCntx::TSO_LayoutLoad;


TSOCntx TSOCntx::TSO_Init;
TSOCntx TSOCntx::TSO_ProjectStore;
TSOCntx TSOCntx::TSO_ProjectLoad;
TSOCntx TSOCntx::TSO_ProjectNew;


int DrawCntx::w(){
  return m_centralWidget->width();
}

int DrawCntx::h(){
  return m_centralWidget->height();
}

void DrawCntx::update(){
  m_centralWidget->update();
}

ModernOpenGLRenderer& DrawCntx::modernRenderer()
{
  return m_centralWidget->modernRenderer();
}

QOpenGLWidget* DrawCntx::glWidget(){
  return m_centralWidget;
}

DrawCntx::DrawCntx(CentralWidget* cw):
  m_centralWidget(cw){}


DrawCntx::~DrawCntx(){

}
