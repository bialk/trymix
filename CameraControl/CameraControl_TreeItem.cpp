#include "CameraControl_TreeItem.h"
#include "CommonComponents/lights.h"
#include "CommonComponents/toolspanel.h"
#include "CommonComponents/viewctrl.h"
#include "CommonComponents/EventHandling.h"
#include "CommonComponents/CentralWidget.h"
#include "CommonComponents/mainwindow.h"
#include "CommonComponents/ModernOpenGLRenderer.h"

#include "QMainWindow"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QDebug>
#include <QMatrix4x4>

#include <glm/gtc/type_ptr.hpp>

#ifdef TRYMIX_ENABLE_VTK
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkConeSource.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>

#include <algorithm>
#include <array>
#include <random>
#include <utility>
#endif


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



CameraControl_TreeItem::CameraControl_TreeItem()
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
  m_toolsPanel->TreeScan(&TSOCntx::TSO_Init);


  setData(0,Qt::DisplayRole, name());
  setData(1,Qt::DisplayRole, "");

  buildContextMenuStandardItems();

  m_dockWidget.reset(new QDockWidget);
  m_panel.setupUi(m_dockWidget.data());

#ifdef TRYMIX_ENABLE_VTK
  enum class Shape { Sphere, Cone, Box };
  std::array shapes{Shape::Sphere, Shape::Sphere, Shape::Cone, Shape::Box};
  std::mt19937 randomEngine(std::random_device{}());
  std::shuffle(shapes.begin(), shapes.end(), randomEngine);
  std::uniform_real_distribution<double> size(0.45, 0.75);
  std::uniform_real_distribution<double> color(0.20, 0.95);

  constexpr std::array<std::array<double, 3>, 4> positions{{
    {{-1.0,  1.0, 0.0}}, {{1.0,  1.0, 0.0}},
    {{-1.0, -1.0, 0.0}}, {{1.0, -1.0, 0.0}}
  }};

  m_vtkObjects.reserve(shapes.size());
  for(std::size_t i = 0; i < shapes.size(); ++i) {
    VtkObject object;
    const auto& position = positions[i];
    const double objectSize = size(randomEngine);

    if(shapes[i] == Shape::Sphere) {
      auto sphere = vtkSmartPointer<vtkSphereSource>::New();
      sphere->SetCenter(position[0], position[1], position[2]);
      sphere->SetRadius(objectSize);
      sphere->SetThetaResolution(48);
      sphere->SetPhiResolution(32);
      object.model = sphere;
    } else if(shapes[i] == Shape::Cone) {
      auto cone = vtkSmartPointer<vtkConeSource>::New();
      cone->SetCenter(position[0], position[1], position[2]);
      cone->SetDirection(0.0, 1.0, 0.0);
      cone->SetHeight(objectSize * 2.0);
      cone->SetRadius(objectSize);
      cone->SetResolution(48);
      object.model = cone;
    } else {
      auto box = vtkSmartPointer<vtkCubeSource>::New();
      box->SetCenter(position[0], position[1], position[2]);
      box->SetXLength(objectSize * 1.7);
      box->SetYLength(objectSize * 1.7);
      box->SetZLength(objectSize * 1.7);
      object.model = box;
    }

    object.mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    object.mapper->SetInputConnection(object.model->GetOutputPort());
    object.actor = vtkSmartPointer<vtkActor>::New();
    object.actor->SetMapper(object.mapper);
    object.actor->GetProperty()->SetColor(
      color(randomEngine), color(randomEngine), color(randomEngine));
    m_vtkObjects.push_back(std::move(object));
  }

  m_vtkCamera = vtkSmartPointer<vtkCamera>::New();
#endif
}

CameraControl_TreeItem::~CameraControl_TreeItem(){}

#ifdef TRYMIX_ENABLE_VTK
void CameraControl_TreeItem::showVtkModel(VtkDrawCntx* context)
{
  vtkRenderer* renderer = context->renderer();
  renderer->SetBackground(0.08, 0.10, 0.14);
  renderer->SetActiveCamera(m_vtkCamera);
  for(const auto& object : m_vtkObjects)
    renderer->AddActor(object.actor);
  if(!m_vtkCameraInitialized) {
    renderer->ResetCamera();
    m_vtkCameraInitialized = true;
  }
}
#endif

void
CameraControl_TreeItem::showModel(DrawCntx* cx)
{
  m_viewCtrl->Draw(cx);
  m_lights->Draw(cx);


  m_toolsPanel->Draw(cx);

  cx->openGLFunctions().glClear(GL_DEPTH_BUFFER_BIT);
  // GLM exposes column-major storage, while this QMatrix4x4 constructor reads
  // the supplied values in row-major order. Transpose after copying so the
  // shader receives the same matrices used by the legacy OpenGL path.
  cx->modernRenderer().drawTestScene(
    cx->projectionMatrix(), cx->modelViewMatrix());
}

void
CameraControl_TreeItem::activateProjectTreeItem(QDockWidget* dock, bool activate){
  auto mainwin = findParentOfType<MainWindow>(dock);
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
