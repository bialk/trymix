#ifndef SFSBUILDER_TREEITEM_H
#define SFSBUILDER_TREEITEM_H

#include "CommonComponents/Projects_TreeItem.h"
#include "CommonComponents/FileHistory.h"
#include "ui_SFSBuilder_panel.h"

#include <array>
#include <QString>

class QDockWidget;
class QAction;
class QMenu;
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
  static QString name(){ return "SFS Builder Test"; }
  static QString iconPath() { return ":/system/images/trymix.png"; }
  void showModel(DrawCntx* gl) override;
  void activateProjectTreeItem(QDockWidget* dock, bool activate)  override;
private:
  void createActions();
  void bindActions();
  void restoreState();
  void updateConfigFileLabel();
  void openConfigInVSCode();
  void showConfigInFileExplorer();
  void rebuildRecentConfigsMenu();
  void loadConfig();
  bool loadConfigFrom(const QString& filePath, bool showErrors = true);
  void saveConfig();
  void saveConfigAs();
  bool saveConfigTo(const QString& filePath);
  void selectImageSlot(int slot);
  void runModel();

  QScopedPointer<QDockWidget> m_dockWidget;
  Ui::SFSBuilder_panel m_panel;
  FileHistory m_configFileHistory;
  std::array<QAction*, 4> m_slotActions{};
  QAction* m_loadConfigAction = nullptr;
  QAction* m_saveConfigAction = nullptr;
  QAction* m_saveConfigAsAction = nullptr;
  QAction* m_runModelAction = nullptr;
  QAction* m_openConfigInVSCodeAction = nullptr;
  QAction* m_showConfigInExplorerAction = nullptr;
  QMenu* m_recentConfigsMenu = nullptr;
  std::unique_ptr<ImagePlane> m_imagePlane;
  std::unique_ptr<ViewCtrl> m_viewCtrl;
  std::unique_ptr<EventHandler3D> m_viewCtrlEH;
  std::unique_ptr<Lights> m_lights;
  std::unique_ptr<ToolPanel> m_toolsPanel;
};

#endif // SFSBUILDER_TREEITEM_H
