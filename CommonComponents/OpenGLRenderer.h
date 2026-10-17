#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H

#include "OpenGLMesh.h"

#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QVector2D>

class QMatrix4x4;
class QOpenGLFunctions;
class QOpenGLFunctions_3_3_Core;

class OpenGLRenderer
{
public:
  OpenGLRenderer();

  bool initialize();
  void destroy();
  void draw(OpenGLMesh& mesh,
            const QMatrix4x4& projection, const QMatrix4x4& modelView,
            const QVector3D& colorMultiplier = {1.0F, 1.0F, 1.0F});
  void drawCube(const QMatrix4x4& projection, const QMatrix4x4& modelView);
  void drawCone(const QMatrix4x4& projection, const QMatrix4x4& modelView);
  void drawTestScene(const QMatrix4x4& projection, const QMatrix4x4& view);
  void drawLightIcon(const QMatrix4x4& projection, const QMatrix4x4& modelView,
                     const QVector3D& colorMultiplier);
  void drawTexturedQuad(unsigned int textureId, const QMatrix4x4& projection,
                        const QMatrix4x4& modelView,
                        const QVector2D& textureScale);
  void drawLines(OpenGLMesh& mesh, const QMatrix4x4& projection,
                 const QMatrix4x4& modelView, float width);
  void drawPoints(OpenGLMesh& mesh, const QMatrix4x4& projection,
                  const QMatrix4x4& modelView, float size);
  void setDirectionalLights(const QVector3D& firstDirection, bool firstEnabled,
                            const QVector3D& secondDirection, bool secondEnabled);
  bool isInitialized() const;

private:
  void drawPrimitive(OpenGLMesh& mesh, const QMatrix4x4& projection,
                     const QMatrix4x4& modelView,
                     const QVector3D& colorMultiplier,
                     unsigned int primitiveType, bool lightingEnabled,
                     bool depthTestEnabled = true);

  QOpenGLShaderProgram m_program;
  QOpenGLShaderProgram m_textureProgram;
  OpenGLMesh m_cubeMesh;
  OpenGLMesh m_coneMesh;
  OpenGLMesh m_lightIconMesh;
  OpenGLMesh m_imageQuadMesh;
  QVector3D m_lightDirections[2]{{0.0F, 0.0F, 1.0F}, {0.0F, 0.0F, 1.0F}};
  bool m_lightEnabled[2]{true, true};
  QOpenGLFunctions* m_functions = nullptr;
  QOpenGLFunctions_3_3_Core* m_coreFunctions = nullptr;
  bool m_initialized = false;
};

#endif // OPENGLRENDERER_H
