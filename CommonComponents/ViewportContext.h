#ifndef VIEWPORTCONTEXT_H
#define VIEWPORTCONTEXT_H

class ViewportDispatchContext
{
public:
  void beginDispatch() { m_implemented = true; }
  void markNotImplemented() { m_implemented = false; }
  bool isImplemented() const { return m_implemented; }

private:
  bool m_implemented = true;
};

#ifdef TRYMIX_ENABLE_VTK
class vtkRenderer;

class VtkDrawCntx : public ViewportDispatchContext
{
public:
  explicit VtkDrawCntx(vtkRenderer* renderer) : m_renderer(renderer) {}
  vtkRenderer* renderer() const { return m_renderer; }

private:
  vtkRenderer* m_renderer = nullptr;
};
#endif

#endif
