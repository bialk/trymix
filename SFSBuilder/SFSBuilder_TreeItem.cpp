#include "SFSBuilder_TreeItem.h"
#include "QMainWindow"

SFSBuilder_TreeItem::SFSBuilder_TreeItem()
{
  setData(0,Qt::DisplayRole,"SFS Builder");
  setData(1,Qt::DisplayRole, "On");
  setData(2,Qt::DisplayRole, "On");

  m_dockWidget.reset(new QDockWidget);
  m_panel.setupUi(m_dockWidget.data());
}

void
SFSBuilder_TreeItem::showModel(QOpenGLWidget* gl){

}

void
SFSBuilder_TreeItem::activateProjectTreeItem(QDockWidget* dock, bool activate){
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
