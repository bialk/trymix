#include "CentralWidget.h"
#include "ui_CentralWidget.h"
#include "ProjectTree.h"
#include "EventHandling.h"
#include "SFSBuilder/editviewobj.h"

#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QObject>
#include <QOpenGLFunctions>

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
{
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

DrawCntx&
CentralWidget::drawContext(){
  return m_treeEventFilter->m_drawContext;
}


void
CentralWidget::initializeGL(){
}

void
CentralWidget::resizeGL(int w, int h){}

void
CentralWidget::paintGL(){
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
    delete ui;
}
