#include "Projects_TreeItem.h"
#include "ProjectTree.h"

#include <QAction>

Projects_TreeItem::Projects_TreeItem()
{
  setData(0,Qt::DisplayRole, "Projects");
  setData(1,Qt::DisplayRole, "Enable");
  setData(2,Qt::DisplayRole, "View");

  for(auto& i: ProjectTree::TreeItemFactoryList()){
    auto newAct = new QAction(QIcon(":/Resource/warning32.ico"),
                              QString(QObject::tr("Create new \"%1\" item")).arg(i.first));
    newAct->setStatusTip(QObject::tr("Create new project item"));
    newAct->connect(newAct, &QAction::triggered,
      [=]()
      {
          auto item = i.second();
          auto pt  = dynamic_cast<ProjectTree*>(this->treeWidget());
          pt->addContextMenuStandardItems(item);
          insertChild(0, item);
          setExpanded(true);
      }
    );
    m_actions.append(newAct);
  }

  auto newAct = new QAction(QIcon(":/Resource/warning32.ico"), QObject::tr("Clear Tree"));
  newAct->setStatusTip(QObject::tr("Clear project tree"));
  newAct->connect(newAct, &QAction::triggered,
    [=]()
    {
      while(childCount())
          delete this->child(0);
    }
  );
  m_actions.append(newAct);
}

void
Projects_TreeItem::activateProjectTreeItem(QDockWidget* dock, bool activate){
  if(activate)
    dock->show();
  else
    dock->hide();
}
