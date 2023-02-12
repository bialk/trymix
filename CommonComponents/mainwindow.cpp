#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "CentralWidget.h"

#include <QCursor>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionQuit, &QAction::triggered, [](bool){ qApp->quit();});
    setCentralWidget(ui->treeWidget_project->gl());
    setWindowIcon(QIcon(":/system/images/zeroapp-icon.png"));
}

Ui::MainWindow* MainWindow::UI(){
  return ui;
}

MainWindow::~MainWindow()
{
    delete ui;
}

