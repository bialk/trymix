#include "Projects_TreeItem.h"
#include "ProjectTree.h"

#include <QAction>


void ProjectTreeItem::buildContextMenuStandardItems(){
    for(auto& i: ProjectTree::TreeItemFactoryList()){
      auto newAct = new QAction(QIcon(i->iconPath()),
                                QString(QObject::tr("Create \"%1\"")).arg(i->name()));
      newAct->setStatusTip(QObject::tr("Create new project item"));
      newAct->connect(newAct, &QAction::triggered, newAct,
        [=]()
        {
          auto p = parent();
          if(!p)
            p = dynamic_cast<ProjectTree*>(treeWidget())->invisibleRootItem();
          auto newItem = i->create();
          newItem->setIcon(0,QIcon(i->iconPath()));
          p->insertChild(p->indexOfChild(this)+1, newItem);
          newItem->buildContextMenuStandardItems();
        }
      );
      m_actions.append(newAct);
    }

    auto newAct = new QAction(QIcon(QPixmap(":/system/images/trymix.ico")),
                              QString(QObject::tr("Remove \"%1\" item")).arg(text(0)));
    newAct->setStatusTip(QObject::tr("Remove \"%1\" item").arg(text(0)));
    newAct->connect(newAct, &QAction::triggered, newAct,
      [=]()
      {
        dynamic_cast<ProjectTree*>(treeWidget())->removeItem(this);
      }
    );
    m_actions.append(newAct);
}

