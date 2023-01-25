#include "viewctrl.h"
#include "apputil/serializer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/geometric.hpp>

#include <fstream>
#include <math.h>

#include <QOpenGLFunctions>
namespace {
class GLAnimator{
public:
  virtual void Init()=0;
  virtual int Step()=0;  
} *glanimator=0;

class Rotation360a: public GLAnimator{
  int play;
  virtual void Init(){ play=100; };
  virtual int Step(){
    if( play > 0 ) {    
      glRotatef(360.0*(101-play)/100,0,1,0);
      play--;
    }
    return play;
  };  
} rot360a;

class Rotation360b: public GLAnimator{
  int play;
  virtual void Init(){ play=100; };
  virtual int Step(){
    if( play > 0 ) {    
      glRotatef(360.0*(101-play)/100,0,1,0.5);
      play--;
    }
    return play;
  };  
} rot360b;

class LeftRight: public GLAnimator{
  int play;
  virtual void Init(){ play=100; };
  virtual int Step(){
    int i;
    if(play>50) i=play-50;
    else i=50-play;
    glRotatef(45.0*(i-25)/50,0,1,0);
    play-=2;
    return play;
  };  
} leftright;
} //end of namespace

int play_method;
static class GLAnimator *animators[]={&rot360a,&rot360b,&leftright};

//========================  UI control routines  ===========================

void ViewCtrl::fplay(){
  if(glanimator!=0) { 
    glanimator=0;
  }
  else { 
    glanimator=animators[play_method]; 
    glanimator->Init();
  }
}


ViewCtrl::~ViewCtrl()
{
}

ViewCtrl::ViewCtrl()
   :m_zoom(1)
   ,play(0)
   ,background(0)
   ,prjtype(0)
   ,drawsimple(1)
   ,glroshow(0)
   ,play_method(0)
{
  // initialize model view matrix as identity matrix 
  updateModelViewMtrx();
}


// STATE AND SERIALIZATION

void ViewCtrl::AskForData(Serializer *s){
  if(s->ss->storageid==SRLZ_LAYOUT){
    //s->Item("InitialModelMatrix",Sync(InitialModelMatrix,16));
    s->Item("glroshow",Sync(&glroshow));
    s->Item("zoom",Sync(&m_zoom));
    s->Item("prjtype",Sync(&prjtype));
    s->Item("drawsimple",Sync(&drawsimple));
    //s->Item("show",Sync(&dv->toolpanel->show));
  }
}

void ViewCtrl::TreeScan(TSOCntx *cntx){
  if(cntx==&TSOCntx::TSO_LayoutLoad){
    //initialize inverse model view matrix and variables;
  }
  if(cntx==&TSOCntx::TSO_Init){
  }
}

glm::mat4 const& ViewCtrl::getModelViewMtrx(){
  return m_MV;
}

glm::mat4 const& ViewCtrl::updateModelViewMtrx(){
//  m_MV = glm::translate(glm::scale(glm::mat4(1.0),glm::vec3(zoom,zoom,zoom))*
//         glm::toMat4(m_rot),m_trans);
  m_MV = glm::toMat4(m_rot)*glm::translate(glm::mat4{1.0},m_trans);
  return m_MV;
}

glm::mat4 const& ViewCtrl::getProjectionMtrx(){
  return m_P;
}

glm::mat4 const& ViewCtrl::getProjectionSelectMtrx(){
  return m_Ps;
}

glm::mat4 const& ViewCtrl::updateProjectionMtrx(int w, int h, int x, int y){

  prjtype = 1;
  float nearPlane = 100.f;  //near plane
  float farPlane =  30000.f;  //far plane

  switch(prjtype){
  case 0: //orthogonal
      //P = glm::scale(glm::mat4{1.0}, {2.0/w, 2.0/h, -1./(farPlane-nearPlane)});
      m_P = glm::ortho<float>(-0.5*w, 0.5*w,-0.5*h, 0.5*h, nearPlane, farPlane);
      break;

  case 1: //perspective
      {
//        float k=400; // focal distance

//        m_P = glm::translate(
//            glm::frustum<float>(-w/2,w/2,-h/2,h/2,k,k+300),
//                                          {0,0,-k-300/2});

//        qDebug() << m_P[0][0] << m_P[0][1] << m_P[0][2] << m_P[0][3] <<
//                    m_P[1][0] << m_P[1][1] << m_P[1][2] << m_P[1][3] <<
//                    m_P[2][0] << m_P[2][1] << m_P[2][2] << m_P[2][3] <<
//                    m_P[3][0] << m_P[3][1] << m_P[3][2] << m_P[3][3];

        // perspective matrix looking in
        // z direction (from the camera), y to the up, x to the right
        m_P = { nearPlane*2.0/w, .0, .0, .0,
                .0, nearPlane*2.0/h, .0, .0,
                .0, .0, -(nearPlane+farPlane)/(farPlane-nearPlane), -1,
                .0, .0, -2*nearPlane*farPlane/(farPlane-nearPlane), 0
        };

        float aspect = float(w)/h;
        m_P = glm::perspective(m_fovY, aspect, nearPlane, farPlane);

        // translate camera position to the origin (center of the near plane is origin)
        //m_P = glm::translate(m_P,{0.,0.,-fd});

//        qDebug() << m_P[0][0] << m_P[0][1] << m_P[0][2] << m_P[0][3] <<
//                    m_P[1][0] << m_P[1][1] << m_P[1][2] << m_P[1][3] <<
//                    m_P[2][0] << m_P[2][1] << m_P[2][2] << m_P[2][3] <<
//                    m_P[3][0] << m_P[3][1] << m_P[3][2] << m_P[3][3];

        //m_Ps = glm::scale(glm::mat4{1},{w/2,h/2,1.})*glm::translate(glm::mat4{1.0}, {-wx,-wy,0.})*m_P;

      }

    break;
  }
  {
    auto wx=2*(x-w/2.)/w;
    auto wy=2*(h/2. -y)/h;
    m_Ps = glm::scale(glm::mat4{1.0},{w/2,h/2,1.})*
        glm::translate(glm::mat4{1.0}, {-wx,-wy,0.})*m_P;
  }


#if NDEBUG && off //user for debugging object selection (instant zoom on every second mouse click)
  {
    bool static useSelectMatrix = false;
    if(useSelectMatrix)
      m_P = m_Ps;
    useSelectMatrix = !useSelectMatrix;
  }
#endif

  return m_P;
}

void ViewCtrl::Draw(DrawCntx *cntx){

  glClearDepth(1.0);
  if(background==0)
    glClearColor(.0, .0, .0, 0.0);
  else
    glClearColor(1.0, 1.0, 1.0, 0.0);  

  glClear(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT |
          GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );  

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if(glanimator && (glanimator->Step()>0)) {
    cntx->update();
  }else{
    glanimator=0;
  }

  glMultMatrixf(glm::value_ptr(getModelViewMtrx()));

  glMatrixMode(GL_PROJECTION);
  GLint mode;
  glGetIntegerv(GL_RENDER_MODE,&mode);
  switch (mode){
  case GL_RENDER:
    glLoadMatrixf(glm::value_ptr(getProjectionMtrx()));
    break;
  case GL_SELECT:
    glLoadMatrixf(glm::value_ptr(getProjectionSelectMtrx()));
    break;
  }

}



// ZOOMING, ROTATION AND SHIFT PROCESSING
std::function<void(float x, float y)>
ViewCtrl::movestart(ViewCtrl::Opercode opercode, float x, float y){
  switch(opercode){
  case Translate:
      return [this, oldX=x, oldY=y, transOld = m_trans](float x, float y)
      {
          auto mi = glm::inverse(m_P*glm::toMat4(m_rot));
          auto from = mi*glm::vec4{oldX, oldY, 0, 1};
          from /= from[3];
          auto to = mi*glm::vec4{x, y, 0., 1.};
          to /= to[3];

          m_trans = transOld + glm::vec3(to) - glm::vec3(from);
          updateModelViewMtrx();
      };
      break;
  case Rotate:
      return [this, oldX=x, oldY=y, rotOld=m_rot](float x, float y)
      {
        auto mi = glm::inverse(m_P);
        auto from = glm::vec3(mi*glm::vec4{oldX, oldY, 0, 1});
        auto to = glm::vec3(mi*glm::vec4{x, y, 0., 1.});
        if(glm::distance(glm::vec2(oldX,oldY),{0.,0.}) > 0.8f){
          from[2]=to[2]=0;
        }
        auto rot = glm::rotation(glm::normalize(from),glm::normalize(to));
        m_rot = rot * rotOld;
        updateModelViewMtrx();
      };

      break;
  case Scale:
      return [this, transOld=m_trans, oldY=y](float x, float y)
      {
        auto mi = glm::inverse(m_P*glm::toMat4(m_rot));
        auto dir4 = mi*glm::vec4{0, 0, 0, 1};
        auto dir = glm::vec3(dir4/dir4[3]);

        float dy = (oldY-y);
        m_trans = transOld + dir * float(-3 * dy);
        updateModelViewMtrx();
      };

      return [this, zoomOld=m_zoom, oldY=y](float x, float y)
      {
        int dy = (oldY-y);
        m_zoom = zoomOld*exp(-3 * dy);
        updateModelViewMtrx();
      };
      break;
  }
  return {};
}

void ViewCtrl::reset(){
  m_zoom = 1.;
  m_rot = {1.0,.0,.0,.0};
  m_trans = {.0,.0,-700};
  updateModelViewMtrx();
}

