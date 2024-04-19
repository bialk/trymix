#ifndef CAMERACONTROL_TREEITEM_H
#define CAMERACONTROL_TREEITEM_H

#include "CommonComponents/Projects_TreeItem.h"
#include "ui_CameraControl_panel.h"

class QDockWidget;
class PolygonTest;
class ImagePlane;
class ViewCtrl;
class Lights;
class ToolPanel;
class EventHandler3D;

class CameraControl_TreeItem : public ProjectTreeItem
{
public:
  CameraControl_TreeItem();
  ~CameraControl_TreeItem();
  static QString name(){ return "Camera Control"; }
  static QString iconPath() { return ":/system/images/trymix.png"; }
  void showModel(DrawCntx* gl) override;
  void activateProjectTreeItem(QDockWidget* dock, bool activate)  override;
private:
  QScopedPointer<QDockWidget> m_dockWidget;
  Ui::CameraControl_panel m_panel;
  std::unique_ptr<ViewCtrl> m_viewCtrl;
  std::unique_ptr<EventHandler3D> m_viewCtrlEH;
  std::unique_ptr<Lights> m_lights;
  std::unique_ptr<ToolPanel> m_toolsPanel;
};

#endif // CAMERACONTROL_TREEITEM_H
