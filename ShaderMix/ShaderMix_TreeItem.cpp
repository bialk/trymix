#include "ShaderMix_TreeItem.h"
#include "CommonComponents/lights.h"
#include "CommonComponents/toolspanel.h"
#include "CommonComponents/viewctrl.h"
#include "CommonComponents/EventHandling.h"
#include "CommonComponents/CentralWidget.h"
#include "CommonComponents/ModernOpenGLRenderer.h"

#include "QMainWindow"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QDebug>
#include <QMatrix4x4>

#include <glm/gtc/type_ptr.hpp>


namespace {

class EventHandler_PositionController: public EventHandler3D{

public:
  ViewCtrl* m_vp = nullptr;
  Lights* m_lights = nullptr;

  ViewCtrl::Opercode Op{ViewCtrl::Opercode::origRotate};

  EventHandler_PositionController(ViewCtrl* vc, Lights* lights, ToolPanel* tools)
    :m_vp(vc)
    ,m_lights(lights)
    ,m_tools(tools)
  {
    addReact("M:L:DOWN") = [this](EventContext3D& cx)
    {
      if(m_lights->focus(m_tools->pick(cx.x(), cx.y()))){
        m_lights->lightrstart(cx.x(),cx.y());
        cx.pushHandler(&m_mouseDragLight);
      }
      else{
        auto dragProcessor = m_vp->startOperation(Op,cx.glx(),cx.gly());
        m_dragHandler = std::make_unique<EventHandler3D>();
        m_dragHandler->addReact("M:MOVE") = [dragProcessor](EventContext3D& cx){
          dragProcessor(cx.glx(),cx.gly());
          cx.update();
        };
        m_dragHandler->addReact("M:L:UP") =  [](EventContext3D& cx){
          cx.popHandler();
          cx.update();
        };

        addReact("S:RESIZE") = [this](EventContext3D& cx)
        {
          m_vp->updateProjectionMtrx(cx.w(),cx.h());
          cx.update();
        };

        cx.pushHandler(m_dragHandler.get());
      }
      cx.update();
    };

    m_mouseDragLight.addReact("M:MOVE") = [this](EventContext3D& cx){
      m_lights->lightrcont(cx.x(),cx.y());
      cx.update();
    };

    m_mouseDragLight.addReact("M:L:UP") =  [](EventContext3D& cx){
      cx.update();
      cx.popHandler();
    };

    addReact("K:C:DOWN") = [this](EventContext3D& cx){  Op = ViewCtrl::Opercode::Scale; };
    addReact("K:X:DOWN") = [this](EventContext3D& cx){  Op = ViewCtrl::Opercode::origRotate; };
    addReact("K:Z:DOWN") = [this](EventContext3D& cx){  Op = ViewCtrl::Opercode::CamRotate; };
    addReact("K:V:DOWN") = [this](EventContext3D& cx){  Op = ViewCtrl::Opercode::FoV; };

    addReact("S:RESIZE") = [this](EventContext3D& cx)
    {
      m_vp->updateProjectionMtrx(cx.w(),cx.h());
      cx.update();
    };
  }
  std::unique_ptr<EventHandler3D> m_dragHandler;
  EventHandler3D m_mouseDragLight;
  ToolPanel* m_tools = nullptr;
};
}



ShaderMix_TreeItem::ShaderMix_TreeItem()
  :m_viewCtrl(new ViewCtrl)
  ,m_lights(new Lights)
  ,m_toolsPanel(new ToolPanel)
{
  m_viewCtrl->TreeScan(&TSOCntx::TSO_Init);
  m_toolsPanel->Add(&m_lights->glic1);
  m_toolsPanel->Add(&m_lights->glic2);
  m_lights->TreeScan(&TSOCntx::TSO_Init);

  //m_toolsPanel->TreeScan(&TSOCntx::TSO_Init);
  m_toolsPanel->m_viewctrl = m_viewCtrl.get();

  m_viewCtrl->TreeScan(&TSOCntx::TSO_LayoutLoad);
  m_viewCtrlEH.reset(new EventHandler_PositionController(
    m_viewCtrl.get(), m_lights.get(), m_toolsPanel.get()));


  setData(0,Qt::DisplayRole, name());
  setData(1,Qt::DisplayRole, "");

  buildContextMenuStandardItems();

  m_dockWidget.reset(new QDockWidget);
  m_panel.setupUi(m_dockWidget.data());

}

ShaderMix_TreeItem::~ShaderMix_TreeItem(){}

void
ShaderMix_TreeItem::showModel(DrawCntx* cx)
{
  m_viewCtrl->Draw(cx);
  m_lights->Draw(cx);


  m_toolsPanel->Draw(cx);

  cx->openGLFunctions().glClear(GL_DEPTH_BUFFER_BIT);
  cx->modernRenderer().drawTestScene(
    cx->projectionMatrix(), cx->modelViewMatrix());
}

void
ShaderMix_TreeItem::activateProjectTreeItem(QDockWidget* dock, bool activate){
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
    setData(1,Qt::DisplayRole, "*");
  }
  else{
    m_dockWidget->hide();
    //cw->eventHandler().removeChild(m_viewCtrlEH.get());
    cw->eventContext().popHandler();
    setData(1,Qt::DisplayRole, "");
  }
}
