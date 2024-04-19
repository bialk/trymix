#ifndef POLYGONTESTS_TREEITEM_H
#define POLYGONTESTS_TREEITEM_H

#include "CommonComponents/Projects_TreeItem.h"
#include "ui_PolygonTests_panel.h"

class QDockWidget;
class PolygonTest;
class EventHandler3D;

class PolygonTests_TreeItem : public ProjectTreeItem
{
public:
  PolygonTests_TreeItem();
  ~PolygonTests_TreeItem();
  static QString name(){ return "Polygon Test"; }
  static QString iconPath() { return ":/system/images/trymix.png"; }
  void showModel(DrawCntx* cx) override;
  void activateProjectTreeItem(QDockWidget* dock, bool activate = true) override;

  void tryConvexPartitioning(int polygonSize);
  void tryMonotonePartitioning(int polygonSize);
  void tryConformingDelanay(int polygonSize,  float meshCellSize);

private:
  QScopedPointer<QDockWidget> m_dockWidget;
  Ui::PolygonTests_panel m_panel;
  std::unique_ptr<PolygonTest> polygentest;
  std::unique_ptr<EventHandler3D> m_vpceh;
};

#endif // POLYGONTESTS_TREEITEM_H
