#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QOpenGLWidget>
#include <QMetaObject>

#include <memory>

namespace Ui {
class CentralWidget;
}

class ProjectTree;
class TreeEventFilter;
class EventContext3D;
class EventHandler3D;
class QEvent;
class DrawCntx;
class ModernOpenGLRenderer;
class QOpenGLDebugLogger;


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
    ModernOpenGLRenderer& modernRenderer();
private:
    void cleanupOpenGL();

    Ui::CentralWidget *ui;
    ProjectTree* m_projectTree = nullptr;
    TreeEventFilter* m_treeEventFilter = nullptr;
    std::unique_ptr<ModernOpenGLRenderer> m_modernRenderer;
    std::unique_ptr<QOpenGLDebugLogger> m_openGLDebugLogger;
    QMetaObject::Connection m_contextCleanupConnection;
};

#endif // CENTRALWIDGET_H
