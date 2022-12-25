#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include "ViewPortController.h"
#include <QOpenGLWidget>

namespace Ui {
class CentralWidget;
}

class ProjectTree;
class TreeEventFilter;
class QEvent;

class CentralWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit CentralWidget(QWidget *parent = nullptr);
    ~CentralWidget();

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void enterEvent(QEvent *event) override;
    void setProjectTree(ProjectTree *pt);

    EventHandler3D& eventHandler();
private:
    Ui::CentralWidget *ui;
    ProjectTree* m_projectTree = nullptr;
    TreeEventFilter* m_treeEventFilter = nullptr;
};

#endif // CENTRALWIDGET_H
