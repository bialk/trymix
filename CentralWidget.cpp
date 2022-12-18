#include "CentralWidget.h"
#include "ui_CentralWidget.h"
#include "Projects_TreeItem.h"
#include "ProjectTree.h"
//#include "PolygonTests/PolygonTestConvexPartitioning.h"
//#include "PolygonTests/PolygonTestMonotonePartitioning.h"
//#include "PolygonTests/PolygonTestConformingDelanay.h"

//#include <windows.h>
//#include <GL/GL.h>
//#include <QOpenGLFunctions>
#include <QPainter>

//void CentralWidget::tryConvexPartitioning(int polygonSize){
//    polygentest.reset(new PolygonTestConvexPartitioning(polygonSize));
//    update();
//}

//void CentralWidget::tryMonotonePartitioning(int polygonSize){
//    polygentest.reset(new PolygonTestMonotonePartitioning(polygonSize));
//    update();
//}

//void CentralWidget::tryConformingDelanay(int polygonSize, float meshCellSize){
//    polygentest.reset(new PolygonTestConformingDelanay(polygonSize, meshCellSize));
//    update();
//}

//void CentralWidget::setActiveTreeItem(ProjectTreeItem *ti){
//  // m_activeTreeItem = ti;
//}


CentralWidget::CentralWidget(QWidget *parent) :
    QOpenGLWidget(parent),
//    polygentest(new PolygonTestConvexPartitioning(50)),
    ui(new Ui::CentralWidget)
{
    ui->setupUi(this);        
}

void
CentralWidget::initializeGL(){

}

void
CentralWidget::resizeGL(int w, int h){}

void
CentralWidget::paintGL(){

//  if(m_activeTreeItem)
//    m_activeTreeItem->showModel(this);

  m_projectTree->showModel(this);
#ifdef off
    QPainter p(this);
    p.setWindow(0, height(), width(), -height());
    p.setPen(QPen(Qt::white, 3));
    QTransform t;
    t.translate(width()/2,height()/2);
    float scale = 0.45/600;
    scale = min(width()*scale, height()*scale);
    t.scale(scale, scale);
    p.setTransform(t);
    p.drawPolyline(polygentest->getPolyline());
    p.setPen(QPen(Qt::red, 2, Qt::DotLine));
    p.drawLines(polygentest->getEdges());
#endif
}

void
CentralWidget::setProjectTree(ProjectTree *pt){
  m_projectTree = pt;
}

CentralWidget::~CentralWidget()
{
    delete ui;
}
