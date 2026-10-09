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
#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
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
  QSurfaceFormat coreFormat;
  coreFormat.setRenderableType(QSurfaceFormat::OpenGL);
  coreFormat.setVersion(3, 3);
  coreFormat.setProfile(QSurfaceFormat::CoreProfile);
  coreFormat.setDepthBufferSize(24);
  coreFormat.setStencilBufferSize(8);
#ifndef NDEBUG
  coreFormat.setOption(QSurfaceFormat::DebugContext);
#endif
  setFormat(coreFormat);

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
  QObject::disconnect(m_contextCleanupConnection);
  m_contextCleanupConnection = connect(
    context(), &QOpenGLContext::aboutToBeDestroyed,
    this, &CentralWidget::cleanupOpenGL, Qt::DirectConnection);
  const QSurfaceFormat actualFormat = context()->format();
  qInfo() << "OpenGL context:"
          << actualFormat.majorVersion() << "."
          << actualFormat.minorVersion()
          << (actualFormat.profile() == QSurfaceFormat::CoreProfile
                ? "core profile" : "non-core profile");
  if(actualFormat.profile() != QSurfaceFormat::CoreProfile
      || actualFormat.version() < qMakePair(3, 3)) {
    qWarning() << "OpenGL 3.3 Core Profile was requested but not provided";
  }
#ifndef NDEBUG
  m_openGLDebugLogger = std::make_unique<QOpenGLDebugLogger>(this);
  if(m_openGLDebugLogger->initialize()) {
    connect(m_openGLDebugLogger.get(), &QOpenGLDebugLogger::messageLogged,
            this, [](const QOpenGLDebugMessage& message) {
      qWarning().noquote() << "OpenGL:" << message.message();
    });
    m_openGLDebugLogger->disableMessages(
      QOpenGLDebugMessage::AnySource,
      QOpenGLDebugMessage::AnyType,
      QOpenGLDebugMessage::NotificationSeverity);
    m_openGLDebugLogger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
    qInfo() << "OpenGL debug logger enabled";
  } else {
    qWarning() << "OpenGL debug context/logger is unavailable";
    m_openGLDebugLogger.reset();
  }
#endif
  m_modernRenderer->initialize();
}

void
CentralWidget::resizeGL(int w, int h) {}

void
CentralWidget::paintGL(){
  QOpenGLFunctions* functions = context()->functions();
  functions->glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  functions->glClear(
    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#ifdef off // physical viewport size test
  float viewport[4];
  functions->glGetFloatv(GL_VIEWPORT,viewport);
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
    // QOpenGLWidget can destroy its context from its base destructor, after
    // the CentralWidget part of the object no longer exists. Disconnect the
    // derived-class callback before reaching that stage.
    QObject::disconnect(m_contextCleanupConnection);
    cleanupOpenGL();
    delete ui;
}

void CentralWidget::cleanupOpenGL()
{
  if(!context())
    return;

  makeCurrent();
  if(m_openGLDebugLogger) {
    m_openGLDebugLogger->stopLogging();
    m_openGLDebugLogger.reset();
  }
  m_modernRenderer->destroy();
  doneCurrent();
}
