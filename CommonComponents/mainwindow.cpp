#include "mainwindow.h"
#include "qevent.h"
#include "ui_mainwindow.h"
#include "CentralWidget.h"

#include <QCursor>
#include <QDebug>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionQuit, &QAction::triggered, [](bool){ qApp->quit();});
    setCentralWidget(ui->treeWidget_project->gl());
    setWindowIcon(QIcon(":/system/images/zeroapp-icon.png"));

    // should be last block (after interface is constructed)
    QSettings settings("", QCoreApplication::applicationName());
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

Ui::MainWindow* MainWindow::UI(){
  return ui;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
  QSettings settings("MyCompany", QCoreApplication::applicationName());
  settings.setValue("geometry",saveGeometry());
  settings.setValue("windowState",saveState());
  QMainWindow::closeEvent(e);
}
