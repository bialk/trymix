#include "SFSBuilder_TreeItem.h"
#include "QMainWindow"
#include "imageplane.h"
#include "viewctrl.h"
#include "lights.h"
#include "toolspanel.h"
#include "EventHandling.h"
#include "CentralWidget.h"

#include <QOpenGLWidget>
#include <QPainter>
#include <QDebug>

namespace {
  class EventHandler_PositionController2: public EventHandler3D{

  public:
    ViewCtrl* vpc = nullptr;
    EventHandler_PositionController2(ViewCtrl* vc)
      :vpc(vc)
    {
      m_mouseDrag.addReact("M:MOVE") = [this](EventContext3D& cx){
        vpc->movecont(cx.x(),cx.y());
        cx.update();
      };

      m_mouseDrag.addReact("M:L:UP") =  [this](EventContext3D& cx){
        cx.glWidget()->makeCurrent();
        vpc->movestop(cx.x(),cx.y());
        cx.update();
        cx.popHandler();
      };

      addReact("K:Z:DOWN+M:L:DOWN") = [=](EventContext3D& cx)
      {
        vpc->movestart(3,cx.x(),cx.y());
        cx.update();
        cx.pushHandler(&m_mouseDrag);
      };

      addReact("K:X:DOWN+M:L:DOWN") = [=](EventContext3D& cx)
      {
        vpc->movestart(1,cx.x(),cx.y());
        cx.update();
        cx.pushHandler(&m_mouseDrag);
      };

      addReact("K:C:DOWN+M:L:DOWN") = [=](EventContext3D& cx)
      {
        vpc->movestart(2,cx.x(),cx.y());
        cx.update();
        cx.pushHandler(&m_mouseDrag);
      };
      addReact("S:RESIZE") = [=](EventContext3D& cx)
      {
        vpc->mssh.wndw=cx.x(); vc->mssh.wndh=cx.y();
        vpc->SetProjectionMatrix();
        cx.update();
      };
      addReact("M:L:DOWN") = [=](EventContext3D& cx){
        cx.glWidget()->makeCurrent();
        vpc->mssh.sx0=cx.x(); vpc->mssh.sy0=cx.y();
        vpc->SetProjectionMatrix();
        glSelectBuffer (1024, vpc->selectBuf);
        glRenderMode (GL_SELECT);
        glInitNames();
        cx.glWidget()->paintGL();
        vpc->hits = glRenderMode (GL_RENDER);
        if(vpc->hits==-1){
              printf("ViewCtrl::SelectObj2 \"Selection Buffer overflow\"\n");
          vpc->hits=0;
        }
        unsigned int stack[]={0};
        int id = vpc->ProcessHits2(0, stack);


//        int id = vpc->SelectObj(cx.x(),cx.y());
        qDebug() << "Selected name: " << id << Qt::endl;
        cx.glWidget()->update();

//        if( glName(id) ) {

//          gl->dv->reselect();
//          gl->select();
//          gl->lightrstart(eventball->x,eventball->y);
//          eventball->genstate(state_drag);

//          eventball->stop();
//          gl->dv->redraw();

//        }

      };
    }
    EventHandler3D m_mouseDrag;
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
  m_toolsPanel->viewctrl = m_viewCtrl.get();

  m_viewCtrl->TreeScan(&TSOCntx::TSO_LayoutLoad);
  m_viewCtrlEH.reset(new EventHandler_PositionController2(m_viewCtrl.get()));


  setData(0,Qt::DisplayRole,"SFS Builder");
  setData(1,Qt::DisplayRole, "On");
  setData(2,Qt::DisplayRole, "On");

  m_dockWidget.reset(new QDockWidget);
  m_panel.setupUi(m_dockWidget.data());


  m_sfs->TreeScan(&TSOCntx::TSO_ProjectLoad);
  m_sfs->Build();
}

SFSBuilder_TreeItem::~SFSBuilder_TreeItem(){}

void
SFSBuilder_TreeItem::showModel(DrawCntx* cx)
{
  glViewport(0,0,cx->glWidget()->width(),cx->glWidget()->height());

  glClearDepth(1.0);
  if(m_viewCtrl->background==0)
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

  m_sfs->image_mode = ImagePlane::image_mode_image;
  m_sfs->shape_mode = ImagePlane::shape_mode_image;
  m_sfs->edit_mode = ImagePlane::edit_mode_off;
  m_sfs->Draw(cx);

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
    m_viewCtrl->mssh.wndw=cw->width(); m_viewCtrl->mssh.wndh=cw->height();
    m_viewCtrl->SetProjectionMatrix();
  }
  else{
    m_dockWidget->hide();
    //cw->eventHandler().removeChild(m_viewCtrlEH.get());
    cw->eventContext().popHandler();
  }
}
