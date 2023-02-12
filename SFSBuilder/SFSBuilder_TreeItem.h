#ifndef SFSBUILDER_TREEITEM_H
#define SFSBUILDER_TREEITEM_H

#include "CommonComponents/Projects_TreeItem.h"
#include "ui_SFSBuilder_panel.h"

class QDockWidget;
class PolygonTest;
class ImagePlane;
class ViewCtrl;
class Lights;
class ToolPanel;
class EventHandler3D;

class SFSBuilder_TreeItem : public ProjectTreeItem
{
public:
  SFSBuilder_TreeItem();
  ~SFSBuilder_TreeItem();
  void showModel(DrawCntx* gl) override;
  void activateProjectTreeItem(QDockWidget* dock, bool activate)  override;
private:
  QScopedPointer<QDockWidget> m_dockWidget;
  Ui::SFSBuilder_panel m_panel;
  std::unique_ptr<ImagePlane> m_sfs;
  std::unique_ptr<ViewCtrl> m_viewCtrl;
  std::unique_ptr<EventHandler3D> m_viewCtrlEH;
  std::unique_ptr<Lights> m_lights;
  std::unique_ptr<ToolPanel> m_toolsPanel;
};

#endif // SFSBUILDER_TREEITEM_H
