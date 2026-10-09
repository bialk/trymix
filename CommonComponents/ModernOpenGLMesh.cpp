#include "ModernOpenGLMesh.h"

#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <cstddef>

ModernOpenGLMesh::ModernOpenGLMesh()
  : m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
  , m_indexBuffer(QOpenGLBuffer::IndexBuffer)
{
}

bool ModernOpenGLMesh::initialize(
    const ModernOpenGLVertex* vertices, int vertexCount,
    const unsigned int* indices, int indexCount)
{
  if(m_initialized)
    return true;

  auto* context = QOpenGLContext::currentContext();
  if(!context || !vertices || vertexCount <= 0 || !indices || indexCount <= 0) {
    qWarning() << "Modern OpenGL mesh initialization has invalid input or no context";
    return false;
  }

  if(!m_vao.create() || !m_vertexBuffer.create() || !m_indexBuffer.create()) {
    qWarning() << "Modern OpenGL mesh buffer creation failed";
    destroy();
    return false;
  }

  {
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_vertexBuffer.bind();
    m_vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertexBuffer.allocate(
      vertices, vertexCount * static_cast<int>(sizeof(ModernOpenGLVertex)));

    m_indexBuffer.bind();
    m_indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_indexBuffer.allocate(
      indices, indexCount * static_cast<int>(sizeof(unsigned int)));

    auto* functions = context->functions();
    functions->glEnableVertexAttribArray(0);
    functions->glVertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE, sizeof(ModernOpenGLVertex),
      reinterpret_cast<const void*>(offsetof(ModernOpenGLVertex, position)));
    functions->glEnableVertexAttribArray(1);
    functions->glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, sizeof(ModernOpenGLVertex),
      reinterpret_cast<const void*>(offsetof(ModernOpenGLVertex, color)));
    functions->glEnableVertexAttribArray(2);
    functions->glVertexAttribPointer(
      2, 3, GL_FLOAT, GL_FALSE, sizeof(ModernOpenGLVertex),
      reinterpret_cast<const void*>(offsetof(ModernOpenGLVertex, normal)));
    functions->glEnableVertexAttribArray(3);
    functions->glVertexAttribPointer(
      3, 2, GL_FLOAT, GL_FALSE, sizeof(ModernOpenGLVertex),
      reinterpret_cast<const void*>(
        offsetof(ModernOpenGLVertex, textureCoordinate)));
  }

  m_vertexBuffer.release();
  m_indexBuffer.release();
  m_indexCount = indexCount;
  m_initialized = true;
  return true;
}

bool ModernOpenGLMesh::update(
    const ModernOpenGLVertex* vertices, int vertexCount,
    const unsigned int* indices, int indexCount)
{
  if(!m_initialized)
    return initialize(vertices, vertexCount, indices, indexCount);
  if(!QOpenGLContext::currentContext() || !vertices || vertexCount <= 0
      || !indices || indexCount <= 0)
    return false;

  m_vertexBuffer.bind();
  m_vertexBuffer.allocate(
    vertices, vertexCount * static_cast<int>(sizeof(ModernOpenGLVertex)));
  m_vertexBuffer.release();
  m_indexBuffer.bind();
  m_indexBuffer.allocate(
    indices, indexCount * static_cast<int>(sizeof(unsigned int)));
  m_indexBuffer.release();
  m_indexCount = indexCount;
  return true;
}

void ModernOpenGLMesh::destroy()
{
  m_vao.destroy();
  m_vertexBuffer.destroy();
  m_indexBuffer.destroy();
  m_indexCount = 0;
  m_initialized = false;
}

bool ModernOpenGLMesh::isInitialized() const
{
  return m_initialized;
}

void ModernOpenGLMesh::bind()
{
  m_vao.bind();
}

void ModernOpenGLMesh::release()
{
  m_vao.release();
}

int ModernOpenGLMesh::indexCount() const
{
  return m_indexCount;
}
