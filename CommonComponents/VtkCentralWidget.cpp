#include "VtkCentralWidget.h"

#include "ProjectTree.h"

#include <QShowEvent>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

VtkCentralWidget::VtkCentralWidget(QWidget* parent)
  : QVTKOpenGLNativeWidget(parent)
  , m_renderer(vtkSmartPointer<vtkRenderer>::New())
{
  setFormat(QVTKOpenGLNativeWidget::defaultFormat());

  vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
  setRenderWindow(renderWindow);
  renderWindow->AddRenderer(m_renderer);

  vtkNew<vtkInteractorStyleTrackballCamera> cameraInteraction;
  renderWindow->GetInteractor()->SetInteractorStyle(cameraInteraction);
  m_renderer->SetBackground(0.08, 0.10, 0.14);
}

void VtkCentralWidget::setProjectTree(ProjectTree* projectTree)
{
  m_projectTree = projectTree;
}

void VtkCentralWidget::refreshModel()
{
  m_renderer->RemoveAllViewProps();
  if(m_projectTree)
    m_projectTree->showVtkModel(m_renderer);

  m_renderer->ResetCamera();
  m_renderPending = true;
  renderIfVisible();
}

void VtkCentralWidget::showEvent(QShowEvent* event)
{
  QVTKOpenGLNativeWidget::showEvent(event);
  renderIfVisible();
}

void VtkCentralWidget::renderIfVisible()
{
  if(m_renderPending && isVisible()) {
    renderWindow()->Render();
    m_renderPending = false;
  }
}
