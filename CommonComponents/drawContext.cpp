#include "drawContext.h"
#include "CentralWidget.h"
#include "ModernOpenGLRenderer.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

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

void DrawCntx::setViewMatrices(
    const QMatrix4x4& projection, const QMatrix4x4& modelView)
{
  m_projection = projection;
  m_modelView = modelView;
}

const QMatrix4x4& DrawCntx::projectionMatrix() const
{
  return m_projection;
}

const QMatrix4x4& DrawCntx::modelViewMatrix() const
{
  return m_modelView;
}

ModernOpenGLRenderer& DrawCntx::modernRenderer()
{
  return m_centralWidget->modernRenderer();
}

QOpenGLFunctions& DrawCntx::openGLFunctions()
{
  QOpenGLContext* context = QOpenGLContext::currentContext();
  Q_ASSERT(context);
  return *context->functions();
}

QOpenGLWidget* DrawCntx::glWidget(){
  return m_centralWidget;
}

DrawCntx::DrawCntx(CentralWidget* cw):
  m_centralWidget(cw){}


DrawCntx::~DrawCntx(){

}
