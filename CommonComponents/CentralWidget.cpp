#include "CentralWidget.h"
#include "ui_CentralWidget.h"
#include "ProjectTree.h"
#include "EventHandling.h"
#include "drawContext.h"
#include "ModernOpenGLRenderer.h"

#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QObject>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>

#include <vector>
#include <unordered_set>
#include <stack>

class TreeEventFilter: public QObject{
public:
  TreeEventFilter(CentralWidget* cw)
    :m_eventContext3D(cw)
    ,m_drawContext(cw)
  {
    connect(qApp, &QGuiApplication::applicationStateChanged,
                     [](Qt::ApplicationState state){
      if(state == Qt::ApplicationState::ApplicationActive){
        //qDebug() << "Application state change to active" << Qt::endl;
      }
    });
  }
  bool eventFilter(QObject *obj, QEvent* e) override{
    m_eventContext3D.setEvent(e);
    if(!m_eventContext3D.tryProcessCapture())
      m_handlerTree.handle(m_eventContext3D);
    return QObject::event(e);
  }
  EventContext3D m_eventContext3D;
  EventHandler3D m_handlerTree;
  DrawCntx m_drawContext;
};


CentralWidget::CentralWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , ui(new Ui::CentralWidget)
    , m_modernRenderer(std::make_unique<ModernOpenGLRenderer>())
{
  QSurfaceFormat legacyFormat;
  legacyFormat.setRenderableType(QSurfaceFormat::OpenGL);
  legacyFormat.setVersion(3, 3);
  legacyFormat.setProfile(QSurfaceFormat::CompatibilityProfile);
  legacyFormat.setDepthBufferSize(24);
  legacyFormat.setStencilBufferSize(8);
  setFormat(legacyFormat);

  ui->setupUi(this);
  setMouseTracking(true);
  setFocusPolicy (Qt::StrongFocus);
  m_treeEventFilter = new TreeEventFilter(this);
  installEventFilter(m_treeEventFilter);
}

EventHandler3D&
CentralWidget::eventHandler(){
  return m_treeEventFilter->m_handlerTree;
}

EventContext3D&
CentralWidget::eventContext(){
  return m_treeEventFilter->m_eventContext3D;
}

ModernOpenGLRenderer& CentralWidget::modernRenderer()
{
  return *m_modernRenderer;
}


void
CentralWidget::initializeGL(){
  m_modernRenderer->initialize();
}

void
CentralWidget::resizeGL(int w, int h) {}

void
CentralWidget::paintGL(){
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#ifdef off // physical viewport size test
  float viewport[4];
  glGetFloatv(GL_VIEWPORT,viewport);
  qDebug() << "viewport" << viewport[0] << viewport[1] << viewport[2] << viewport[3];
#endif
  m_projectTree->showModel(&m_treeEventFilter->m_drawContext);
}

void
CentralWidget::enterEvent(QEnterEvent* event){
  QOpenGLWidget::enterEvent(event);
  setFocus(Qt::MouseFocusReason);
}

void
CentralWidget::setProjectTree(ProjectTree *pt){
  m_projectTree = pt;
}

CentralWidget::~CentralWidget()
{
    if(context()) {
      makeCurrent();
      m_modernRenderer->destroy();
      doneCurrent();
    }
    delete ui;
}
