#include "Projects_TreeItem.h"
#include "PolygonTests/PolygonTests_TreeItem.h"
#include "ProjectTree.h"

#include <QAction>

Projects_TreeItem::Projects_TreeItem()
{
  setData(0,Qt::DisplayRole, "Projects");
  setData(1,Qt::DisplayRole, "Enable");
  setData(2,Qt::DisplayRole, "View");

  auto newAct = new QAction(QIcon(":/Resource/warning32.ico"), QObject::tr("Polygon Test"));
  newAct->setStatusTip(QObject::tr("Create new project item"));
  newAct->connect(newAct, &QAction::triggered,
    [=]()
    {

        auto item = new PolygonTests_TreeItem;
        auto pt  = dynamic_cast<ProjectTree*>(this->treeWidget());
        pt->addContextMenuStandardItems(item);
        insertChild(0, item);
        setExpanded(true);
    }
  );
  m_actions.append(newAct);
  newAct = new QAction(QIcon(":/Resource/warning32.ico"), QObject::tr("Clear Tree"));
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
