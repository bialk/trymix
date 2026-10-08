#ifndef PROJECTS_TREEITEM_H
#define PROJECTS_TREEITEM_H

#include "drawContext.h"
#include <QDockWidget>
#include <QTreeWidgetItem>
#include <QWidget>

class CentralWidget;
class QDockWidget;
class DrawCntx;
#ifdef TRYMIX_ENABLE_VTK
class vtkRenderer;
#endif

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
  virtual void showModel(DrawCntx* gl) {};
#ifdef TRYMIX_ENABLE_VTK
  virtual void showVtkModel(vtkRenderer*) {}
#endif
  virtual void activateProjectTreeItem(QDockWidget* dock, bool activate = true) {}
  QList<QAction*>& contextMenuActions(){return m_actions;}
  void buildContextMenuStandardItems();
protected:
  QList<QAction*> m_actions;
};

#endif // PROJECTS_TREEITEM_H
