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
private:
  ProjectTreeItem* m_activeTreeItem = nullptr;
  CentralWidget* m_gl = nullptr;
};

#endif // PROJECTTREE_H
