#ifndef BLURTESTS_TREEITEM_H
#define BLURTESTS_TREEITEM_H

#include "CommonComponents/Projects_TreeItem.h"
#include "ui_BlurTests_panel.h"

class QDockWidget;
class PolygonTest;
class EventHandler3D;

class BlurTests_TreeItem : public ProjectTreeItem
{
public:
  BlurTests_TreeItem();
  ~BlurTests_TreeItem();
  void showModel(DrawCntx* cx) override;
  void activateProjectTreeItem(QDockWidget* dock, bool activate = true) override;

//  void tryConvexPartitioning(int polygonSize);
//  void tryMonotonePartitioning(int polygonSize);
//  void tryConformingDelanay(int polygonSize,  float meshCellSize);

private:
  QScopedPointer<QDockWidget> m_dockWidget;
  Ui::BlurTests_panel m_panel;
  //std::unique_ptr<PolygonTest> polygentest;
  QImage m_img;
  std::unique_ptr<EventHandler3D> m_vpceh;
};

#endif // BLURTESTS_TREEITEM_H
