#include "ModernOpenGLRenderer.h"

#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QTextStream>

#include <array>
#include <cmath>
#include <numbers>
#include <vector>
ModernOpenGLRenderer::ModernOpenGLRenderer()
{
}

bool ModernOpenGLRenderer::initialize()
{
  if(m_initialized)
    return true;

  auto* context = QOpenGLContext::currentContext();
  if(!context) {
    qWarning() << "Modern OpenGL renderer requires a current context";
    return false;
  }
  m_functions = context->functions();

  constexpr auto vertexShader = R"(
    #version 330 core
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec3 color;
    layout(location = 2) in vec3 normal;

    uniform mat4 modelViewProjection;
    uniform mat3 normalMatrix;
    out vec3 vertexColor;
    out vec3 eyeNormal;

    void main()
    {
      vertexColor = color;
      eyeNormal = normalize(normalMatrix * normal);
      gl_Position = modelViewProjection * vec4(position, 1.0);
    }
  )";

  constexpr auto fragmentShader = R"(
    #version 330 core
    in vec3 vertexColor;
    in vec3 eyeNormal;
    uniform vec3 lightDirection1;
    uniform vec3 lightDirection2;
    uniform vec3 colorMultiplier;
    uniform float lightEnabled1;
    uniform float lightEnabled2;
    uniform float lightingEnabled;
    out vec4 fragmentColor;

    void main()
    {
      vec3 n = normalize(eyeNormal);
      float diffuse = lightEnabled1
        * max(dot(n, normalize(lightDirection1)), 0.0);
      diffuse += lightEnabled2
        * max(dot(n, normalize(lightDirection2)), 0.0);
      float enabledLightCount = lightEnabled1 + lightEnabled2;
      if (enabledLightCount > 0.0)
        diffuse /= enabledLightCount;
      float litIntensity = 0.18 + 0.72 * diffuse;
      float intensity = mix(1.0, litIntensity, lightingEnabled);
      fragmentColor = vec4(vertexColor * colorMultiplier * intensity, 1.0);
    }
  )";

  if(!m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader)
      || !m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader)
      || !m_program.link()) {
    qWarning() << "Modern OpenGL shader setup failed:" << m_program.log();
    return false;
  }

  constexpr auto textureVertexShader = R"(
    #version 330 core
    layout(location = 0) in vec3 position;
    layout(location = 3) in vec2 textureCoordinate;

    uniform mat4 modelViewProjection;
    uniform vec2 textureScale;
    out vec2 imageCoordinate;

    void main()
    {
      imageCoordinate = textureCoordinate * textureScale;
      gl_Position = modelViewProjection * vec4(position, 1.0);
    }
  )";

  constexpr auto textureFragmentShader = R"(
    #version 330 core
    in vec2 imageCoordinate;
    uniform sampler2D imageTexture;
    out vec4 fragmentColor;

    void main()
    {
      fragmentColor = texture(imageTexture, imageCoordinate);
    }
  )";

  if(!m_textureProgram.addShaderFromSourceCode(
       QOpenGLShader::Vertex, textureVertexShader)
      || !m_textureProgram.addShaderFromSourceCode(
        QOpenGLShader::Fragment, textureFragmentShader)
      || !m_textureProgram.link()) {
    qWarning() << "Modern OpenGL texture shader setup failed:"
               << m_textureProgram.log();
    m_program.removeAllShaders();
    return false;
  }

  // Each face has independent vertices so its solid colour is not
  // interpolated from colours assigned to neighbouring cube corners.
  constexpr std::array<ModernOpenGLVertex, 24> vertices{{
    // Top: green.
    {{ 1.0F,  1.0F, -1.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 1.0F, 0.0F}},
    {{-1.0F,  1.0F, -1.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 1.0F, 0.0F}},
    {{-1.0F,  1.0F,  1.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 1.0F, 0.0F}},
    {{ 1.0F,  1.0F,  1.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 1.0F, 0.0F}},
    // Bottom: orange.
    {{ 1.0F, -1.0F,  1.0F}, {1.0F, 0.5F, 0.0F}, {0.0F, -1.0F, 0.0F}},
    {{-1.0F, -1.0F,  1.0F}, {1.0F, 0.5F, 0.0F}, {0.0F, -1.0F, 0.0F}},
    {{-1.0F, -1.0F, -1.0F}, {1.0F, 0.5F, 0.0F}, {0.0F, -1.0F, 0.0F}},
    {{ 1.0F, -1.0F, -1.0F}, {1.0F, 0.5F, 0.0F}, {0.0F, -1.0F, 0.0F}},
    // Front: red.
    {{ 1.0F,  1.0F,  1.0F}, {1.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 1.0F}},
    {{-1.0F,  1.0F,  1.0F}, {1.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 1.0F}},
    {{-1.0F, -1.0F,  1.0F}, {1.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 1.0F}},
    {{ 1.0F, -1.0F,  1.0F}, {1.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 1.0F}},
    // Back: yellow.
    {{ 1.0F, -1.0F, -1.0F}, {1.0F, 1.0F, 0.0F}, {0.0F, 0.0F, -1.0F}},
    {{-1.0F, -1.0F, -1.0F}, {1.0F, 1.0F, 0.0F}, {0.0F, 0.0F, -1.0F}},
    {{-1.0F,  1.0F, -1.0F}, {1.0F, 1.0F, 0.0F}, {0.0F, 0.0F, -1.0F}},
    {{ 1.0F,  1.0F, -1.0F}, {1.0F, 1.0F, 0.0F}, {0.0F, 0.0F, -1.0F}},
    // Right: magenta.
    {{ 1.0F,  1.0F, -1.0F}, {1.0F, 0.0F, 1.0F}, {1.0F, 0.0F, 0.0F}},
    {{ 1.0F,  1.0F,  1.0F}, {1.0F, 0.0F, 1.0F}, {1.0F, 0.0F, 0.0F}},
    {{ 1.0F, -1.0F,  1.0F}, {1.0F, 0.0F, 1.0F}, {1.0F, 0.0F, 0.0F}},
    {{ 1.0F, -1.0F, -1.0F}, {1.0F, 0.0F, 1.0F}, {1.0F, 0.0F, 0.0F}},
    // Left: blue.
    {{-1.0F,  1.0F,  1.0F}, {0.0F, 0.0F, 1.0F}, {-1.0F, 0.0F, 0.0F}},
    {{-1.0F,  1.0F, -1.0F}, {0.0F, 0.0F, 1.0F}, {-1.0F, 0.0F, 0.0F}},
    {{-1.0F, -1.0F, -1.0F}, {0.0F, 0.0F, 1.0F}, {-1.0F, 0.0F, 0.0F}},
    {{-1.0F, -1.0F,  1.0F}, {0.0F, 0.0F, 1.0F}, {-1.0F, 0.0F, 0.0F}},
  }};

  constexpr std::array<unsigned int, 36> indices{{
     0,  1,  2,  2,  3,  0,
     4,  5,  6,  6,  7,  4,
     8,  9, 10, 10, 11,  8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20,
  }};

  if(!m_cubeMesh.initialize(
       vertices.data(), static_cast<int>(vertices.size()),
       indices.data(), static_cast<int>(indices.size()))) {
    m_program.removeAllShaders();
    return false;
  }

  constexpr int coneSegments = 100;
  std::vector<ModernOpenGLVertex> coneVertices;
  std::vector<unsigned int> coneIndices;
  coneVertices.reserve(2 + coneSegments * 2);
  coneIndices.reserve(coneSegments * 6);

  coneVertices.push_back(
    {{0.0F, 3.0F, 0.0F}, {1.0F, 0.0F, 0.0F}, {0.0F, 1.0F, 0.0F}});
  for(int segment = 0; segment < coneSegments; ++segment) {
    const float angle = 2.0F * std::numbers::pi_v<float>
                        * static_cast<float>(segment)
                        / static_cast<float>(coneSegments);
    const QVector3D normal =
      QVector3D(6.0F * std::cos(angle), 1.0F,
                3.0F * std::sin(angle)).normalized();
    coneVertices.push_back(
      {{std::cos(angle), -3.0F, 2.0F * std::sin(angle)},
       {1.0F, 0.0F, 0.0F}, {normal.x(), normal.y(), normal.z()}});
  }
  for(int segment = 0; segment < coneSegments; ++segment) {
    const auto current = static_cast<unsigned int>(segment + 1);
    const auto next = static_cast<unsigned int>((segment + 1) % coneSegments + 1);
    coneIndices.insert(coneIndices.end(), {0U, next, current});
  }

  const auto bottomCenter = static_cast<unsigned int>(coneVertices.size());
  coneVertices.push_back(
    {{0.0F, -3.0F, 0.0F}, {0.0F, 1.0F, 1.0F}, {0.0F, -1.0F, 0.0F}});
  const auto bottomRing = static_cast<unsigned int>(coneVertices.size());
  for(int segment = 0; segment < coneSegments; ++segment) {
    const float angle = 2.0F * std::numbers::pi_v<float>
                        * static_cast<float>(segment)
                        / static_cast<float>(coneSegments);
    coneVertices.push_back(
      {{std::cos(angle), -3.0F, 2.0F * std::sin(angle)},
       {0.0F, 1.0F, 1.0F}, {0.0F, -1.0F, 0.0F}});
  }
  for(int segment = 0; segment < coneSegments; ++segment) {
    const auto current = bottomRing + static_cast<unsigned int>(segment);
    const auto next = bottomRing
      + static_cast<unsigned int>((segment + 1) % coneSegments);
    coneIndices.insert(coneIndices.end(), {bottomCenter, current, next});
  }

  if(!m_coneMesh.initialize(
       coneVertices.data(), static_cast<int>(coneVertices.size()),
       coneIndices.data(), static_cast<int>(coneIndices.size()))) {
    m_cubeMesh.destroy();
    m_program.removeAllShaders();
    return false;
  }

  constexpr std::array<ModernOpenGLVertex, 4> imageQuadVertices{{
    {{0.0F, 0.0F, 0.0F}, {1.0F, 1.0F, 1.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 0.0F}},
    {{1.0F, 0.0F, 0.0F}, {1.0F, 1.0F, 1.0F}, {0.0F, 0.0F, 1.0F}, {1.0F, 0.0F}},
    {{1.0F, 1.0F, 0.0F}, {1.0F, 1.0F, 1.0F}, {0.0F, 0.0F, 1.0F}, {1.0F, 1.0F}},
    {{0.0F, 1.0F, 0.0F}, {1.0F, 1.0F, 1.0F}, {0.0F, 0.0F, 1.0F}, {0.0F, 1.0F}},
  }};
  constexpr std::array<unsigned int, 6> imageQuadIndices{{0, 1, 2, 2, 3, 0}};
  if(!m_imageQuadMesh.initialize(
       imageQuadVertices.data(), static_cast<int>(imageQuadVertices.size()),
       imageQuadIndices.data(), static_cast<int>(imageQuadIndices.size()))) {
    m_cubeMesh.destroy();
    m_coneMesh.destroy();
    m_program.removeAllShaders();
    m_textureProgram.removeAllShaders();
    return false;
  }

  QFile iconFile(QCoreApplication::applicationDirPath() + "/torch.3dIco");
  if(iconFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream input(&iconFile);
    QString signature;
    int sourceVertexCount = 0;
    input >> signature >> sourceVertexCount;
    std::vector<QVector3D> sourceVertices;
    sourceVertices.reserve(sourceVertexCount);
    for(int i = 0; i < sourceVertexCount; ++i) {
      float x = 0.0F;
      float y = 0.0F;
      float z = 0.0F;
      input >> x >> y >> z;
      sourceVertices.emplace_back(x, y, z);
    }

    std::vector<ModernOpenGLVertex> iconVertices;
    std::vector<unsigned int> iconIndices;
    const auto appendTriangle = [&](int first, int second, int third) {
      if(first < 0 || second < 0 || third < 0
          || first >= sourceVertexCount || second >= sourceVertexCount
          || third >= sourceVertexCount)
        return;
      const auto& a = sourceVertices[first];
      const auto& b = sourceVertices[second];
      const auto& c = sourceVertices[third];
      const auto normal = QVector3D::crossProduct(b - a, c - a).normalized();
      for(const auto& point : {a, b, c}) {
        iconIndices.push_back(static_cast<unsigned int>(iconVertices.size()));
        iconVertices.push_back(
          {{point.x(), point.y(), point.z()}, {1.0F, 1.0F, 1.0F},
           {normal.x(), normal.y(), normal.z()}});
      }
    };

    while(!input.atEnd()) {
      int vertexCount = 0;
      input >> vertexCount;
      if(input.status() != QTextStream::Ok)
        break;
      int first = 0;
      int second = 0;
      int third = 0;
      QString color;
      if(vertexCount == 3) {
        input >> first >> second >> third >> color;
        appendTriangle(first, second, third);
      } else if(vertexCount == 4) {
        int fourth = 0;
        input >> first >> second >> third >> fourth >> color;
        appendTriangle(first, second, third);
        appendTriangle(third, fourth, first);
      } else {
        qWarning() << "Unsupported torch icon polygon size:" << vertexCount;
        break;
      }
    }

    if(signature != "3DG1" || iconVertices.empty()
        || !m_lightIconMesh.initialize(
          iconVertices.data(), static_cast<int>(iconVertices.size()),
          iconIndices.data(), static_cast<int>(iconIndices.size()))) {
      qWarning() << "Could not create the modern torch icon mesh";
    }
  } else {
    qWarning() << "Could not open the torch icon mesh:" << iconFile.fileName();
  }

  m_initialized = true;
  return true;
}

void ModernOpenGLRenderer::destroy()
{
  m_cubeMesh.destroy();
  m_coneMesh.destroy();
  m_lightIconMesh.destroy();
  m_imageQuadMesh.destroy();
  m_program.removeAllShaders();
  m_textureProgram.removeAllShaders();
  m_functions = nullptr;
  m_initialized = false;
}

void ModernOpenGLRenderer::draw(
    ModernOpenGLMesh& mesh, const QMatrix4x4& projection,
    const QMatrix4x4& modelView, const QVector3D& colorMultiplier)
{
  drawPrimitive(mesh, projection, modelView, colorMultiplier,
                GL_TRIANGLES, true);
}

void ModernOpenGLRenderer::drawPrimitive(
    ModernOpenGLMesh& mesh, const QMatrix4x4& projection,
    const QMatrix4x4& modelView, const QVector3D& colorMultiplier,
    unsigned int primitiveType, bool lightingEnabled, bool depthTestEnabled)
{
  if((!m_initialized && !initialize()) || !mesh.isInitialized())
    return;

  if(depthTestEnabled)
    m_functions->glEnable(GL_DEPTH_TEST);
  else
    m_functions->glDisable(GL_DEPTH_TEST);
  m_functions->glDisable(GL_BLEND);

  m_program.bind();
  m_program.setUniformValue("modelViewProjection", projection * modelView);
  m_program.setUniformValue("normalMatrix", modelView.normalMatrix());
  m_program.setUniformValue("lightDirection1", m_lightDirections[0]);
  m_program.setUniformValue("lightDirection2", m_lightDirections[1]);
  m_program.setUniformValue("colorMultiplier", colorMultiplier);
  m_program.setUniformValue("lightEnabled1", m_lightEnabled[0] ? 1.0F : 0.0F);
  m_program.setUniformValue("lightEnabled2", m_lightEnabled[1] ? 1.0F : 0.0F);
  m_program.setUniformValue("lightingEnabled", lightingEnabled ? 1.0F : 0.0F);
  mesh.bind();
  m_functions->glDrawElements(
    primitiveType, mesh.indexCount(), GL_UNSIGNED_INT, nullptr);
  mesh.release();
  m_program.release();
  if(!depthTestEnabled)
    m_functions->glEnable(GL_DEPTH_TEST);
}

void ModernOpenGLRenderer::drawLines(
    ModernOpenGLMesh& mesh, const QMatrix4x4& projection,
    const QMatrix4x4& modelView, float width)
{
  if(!m_initialized && !initialize())
    return;
  m_functions->glLineWidth(width);
  drawPrimitive(mesh, projection, modelView, {1.0F, 1.0F, 1.0F},
                GL_LINES, false, false);
  m_functions->glLineWidth(1.0F);
}

void ModernOpenGLRenderer::drawPoints(
    ModernOpenGLMesh& mesh, const QMatrix4x4& projection,
    const QMatrix4x4& modelView, float size)
{
  if(!m_initialized && !initialize())
    return;
  ::glPointSize(size);
  drawPrimitive(mesh, projection, modelView, {1.0F, 1.0F, 1.0F},
                GL_POINTS, false, false);
  ::glPointSize(1.0F);
}

void ModernOpenGLRenderer::drawCube(
    const QMatrix4x4& projection, const QMatrix4x4& modelView)
{
  if(!m_initialized && !initialize())
    return;
  draw(m_cubeMesh, projection, modelView);
}

void ModernOpenGLRenderer::drawCone(
    const QMatrix4x4& projection, const QMatrix4x4& modelView)
{
  if(!m_initialized && !initialize())
    return;
  draw(m_coneMesh, projection, modelView);
}

void ModernOpenGLRenderer::drawTestScene(
    const QMatrix4x4& projection, const QMatrix4x4& view)
{
  const auto drawTranslatedCube = [&](const QVector3D& translation, float scale) {
    QMatrix4x4 model;
    model.translate(translation);
    model.scale(scale);
    drawCube(projection, view * model);
  };
  const auto drawTranslatedCone = [&](const QVector3D& translation, float scale) {
    QMatrix4x4 model;
    model.translate(translation);
    model.scale(scale);
    drawCone(projection, view * model);
  };

  drawTranslatedCube({-100.0F, -100.0F, -100.0F}, 30.0F);
  drawTranslatedCube({0.0F, 0.0F, -30.0F}, 30.0F);
  drawTranslatedCone({0.0F, 0.0F, -100.0F}, 20.0F);
  drawTranslatedCone({100.0F, 100.0F, 0.0F}, 30.0F);
}

void ModernOpenGLRenderer::drawLightIcon(
    const QMatrix4x4& projection, const QMatrix4x4& modelView,
    const QVector3D& colorMultiplier)
{
  if(!m_initialized && !initialize())
    return;
  draw(m_lightIconMesh, projection, modelView, colorMultiplier);
}

void ModernOpenGLRenderer::drawTexturedQuad(
    unsigned int textureId, const QMatrix4x4& projection,
    const QMatrix4x4& modelView, const QVector2D& textureScale)
{
  if((!m_initialized && !initialize()) || !m_imageQuadMesh.isInitialized())
    return;

  m_functions->glEnable(GL_DEPTH_TEST);
  m_functions->glDisable(GL_BLEND);
  m_functions->glActiveTexture(GL_TEXTURE0);
  m_functions->glBindTexture(GL_TEXTURE_2D, textureId);

  m_textureProgram.bind();
  m_textureProgram.setUniformValue("modelViewProjection", projection * modelView);
  m_textureProgram.setUniformValue("textureScale", textureScale);
  m_textureProgram.setUniformValue("imageTexture", 0);
  m_imageQuadMesh.bind();
  m_functions->glDrawElements(
    GL_TRIANGLES, m_imageQuadMesh.indexCount(), GL_UNSIGNED_INT, nullptr);
  m_imageQuadMesh.release();
  m_textureProgram.release();

  m_functions->glBindTexture(GL_TEXTURE_2D, 0);
}

void ModernOpenGLRenderer::setDirectionalLights(
    const QVector3D& firstDirection, bool firstEnabled,
    const QVector3D& secondDirection, bool secondEnabled)
{
  m_lightDirections[0] = firstDirection.normalized();
  m_lightDirections[1] = secondDirection.normalized();
  m_lightEnabled[0] = firstEnabled;
  m_lightEnabled[1] = secondEnabled;
}

bool ModernOpenGLRenderer::isInitialized() const
{
  return m_initialized;
}
