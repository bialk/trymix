#include "BlurTests_TreeItem.h"
#include "BlurTests.h"
#include "CommonComponents/CentralWidget.h"
#include "CommonComponents/mainwindow.h"
#include "CommonComponents/EventHandling.h"

#include <QAction>
#include <QOpenGLWidget>
#include <QPainter>
#include <QMainWindow>
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>
#include <QDateTime>


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
      // Setting center of rotation shifted to the half of the viewport area. This is due
      // to additional transformation we apply before (see paint method). We set coordinate
      // system being centered to the viewport thus we apply reverse transformations
      vpc.beginScale(cx.x() - m_offsetx, cx.y()  - m_offsety ); // inverted y axis
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

  inline void setOffset(int x, int y){
    m_offsetx = x; m_offsety = y;
  }

private:
  int m_startXY[2] = {0,0};
  int m_offsetx = 0, m_offsety = 0;
  EventHandler3D m_mouseDragScale;
  EventHandler3D m_mouseDragTranslate;
};

}

BlurTests_TreeItem::BlurTests_TreeItem()
  :m_img(1024,1024,QImage::Format_RGBA32FPx4)
  ,m_vpceh(new EventHandler_PositionController)
{  
  m_img.fill(Qt::magenta);
  setData(0,Qt::DisplayRole,"Blur Test");
  setData(1,Qt::DisplayRole, "On");
  setData(2,Qt::DisplayRole, "On");

  m_dockWidget.reset(new QDockWidget);
  m_panel.setupUi(m_dockWidget.data());

  blurTest(m_panel.spinBox_width->value(), m_panel.spinBox_height->value(),
           m_panel.spinBox_radius->value(), m_panel.spinBox_chessBox->value(), m_img, false);


  QObject::connect(m_panel.pushButton_runBlurCPU, &QPushButton::clicked,
    [&]{
       auto startTime = QDateTime::currentDateTime();
       QApplication::setOverrideCursor(Qt::WaitCursor);
       blurTest(m_panel.spinBox_width->value(), m_panel.spinBox_height->value(),
                m_panel.spinBox_radius->value(), m_panel.spinBox_chessBox->value(), m_img, false);
       findParentOfType<MainWindow>(treeWidget())->centralWidget()->update();
       QApplication::restoreOverrideCursor();
       m_panel.label_timeIndicator->setText(QString("Processing time: %1 ms")
                                            .arg(startTime.msecsTo(QDateTime::currentDateTime())));
    });
  QObject::connect(m_panel.pushButton_runBlurOpenCL, &QPushButton::clicked,
    [&]{
       auto startTime = QDateTime::currentDateTime();
       QApplication::setOverrideCursor(Qt::WaitCursor);
       blurTest(m_panel.spinBox_width->value(), m_panel.spinBox_height->value(),
                m_panel.spinBox_radius->value(), m_panel.spinBox_chessBox->value(), m_img, true);
       findParentOfType<MainWindow>(treeWidget())->centralWidget()->update();
       QApplication::restoreOverrideCursor();
       m_panel.label_timeIndicator->setText(QString("Processing time: %1 ms")
                                            .arg(startTime.msecsTo(QDateTime::currentDateTime())));
    });

   auto& vpc = static_cast<EventHandler_PositionController*>(m_vpceh.get())->vpc;
  // initial postioning
  vpc.scaleX = 0.5;
  // vpc.translationXY[0] = 100;
  // vpc.translationXY[1] = 100;
}

BlurTests_TreeItem::~BlurTests_TreeItem(){
}

void
BlurTests_TreeItem::activateProjectTreeItem(QDockWidget* dock, bool activate){
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
BlurTests_TreeItem::showModel(DrawCntx* cx){

  QPainter p(cx->glWidget());

  // Note that, viewport set automatically from logical size of the canvas
  // it may not correspond to the physical size of the canvas in pixels
  auto vpc = dynamic_cast<EventHandler_PositionController*>(m_vpceh.get());

  auto offsetx = cx->glWidget()->width()*0.5f;
  auto offsety = cx->glWidget()->height()*0.5f;

  QTransform vpt;
  // this is extra constant transformation shifting origin of the viewport
  // transformation to the centre of the window
  vpt.translate(offsetx,offsety);
  // remember offset as we centered viewport coordinate system
  // this is needed to do scaling control around mouse cursor
  vpc->setOffset(offsetx, offsety);

  // applying image translate/scale (view point) transformation
  vpt.translate(vpc->vpc.translationXY[0], vpc->vpc.translationXY[1]);
  vpt.scale(vpc->vpc.scaleX, vpc->vpc.scaleX);

  // centering image coordinate space - the origin in the middle of the image
  vpt.translate(-m_img.width()*0.5, -m_img.height()*0.5);
  p.setTransform(vpt, true);

  p.drawImage(0, 0, m_img);
}
