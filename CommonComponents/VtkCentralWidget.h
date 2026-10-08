#ifndef VTKCENTRALWIDGET_H
#define VTKCENTRALWIDGET_H

#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>

class ProjectTree;
class QShowEvent;
class vtkRenderer;

class VtkCentralWidget : public QVTKOpenGLNativeWidget
{
public:
  explicit VtkCentralWidget(QWidget* parent = nullptr);

  void setProjectTree(ProjectTree* projectTree);
  void refreshModel();

protected:
  void showEvent(QShowEvent* event) override;

private:
  void renderIfVisible();

  ProjectTree* m_projectTree = nullptr;
  vtkSmartPointer<vtkRenderer> m_renderer;
  bool m_renderPending = true;
};

#endif // VTKCENTRALWIDGET_H
