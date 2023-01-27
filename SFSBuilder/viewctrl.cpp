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
   :m_scale(1)
   ,background(0)
   ,prjtype(0)
   ,play_method(0)
{
  // initialize model view matrix as identity matrix 
  updateModelViewMtrx();
}


// STATE AND SERIALIZATION

void ViewCtrl::AskForData(Serializer *s){
  if(s->ss->storageid==SRLZ_LAYOUT){
    //s->Item("InitialModelMatrix",Sync(InitialModelMatrix,16));
    s->Item("prjtype",Sync(&prjtype));
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

glm::mat4 const& ViewCtrl::getSelectionMtrx(){
  return m_Ps;
}

glm::mat4 const& ViewCtrl::updateProjectionMtrx(int w, int h){
  m_w = w; m_h = h;
  return updateProjectionMtrx();
}

glm::mat4 const& ViewCtrl::updateProjectionMtrx(){

  prjtype = 1;
  float nearPlane = 100.f;  //near plane
  float farPlane =  30000.f;  //far plane

  switch(prjtype){
  case 0: //orthogonal
      m_P = glm::ortho<float>(-0.5*m_w, 0.5*m_w,-0.5*m_h, 0.5*m_h, nearPlane, farPlane);
      break;
  case 1: //perspective
      {
        float aspect = float(m_w)/m_h;
        m_P = glm::perspective(m_fovY, aspect, nearPlane, farPlane);
      }

    break;
  }
  return m_P;
}

glm::mat4 const& ViewCtrl::updateSelectionMtrx(float gx, float gy){


  m_Ps = glm::scale(glm::mat4{1.0},{m_w/2,m_h/2,1.})*
      glm::translate(glm::mat4{1.0}, {-gx,-gy,0.})*m_P;

//use for debugging object selection (instant zoom on every second mouse click)
#if !NDEBUG && off
  {
    bool static useSelectMatrix = false;
    if(useSelectMatrix)
      m_P = m_Ps;
    else
      updateProjectionMtrx();
    useSelectMatrix = !useSelectMatrix;
  }
#endif

  return m_Ps;
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
    glLoadMatrixf(glm::value_ptr(getSelectionMtrx()));
    break;
  }

}



// ZOOMING, ROTATION AND SHIFT PROCESSING
std::function<void(float x, float y)>
ViewCtrl::startOperation(ViewCtrl::Opercode opercode, float x, float y){
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
        m_trans = transOld + dir * float(3 * dy);
        updateModelViewMtrx();
      };

      return [this, zoomOld=m_scale, oldY=y](float x, float y)
      {
        float dy = (oldY-y);
        m_scale = zoomOld*exp(3 * dy);
        updateModelViewMtrx();
      };
      break;
    case FoV:
      return [this, fovYOld=m_fovY, oldY=y](float x, float y)
      {
        float dy = (oldY-y);
        m_fovY = std::clamp(fovYOld*expf(3 * dy),glm::radians(5.f),glm::radians(70.f));
        updateProjectionMtrx();
      };
      break;

  }
  return {};
}

void ViewCtrl::reset(){
  m_scale = 1.;
  m_rot = {1.0,.0,.0,.0};
  m_trans = {.0,.0,-700};
  updateModelViewMtrx();
}

