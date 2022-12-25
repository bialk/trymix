#include "PolygonTests_TreeItem.h"
#include "PolygonTests/PolygonTestConvexPartitioning.h"
#include "PolygonTests/PolygonTestMonotonePartitioning.h"
#include "PolygonTests/PolygonTestConformingDelanay.h"
#include "CentralWidget.h"
#include "mainwindow.h"
#include "ViewPortController.h"

#include <QAction>
#include <QOpenGLWidget>
#include <QPainter>
#include <QMainWindow>
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>

class EventParser{
public:
  EventParser(QEvent* e);
  bool mouseButtonPress(Qt::MouseButton buttons);
  bool mouseButtonRelease(Qt::MouseButton buttons);
  bool mouseButtonIsPressed(Qt::MouseButton buttons);
  bool mouseMove();
  int x();
  int y();
  bool keyPress(Qt::Key button);
  bool keyRelease(Qt::Key button);

private:
  QEvent* m_e = nullptr;
};

EventParser::EventParser(QEvent* e)
  :m_e(e){}
bool EventParser::mouseButtonPress(Qt::MouseButton buttons){
  if(m_e->type() == QEvent::MouseButtonPress){
    QMouseEvent* e = static_cast<QMouseEvent*>(m_e);
    return e->button() & buttons;
  }
  return false;
}
bool EventParser::mouseButtonRelease(Qt::MouseButton buttons){
  if(m_e->type() == QEvent::MouseButtonRelease){
    QMouseEvent* e = static_cast<QMouseEvent*>(m_e);
    return e->button() & buttons;
  }
  return false;
}
bool EventParser::mouseButtonIsPressed(Qt::MouseButton buttons){
  QMouseEvent* e = static_cast<QMouseEvent*>(m_e);
  return e? bool(e->buttons() & buttons): false;
}
bool EventParser::mouseMove(){
  return m_e->type() == QEvent::MouseMove;
}
int EventParser::x(){
  QMouseEvent* e = static_cast<QMouseEvent*>(m_e);
  return e?e->x():INT_MIN;
}
int EventParser::y(){
  QMouseEvent* e = static_cast<QMouseEvent*>(m_e);
  return e?e->y():INT_MIN;
}
bool EventParser::keyPress(Qt::Key button){
  if(m_e->type() == QEvent::KeyPress){
    QKeyEvent* e = static_cast<QKeyEvent*>(m_e);
    return e->key() == button;
  }
  return false;
}
bool EventParser::keyRelease(Qt::Key button){
  if(m_e->type() == QEvent::KeyRelease){
    QKeyEvent* e = static_cast<QKeyEvent*>(m_e);
    return e->key() == button;
  }
  return false;
}




class ViewportPositionController {
public:

    ViewportPositionController();

    void reset();
    void beginTranslateXY();
    void continueTranslateXY(float dx, float dy);
    void beginScale(float x, float y);
    void continueScale(float dy);
    float* getMatrix4x4();

    float translationXY[2] = {0,0};
    float scaleX = 1.;

private:
    float translationXYOld[2] = {0,0};
    float translateOfScaling[2] = {0,0};
    float scaleXOld = 1.;
    float matrix[16] = { 1.,0.,0.,0.,
                         0.,1.,0.,0.,
                         0.,0.,1.,0.,
                         0.,0.,0.,1. };
};


ViewportPositionController::ViewportPositionController(){}

void ViewportPositionController::reset(){
  scaleX = 1.;
  translationXY[0] = translationXY[1] = 0;
}

void ViewportPositionController::beginTranslateXY() {
  translationXYOld[0] = translationXY[0];
  translationXYOld[1] = translationXY[1];
}

void ViewportPositionController::continueTranslateXY(float dx, float dy) {
  float p0[4] = { dx, dy, 1, 1 };
  translationXY[0] = translationXYOld[0] + p0[0];
  translationXY[1] = translationXYOld[1] + p0[1];
}

void ViewportPositionController::beginScale(float x, float y) {
  scaleXOld = scaleX;
  translateOfScaling[0] = translationXY[0] - x;
  translateOfScaling[1] = translationXY[1] - y;
  translationXYOld[0] = translationXY[0] - translateOfScaling[0];
  translationXYOld[1] = translationXY[1] - translateOfScaling[1];
}

void ViewportPositionController::continueScale(float dy) {
  float scalingFactor = exp(-3 * dy);
  scaleX = scaleXOld * scalingFactor;
  translationXY[0] = translationXYOld[0] + translateOfScaling[0]*scalingFactor;
  translationXY[1] = translationXYOld[1] + translateOfScaling[1]*scalingFactor;
}

float* ViewportPositionController::getMatrix4x4() {
  for(auto& v: matrix) // reset matrix to zero
    v = 0;
  matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1;
  matrix[12] += translationXY[0];
  matrix[13] += translationXY[1];
  return matrix;
}





class EventHandler_PositionController: public EventHandler3D{
public:
  ViewportPositionController vpc;
  void handle(EventContext3D& ecntx) override{
    EventParser ep(ecntx.event());
    if(ep.mouseButtonPress(Qt::LeftButton) && mode != NoOp){
      if(mode == Scale){
        //qDebug() << "start scale: " << ep.x() << ep.y() << Qt::endl;
        vpc.beginScale(ep.x(),
                       ecntx.glWidget()->height() - ep.y()); // inverted y axis
        m_startXY[0] = ep.x();
        m_startXY[1] = ep.y();

        ecntx.glWidget()->update();

        ecntx.setCapture([this](EventContext3D& ecntx){
          EventParser ep(ecntx.event());
          if(ep.mouseButtonRelease(Qt::LeftButton)){
            ecntx.releaseCapture();
              //qDebug() << "end scale: " << ep.x() << ep.y() << Qt::endl;
              vpc.continueScale(2.*(ep.y() - double(m_startXY[1]))/ecntx.glWidget()->height());
              ecntx.glWidget()->update();
          }
          else if(ep.mouseMove()){
              //qDebug() << "continue scale: " << ep.x() << ep.y() << Qt::endl;
              vpc.continueScale(2*(ep.y() - double(m_startXY[1]))/ecntx.glWidget()->height());
              ecntx.glWidget()->update();
          }
          return true;
        });
      }
      else if(mode == Translate){
        //qDebug() << "start translate: " << ep.x() << ep.y() << Qt::endl;
        vpc.beginTranslateXY();
        m_startXY[0] = ep.x();
        m_startXY[1] = ep.y();
        ecntx.glWidget()->update();

        ecntx.setCapture([this](EventContext3D& ecntx){
          EventParser ep(ecntx.event());
          if(ep.mouseButtonRelease(Qt::LeftButton)){
            ecntx.releaseCapture();
              //qDebug() << "end translate: " << ep.x() << ep.y() << Qt::endl;
              float deltaXY[2] = {
                float((ep.x() - m_startXY[0])),
                float(-(ep.y() - m_startXY[1])) // inverted y axis
              };
              vpc.continueTranslateXY(deltaXY[0],deltaXY[1]);
              ecntx.glWidget()->update();
          }
          else if(ep.mouseMove()){
              //qDebug() << "continue translate: " << ep.x() << ep.y() << Qt::endl;
              float deltaXY[2] = {
                float((ep.x() - m_startXY[0])),
                float(-(ep.y() - m_startXY[1])) // inverted y axis
              };
              vpc.continueTranslateXY(deltaXY[0],deltaXY[1]);
              ecntx.glWidget()->update();
          }
          return true;
        });
      }
    }
    else if(ep.keyPress(Qt::Key_Z)){
       //qDebug() << "Key_Z pressed" << Qt::endl;
       mode = Scale;
    }
    else if(ep.keyRelease(Qt::Key_Z)){
       //qDebug() << "Key_Z released" << Qt::endl;
       mode = NoOp;
    }
    else if(ep.keyPress(Qt::Key_X)){
       //qDebug() << "Key_X pressed" << Qt::endl;
       mode = Translate;
    }
    else if(ep.keyRelease(Qt::Key_X)){
       qDebug() << "Key_X released" << Qt::endl;
       mode = NoOp;
    }
    else if(ep.keyPress(Qt::Key_C)){
       //qDebug() << "Key_C pressed" << Qt::endl;
    }
    else if(ep.keyRelease(Qt::Key_C)){
       //qDebug() << "Qt::Key_C released" << Qt::endl;
       mode = NoOp;
    }
  }
private:
  enum {
    NoOp,
    Scale,
    Translate
  } mode = NoOp;

  int m_startXY[2] = {0,0};
};


PolygonTests_TreeItem::PolygonTests_TreeItem()
  :polygentest(new PolygonTestConvexPartitioning(50))
  ,m_eh_pc(new EventHandler_PositionController)
{
  setData(0,Qt::DisplayRole,"Polygon Test");
  setData(1,Qt::DisplayRole, "On");
  setData(2,Qt::DisplayRole, "On");

  m_dockWidget.reset(new QDockWidget);
  m_panel.setupUi(m_dockWidget.data());

  QObject::connect(m_panel.pushButton_tryConvexPartitioning, &QPushButton::clicked,
    [=](bool){
       QApplication::setOverrideCursor(Qt::WaitCursor);
       tryConvexPartitioning(m_panel.spinBox->value());
       QApplication::restoreOverrideCursor();
    });
  QObject::connect(m_panel.pushButton_tryMonotonePartitioning, &QPushButton::clicked,
    [=](bool){
       QApplication::setOverrideCursor(Qt::WaitCursor);
       tryMonotonePartitioning(m_panel.spinBox->value());
       QApplication::restoreOverrideCursor();
    });
  QObject::connect(m_panel.pushButton_tryConformingDelanay, &QPushButton::clicked,
    [=](bool){
       QApplication::setOverrideCursor(Qt::WaitCursor);
       tryConformingDelanay(m_panel.spinBox->value(), m_panel.doubleSpinBox_MeshSellSize->value());
       QApplication::restoreOverrideCursor();
    });
}

PolygonTests_TreeItem::~PolygonTests_TreeItem(){
}

void
PolygonTests_TreeItem::activateProjectTreeItem(QDockWidget* dock, bool activate){
  auto mainwin = findParentOfType<MainWindow>(dock);
  CentralWidget* cw= mainwin->findChild<CentralWidget*>();
  if(activate){
    if(!m_dockWidget->isActiveWindow()){
      mainwin->tabifyDockWidget(dock,m_dockWidget.get());
    }
    m_dockWidget->activateWindow();
    m_dockWidget->show();
    m_dockWidget->raise();
    cw->eventHandler().addChild(m_eh_pc.get());
  }
  else{
    m_dockWidget->hide();
    cw->eventHandler().removeChild(m_eh_pc.get());
  }
}

void
PolygonTests_TreeItem::showModel(QOpenGLWidget* gl){

  QPainter p(gl);
  auto h = gl->height(); auto w = gl->width();
  p.setWindow(0, h, w, -h);
  p.setPen(QPen(Qt::white, 3));

  QTransform vpt;
  vpt.translate(m_eh_pc->vpc.translationXY[0],m_eh_pc->vpc.translationXY[1]);
  vpt.scale(m_eh_pc->vpc.scaleX,m_eh_pc->vpc.scaleX);
  p.setTransform(vpt, true);

  QTransform t;
  t.translate(w/2,h/2);
//  float scale = 0.45/600;
//  scale = fmin(1024*scale, 1024*scale);
//  t.scale(scale, scale);

  p.setTransform(t, true);
  p.drawPolyline(polygentest->getPolyline());
  p.setPen(QPen(Qt::red, 2/m_eh_pc->vpc.scaleX, Qt::DotLine));
  p.drawLines(polygentest->getEdges());
}

void PolygonTests_TreeItem::tryConvexPartitioning(int polygonSize){
  polygentest.reset(new PolygonTestConvexPartitioning(polygonSize));
  findParentOfType<MainWindow>(treeWidget())->centralWidget()->update();
}

void PolygonTests_TreeItem::tryMonotonePartitioning(int polygonSize){
  polygentest.reset(new PolygonTestMonotonePartitioning(polygonSize));
  findParentOfType<MainWindow>(treeWidget())->centralWidget()->update();
}

void PolygonTests_TreeItem::tryConformingDelanay(int polygonSize, float meshCellSize){
  polygentest.reset(new PolygonTestConformingDelanay(polygonSize, meshCellSize));
  findParentOfType<MainWindow>(treeWidget())->centralWidget()->update();
}
