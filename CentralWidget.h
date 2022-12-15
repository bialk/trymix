#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QOpenGLWidget>

namespace Ui {
class CentralWidget;
}

class ProjectTree;

class CentralWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit CentralWidget(QWidget *parent = nullptr);
    ~CentralWidget();

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void setProjectTree(ProjectTree *pt);
private:
    Ui::CentralWidget *ui;
    ProjectTree* m_projectTree = nullptr;
};

#endif // CENTRALWIDGET_H
