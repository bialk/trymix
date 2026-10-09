#include "ImageTile.h"
#include "drawContext.h"
#include "ModernOpenGLRenderer.h"
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QDebug>
#include <QMatrix4x4>
#include <algorithm>

GLTexSetHandle::~GLTexSetHandle()
{
  clear();
}

bool GLTexSetHandle::owningContextIsCurrent() const
{
  QOpenGLContext* current = QOpenGLContext::currentContext();
  return current && m_context
    && (current == m_context
        || current->shareGroup() == m_context->shareGroup());
}

bool GLTexSetHandle::isValidForCurrentContext() const
{
  return !texid.empty() && owningContextIsCurrent();
}

void GLTexSetHandle::clear()
{
  if(texid.empty())
    return;

  if(owningContextIsCurrent()) {
    QOpenGLContext::currentContext()->functions()->glDeleteTextures(
      static_cast<GLsizei>(texid.size()), texid.data());
  }
  // If the owning context is not current, issuing glDeleteTextures would be
  // unsafe. Dropping the names is still correct: their storage is reclaimed
  // when the owning context (or its share group) is destroyed.
  texid.clear();
  m_context.clear();
}

bool GLTexSetHandle::resize(unsigned int size)
{
  if(texid.size() == size && owningContextIsCurrent())
    return true;

  clear();
  QOpenGLContext* current = QOpenGLContext::currentContext();
  if(!current) {
    qWarning() << "Cannot create image textures without a current OpenGL context";
    return false;
  }

  m_context = current;
  texid.resize(size);
  current->functions()->glGenTextures(
    static_cast<GLsizei>(texid.size()), texid.data());
  return true;
}

void ImageTile::clear(){
  texset.clear();
  tile_size = 0;
  w = h = 0;
  ntilew = ntileh = 0;
}
int ImageTile::empty(){
  return texset.texid.empty();
}
bool ImageTile::isValidForCurrentContext() const{
  return texset.isValidForCurrentContext();
}

void ImageTile::LoadBGRA(const unsigned char* bgra, int ww, int hh){
  clear();
  if(!bgra || ww <= 0 || hh <= 0)
    return;

  QOpenGLContext* context = QOpenGLContext::currentContext();
  if(!context) {
    qWarning() << "Cannot upload an image without a current OpenGL context";
    return;
  }

  GLint maximumTextureSize = 0;
  QOpenGLFunctions* functions = context->functions();
  functions->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maximumTextureSize);
  if(maximumTextureSize <= 0) {
    qWarning() << "Unable to determine the maximum OpenGL texture size";
    return;
  }

  w = ww;
  h = hh;
  tile_size = maximumTextureSize;
  ntilew = (w + tile_size - 1) / tile_size;
  ntileh = (h + tile_size - 1) / tile_size;
  if(!texset.resize(static_cast<unsigned int>(ntilew * ntileh))) {
    clear();
    return;
  }

  GLint previousRowLength = 0;
  GLint previousAlignment = 0;
  functions->glGetIntegerv(GL_UNPACK_ROW_LENGTH, &previousRowLength);
  functions->glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
  functions->glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
  functions->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for(int row = 0; row < ntileh; ++row) {
    for(int column = 0; column < ntilew; ++column) {
      const int x = column * tile_size;
      const int y = row * tile_size;
      const int tileWidth = std::min(tile_size, w - x);
      const int tileHeight = std::min(tile_size, h - y);
      const auto* tilePixels = bgra + 4LL * (y * static_cast<long long>(w) + x);

      functions->glBindTexture(
        GL_TEXTURE_2D, texset.texid[ntilew * row + column]);
      functions->glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      functions->glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      functions->glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      functions->glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      functions->glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8, tileWidth, tileHeight, 0,
        GL_BGRA, GL_UNSIGNED_BYTE, tilePixels);
    }
  }

  functions->glBindTexture(GL_TEXTURE_2D, 0);
  functions->glPixelStorei(GL_UNPACK_ROW_LENGTH, previousRowLength);
  functions->glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);
}


void ImageTile::Draw(DrawCntx* context){

  if(empty()) return;

  const QMatrix4x4& projection = context->projectionMatrix();
  const QMatrix4x4& baseModelView = context->modelViewMatrix();

  int i,j;
  for(i=0;i<ntileh;i++){
    for(j=0;j<ntilew;j++){
      float tw=std::min(tile_size,w-j*tile_size);
      float th=std::min(tile_size,h-i*tile_size);

      float w0=j*tile_size;
      float h0=i*tile_size;

      QMatrix4x4 tileModel;
      tileModel.translate(w0, h0, 0.0F);
      tileModel.scale(tw, th, 1.0F);
      context->modernRenderer().drawTexturedQuad(
        texset.texid[ntilew*i+j], projection, baseModelView * tileModel,
        {1.0F, 1.0F});

    }  
  }
}
