#ifndef glhelper_h
#define glhelper_h

#include <QOpenGLFunctions>
//#include "mathlib/mathutl/mymath.h"
#include "mathlib/mathutl/meshandalg.h"
//#include "mathlib/mathutl/imageandfft.h"




/*

  Compound texture: structure of rgb array  
  and meaning of x,y coordinate in 
  function Map and MapTriangle in terms of the
  rgb array.

  Mind that Y coordinate is down direction !

  
  +----0---------------------------szy---> x
  |
  0  rgb[0]            ...     rgb[szx-1]
  |  rgb[szx]          ...     rgb[2*szx-1]
  | 
  |                    . 
  |                    .
  |                    .
  |
 szy  rgb[szx*(szy-1)]  ...     rgb[szx*szy-1]      
  |
  v
  y

*/

/*
  // Example: using of texture (was kept just for memory)

  float txt[512][512][3];
  GLuint txtName;
  int i,j;
  for(i=0;i<512;i++){
    for(j=0;j<512;j++){
      txt[i][j][0]=((((i/8)%2)==((j/8)%2))?1:0);
      txt[i][j][1]=((((i/8)%2)==((j/8)%2))?0:1);
      txt[i][j][2]=((((i/8)%2)==((j/8)%2))?0.5:0.5);
    }
  }


  glGenTextures(1, &txtName); 
  glBindTexture(GL_TEXTURE_2D, txtName);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, GL_DECAL);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 
	       0, GL_RGB, GL_FLOAT, txt);


  //----------------------------------------
    glBindTexture(GL_TEXTURE_2D, txtName);

    glBegin(GL_TRIANGLES);
    glNormal3f(0,0,1);

    glTexCoord2f(2.0/512,40.0/512);
    glVertex3f(2,40,1);  
    glTexCoord2f(40.0/512,1.0/512);
    glVertex3f(40,1,1);  
    glTexCoord2f(1.0/512,1.0/512);
    glVertex3f(1,1,1);
    glEnd();
  //-----------------------------------------

*/

class CompoundTexture{
public:
  GLint maxtxs;
  GLuint texNames[1000];
  int rgbxa, rgbya;
  int txx,txy;
  int szx,szy;
  int border;  
  
  void Before();
  void After();
  void SetTexture(float *rgb,int rgbx, int rgby, int brd=1);
  void SetTestTexture(int szx=720, int szy=576);
  void MapTriangle(float x, float y);
  void Map(float x, float y);
};


class Texture{
public:
  CompoundTexture cmpdtxtr;
  float ma[2][4]; 
  void Begin();
  void End();
  void MapTriangle(Ptn x1, Ptn x2, Ptn x3);
  void Map(float x, float y, float z);
  void MapTriangle2d(float x, float y);    
  void Map2d(float x, float y);
  int Load(const char *name);

  /*
  int Load2(const char *name="test"){    
    ma[0][0]=-1.8; ma[0][1]=0; ma[0][2]=0; ma[0][3]=480;
    ma[1][0]=0; ma[1][1]=0; ma[1][2]=-1.5; ma[1][3]=510;

    StereoImg img;
    if( img.Load(name) ) return 1;

    std::vector<float> f(img.szx*img.szy*3);
    int i,j;
    for(i=0;i<img.szx;i++){
      for(j=0;j<img.szy;j++){
	float *val = img.val(i,j);
	f[(j*img.szx+i)*3+0]=0.2+(val[0]+ val[3]+val[6]+ val[9])/float(255*4);
	f[(j*img.szx+i)*3+1]=0.2+(val[1]+ val[4]+val[7]+ val[10])/float(255*4);
	f[(j*img.szx+i)*3+2]=0.2+(val[2]+ val[5]+val[8]+ val[11])/float(255*4);
      }
    }
    cmpdtxtr.SetTexture(&f[0],img.szx,img.szy,4);
    return 0;
  }
  */
};


class Surf2GL{
public:
  Surf *surf = nullptr;
  Texture *txtr = nullptr;
  MapTxtrCoord *maptxtr = nullptr;
  std::vector<Ptn> norms;

  int SetTxtr(Surf *surface,Texture *texture, MapTxtrCoord *maptexture);
  void SmoothNorms();
  void operator()();
};


class Surf2GL2: public Surf2GL{
public:
  SurfTxtr *stxr;
  int SetTxtr(Surf *surface,Texture *texture, SurfTxtr *surftx);
  void operator()();
};


float glInvMat(float *m);
Ptn glMulMat(float *m, Ptn p, float w = 1.0);
Ptn glMulMatLeft(Ptn p, float *m, float w = 1.0);

void print_matrixf(float *m,int row, int col);
void print_matrixd(double *m,int row, int col);



class GLTexHandle{
 public:
  GLuint txtid;

  GLTexHandle():txtid(0){}
  ~GLTexHandle(){ clear(); }

  void clear(){
    if(txtid){ 
      glDeleteTextures(1,&txtid);
      txtid=0;
    }
  }

  GLuint operator()(){
    if(!txtid){
      glGenTextures(1,&txtid);
    }
    return txtid;  
  }
};

class GLTex2D: public GLTexHandle{
 public:
  GLTex2D(){}
};


class GLListHandle{
 public:
  GLuint id;

  GLListHandle():id(0){}
  ~GLListHandle(){ clear(); }

  void clear(){
    if(id){ 
      glDeleteLists(id,1);
      id=0;
    }
  }

  GLuint operator()(){
    if(!id) id = glGenLists(1);
    return id;  
  }
};


#endif
