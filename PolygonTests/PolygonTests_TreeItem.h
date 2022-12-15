#ifndef POLYGONTESTS_TREEITEM_H
#define POLYGONTESTS_TREEITEM_H

#include "../Projects_TreeItem.h"
#include "ui_PolygonTests_panel.h"

class QDockWidget;
class PolygonTest;

class PolygonTests_TreeItem : public ProjectTreeItem
{
public:
  PolygonTests_TreeItem();
  ~PolygonTests_TreeItem();
  void showModel(QOpenGLWidget* gl) override;
  void activateProjectTreeItem(QDockWidget* dock, bool activate = true) override;

  void tryConvexPartitioning(int polygonSize);
  void tryMonotonePartitioning(int polygonSize);
  void tryConformingDelanay(int polygonSize,  float meshCellSize);

private:
  QScopedPointer<QDockWidget> m_dockWidget;
  Ui::PolygonTests_panel m_panel;
  std::unique_ptr<PolygonTest> polygentest;
};

#endif // POLYGONTESTS_TREEITEM_H
