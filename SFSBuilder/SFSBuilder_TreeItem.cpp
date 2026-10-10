#include "SFSBuilder_TreeItem.h"
#include "QMainWindow"
#include "imageplane.h"
#include "CommonComponents/lights.h"
#include "CommonComponents/toolspanel.h"
#include "CommonComponents/viewctrl.h"
#include "CommonComponents/EventHandling.h"
#include "CommonComponents/CentralWidget.h"
#include "CommonComponents/FileLauncher.h"
#include "CommonComponents/PaletteWidgets.h"
#include "CommonComponents/QScintillaEditor.h"
#include "CommonComponents/ReleaseSelectingMenu.h"
#include "CommonComponents/mainwindow.h"
#include "CommonComponents/ModernOpenGLRenderer.h"
#include "CommonComponents/MatrixConversions.h"
#include "CommonComponents/ScreenSpacePicking.h"

#include <QOpenGLWidget>
#include <QPainter>
#include <QDebug>
#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSaveFile>
#include <QSettings>
#include <QStandardItemModel>
#include <QToolButton>
#include <QWidgetAction>
#include <QMatrix4x4>
#include <QVector4D>

#include <yaml-cpp/yaml.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <stdexcept>


namespace {

constexpr auto settingsGroup = "SFSBuilder";
constexpr auto solverSettingsKey = "solver";

class EventHandler_PositionController: public EventHandler3D{

public:
  ViewCtrl* m_vp = nullptr;
  Lights* m_lights = nullptr;
  ImagePlane* m_imagePlane = nullptr;
  ToolPanel* m_tools = nullptr;
  

  ViewCtrl::Opercode Op{ViewCtrl::Opercode::origRotate};

  EventHandler_PositionController(
      ViewCtrl* vc, Lights* lights, ImagePlane* imagePlane, ToolPanel* tools)
    :m_vp(vc)
    ,m_lights(lights)
    ,m_imagePlane(imagePlane)
    ,m_tools(tools)
  {
    addReact("M:L:DOWN") = [this](EventContext3D& cx)
    {
      if(m_imagePlane->edit_mode == ImagePlane::edit_mode_on
              && selectImageLightAt(cx.x(), cx.y(), cx.w(), cx.h())) {
        m_imageLightDragHandler = std::make_unique<EventHandler3D>();
        m_imageLightDragHandler->addReact("M:MOVE") = [this](EventContext3D& cx) {
          moveSelectedImageLight(cx.glx(), cx.gly());
          cx.update();
        };
        m_imageLightDragHandler->addReact("M:L:UP") = [this](EventContext3D& cx) {
          m_imagePlane->selectedLightIndex = -1;
          cx.popHandler();
          cx.update();
        };
        cx.pushHandler(m_imageLightDragHandler.get());
      }
      else if(m_lights->focus(m_tools->pick(cx.x(), cx.y()))){
        m_lights->lightrstart(cx.x(),cx.y());
        cx.pushHandler(&m_mouseDragLight);
      }
      else{
        auto dragProcessor = m_vp->startOperation(Op,cx.glx(),cx.gly());
        m_dragHandler = std::make_unique<EventHandler3D>();
        m_dragHandler->addReact("M:MOVE") = [dragProcessor](EventContext3D& cx){
          dragProcessor(cx.glx(),cx.gly());
          cx.update();
        };
        m_dragHandler->addReact("M:L:UP") =  [](EventContext3D& cx){
          cx.popHandler();
          cx.update();
        };

        addReact("S:RESIZE") = [this](EventContext3D& cx)
        {
          m_vp->updateProjectionMtrx(cx.w(),cx.h());
          cx.update();
        };

        cx.pushHandler(m_dragHandler.get());
      }
      cx.update();
    };

    m_mouseDragLight.addReact("M:MOVE") = [this](EventContext3D& cx){
      m_lights->lightrcont(cx.x(),cx.y());
      cx.update();
    };

    m_mouseDragLight.addReact("M:L:UP") =  [](EventContext3D& cx){
      cx.update();
      cx.popHandler();
    };

    addReact("K:C:DOWN") = [this](EventContext3D& cx){  Op = ViewCtrl::Opercode::Scale; };
    addReact("K:X:DOWN") = [this](EventContext3D& cx){  Op = ViewCtrl::Opercode::origRotate; };
    addReact("K:Z:DOWN") = [this](EventContext3D& cx){  Op = ViewCtrl::Opercode::CamRotate; };
    addReact("K:V:DOWN") = [this](EventContext3D& cx){  Op = ViewCtrl::Opercode::FoV; };

    addReact("S:RESIZE") = [this](EventContext3D& cx)
    {
      m_vp->updateProjectionMtrx(cx.w(),cx.h());
      cx.update();
    };
  }
  std::unique_ptr<EventHandler3D> m_dragHandler;
  std::unique_ptr<EventHandler3D> m_imageLightDragHandler;
  EventHandler3D m_mouseDragLight;

private:
  bool selectImageLightAt(int mouseX, int mouseY, int viewportWidth,
                          int viewportHeight)
  {
    if(viewportWidth <= 0 || viewportHeight <= 0)
      return false;

    const QMatrix4x4 projection = toQMatrix4x4(m_vp->getProjectionMtrx());
    const QMatrix4x4 view = toQMatrix4x4(m_vp->getModelViewMtrx());
    const QMatrix4x4 modelViewProjection = projection * view;

    constexpr float pickRadius = 12.0F;
    std::vector<QPointF> screenPoints;
    screenPoints.reserve(4);
    for(int index = 0; index < 4; ++index) {
      const QVector4D clip = modelViewProjection * QVector4D(
        (m_imagePlane->w + m_imagePlane->lights[index][0]
                            * m_imagePlane->h) * 0.5F,
        (m_imagePlane->h + m_imagePlane->lights[index][1]
                            * m_imagePlane->h) * 0.5F,
        0.0F, 1.0F);
      if(clip.w() <= 0.0F) {
        screenPoints.emplace_back(-1000000.0, -1000000.0);
        continue;
      }

      const float screenX = (clip.x() / clip.w() + 1.0F)
                            * viewportWidth * 0.5F;
      const float screenY = (1.0F - clip.y() / clip.w())
                            * viewportHeight * 0.5F;
      screenPoints.emplace_back(screenX, screenY);
    }
    const int closestIndex = pickClosestScreenPoint(
      QPointF(static_cast<qreal>(mouseX), static_cast<qreal>(mouseY)),
      screenPoints, pickRadius);

    if(closestIndex < 0)
      return false;
    m_imagePlane->selectedLightIndex = closestIndex;
    return true;
  }

  void moveSelectedImageLight(float normalizedX, float normalizedY)
  {
    const int selected = m_imagePlane->selectedLightIndex;
    if(selected < 0 || m_imagePlane->h <= 0)
      return;

    const QMatrix4x4 projection = toQMatrix4x4(m_vp->getProjectionMtrx());
    const QMatrix4x4 view = toQMatrix4x4(m_vp->getModelViewMtrx());
    bool invertible = false;
    const QMatrix4x4 inverse = (projection * view).inverted(&invertible);
    if(!invertible)
      return;

    QVector4D nearPoint = inverse * QVector4D(normalizedX, normalizedY, -1.0F, 1.0F);
    QVector4D farPoint = inverse * QVector4D(normalizedX, normalizedY, 1.0F, 1.0F);
    nearPoint /= nearPoint.w();
    farPoint /= farPoint.w();
    const float depthDelta = farPoint.z() - nearPoint.z();
    if(qFuzzyIsNull(depthDelta))
      return;
    const float interpolation = -nearPoint.z() / depthDelta;
    const QVector4D planePoint = nearPoint + (farPoint - nearPoint) * interpolation;

    float lightX = (2.0F * planePoint.x() - m_imagePlane->w)
                   / m_imagePlane->h;
    float lightY = (2.0F * planePoint.y() - m_imagePlane->h)
                   / m_imagePlane->h;
    const float radiusSquared = lightX * lightX + lightY * lightY;
    float lightZ = 0.0F;
    if(radiusSquared < 1.0F) {
      lightZ = std::sqrt(1.0F - radiusSquared);
    } else {
      const float length = std::sqrt(radiusSquared);
      lightX /= length;
      lightY /= length;
    }
    m_imagePlane->lights[selected][0] = lightX;
    m_imagePlane->lights[selected][1] = lightY;
    m_imagePlane->lights[selected][2] = lightZ;
  }
};

}



SFSBuilder_TreeItem::SFSBuilder_TreeItem()
  :m_configFileHistory(
     QStringLiteral("SFSBuilder"), 10,
     QStringLiteral("lastConfigFile"), QStringLiteral("recentConfigFiles"))
  ,m_imagePlane(new ImagePlane)
  ,m_viewCtrl(new ViewCtrl)
  ,m_lights(new Lights)
  ,m_toolsPanel(new ToolPanel)
{
  m_viewCtrl->TreeScan(&TSOCntx::TSO_Init);
  m_imagePlane->TreeScan(&TSOCntx::TSO_Init);
  m_toolsPanel->Add(&m_lights->glic1);
  m_toolsPanel->Add(&m_lights->glic2);
  m_lights->TreeScan(&TSOCntx::TSO_Init);

  //m_toolsPanel->TreeScan(&TSOCntx::TSO_Init);
  m_toolsPanel->m_viewctrl = m_viewCtrl.get();

  m_viewCtrl->TreeScan(&TSOCntx::TSO_LayoutLoad);
  m_viewCtrlEH.reset(new EventHandler_PositionController(
    m_viewCtrl.get(), m_lights.get(), m_imagePlane.get(), m_toolsPanel.get()));


  setData(0,Qt::DisplayRole, name());
  setData(1,Qt::DisplayRole, "");

  buildContextMenuStandardItems();

  m_dockWidget.reset(new QDockWidget);
  m_panel.setupUi(m_dockWidget.data());
  m_panel.radioButton_image_off->setChecked(true);
  m_imagePlane->SetShapeMode(ImagePlane::shape_mode_off);
  applyPalettePopupStyle(m_panel.comboBox_solver);
  m_panel.comboBox_solver->setItemData(
    0, static_cast<int>(LinSolverKind::MT));
  m_panel.comboBox_solver->setItemData(
    1, static_cast<int>(LinSolverKind::Eigen));
  m_panel.comboBox_solver->setItemData(
    2, static_cast<int>(LinSolverKind::MPI));

#if defined(SOLVER_MPI)
  m_panel.comboBox_solver->setCurrentIndex(2);
#else
  m_panel.comboBox_solver->setCurrentIndex(0);
#endif
#if !defined(SOLVER_MPI)
  if (auto* model = qobject_cast<QStandardItemModel*>(
        m_panel.comboBox_solver->model())) {
    model->item(2)->setEnabled(false);
    model->item(2)->setToolTip(
      QObject::tr("Configure with TRYMIX_SFS_USE_MPI=ON to enable MPI"));
  }
#endif

  QScintillaEditor::configureYaml(m_panel.configEditor);
  m_panel.configEditor->setText(
R"(
SFSModelConfig:
  workdir: C:/Users/Alex/D/zero-devel/research/pmetrics/data/
  images:
    - bln1x2.ppm: [ -0.5, 0.7, 2.]
    - bln2x2.ppm: [  0.5, 0.7, 2.]
    - bln3x2.ppm: [  0.5, -0.7, 2.]
    - bln4x2.ppm: [ -0.5, -0.7, 2.]
)"
  );
  m_panel.configEditor->recolor();
  m_panel.configEditor->setCursorPosition(0, 0);
  restoreState();
  updateConfigFileLabel();

  createActions();
  bindActions();
}

void SFSBuilder_TreeItem::createActions()
{
  auto* slotActionGroup = new QActionGroup(m_dockWidget.data());
  slotActionGroup->setExclusive(true);
  m_noImageSlotAction = new QAction(
    QObject::tr("No Image"), m_dockWidget.data());
  m_noImageSlotAction->setCheckable(true);
  m_noImageSlotAction->setData(-1);
  slotActionGroup->addAction(m_noImageSlotAction);
  for (int slot = 0; slot < static_cast<int>(m_slotActions.size()); ++slot) {
    auto* action = new QAction(
      QObject::tr("Image Slot %1").arg(slot + 1), m_dockWidget.data());
    action->setCheckable(true);
    action->setData(slot);
    slotActionGroup->addAction(action);
    m_slotActions[slot] = action;
  }
  m_noImageSlotAction->setChecked(true);
  QObject::connect(
    slotActionGroup, &QActionGroup::triggered, m_dockWidget.data(),
    [this](QAction* action) { selectImageSlot(action->data().toInt()); });

  m_loadConfigAction = new QAction(
    QObject::tr("Load Configuration"), m_dockWidget.data());
  m_saveConfigAction = new QAction(
    QObject::tr("Save Configuration"), m_dockWidget.data());
  m_saveConfigAsAction = new QAction(
    QObject::tr("Save Configuration As"), m_dockWidget.data());
  m_syncModelAction = new QAction(
    QObject::tr("Apply YAML Configuration"), m_dockWidget.data());
  m_runModelAction = new QAction(QObject::tr("Run Model"), m_dockWidget.data());
  m_openConfigInVSCodeAction = new QAction(
    QObject::tr("Open Configuration in VS Code"), m_dockWidget.data());
  m_showConfigInExplorerAction = new QAction(
    QObject::tr("Show Configuration in File Explorer"), m_dockWidget.data());
  m_saveConfigAction->setShortcut(QKeySequence::Save);
  m_saveConfigAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  m_runModelAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+R")));
  m_recentConfigsMenu = new ReleaseSelectingMenu(m_dockWidget.data());

  QObject::connect(
    m_loadConfigAction, &QAction::triggered, m_dockWidget.data(),
    [this]() { loadConfig(); });
  QObject::connect(
    m_saveConfigAction, &QAction::triggered, m_dockWidget.data(),
    [this]() { saveConfig(); });
  QObject::connect(
    m_saveConfigAsAction, &QAction::triggered, m_dockWidget.data(),
    [this]() { saveConfigAs(); });
  QObject::connect(
    m_syncModelAction, &QAction::triggered, m_dockWidget.data(),
    [this]() { syncModelConfiguration(); });
  QObject::connect(
    m_runModelAction, &QAction::triggered, m_dockWidget.data(),
    [this]() { runModel(); });
  QObject::connect(
    m_openConfigInVSCodeAction, &QAction::triggered, m_dockWidget.data(),
    [this]() { openConfigInVSCode(); });
  QObject::connect(
    m_showConfigInExplorerAction, &QAction::triggered, m_dockWidget.data(),
    [this]() { showConfigInFileExplorer(); });
  QObject::connect(
    m_recentConfigsMenu, &QMenu::aboutToShow, m_dockWidget.data(),
    [this]() { rebuildRecentConfigsMenu(); });
}

void SFSBuilder_TreeItem::bindActions()
{
  const auto bindButton = [](QToolButton* button, QAction* action) {
    button->setEnabled(action->isEnabled());
    QObject::connect(button, &QToolButton::clicked, action, &QAction::trigger);
    QObject::connect(action, &QAction::changed, button, [button, action]() {
      button->setEnabled(action->isEnabled());
    });
  };

  const std::array slotButtons{
    m_panel.toolButton_slot1,
    m_panel.toolButton_slot2,
    m_panel.toolButton_slot3,
    m_panel.toolButton_slot4
  };
  for (int slot = 0; slot < static_cast<int>(slotButtons.size()); ++slot) {
    slotButtons[slot]->setCheckable(true);
    bindButton(slotButtons[slot], m_slotActions[slot]);
  }
  m_panel.toolButton_slotNone->setCheckable(true);
  bindButton(m_panel.toolButton_slotNone, m_noImageSlotAction);
  selectImageSlot(m_imagePlane->curslot);

  const auto updateSurfaceMode = [this](bool checked) {
    if(!checked)
      return;
    updateDisplayModes();
    if(auto* mainWindow = findParentOfType<MainWindow>(m_dockWidget.data()))
      mainWindow->refreshViews();
  };
  QObject::connect(
    m_panel.radioButton_image_off, &QRadioButton::toggled,
    m_dockWidget.data(), updateSurfaceMode);
  QObject::connect(
    m_panel.radioButton_image_albedo, &QRadioButton::toggled,
    m_dockWidget.data(), updateSurfaceMode);
  QObject::connect(
    m_panel.radioButton_image_image, &QRadioButton::toggled,
    m_dockWidget.data(), updateSurfaceMode);

  bindButton(m_panel.toolButton_ConfigLoad, m_loadConfigAction);
  m_panel.toolButton_ConfigLoad->setMenu(m_recentConfigsMenu);
  m_panel.toolButton_ConfigLoad->setPopupMode(QToolButton::DelayedPopup);
  bindButton(m_panel.toolButton_ConfigSave, m_saveConfigAction);
  bindButton(m_panel.toolButton_2, m_saveConfigAsAction);
  bindButton(m_panel.toolButton_SyncModel, m_syncModelAction);
  bindButton(m_panel.toolButton_RunModel, m_runModelAction);
  bindButton(
    m_panel.toolButton_OpenConfigInVSCode, m_openConfigInVSCodeAction);
  bindButton(
    m_panel.toolButton_ShowConfigInExplorer, m_showConfigInExplorerAction);
  QObject::connect(
    m_panel.comboBox_solver, &QComboBox::currentIndexChanged,
    m_dockWidget.data(), [this](int) {
      QSettings settings;
      settings.beginGroup(QString::fromLatin1(settingsGroup));
      settings.setValue(
        QString::fromLatin1(solverSettingsKey),
        m_panel.comboBox_solver->currentData().toInt());
    });
  QObject::connect(
    m_panel.checkBox_editLights, &QCheckBox::toggled,
    m_dockWidget.data(), [this](bool enabled) {
      m_imagePlane->edit_mode = enabled
        ? ImagePlane::edit_mode_on : ImagePlane::edit_mode_off;
      if(!enabled)
        m_imagePlane->selectedLightIndex = -1;
      if(auto* mainWindow = findParentOfType<MainWindow>(m_dockWidget.data()))
        mainWindow->refreshViews();
    });
  m_dockWidget->addAction(m_saveConfigAction);
  m_dockWidget->addAction(m_runModelAction);
  updateConfigFileLabel();
}

void SFSBuilder_TreeItem::loadConfig()
{
  const QString filePath = QFileDialog::getOpenFileName(
    m_dockWidget.data(), QObject::tr("Load Configuration"),
    m_configFileHistory.currentFile(),
    QObject::tr("YAML files (*.yaml *.yml);;All files (*.*)"));
  if (filePath.isEmpty())
    return;

  loadConfigFrom(filePath);
}

bool SFSBuilder_TreeItem::loadConfigFrom(
  const QString& filePath, bool showErrors)
{
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    if (showErrors) {
      QMessageBox::warning(
        m_dockWidget.data(), QObject::tr("Load Configuration"),
        QObject::tr("Could not open %1:\n%2")
          .arg(QFileInfo(filePath).fileName(), file.errorString()));
    }
    return false;
  }

  m_panel.configEditor->setText(QString::fromUtf8(file.readAll()));
  m_panel.configEditor->recolor();
  m_panel.configEditor->setCursorPosition(0, 0);
  m_configFileHistory.rememberFile(filePath);
  updateConfigFileLabel();
  loadConfiguredImages(showErrors);
  return true;
}

void SFSBuilder_TreeItem::restoreState()
{
  const QString filePath = m_configFileHistory.currentFile();
  if (!filePath.isEmpty() && !loadConfigFrom(filePath, false))
    m_configFileHistory.clearCurrentFile();

  QSettings settings;
  settings.beginGroup(QString::fromLatin1(settingsGroup));
  const int storedSolver = settings.value(
    QString::fromLatin1(solverSettingsKey),
    m_panel.comboBox_solver->currentData()).toInt();
  const int storedIndex = m_panel.comboBox_solver->findData(storedSolver);
  auto* model = qobject_cast<QStandardItemModel*>(
    m_panel.comboBox_solver->model());
  if (storedIndex >= 0 && (!model || model->item(storedIndex)->isEnabled())) {
    m_panel.comboBox_solver->setCurrentIndex(storedIndex);
  } else {
    m_panel.comboBox_solver->setCurrentIndex(
      m_panel.comboBox_solver->findData(static_cast<int>(LinSolverKind::MT)));
  }
}

void SFSBuilder_TreeItem::updateConfigFileLabel()
{
  const QString filePath = m_configFileHistory.currentFile();
  const bool hasFile = QFileInfo::exists(filePath);
  if (m_openConfigInVSCodeAction)
    m_openConfigInVSCodeAction->setEnabled(hasFile);
  if (m_showConfigInExplorerAction)
    m_showConfigInExplorerAction->setEnabled(hasFile);

  if (!hasFile) {
    m_panel.label_ConfigFilePath->setFullText(
      QObject::tr("Unsaved configuration"));
    m_panel.label_ConfigFilePath->setToolTip({});
    return;
  }

  const QString displayPath = QDir::toNativeSeparators(filePath);
  m_panel.label_ConfigFilePath->setFullText(displayPath);
  m_panel.label_ConfigFilePath->setToolTip(displayPath);
}

void SFSBuilder_TreeItem::openConfigInVSCode()
{
  const QString filePath = m_configFileHistory.currentFile();
  if (!QFileInfo::exists(filePath)) {
    updateConfigFileLabel();
    return;
  }

  if (!FileLauncher::openInVSCode(filePath)) {
    QMessageBox::warning(
      m_dockWidget.data(), QObject::tr("Open in VS Code"),
      QObject::tr("Could not find or start Visual Studio Code."));
  }
}

void SFSBuilder_TreeItem::showConfigInFileExplorer()
{
  const QString filePath = m_configFileHistory.currentFile();
  if (!QFileInfo::exists(filePath)) {
    updateConfigFileLabel();
    return;
  }

  if (!FileLauncher::showInFileBrowser(filePath)) {
    QMessageBox::warning(
      m_dockWidget.data(), QObject::tr("Show in File Explorer"),
      QObject::tr("Could not open File Explorer."));
  }
}

void SFSBuilder_TreeItem::rebuildRecentConfigsMenu()
{
  m_recentConfigsMenu->clear();

  for (const QString& filePath : m_configFileHistory.existingRecentFiles()) {
    const QFileInfo fileInfo(filePath);
    auto* rowAction = new QWidgetAction(m_recentConfigsMenu);
    auto* row = new PaletteHoverRow(m_recentConfigsMenu);
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(4, 1, 4, 1);
    layout->setSpacing(4);

    auto* openButton = new QPushButton(
      QStringLiteral("%1 — %2").arg(fileInfo.fileName(), fileInfo.path()), row);
    openButton->setFlat(true);
    openButton->setToolTip(filePath);
    openButton->setStyleSheet(QStringLiteral("text-align: left;"));
    layout->addWidget(openButton, 1);

    auto* removeButton = new QToolButton(row);
    removeButton->setText(QStringLiteral("×"));
    removeButton->setToolTip(QObject::tr("Remove from recent configurations"));
    removeButton->setAutoRaise(true);
    layout->addWidget(removeButton);

    rowAction->setDefaultWidget(row);
    m_recentConfigsMenu->addAction(rowAction);

    QObject::connect(openButton, &QPushButton::clicked, m_dockWidget.data(),
      [this, filePath]() {
        m_recentConfigsMenu->close();
        loadConfigFrom(filePath);
      });
    QObject::connect(rowAction, &QAction::triggered, m_dockWidget.data(),
      [this, filePath]() {
        m_recentConfigsMenu->close();
        loadConfigFrom(filePath);
      });
    QObject::connect(removeButton, &QToolButton::clicked, m_dockWidget.data(),
      [this, filePath, rowAction]() {
        m_configFileHistory.removeFile(filePath);
        m_recentConfigsMenu->removeAction(rowAction);
        rowAction->deleteLater();
        if (m_recentConfigsMenu->actions().isEmpty()) {
          auto* emptyAction = m_recentConfigsMenu->addAction(
            QObject::tr("No recent configurations"));
          emptyAction->setEnabled(false);
        }
      });
  }

  if (m_recentConfigsMenu->isEmpty()) {
    auto* emptyAction = m_recentConfigsMenu->addAction(
      QObject::tr("No recent configurations"));
    emptyAction->setEnabled(false);
  }
}

void SFSBuilder_TreeItem::saveConfig()
{
  const QString filePath = m_configFileHistory.currentFile();
  if (filePath.isEmpty()) {
    saveConfigAs();
    return;
  }

  saveConfigTo(filePath);
}

void SFSBuilder_TreeItem::saveConfigAs()
{
  const QString currentPath = m_configFileHistory.currentFile();
  const QString initialPath = currentPath.isEmpty()
    ? QStringLiteral("sfs-config.yaml")
    : currentPath;
  const QString filePath = QFileDialog::getSaveFileName(
    m_dockWidget.data(), QObject::tr("Save Configuration As"),
    initialPath, QObject::tr("YAML files (*.yaml *.yml);;All files (*.*)"));
  if (filePath.isEmpty())
    return;

  if (saveConfigTo(filePath)) {
    m_configFileHistory.rememberFile(filePath);
    updateConfigFileLabel();
  }
}

bool SFSBuilder_TreeItem::saveConfigTo(const QString& filePath)
{
  QSaveFile file(filePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(
      m_dockWidget.data(), QObject::tr("Save Configuration"),
      QObject::tr("Could not open %1 for writing:\n%2")
        .arg(QFileInfo(filePath).fileName(), file.errorString()));
    return false;
  }

  const QByteArray contents = m_panel.configEditor->text().toUtf8();
  if (file.write(contents) != contents.size() || !file.commit()) {
    QMessageBox::warning(
      m_dockWidget.data(), QObject::tr("Save Configuration"),
      QObject::tr("Could not save %1:\n%2")
        .arg(QFileInfo(filePath).fileName(), file.errorString()));
    return false;
  }

  return true;
}

void SFSBuilder_TreeItem::selectImageSlot(int slot)
{
  if (slot < -1 || slot >= static_cast<int>(m_slotActions.size()))
    return;

  if(slot >= 0 && m_imagePlane->images[slot].empty()) {
    m_imagePlane->Open(slot);
    if(m_imagePlane->images[slot].empty())
      slot = -1;
  }

  const std::array slotButtons{
    m_panel.toolButton_slot1,
    m_panel.toolButton_slot2,
    m_panel.toolButton_slot3,
    m_panel.toolButton_slot4
  };
  for (int index = 0; index < static_cast<int>(slotButtons.size()); ++index)
    slotButtons[index]->setChecked(index == slot);
  m_panel.toolButton_slotNone->setChecked(slot == -1);
  if(m_noImageSlotAction)
    m_noImageSlotAction->setChecked(slot == -1);
  for(int index = 0; index < static_cast<int>(m_slotActions.size()); ++index) {
    if(m_slotActions[index])
      m_slotActions[index]->setChecked(index == slot);
  }

  m_imagePlane->SelectImageSlot(slot);
  updateDisplayModes();
  if(auto* mainWindow = findParentOfType<MainWindow>(m_dockWidget.data()))
    mainWindow->refreshViews();
}

void SFSBuilder_TreeItem::updateDisplayModes()
{
  if(m_panel.radioButton_image_albedo->isChecked()) {
    m_imagePlane->SetShapeMode(ImagePlane::shape_mode_albedo);
    m_imagePlane->image_mode = ImagePlane::image_mode_off;
  } else if(m_panel.radioButton_image_image->isChecked()) {
    m_imagePlane->SetShapeMode(ImagePlane::shape_mode_image);
    m_imagePlane->image_mode = ImagePlane::image_mode_off;
  } else {
    m_imagePlane->SetShapeMode(ImagePlane::shape_mode_off);
    m_imagePlane->image_mode = m_imagePlane->curslot >= 0
      ? ImagePlane::image_mode_image : ImagePlane::image_mode_off;
  }
}

bool SFSBuilder_TreeItem::loadConfiguredImages(
  bool showErrors, bool loadAll, bool forceReload)
{
  const QByteArray yamlText = m_panel.configEditor->text().toUtf8();
  try {
    const YAML::Node document = YAML::Load(std::string(
      yamlText.constData(), static_cast<std::size_t>(yamlText.size())));
    const YAML::Node config = document["SFSModelConfig"];
    if(!config || !config["workdir"] || !config["images"])
      throw std::runtime_error("SFSModelConfig requires workdir and images");

    const auto workdir = config["workdir"].as<std::string>();
    const YAML::Node images = config["images"];
    std::array<std::string, 4> configuredPaths;
    std::array<std::array<float, 3>, 4> configuredLights{};
    int slot = 0;
    for(const auto& image : images) {
      if(slot >= 4)
        break;
      const auto imageShortName = image.begin()->first.as<std::string>();
      const auto imageFile = std::filesystem::path(workdir) / imageShortName;
      const auto lightPos = image.begin()->second.as<std::vector<float>>();
      if(lightPos.size() != 3)
        throw std::runtime_error("each image light position requires 3 values");
      configuredPaths[slot] = imageFile.generic_string();
      for(int component = 0; component < 3; ++component)
        configuredLights[slot][component] = lightPos[component];
      ++slot;
    }

    if(forceReload) {
      for(auto& image : m_imagePlane->images)
        image.clear();
      m_imagePlane->cache_slot = -1;
      m_imagePlane->image_tile.clear();
    }

    for(int index = 0; index < 4; ++index) {
      if(m_imagePlane->imagefname[index] != configuredPaths[index]) {
        m_imagePlane->imagefname[index] = configuredPaths[index];
        m_imagePlane->images[index].clear();
        if(m_imagePlane->cache_slot == index) {
          m_imagePlane->cache_slot = -1;
          m_imagePlane->image_tile.clear();
        }
      }
      for(int component = 0; component < 3; ++component)
        m_imagePlane->lights[index][component] =
          configuredLights[index][component];
    }

    m_imagePlane->albedo.clear();
    m_imagePlane->data_attd.clear();
    m_imagePlane->surfaceMeshDirty = true;

    if(loadAll) {
      for(int index = 0; index < slot; ++index) {
        if(m_imagePlane->images[index].empty())
          m_imagePlane->Open(index);
      }
    } else if(slot > 0) {
      selectImageSlot(0);
      const bool firstImageLoaded = !m_imagePlane->images[0].empty();
      return firstImageLoaded;
    } else {
      selectImageSlot(-1);
      return false;
    }

    if(slot != 4)
      return false;
    const auto& firstImage = m_imagePlane->images.front();
    const bool compatibleImages = !firstImage.empty()
      && std::all_of(
        m_imagePlane->images.begin(), m_imagePlane->images.end(),
        [&firstImage](const auto& image) {
          return !image.empty()
            && image.width == firstImage.width
            && image.height == firstImage.height;
        });
    if(compatibleImages) {
      m_imagePlane->w = firstImage.width;
      m_imagePlane->h = firstImage.height;
    }
    return compatibleImages;
  }
  catch(const std::exception& error) {
    selectImageSlot(-1);
    if(showErrors) {
      QMessageBox::warning(
        m_dockWidget.data(), QObject::tr("Load SFS Images"),
        QObject::tr("Could not load configured images:\n%1")
          .arg(QString::fromUtf8(error.what())));
    } else {
      qWarning() << "Could not load configured SFS images:" << error.what();
    }
    return false;
  }
}

void SFSBuilder_TreeItem::syncModelConfiguration()
{
  loadConfiguredImages(true, false, true);
}

void SFSBuilder_TreeItem::runModel()
{
  if(!loadConfiguredImages(true, true, true)) {
    QMessageBox::warning(
      m_dockWidget.data(), QObject::tr("Run SFS Model"),
      QObject::tr("Four successfully loaded images of the same size are required."));
    return;
  }
  m_syncModelAction->setEnabled(false);
  m_runModelAction->setEnabled(false);
  const auto solverKind = static_cast<LinSolverKind>(
    m_panel.comboBox_solver->currentData().toInt());
  m_imagePlane->Build(
    m_dockWidget.data(),
    solverKind,
    [this](ImagePlane::BuildResult result) {
      m_syncModelAction->setEnabled(true);
      m_runModelAction->setEnabled(true);

      if(result.status != ImagePlane::BuildStatus::Succeeded || !result.output) {
        QMessageBox::critical(
          m_dockWidget.data(), QObject::tr("SFS Calculation Failed"),
          result.error.isEmpty()
            ? QObject::tr("The SFS calculation did not produce a result.")
            : result.error);
        return;
      }

      m_imagePlane->ApplyBuildResult(std::move(*result.output));
      selectImageSlot(-1);
      m_panel.radioButton_image_image->setChecked(true);
      if(auto* mainWindow = findParentOfType<MainWindow>(m_dockWidget.data()))
        mainWindow->refreshViews();
    });
}

SFSBuilder_TreeItem::~SFSBuilder_TreeItem(){}

void
SFSBuilder_TreeItem::showModel(DrawCntx* cx)
{
  m_viewCtrl->Draw(cx);
  m_lights->Draw(cx);

  m_imagePlane->Draw(cx);

  m_toolsPanel->Draw(cx);
}

void
SFSBuilder_TreeItem::activateProjectTreeItem(QDockWidget* dock, bool activate){
  auto mainwin = findParentOfType<QMainWindow>(dock);
  CentralWidget* cw= mainwin->findChild<CentralWidget*>();
  if(activate){
    if(!m_dockWidget->isActiveWindow()){
      mainwin->tabifyDockWidget(dock,m_dockWidget.get());
    }
    m_dockWidget->activateWindow();
    m_dockWidget->show();
    m_dockWidget->raise();
    //cw->eventHandler().addChild(m_viewCtrlEH.get());
    cw->eventContext().pushHandler(m_viewCtrlEH.get());

    // update viewport size if it was changed before
    m_viewCtrl->updateProjectionMtrx(cw->width(),cw->height());
    setData(1,Qt::DisplayRole, "*");
  }
  else{
    m_dockWidget->hide();
    //cw->eventHandler().removeChild(m_viewCtrlEH.get());
    cw->eventContext().popHandler();
    setData(1,Qt::DisplayRole, "");
  }
}
