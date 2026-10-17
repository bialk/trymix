#include "OpenGLMesh.h"

#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <cstddef>

OpenGLMesh::OpenGLMesh()
  : m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
  , m_indexBuffer(QOpenGLBuffer::IndexBuffer)
{
}

bool OpenGLMesh::initialize(
    const OpenGLVertex* vertices, int vertexCount,
    const unsigned int* indices, int indexCount)
{
  if(isInitialized())
    return true;

  if(m_initialized)
    destroy();

  auto* context = QOpenGLContext::currentContext();
  if(!context || !vertices || vertexCount <= 0 || !indices || indexCount <= 0) {
    qWarning() << "OpenGL mesh initialization has invalid input or no context";
    return false;
  }

  if(!m_vao.create() || !m_vertexBuffer.create() || !m_indexBuffer.create()) {
    qWarning() << "OpenGL mesh buffer creation failed";
    destroy();
    return false;
  }

  {
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_vertexBuffer.bind();
    m_vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertexBuffer.allocate(
      vertices, vertexCount * static_cast<int>(sizeof(OpenGLVertex)));

    m_indexBuffer.bind();
    m_indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_indexBuffer.allocate(
      indices, indexCount * static_cast<int>(sizeof(unsigned int)));

    auto* functions = context->functions();
    functions->glEnableVertexAttribArray(0);
    functions->glVertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE, sizeof(OpenGLVertex),
      reinterpret_cast<const void*>(offsetof(OpenGLVertex, position)));
    functions->glEnableVertexAttribArray(1);
    functions->glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, sizeof(OpenGLVertex),
      reinterpret_cast<const void*>(offsetof(OpenGLVertex, color)));
    functions->glEnableVertexAttribArray(2);
    functions->glVertexAttribPointer(
      2, 3, GL_FLOAT, GL_FALSE, sizeof(OpenGLVertex),
      reinterpret_cast<const void*>(offsetof(OpenGLVertex, normal)));
    functions->glEnableVertexAttribArray(3);
    functions->glVertexAttribPointer(
      3, 2, GL_FLOAT, GL_FALSE, sizeof(OpenGLVertex),
      reinterpret_cast<const void*>(
        offsetof(OpenGLVertex, textureCoordinate)));
  }

  m_vertexBuffer.release();
  m_indexBuffer.release();
  m_indexCount = indexCount;
  m_initialized = true;
  return true;
}

bool OpenGLMesh::update(
    const OpenGLVertex* vertices, int vertexCount,
    const unsigned int* indices, int indexCount)
{
  if(!isInitialized()) {
    m_initialized = false;
    return initialize(vertices, vertexCount, indices, indexCount);
  }
  if(!QOpenGLContext::currentContext() || !vertices || vertexCount <= 0
      || !indices || indexCount <= 0)
    return false;

  m_vertexBuffer.bind();
  m_vertexBuffer.allocate(
    vertices, vertexCount * static_cast<int>(sizeof(OpenGLVertex)));
  m_vertexBuffer.release();
  m_indexBuffer.bind();
  m_indexBuffer.allocate(
    indices, indexCount * static_cast<int>(sizeof(unsigned int)));
  m_indexBuffer.release();
  m_indexCount = indexCount;
  return true;
}

void OpenGLMesh::destroy()
{
  m_vao.destroy();
  m_vertexBuffer.destroy();
  m_indexBuffer.destroy();
  m_indexCount = 0;
  m_initialized = false;
}

bool OpenGLMesh::isInitialized() const
{
  return m_initialized && m_vao.isCreated()
    && m_vertexBuffer.isCreated() && m_indexBuffer.isCreated();
}

void OpenGLMesh::bind()
{
  m_vao.bind();
}

void OpenGLMesh::release()
{
  m_vao.release();
}

int OpenGLMesh::indexCount() const
{
  return m_indexCount;
}
