#ifndef PROJECTTREE_H
#define PROJECTTREE_H

#include "drawContext.h"
#include <QTreeWidget>

class ProjectTreeItem;
class CentralWidget;
class DrawCntx;

class ProjectTree : public QTreeWidget
{
public:
  ProjectTree(QWidget* parent);
  virtual void showModel(DrawCntx* cx);
  CentralWidget* gl();
  void addContextMenuStandardItems(ProjectTreeItem* item);
  static std::vector<std::pair<QString,std::function<ProjectTreeItem*()>>>
    TreeItemFactoryList();
private:
  ProjectTreeItem* m_activeTreeItem = nullptr;
  CentralWidget* m_gl = nullptr;
};

#endif // PROJECTTREE_H
