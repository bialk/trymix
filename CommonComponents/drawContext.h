#ifndef editviewobj_h
#define editviewobj_h

#include <QMatrix4x4>
#include "ViewportContext.h"

namespace sV2 { class Serializer; }
class QOpenGLWidget;
class CentralWidget;
class EventContext3D;
class OpenGLRenderer;
class QOpenGLFunctions;

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

class DrawCntx : public ViewportDispatchContext {
 public:
  DrawCntx(CentralWidget* cw);
  virtual ~DrawCntx();

  QOpenGLWidget* glWidget();
  int w();
  int h();
  void update();
  void setViewMatrices(const QMatrix4x4& projection,
                       const QMatrix4x4& modelView);
  const QMatrix4x4& projectionMatrix() const;
  const QMatrix4x4& modelViewMatrix() const;
  OpenGLRenderer& openGLRenderer();
  QOpenGLFunctions& openGLFunctions();

private:  
  CentralWidget* m_centralWidget{nullptr};
  QMatrix4x4 m_projection;
  QMatrix4x4 m_modelView;
};

class EditViewObj{
 public:
  virtual ~EditViewObj(){};

  virtual void Draw(DrawCntx *cntx) = 0;
  virtual void DrawWithMatrices(DrawCntx* cntx,
                                const QMatrix4x4& projection,
                                const QMatrix4x4& modelView)
  {
    Draw(cntx);
  }
  virtual void TreeScan(TSOCntx *){};
  virtual void AskForData(sV2::Serializer *s){};

};

#endif
