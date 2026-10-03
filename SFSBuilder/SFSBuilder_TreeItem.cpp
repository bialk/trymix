#include "SFSBuilder_TreeItem.h"
#include "QMainWindow"
#include "imageplane.h"
#include "CommonComponents/lights.h"
#include "CommonComponents/toolspanel.h"
#include "CommonComponents/viewctrl.h"
#include "CommonComponents/EventHandling.h"
#include "CommonComponents/CentralWidget.h"
#include "CommonComponents/QScintillaEditor.h"
#include "CommonComponents/testScene.h"

#include <QOpenGLWidget>
#include <QPainter>
#include <QDebug>
#include <QAction>
#include <QActionGroup>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QMessageBox>
#include <QSaveFile>
#include <QToolButton>

#include <yaml-cpp/yaml.h>
#include <filesystem>


namespace {

class EventHandler_PositionController: public EventHandler3D{

public:
  ViewCtrl* m_vp = nullptr;
  Lights* m_lights = nullptr;
  

  ViewCtrl::Opercode Op{ViewCtrl::Opercode::origRotate};

  EventHandler_PositionController(ViewCtrl* vc, Lights* lights)
    :m_vp(vc)
    ,m_lights(lights)
  {
    addReact("M:L:DOWN") = [this](EventContext3D& cx)
    {
      //try to see if anything selected on screen
      auto id = cx.select();
      if(m_lights->isfocus(id)){
        m_lights->lightrstart(cx.x(),cx.y());
        cx.pushHandler(&m_mouseDragLight);
        qDebug() << "Selected name: " << id << Qt::endl;
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
  EventHandler3D m_mouseDragLight;
};

}



SFSBuilder_TreeItem::SFSBuilder_TreeItem()
  :m_imagePlane(new ImagePlane)
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
  m_viewCtrlEH.reset(new EventHandler_PositionController(m_viewCtrl.get(),m_lights.get()));


  setData(0,Qt::DisplayRole, name());
  setData(1,Qt::DisplayRole, "");

  buildContextMenuStandardItems();

  m_dockWidget.reset(new QDockWidget);
  m_panel.setupUi(m_dockWidget.data());

  drawTestScene();

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

  createActions();
  bindActions();
}

void SFSBuilder_TreeItem::createActions()
{
  auto* slotActionGroup = new QActionGroup(m_dockWidget.data());
  slotActionGroup->setExclusive(true);
  for (int slot = 0; slot < static_cast<int>(m_slotActions.size()); ++slot) {
    auto* action = new QAction(
      QObject::tr("Image Slot %1").arg(slot + 1), m_dockWidget.data());
    action->setCheckable(true);
    action->setData(slot);
    slotActionGroup->addAction(action);
    m_slotActions[slot] = action;
  }
  m_slotActions[0]->setChecked(true);
  QObject::connect(
    slotActionGroup, &QActionGroup::triggered, m_dockWidget.data(),
    [this](QAction* action) { selectImageSlot(action->data().toInt()); });

  m_loadConfigAction = new QAction(
    QObject::tr("Load Configuration"), m_dockWidget.data());
  m_saveConfigAction = new QAction(
    QObject::tr("Save Configuration"), m_dockWidget.data());
  m_saveConfigAsAction = new QAction(
    QObject::tr("Save Configuration As"), m_dockWidget.data());
  m_runModelAction = new QAction(QObject::tr("Run Model"), m_dockWidget.data());
  m_runModelAction->setShortcut(QKeySequence(QStringLiteral("Ctrl+R")));

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
    m_runModelAction, &QAction::triggered, m_dockWidget.data(),
    [this]() { runModel(); });
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
  slotButtons[0]->setChecked(true);

  bindButton(m_panel.toolButton_ConfigLoad, m_loadConfigAction);
  bindButton(m_panel.toolButton_ConfigSave, m_saveConfigAction);
  bindButton(m_panel.toolButton_2, m_saveConfigAsAction);
  bindButton(m_panel.toolButton_RunModel, m_runModelAction);
  m_dockWidget->addAction(m_runModelAction);
}

void SFSBuilder_TreeItem::loadConfig()
{
  const QString filePath = QFileDialog::getOpenFileName(
    m_dockWidget.data(), QObject::tr("Load Configuration"),
    m_configFilePath, QObject::tr("YAML files (*.yaml *.yml);;All files (*.*)"));
  if (filePath.isEmpty())
    return;

  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::warning(
      m_dockWidget.data(), QObject::tr("Load Configuration"),
      QObject::tr("Could not open %1:\n%2")
        .arg(QFileInfo(filePath).fileName(), file.errorString()));
    return;
  }

  m_panel.configEditor->setText(QString::fromUtf8(file.readAll()));
  m_panel.configEditor->recolor();
  m_panel.configEditor->setCursorPosition(0, 0);
  m_configFilePath = filePath;
}

void SFSBuilder_TreeItem::saveConfig()
{
  if (m_configFilePath.isEmpty()) {
    saveConfigAs();
    return;
  }

  saveConfigTo(m_configFilePath);
}

void SFSBuilder_TreeItem::saveConfigAs()
{
  const QString initialPath = m_configFilePath.isEmpty()
    ? QStringLiteral("sfs-config.yaml")
    : m_configFilePath;
  const QString filePath = QFileDialog::getSaveFileName(
    m_dockWidget.data(), QObject::tr("Save Configuration As"),
    initialPath, QObject::tr("YAML files (*.yaml *.yml);;All files (*.*)"));
  if (filePath.isEmpty())
    return;

  if (saveConfigTo(filePath))
    m_configFilePath = filePath;
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
  if (slot < 0 || slot >= static_cast<int>(m_slotActions.size()))
    return;

  const std::array slotButtons{
    m_panel.toolButton_slot1,
    m_panel.toolButton_slot2,
    m_panel.toolButton_slot3,
    m_panel.toolButton_slot4
  };
  for (int index = 0; index < static_cast<int>(slotButtons.size()); ++index)
    slotButtons[index]->setChecked(index == slot);

  m_imagePlane->curslot = slot;
  if(auto* mainWindow = findParentOfType<QMainWindow>(m_dockWidget.data())) {
    if(auto* viewport = mainWindow->findChild<CentralWidget*>())
      viewport->update();
  }
}

void SFSBuilder_TreeItem::runModel()
{
  const QByteArray yamlText = m_panel.configEditor->text().toUtf8();
  try {
    const YAML::Node document = YAML::Load(
      std::string(yamlText.constData(), static_cast<std::size_t>(yamlText.size())));
    const YAML::Node config = document["SFSModelConfig"];
    const auto workdir = config["workdir"].as<std::string>();
    const YAML::Node images = config["images"];
    auto idx = 0;
    for( auto i: images ) {
      const auto imageShortName =  i.begin()->first.as<std::string>();
      const auto imageFile = std::filesystem::path(workdir) / imageShortName;
      const std::vector<float> lightPos = i.begin()->second.as<std::vector<float>>();
      if(lightPos.size() == 3) {
        m_imagePlane->imagefname[idx] = imageFile.generic_string();
        m_imagePlane->lights[idx][0] = lightPos[0];
        m_imagePlane->lights[idx][1] = lightPos[1];
        m_imagePlane->lights[idx][2] = lightPos[2];
      } else {
        qWarning() << "Invalid light position for image" << QString::fromStdString(imageShortName) << "- expected 3 values, got" << lightPos.size();
        return;
      }
      // Here you can use imageFile and lightPos as needed
      qDebug() << "Parsed image:" << QString::fromStdString(imageShortName) << "at path:" << QString::fromStdString(imageFile.generic_string())
              << "with light position:" << lightPos[0] << lightPos[1] << lightPos[2];
      idx++;
    }
  }
  catch (const YAML::Exception& error) {
    qWarning() << "Failed to parse SFS model YAML:" << error.what();
  }

  m_imagePlane->TreeScan(&TSOCntx::TSO_ProjectLoad);
  m_runModelAction->setEnabled(false);
  m_imagePlane->Build(
    m_dockWidget.data(),
    [this](DataExchangeBlock data) {
      m_imagePlane->ApplyBuildResult(std::move(data));
      m_runModelAction->setEnabled(true);
      if(auto* mainWindow = findParentOfType<QMainWindow>(m_dockWidget.data())) {
        if(auto* viewport = mainWindow->findChild<CentralWidget*>())
          viewport->update();
      }
    });
}

SFSBuilder_TreeItem::~SFSBuilder_TreeItem(){}

void
SFSBuilder_TreeItem::showModel(DrawCntx* cx)
{
  glClearDepth(1.0);
  if(m_viewCtrl->m_background==0)
    glClearColor(.0, .0, .0, 0.0);
  else
    glClearColor(1.0, 1.0, 1.0, 0.0);

  glClear(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT |
          GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE); //need to support lights
  glDepthFunc(GL_LEQUAL);

  glDisable(GL_CLIP_PLANE0);
  glDisable(GL_CLIP_PLANE1);
  glDisable(GL_CLIP_PLANE2);
  glDisable(GL_CLIP_PLANE3);
  glDisable(GL_CLIP_PLANE4);
  glDisable(GL_CLIP_PLANE5);

  // Default Settings
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_TEXTURE_2D);

  m_viewCtrl->Draw(cx);
  m_lights->Draw(cx);


  m_imagePlane->image_mode = ImagePlane::image_mode_image;
  m_imagePlane->shape_mode = ImagePlane::shape_mode_image;
  m_imagePlane->edit_mode = ImagePlane::edit_mode_off;
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
