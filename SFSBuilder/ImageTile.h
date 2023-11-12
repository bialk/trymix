#ifndef GLImageTile_h
#define GLImageTile_h

#include <vector>
#include <QOpenGLExtraFunctions>

class GLTexSetHandle{
 public:
  std::vector<GLuint> texid;

  ~GLTexSetHandle(){
    clear();
  }
  void clear(){
    if(!texid.empty()){
      glDeleteTextures((GLsizei)texid.size(),texid.data());
      texid.clear();
    }
  }
  void resize(unsigned int i){
    if(texid.size()!=i){
      clear();
      texid.resize(i);
      glGenTextures((GLsizei)texid.size(),texid.data());
    }
  }
};


class ImageTile{
 public:

  int tile_size;
  int w,h;
  int ntilew,ntileh;
  
  GLTexSetHandle texset;

  void clear();
  int empty();
  void LoadBGRA(unsigned char* bgra, int ww, int hh);
  void Draw();
};

#endif
