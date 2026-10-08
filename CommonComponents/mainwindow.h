#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CentralWidget;
class QTabWidget;
#ifdef TRYMIX_ENABLE_VTK
class VtkCentralWidget;
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void closeEvent(QCloseEvent* e) override;
    Ui::MainWindow* UI();
    void refreshViews();
    void showOpenGLView();
#ifdef TRYMIX_ENABLE_VTK
    void showVtkView();
#endif
private:
    Ui::MainWindow *ui;
    QTabWidget* m_viewTabs = nullptr;
    CentralWidget* m_openGLView = nullptr;
#ifdef TRYMIX_ENABLE_VTK
    VtkCentralWidget* m_vtkView = nullptr;
#endif
};
#endif // MAINWINDOW_H
