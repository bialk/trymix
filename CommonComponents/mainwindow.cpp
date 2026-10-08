#include "mainwindow.h"
#include "qevent.h"
#include "ui_mainwindow.h"
#include "CentralWidget.h"

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
    m_viewTabs->addTab(m_openGLView, tr("OpenGL"));
#ifdef TRYMIX_ENABLE_VTK
    m_vtkView = new VtkCentralWidget(m_viewTabs);
    m_vtkView->setProjectTree(ui->treeWidget_project);
    m_viewTabs->addTab(m_vtkView, tr("VTK"));
#endif
    setCentralWidget(m_viewTabs);
    connect(m_viewTabs, &QTabWidget::currentChanged, this, [this](int) {
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
