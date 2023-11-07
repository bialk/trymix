#include "ProjectTree.h"
#include "Projects_TreeItem.h"
#include "PolygonTests/PolygonTests_TreeItem.h"
#include "SFSBuilder/SFSBuilder_TreeItem.h"
#include "BlurTests/BlurTests_TreeItem.h"
#include "CentralWidget.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMenu>

ProjectTree::ProjectTree(QWidget *parent)
  :QTreeWidget(parent)
  ,m_gl(new CentralWidget)
{

  m_gl->setProjectTree(this);
  addTopLevelItem(new Projects_TreeItem);

  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &QTreeWidget::customContextMenuRequested,
          this, [=](const QPoint& pos)
  {
    ProjectTreeItem *nd = dynamic_cast<ProjectTreeItem*>(this->itemAt( pos ));
    if(nd){
      auto mainwnd = findParentOfType<MainWindow>(this->parent());      
      if(m_activeTreeItem)
        m_activeTreeItem->activateProjectTreeItem(mainwnd->UI()->dockWidget_params, false);
      m_activeTreeItem = nd;
      nd->activateProjectTreeItem(mainwnd->UI()->dockWidget_params);

      QMenu menu;
      menu.addActions(nd->contextMenuActions());
      menu.exec( this->mapToGlobal(pos) );
    }
  });

  connect(this, &QTreeWidget::itemClicked,
          this, [=](QTreeWidgetItem* item, int col){

    auto mainwnd = findParentOfType<MainWindow>(this->parent());

    if(m_activeTreeItem)
      m_activeTreeItem->activateProjectTreeItem(mainwnd->UI()->dockWidget_params, false);
    m_activeTreeItem=dynamic_cast<ProjectTreeItem*>(item);
    dynamic_cast<ProjectTreeItem*>(item)->activateProjectTreeItem(mainwnd->UI()->dockWidget_params);
    m_gl->update();
  });
}

void
ProjectTree::showModel(DrawCntx* cx)
{
  if(m_activeTreeItem)
    m_activeTreeItem->showModel(cx);
}

CentralWidget*
ProjectTree::gl(){
  return m_gl;
}

std::vector<std::pair<QString,std::function<ProjectTreeItem*()>>>
ProjectTree::TreeItemFactoryList(){
  return {
    {QObject::tr("Polygon Test"), [](){ return new PolygonTests_TreeItem;}},
    {QObject::tr("SFS Builder"),  [](){ return new SFSBuilder_TreeItem;}},
    {QObject::tr("Blur Test"),    [](){ return new BlurTests_TreeItem;}}
  };
}


void ProjectTree::addContextMenuStandardItems(ProjectTreeItem* item){
    for(auto& i: TreeItemFactoryList()){
      auto newAct = new QAction(QIcon(":/Resource/warning32.ico"),
                                QString(QObject::tr("Create new \"%1\" item")).arg(i.first));
      newAct->setStatusTip(QObject::tr("Create new project item"));
      newAct->connect(newAct, &QAction::triggered,
        [=]()
        {
          auto p = item->parent();
          auto newItem = i.second();
          p->insertChild(p->indexOfChild(item)+1, newItem);
          addContextMenuStandardItems(newItem);
        }
      );
      item->actions().append(newAct);
    }

    auto newAct = new QAction(QIcon(":/Resource/warning32.ico"),
                              QString(QObject::tr("Remove \"%1\" item")).arg(item->text(0)));
    newAct->setStatusTip(QObject::tr("Remove \"Polygon Test\" item"));
    newAct->connect(newAct, &QAction::triggered,
      [=]()
      {
        m_activeTreeItem = dynamic_cast<Projects_TreeItem*>(invisibleRootItem()->child(0));
        auto mainwnd = findParentOfType<MainWindow>(this->parent());
        m_activeTreeItem->activateProjectTreeItem(mainwnd->UI()->dockWidget_params);
        delete item;
      }
    );
    item->actions().append(newAct);
}
