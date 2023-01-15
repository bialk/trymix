#include "PolygonTests_TreeItem.h"
#include "PolygonTests/PolygonTestConvexPartitioning.h"
#include "PolygonTests/PolygonTestMonotonePartitioning.h"
#include "PolygonTests/PolygonTestConformingDelanay.h"
#include "CentralWidget.h"
#include "mainwindow.h"
#include "EventHandling.h"

#include <QAction>
#include <QOpenGLWidget>
#include <QPainter>
#include <QMainWindow>
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>


namespace {
class ViewCtrl {
public:

    ViewCtrl();

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


ViewCtrl::ViewCtrl(){}

void ViewCtrl::reset(){
  scaleX = 1.;
  translationXY[0] = translationXY[1] = 0;
}

void ViewCtrl::beginTranslateXY() {
  translationXYOld[0] = translationXY[0];
  translationXYOld[1] = translationXY[1];
}

void ViewCtrl::continueTranslateXY(float dx, float dy) {
  float p0[4] = { dx, dy, 1, 1 };
  translationXY[0] = translationXYOld[0] + p0[0];
  translationXY[1] = translationXYOld[1] + p0[1];
}

void ViewCtrl::beginScale(float x, float y) {
  scaleXOld = scaleX;
  translateOfScaling[0] = translationXY[0] - x;
  translateOfScaling[1] = translationXY[1] - y;
  translationXYOld[0] = translationXY[0] - translateOfScaling[0];
  translationXYOld[1] = translationXY[1] - translateOfScaling[1];
}

void ViewCtrl::continueScale(float dy) {
  float scalingFactor = exp(-3 * dy);
  scaleX = scaleXOld * scalingFactor;
  translationXY[0] = translationXYOld[0] + translateOfScaling[0]*scalingFactor;
  translationXY[1] = translationXYOld[1] + translateOfScaling[1]*scalingFactor;
}

float* ViewCtrl::getMatrix4x4() {
  for(auto& v: matrix) // reset matrix to zero
    v = 0;
  matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1;
  matrix[12] += translationXY[0];
  matrix[13] += translationXY[1];
  return matrix;
}


class EventHandler_PositionController: public EventHandler3D{

public:
  ViewCtrl vpc;
  EventHandler_PositionController()
  {
    m_mouseDragScale.addReact("M:MOVE") = [this](EventContext3D& cx){
      vpc.continueScale(2.*(cx.y() - double(m_startXY[1]))/cx.h());
      cx.update();
    };

    m_mouseDragScale.addReact("M:L:UP") =  [this](EventContext3D& cx){
      vpc.continueScale(2.*(cx.y() - double(m_startXY[1]))/cx.h());
      cx.update();
      cx.popHandler();
    };


    m_mouseDragTranslate.addReact("M:MOVE") = [this](EventContext3D& cx){
      float deltaXY[2] = {
        float((cx.x() - m_startXY[0])),
        float((cx.y() - m_startXY[1])) // inverted y axis
      };
      vpc.continueTranslateXY(deltaXY[0],deltaXY[1]);
      cx.update();
    };

    m_mouseDragTranslate.addReact("M:L:UP") =  [this](EventContext3D& cx){
      float deltaXY[2] = {
        float((cx.x() - m_startXY[0])),
        float((cx.y() - m_startXY[1])) // inverted y axis
      };
      vpc.continueTranslateXY(deltaXY[0],deltaXY[1]);
      cx.update();
      cx.popHandler();
    };

    addReact("K:Z:DOWN+M:L:DOWN") = [=](EventContext3D& cx)
    {
      vpc.beginScale(cx.x(), cx.y()); // inverted y axis
      m_startXY[0] = cx.x();
      m_startXY[1] = cx.y();
      cx.update();
      cx.pushHandler(&m_mouseDragScale);
    };

    addReact("K:X:DOWN+M:L:DOWN") = [=](EventContext3D& cx)
    {
      vpc.beginTranslateXY();
      m_startXY[0] = cx.x();
      m_startXY[1] = cx.y();
      cx.update();
      cx.pushHandler(&m_mouseDragTranslate);
    };

  }
private:
  int m_startXY[2] = {0,0};
  EventHandler3D m_mouseDragScale;
  EventHandler3D m_mouseDragTranslate;
};

}

PolygonTests_TreeItem::PolygonTests_TreeItem()
  :polygentest(new PolygonTestConvexPartitioning(50))
  ,m_vpceh(new EventHandler_PositionController)
{  
  setData(0,Qt::DisplayRole,"Polygon Test");
  setData(1,Qt::DisplayRole, "On");
  setData(2,Qt::DisplayRole, "On");

  m_dockWidget.reset(new QDockWidget);
  m_panel.setupUi(m_dockWidget.data());

  QObject::connect(m_panel.pushButton_tryConvexPartitioning, &QPushButton::clicked,
    [=]{
       QApplication::setOverrideCursor(Qt::WaitCursor);
       tryConvexPartitioning(m_panel.spinBox->value());
       QApplication::restoreOverrideCursor();
    });
  QObject::connect(m_panel.pushButton_tryMonotonePartitioning, &QPushButton::clicked,
    [=]{
       QApplication::setOverrideCursor(Qt::WaitCursor);
       tryMonotonePartitioning(m_panel.spinBox->value());
       QApplication::restoreOverrideCursor();
    });
  QObject::connect(m_panel.pushButton_tryConformingDelanay, &QPushButton::clicked,
    [=]{
       QApplication::setOverrideCursor(Qt::WaitCursor);
       tryConformingDelanay(m_panel.spinBox->value(), m_panel.doubleSpinBox_MeshSellSize->value());
       QApplication::restoreOverrideCursor();
    });
  auto& vpc = static_cast<EventHandler_PositionController*>(m_vpceh.get())->vpc;

  // initial postioning
  vpc.scaleX = 1./2;
  vpc.translationXY[0] = 100;
  vpc.translationXY[1] = 100;
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
    //cw->eventHandler().addChild(m_vpceh.get());
    cw->eventContext().pushHandler(m_vpceh.get());
  }
  else{
    m_dockWidget->hide();
    //cw->eventHandler().removeChild(m_vpceh.get());
    cw->eventContext().popHandler();
  }
}

void
PolygonTests_TreeItem::showModel(DrawCntx* cx){

  QPainter p(cx->glWidget());
  auto h = cx->glWidget()->height(); auto w = cx->glWidget()->width();
  // viewport set automatically from logical size of the canvas
  // it may not correspond to the physical size in pixels
  //p.setWindow(0, h, w, -h);
  p.setPen(QPen(Qt::white, 3));

  auto vpc = dynamic_cast<EventHandler_PositionController*>(m_vpceh.get());
  QTransform vpt;
  vpt.translate(vpc->vpc.translationXY[0],vpc->vpc.translationXY[1]);
  vpt.scale(vpc->vpc.scaleX,vpc->vpc.scaleX);
  p.setTransform(vpt, true);

  QTransform t;
  t.translate(w/2,h/2);

  p.setTransform(t, true);
  p.drawPolyline(polygentest->getPolyline());
  p.setPen(QPen(Qt::red, 2/vpc->vpc.scaleX, Qt::DotLine));
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
