#ifndef MODERNOPENGLRENDERER_H
#define MODERNOPENGLRENDERER_H

#include "ModernOpenGLMesh.h"

#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QVector2D>

class QMatrix4x4;
class QOpenGLFunctions;

class ModernOpenGLRenderer
{
public:
  ModernOpenGLRenderer();

  bool initialize();
  void destroy();
  void draw(ModernOpenGLMesh& mesh,
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
  void drawLines(ModernOpenGLMesh& mesh, const QMatrix4x4& projection,
                 const QMatrix4x4& modelView, float width);
  void drawPoints(ModernOpenGLMesh& mesh, const QMatrix4x4& projection,
                  const QMatrix4x4& modelView, float size);
  void setDirectionalLights(const QVector3D& firstDirection, bool firstEnabled,
                            const QVector3D& secondDirection, bool secondEnabled);
  bool isInitialized() const;

private:
  void drawPrimitive(ModernOpenGLMesh& mesh, const QMatrix4x4& projection,
                     const QMatrix4x4& modelView,
                     const QVector3D& colorMultiplier,
                     unsigned int primitiveType, bool lightingEnabled,
                     bool depthTestEnabled = true);

  QOpenGLShaderProgram m_program;
  QOpenGLShaderProgram m_textureProgram;
  ModernOpenGLMesh m_cubeMesh;
  ModernOpenGLMesh m_coneMesh;
  ModernOpenGLMesh m_lightIconMesh;
  ModernOpenGLMesh m_imageQuadMesh;
  QVector3D m_lightDirections[2]{{0.0F, 0.0F, 1.0F}, {0.0F, 0.0F, 1.0F}};
  bool m_lightEnabled[2]{true, true};
  QOpenGLFunctions* m_functions = nullptr;
  bool m_initialized = false;
};

#endif // MODERNOPENGLRENDERER_H
