#ifndef PROJECTTREE_H
#define PROJECTTREE_H

#include <QTreeWidget>

class ProjectTreeItem;
class CentralWidget;

class ProjectTree : public QTreeWidget
{
public:
  ProjectTree(QWidget* parent);
  void showModel(QOpenGLWidget* gl);
  CentralWidget* gl();
  void addContextMenuStandardItems(ProjectTreeItem* item);
  static std::vector<std::pair<QString,std::function<ProjectTreeItem*()>>>
    TreeItemFactoryList();
private:
  ProjectTreeItem* m_activeTreeItem = nullptr;
  CentralWidget* m_gl = nullptr;
};

#endif // PROJECTTREE_H
