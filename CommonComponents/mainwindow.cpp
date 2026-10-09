#include "mainwindow.h"
#include "qevent.h"
#include "ui_mainwindow.h"
#include "CentralWidget.h"
#include "Projects_TreeItem.h"

#include <QCursor>
#include <QDebug>
#include <QSettings>
#include <QTabWidget>
#ifdef TRYMIX_ENABLE_VTK
#include "VtkCentralWidget.h"
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->treeWidget_project->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget_project->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget_project->header()->setStretchLastSection(false);

    connect(ui->actionQuit, &QAction::triggered, [](bool){ qApp->quit();});
    m_viewTabs = new QTabWidget(this);
    m_openGLView = ui->treeWidget_project->gl();
    m_viewTabs->addTab(m_openGLView, tr("Canvas"));
#ifdef TRYMIX_ENABLE_VTK
    m_vtkView = new VtkCentralWidget(m_viewTabs);
    m_vtkView->setProjectTree(ui->treeWidget_project);
    m_viewTabs->addTab(m_vtkView, tr("VTK"));
#endif
    setCentralWidget(m_viewTabs);
    connect(m_viewTabs, &QTabWidget::currentChanged, this, [this](int) {
      if(!m_automaticViewportChange) {
        if(auto* item = ui->treeWidget_project->activeTreeItem()) {
        item->setPreferredViewport(
          m_viewTabs->currentWidget() == m_openGLView
            ? ProjectTreeItem::Viewport::Canvas
            : ProjectTreeItem::Viewport::Vtk);
        }
      }
      if(m_viewTabs->currentWidget() == m_openGLView)
        m_openGLView->update();
    });
    setWindowIcon(QIcon(":/system/images/zeroapp-icon.png"));

    // should be last block (after interface is constructed)
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

Ui::MainWindow* MainWindow::UI(){
  return ui;
}

void MainWindow::showOpenGLView()
{
  m_viewTabs->setCurrentWidget(m_openGLView);
}

void MainWindow::restoreViewportFor(ProjectTreeItem* item)
{
  if(!item)
    return;
  if(item->preferredViewport() == ProjectTreeItem::Viewport::Canvas) {
    showOpenGLView();
    return;
  }
#ifdef TRYMIX_ENABLE_VTK
  if(item->preferredViewport() == ProjectTreeItem::Viewport::Vtk) {
    showVtkView();
    return;
  }

  // Remember the tab that was active when this application was first
  // selected. If VTK is active, expose Canvas long enough for its first paint
  // to report whether the hook is implemented. This automatic probe must not
  // be remembered as a manual user choice.
  if(item->initialViewport() == ProjectTreeItem::Viewport::Uninitialized) {
    item->setInitialViewport(
      m_viewTabs->currentWidget() == m_openGLView
        ? ProjectTreeItem::Viewport::Canvas
        : ProjectTreeItem::Viewport::Vtk);
  }
  if(m_viewTabs->currentWidget() != m_openGLView) {
    m_automaticViewportChange = true;
    showOpenGLView();
    m_automaticViewportChange = false;
  }
#endif
}

void MainWindow::initializeViewportFor(ProjectTreeItem* item)
{
  if(!item || item->preferredViewport() != ProjectTreeItem::Viewport::Uninitialized
      || !item->viewportImplementationsKnown())
    return;

  if(item->canvasImplemented() != item->vtkImplemented()) {
    item->setPreferredViewport(item->canvasImplemented()
      ? ProjectTreeItem::Viewport::Canvas
      : ProjectTreeItem::Viewport::Vtk);
    restoreViewportFor(item);
    return;
  }

  item->setPreferredViewport(
    item->initialViewport() != ProjectTreeItem::Viewport::Uninitialized
      ? item->initialViewport()
      : (m_viewTabs->currentWidget() == m_openGLView
          ? ProjectTreeItem::Viewport::Canvas
          : ProjectTreeItem::Viewport::Vtk));
  restoreViewportFor(item);
}

void MainWindow::refreshViews()
{
  m_openGLView->update();
#ifdef TRYMIX_ENABLE_VTK
  m_vtkView->refreshModel();
#endif
}

#ifdef TRYMIX_ENABLE_VTK
void MainWindow::showVtkView()
{
  m_viewTabs->setCurrentWidget(m_vtkView);
}
#endif

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
  QSettings settings;
  settings.setValue("geometry",saveGeometry());
  settings.setValue("windowState",saveState());
  QMainWindow::closeEvent(e);
}
