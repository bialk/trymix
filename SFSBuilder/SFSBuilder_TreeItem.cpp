#include "SFSBuilder_TreeItem.h"
#include "QMainWindow"
#include "imageplane.h"
#include "viewctrl.h"
#include "lights.h"
#include "toolspanel.h"
#include "EventHandling.h"
#include "CentralWidget.h"
#include "testScene.h"

#include <QOpenGLWidget>
#include <QPainter>
#include <QDebug>


namespace {
  class EventHandler_PositionController2: public EventHandler3D{

  public:
    ViewCtrl* m_vp = nullptr;
    Lights* m_lights = nullptr;
    EventHandler_PositionController2(ViewCtrl* vc, Lights* lights)
      :m_vp(vc)
      ,m_lights(lights)
    {
      addReact("K:Z:DOWN+M:L:DOWN") = [=](EventContext3D& cx)
      {
        auto dragProcessor = m_vp->startOperation(ViewCtrl::Scale,cx.glx(),cx.gly());
        m_dragHandler = std::make_unique<EventHandler3D>();
        m_dragHandler->addReact("M:MOVE") = [dragProcessor](EventContext3D& cx){
          dragProcessor(cx.glx(),cx.gly());
          cx.update();
        };
        m_dragHandler->addReact("M:L:UP") =  [](EventContext3D& cx){
          cx.popHandler();
          cx.update();
        };

        cx.pushHandler(m_dragHandler.get());
        cx.update();
      };

      addReact("K:X:DOWN+M:L:DOWN") = [=](EventContext3D& cx)
      {
        auto dragProcessor = m_vp->startOperation(ViewCtrl::origRotate,cx.glx(),cx.gly());
        m_dragHandler = std::make_unique<EventHandler3D>();
        m_dragHandler->addReact("M:MOVE") = [dragProcessor](EventContext3D& cx){
          dragProcessor(cx.glx(),cx.gly());
          cx.update();
        };
        m_dragHandler->addReact("M:L:UP") =  [](EventContext3D& cx){
          cx.popHandler();
          cx.update();
        };

        cx.pushHandler(m_dragHandler.get());
        cx.update();
      };

      addReact("K:C:DOWN+M:L:DOWN") = [=](EventContext3D& cx)
      {
        auto dragProcessor = m_vp->startOperation(ViewCtrl::CamRotate,cx.glx(),cx.gly());
        m_dragHandler = std::make_unique<EventHandler3D>();
        m_dragHandler->addReact("M:MOVE") = [dragProcessor](EventContext3D& cx){
          dragProcessor(cx.glx(),cx.gly());
          cx.update();
        };
        m_dragHandler->addReact("M:L:UP") =  [](EventContext3D& cx){
          cx.popHandler();
          cx.update();
        };

        cx.pushHandler(m_dragHandler.get());
        cx.update();
      };

      addReact("K:V:DOWN+M:L:DOWN") = [=](EventContext3D& cx)
      {
        auto dragProcessor = m_vp->startOperation(ViewCtrl::FoV,cx.glx(),cx.gly());
        m_dragHandler = std::make_unique<EventHandler3D>();
        m_dragHandler->addReact("M:MOVE") = [dragProcessor](EventContext3D& cx){
          dragProcessor(cx.glx(),cx.gly());
          cx.update();
        };
        m_dragHandler->addReact("M:L:UP") =  [](EventContext3D& cx){
          cx.popHandler();
          cx.update();
        };

        cx.pushHandler(m_dragHandler.get());
        cx.update();
      };

      addReact("S:RESIZE") = [=](EventContext3D& cx)
      {
        m_vp->updateProjectionMtrx(cx.w(),cx.h());
        cx.update();
      };

      addReact("M:L:DOWN") = [=](EventContext3D& cx){
      //addReact("M:MOVE") = [=](EventContext3D& cx){
        m_vp->updateSelectionMtrx(cx.glx(), cx.gly());
//        m_vp->mssh.sx0=cx.x(); m_vp->mssh.sy0=cx.y();
//        m_vp->SetProjectionMatrix();
        auto id = cx.select();
//        m_lights->select(id);
//        if(m_lights->isfocus()){
//          m_lights->lightrstart(cx.x(),cx.y());
//          cx.pushHandler(&m_mouseDragLight);
//        }

        qDebug() << "Selected name: " << id << Qt::endl;
        cx.update();
      };

      m_mouseDragLight.addReact("M:MOVE") = [=](EventContext3D& cx){
        m_lights->lightrcont(cx.x(),cx.y());
        cx.update();
      };

      m_mouseDragLight.addReact("M:L:UP") =  [this](EventContext3D& cx){
        m_lights->lightrstop();
        cx.update();
        cx.popHandler();
      };

    }
    std::unique_ptr<EventHandler3D> m_dragHandler;
    EventHandler3D m_mouseDragLight;
  };
}




SFSBuilder_TreeItem::SFSBuilder_TreeItem()
  :m_sfs(new ImagePlane)
  ,m_viewCtrl(new ViewCtrl)
  ,m_lights(new Lights)
  ,m_toolsPanel(new ToolPanel)
{
  m_viewCtrl->TreeScan(&TSOCntx::TSO_Init);
  m_sfs->TreeScan(&TSOCntx::TSO_Init);
  m_toolsPanel->Add(&m_lights->glic1);
  m_toolsPanel->Add(&m_lights->glic2);
  m_lights->TreeScan(&TSOCntx::TSO_Init);

  //m_toolsPanel->TreeScan(&TSOCntx::TSO_Init);
  m_toolsPanel->m_viewctrl = m_viewCtrl.get();

  m_viewCtrl->TreeScan(&TSOCntx::TSO_LayoutLoad);
  m_viewCtrlEH.reset(new EventHandler_PositionController2(m_viewCtrl.get(),m_lights.get()));


  setData(0,Qt::DisplayRole,"SFS Builder");
  setData(1,Qt::DisplayRole, "On");
  setData(2,Qt::DisplayRole, "On");

  m_dockWidget.reset(new QDockWidget);
  m_panel.setupUi(m_dockWidget.data());

  drawTestScene();

  m_sfs->TreeScan(&TSOCntx::TSO_ProjectLoad);
  m_sfs->Build();
}

SFSBuilder_TreeItem::~SFSBuilder_TreeItem(){}

void
SFSBuilder_TreeItem::showModel(DrawCntx* cx)
{
  glClearDepth(1.0);
  if(m_viewCtrl->m_background==0)
    glClearColor(.0, .0, .0, 0.0);
  else
    glClearColor(1.0, 1.0, 1.0, 0.0);

  glClear(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT |
          GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE); //need to support lights
  glDepthFunc(GL_LEQUAL);

  glDisable(GL_CLIP_PLANE0);
  glDisable(GL_CLIP_PLANE1);
  glDisable(GL_CLIP_PLANE2);
  glDisable(GL_CLIP_PLANE3);
  glDisable(GL_CLIP_PLANE4);
  glDisable(GL_CLIP_PLANE5);

  // Default Settings
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_TEXTURE_2D);

  m_viewCtrl->Draw(cx);
  m_lights->Draw(cx);

  drawTestScene();

#ifdef off
  m_sfs->image_mode = ImagePlane::image_mode_image;
  m_sfs->shape_mode = ImagePlane::shape_mode_image;
  m_sfs->edit_mode = ImagePlane::edit_mode_off;
  m_sfs->Draw(cx);
#endif

  m_toolsPanel->Draw(cx);
}

void
SFSBuilder_TreeItem::activateProjectTreeItem(QDockWidget* dock, bool activate){
  auto mainwin = findParentOfType<QMainWindow>(dock);
  CentralWidget* cw= mainwin->findChild<CentralWidget*>();
  if(activate){
    if(!m_dockWidget->isActiveWindow()){
      mainwin->tabifyDockWidget(dock,m_dockWidget.get());
    }
    m_dockWidget->activateWindow();
    m_dockWidget->show();
    m_dockWidget->raise();
    //cw->eventHandler().addChild(m_viewCtrlEH.get());
    cw->eventContext().pushHandler(m_viewCtrlEH.get());

    // update viewport size if it was changed before
    m_viewCtrl->updateProjectionMtrx(cw->width(),cw->height());
//    m_viewCtrl->mssh.wndw=cw->width(); m_viewCtrl->mssh.wndh=cw->height();
//    m_viewCtrl->SetProjectionMatrix();
  }
  else{
    m_dockWidget->hide();
    //cw->eventHandler().removeChild(m_viewCtrlEH.get());
    cw->eventContext().popHandler();
  }
}
