#ifndef MODERNOPENGLMESH_H
#define MODERNOPENGLMESH_H

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

struct ModernOpenGLVertex
{
  float position[3];
  float color[3];
  float normal[3];
  float textureCoordinate[2];
};

class ModernOpenGLMesh
{
public:
  ModernOpenGLMesh();

  bool initialize(const ModernOpenGLVertex* vertices, int vertexCount,
                  const unsigned int* indices, int indexCount);
  bool update(const ModernOpenGLVertex* vertices, int vertexCount,
              const unsigned int* indices, int indexCount);
  void destroy();
  bool isInitialized() const;

  void bind();
  void release();
  int indexCount() const;

private:
  QOpenGLVertexArrayObject m_vao;
  QOpenGLBuffer m_vertexBuffer;
  QOpenGLBuffer m_indexBuffer;
  int m_indexCount = 0;
  bool m_initialized = false;
};

#endif // MODERNOPENGLMESH_H
