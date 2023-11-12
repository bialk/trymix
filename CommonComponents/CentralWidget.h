#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QOpenGLWidget>

namespace Ui {
class CentralWidget;
}

class ProjectTree;
class TreeEventFilter;
class EventContext3D;
class EventHandler3D;
class QEvent;
class DrawCntx;


class CentralWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit CentralWidget(QWidget *parent = nullptr);
    ~CentralWidget();

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void enterEvent(QEnterEvent *event) override;
    void setProjectTree(ProjectTree *pt);

    EventHandler3D& eventHandler();
    EventContext3D& eventContext();
    DrawCntx& drawContext();
private:
    Ui::CentralWidget *ui;
    ProjectTree* m_projectTree = nullptr;
    TreeEventFilter* m_treeEventFilter = nullptr;
    int m_w{0}, m_h{0};
};

#endif // CENTRALWIDGET_H
