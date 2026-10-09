#ifndef PROJECTS_TREEITEM_H
#define PROJECTS_TREEITEM_H

#include "drawContext.h"
#include "ViewportContext.h"
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
  enum class Viewport { Uninitialized, Canvas, Vtk };

  virtual void showModel(DrawCntx* context) {
    context->markNotImplemented();
  }
#ifdef TRYMIX_ENABLE_VTK
  virtual void showVtkModel(VtkDrawCntx* context) {
    context->markNotImplemented();
  }
#endif
  virtual void activateProjectTreeItem(QDockWidget* dock, bool activate = true) {}
  QList<QAction*>& contextMenuActions(){return m_actions;}
  void buildContextMenuStandardItems();
  void recordCanvasImplementation(bool implemented) {
    m_canvasImplemented = implemented;
    m_canvasImplementationKnown = true;
  }
  void recordVtkImplementation(bool implemented) {
    m_vtkImplemented = implemented;
    m_vtkImplementationKnown = true;
  }
  bool viewportImplementationsKnown() const {
    return m_canvasImplementationKnown && m_vtkImplementationKnown;
  }
  bool canvasImplemented() const { return m_canvasImplemented; }
  bool vtkImplemented() const { return m_vtkImplemented; }
  Viewport preferredViewport() const { return m_preferredViewport; }
  void setPreferredViewport(Viewport viewport) { m_preferredViewport = viewport; }
  Viewport initialViewport() const { return m_initialViewport; }
  void setInitialViewport(Viewport viewport) { m_initialViewport = viewport; }
protected:
  QList<QAction*> m_actions;
private:
  bool m_canvasImplementationKnown = false;
  bool m_vtkImplementationKnown = false;
  bool m_canvasImplemented = false;
  bool m_vtkImplemented = false;
  Viewport m_preferredViewport = Viewport::Uninitialized;
  Viewport m_initialViewport = Viewport::Uninitialized;
};

#endif // PROJECTS_TREEITEM_H
