#include "PolygonTests_TreeItem.h"
#include "PolygonTests/PolygonTestConvexPartitioning.h"
#include "PolygonTests/PolygonTestMonotonePartitioning.h"
#include "PolygonTests/PolygonTestConformingDelanay.h"
#include "CentralWidget.h"
#include "mainwindow.h"

#include <QAction>
#include <QOpenGLWidget>
#include <QPainter>
#include <QMainWindow>
#include <QPushButton>

PolygonTests_TreeItem::PolygonTests_TreeItem()
  :polygentest(new PolygonTestConvexPartitioning(50))
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
  m_dockWidget->deleteLater();
}

void
PolygonTests_TreeItem::activateProjectTreeItem(QDockWidget* dock, bool activate){
  if(activate){
    auto mainwin = findParentOfType<QMainWindow>(dock);
    if(!m_dockWidget->isActiveWindow()){
      mainwin->tabifyDockWidget(dock,m_dockWidget.get());
    }
    m_dockWidget->activateWindow();
    m_dockWidget->show();
    m_dockWidget->raise();
  }
  else{
    m_dockWidget->hide();
  }
}

void
PolygonTests_TreeItem::showModel(QOpenGLWidget* gl){
  QPainter p(gl);
  auto h = gl->height(); auto w = gl->width();
  p.setWindow(0, h, w, -h);
  p.setPen(QPen(Qt::white, 3));
  QTransform t;
  t.translate(w/2,h/2);
  float scale = 0.45/600;
  scale = fmin(w*scale, h*scale);
  t.scale(scale, scale);
  p.setTransform(t);
  p.drawPolyline(polygentest->getPolyline());
  p.setPen(QPen(Qt::red, 2, Qt::DotLine));
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
