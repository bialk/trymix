#ifndef GLImageTile_h
#define GLImageTile_h

#include <vector>
#include <QOpenGLExtraFunctions>
#include <QPointer>

class DrawCntx;
class QOpenGLContext;

class GLTexSetHandle{
 public:
  std::vector<GLuint> texid;

  GLTexSetHandle() = default;
  ~GLTexSetHandle();
  GLTexSetHandle(const GLTexSetHandle&) = delete;
  GLTexSetHandle& operator=(const GLTexSetHandle&) = delete;

  void clear();
  bool resize(unsigned int size);

 private:
  bool owningContextIsCurrent() const;
  QPointer<QOpenGLContext> m_context;
};


class ImageTile{
 public:

  int tile_size{0};
  int w{0},h{0};
  int ntilew{0},ntileh{0};
  
  GLTexSetHandle texset;

  void clear();
  int empty();
  void LoadBGRA(const unsigned char* bgra, int ww, int hh);
  void Draw(DrawCntx* context);
};

#endif
