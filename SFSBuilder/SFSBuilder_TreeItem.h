#ifndef SFSBUILDER_TREEITEM_H
#define SFSBUILDER_TREEITEM_H

#include "../Projects_TreeItem.h"
#include "ui_SFSBuilder_panel.h"

class QDockWidget;
class PolygonTest;

class SFSBuilder_TreeItem : public ProjectTreeItem
{
public:
  SFSBuilder_TreeItem();
  void showModel(QOpenGLWidget* gl) override;
  void activateProjectTreeItem(QDockWidget* dock, bool activate)  override;
private:
  QScopedPointer<QDockWidget> m_dockWidget;
  Ui::SFSBuilder_panel m_panel;
};

#endif // SFSBUILDER_TREEITEM_H
