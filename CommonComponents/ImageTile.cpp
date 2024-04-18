#include "ImageTile.h"
//#include <GL/glext.h>
//#include "eventlog.h"
#include <QOpenGLExtraFunctions>
#include <string.h>
#include <algorithm>

inline int align_up(int x, int a){
   int r = x%a;
   return r?(x + a-r):x;
}

void ImageTile::clear(){
  texset.clear();
}
int ImageTile::empty(){
  return texset.texid.empty();
}

void ImageTile::LoadBGRA(unsigned char* bgra, int ww, int hh){  

  w=ww; h=hh;

  glGetError();
  //glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB, &tile_size);
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tile_size);
  GLuint err = glGetError();
  if(err!=GL_NO_ERROR) {
//    err_printf(("GL_TEXTURE_RECTANGLE_ARB is not supported"
//		" for this Graphics Card"));
    clear(); 
    return; 
  }

  //printf("maximum texture size=%i\n",tile_size);
  tile_size=std::min<int>(tile_size,256);
  ntilew = align_up(w,tile_size)/tile_size;
  ntileh = align_up(h,tile_size)/tile_size;

  texset.resize(ntilew*ntileh);
  //printf("texw=%i\n",ntilew);
  //printf("ntileh=%i\n",ntileh);
  
  std::vector<unsigned char> buf(tile_size*w*4);

  glPixelStorei(GL_UNPACK_ROW_LENGTH,w);

  int i,j;
  for(i=0;i<ntileh;i++)
    for(j=0;j<ntilew;j++){
      int tw=std::min<int>(tile_size,w-j*tile_size);
      int th=std::min<int>(tile_size,h-i*tile_size);
      
      /*
      glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texset.texid[ntilew*i+j]);
      glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, 
		      GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
      glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, 
		      GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
      glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, 
		      GL_TEXTURE_WRAP_S, GL_CLAMP); 
      glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, 
		      GL_TEXTURE_WRAP_T, GL_CLAMP); 
      glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_RGBA, tw, th, 0, GL_BGRA,
		   GL_UNSIGNED_BYTE,
		   (GLvoid*)(bgra+(i*tile_size*w+j*tile_size)*4));
      */
      int k=w*h-(i*tile_size*w+j*tile_size);
      memcpy(&buf[0],bgra+(i*tile_size*w+j*tile_size)*4,std::min<int>(k,w*th)*4);

      glBindTexture(GL_TEXTURE_2D, texset.texid[ntilew*i+j]);
      glTexParameteri(GL_TEXTURE_2D, 
		      GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
      glTexParameteri(GL_TEXTURE_2D, 
		      GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
      glTexParameteri(GL_TEXTURE_2D, 
		      GL_TEXTURE_WRAP_S, GL_CLAMP); 
      glTexParameteri(GL_TEXTURE_2D, 
		      GL_TEXTURE_WRAP_T, GL_CLAMP); 
      glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, tile_size, tile_size, 0, GL_BGRA,
		   GL_UNSIGNED_BYTE,
		   (GLvoid*)(&buf[0]));
    }

  glBindTexture(GL_TEXTURE_2D,0);//RECTANGLE_ARB, 0);
  glPixelStorei(GL_UNPACK_ROW_LENGTH,0);      
}


void ImageTile::Draw(){

  if(empty()) return;

  glEnable(GL_TEXTURE_2D);//RECTANGLE_ARB);
  int i,j;
  for(i=0;i<ntileh;i++){
    for(j=0;j<ntilew;j++){
      float tw=std::min(tile_size,w-j*tile_size);
      float th=std::min(tile_size,h-i*tile_size);

      /*	  
      glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texset.texid[ntilew*i+j]);
      float w0=j*tile_size;
      float h0=i*tile_size;

      glColor4f(1,1,1,1);
      glBegin (GL_QUADS);  
      glTexCoord2f (0.0, 0.0);
      glVertex3f (w0, h0, 0.0);
      glTexCoord2f (tw, 0.0);
      glVertex3f (w0+tw, h0, 0.0);
      glTexCoord2f (tw, th);
      glVertex3f (w0+tw, h0+th, 0.0);
      glTexCoord2f (0.0, th);
      glVertex3f (w0, h0+th, 0.0);
      glEnd ();
      */

      glBindTexture(GL_TEXTURE_2D, texset.texid[ntilew*i+j]);
      float w0=j*tile_size;
      float h0=i*tile_size;

      glColor4f(1,1,1,1);
      glBegin (GL_QUADS);  
      glTexCoord2f (0.0, 0.0);
      glVertex3f (w0, h0, 0.0);
      glTexCoord2f (tw/tile_size, 0.0);
      glVertex3f (w0+tw, h0, 0.0);
      glTexCoord2f (tw/tile_size, th/tile_size);
      glVertex3f (w0+tw, h0+th, 0.0);
      glTexCoord2f (0.0, th/tile_size);
      glVertex3f (w0, h0+th, 0.0);
      glEnd ();

    }  
  }
  glBindTexture(GL_TEXTURE_2D,0);//RECTANGLE_ARB, 0);
  glDisable(GL_TEXTURE_2D);//RECTANGLE_ARB);
}


