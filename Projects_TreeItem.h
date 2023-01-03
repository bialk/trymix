#ifndef PROJECTS_TREEITEM_H
#define PROJECTS_TREEITEM_H

#include "SFSBuilder/editviewobj.h"
#include "qdockwidget.h"
#include <QTreeWidgetItem>
#include <QWidget>

class CentralWidget;
class QDockWidget;
class DrawCntx;

template<typename T>
T* findParentOfType(QObject* p){
  QObject* pw = p->parent();
  if(pw == nullptr)
      return nullptr;
  auto pw_out = dynamic_cast<T*>(pw);
  if(pw_out)
    return pw_out;
  else
    return findParentOfType<T>(pw);
}

class ProjectTreeItem : public QTreeWidgetItem{
public:
  virtual QList<QAction*> contextMenuActions() {return m_actions; };
  virtual void showModel(DrawCntx* gl) {};
  virtual void activateProjectTreeItem(QDockWidget* dock, bool activate = true) {}
  QList<QAction*>& actions(){return m_actions;}
protected:
  QList<QAction*> m_actions;
};

class Projects_TreeItem : public ProjectTreeItem
{  
public:
  Projects_TreeItem();
  void activateProjectTreeItem(QDockWidget* mwin, bool activate = true) override;
private:
};

#endif // PROJECTS_TREEITEM_H
