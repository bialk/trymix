#pragma once
#include "CommonComponents/Projects_TreeItem.h"
#include "ui_MRFSegm_panel.h"

class QDockWidget;
class PolygonTest;
class EventHandler3D;

class MRFSegm_TreeItem : public ProjectTreeItem
{
public:
  MRFSegm_TreeItem();
  ~MRFSegm_TreeItem();
  static QString name(){ return "MRF Segm Test"; }
  static QString iconPath() { return ":/system/images/trymix.png"; }
  void showModel(DrawCntx* cx) override;
  void activateProjectTreeItem(QDockWidget* dock, bool activate = true) override;

private:
  QScopedPointer<QDockWidget> m_dockWidget;
  Ui::MRFSegm_panel m_panel;
  QImage m_img;
  std::unique_ptr<EventHandler3D> m_vpceh;
};
