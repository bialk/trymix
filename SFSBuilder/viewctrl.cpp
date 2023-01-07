#include "SFSBuilder/mathlib/lapackcpp/lapackcpp.h"
#include "dispview.h"
#include "CentralWidget.h"
#include <fstream>
#include <math.h>
#include "mathlib/mathutl/mymath.h"
#include "viewctrl.h"
#include "toolspanel.h"
#include "glhelper.h"
#include "apputil/serializer.h"
#include "apputil/eventball.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


static class GLAnimator{
public:
  virtual void Init()=0;
  virtual int Step()=0;  
} *glanimator=0;

static class Rotation360a: public GLAnimator{
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

static class Rotation360b: public GLAnimator{
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


static class LeftRight: public GLAnimator{
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
   :zoom(1)
   ,play(0)
   ,background(0)
   ,prjtype(0)
   ,drawsimple(1)
   ,glroshow(0)
   ,play_method(0)
   ,viewctrleh(this)
{
  // initialize default parameters
  mssh.m =PrjMtrxRender;
  mssh.shift_y = mssh.shift_x = mssh.shift_z = 0;
  mssh.oper = 0;
  mssh.wndw = mssh.wndh = -1;
  mssh.angle_r = mssh.norm_rx = mssh.norm_ry = 0; mssh.norm_rz=1;

  // initialize model view matrix as identity matrix 
  int i;
  for(i=0; i<16; i++) InitialModelMatrix[i]=(i%5)?0.0:1.0;
  memcpy(InvMVMat,InitialModelMatrix,sizeof(InvMVMat));
  glInvMat(InvMVMat);
}


// STATE AND SERIALIZATION

void ViewCtrl::AskForData(Serializer *s){
  if(s->ss->storageid==SRLZ_LAYOUT){
    s->Item("InitialModelMatrix",Sync(InitialModelMatrix,16));
    s->Item("glroshow",Sync(&glroshow));
    s->Item("zoom",Sync(&zoom));
    s->Item("prjtype",Sync(&prjtype));
    s->Item("drawsimple",Sync(&drawsimple));
    //s->Item("show",Sync(&dv->toolpanel->show));
  }
}

void ViewCtrl::TreeScan(TSOCntx *cntx){
  if(cntx==&TSOCntx::TSO_LayoutLoad){
    // initialize inverse model view matrix and variables;
    memcpy(InvMVMat,InitialModelMatrix,sizeof(InvMVMat));
    glInvMat(InvMVMat);
  }
  if(cntx==&TSOCntx::TSO_Init){
    //dv->toolpanel->show=1;
  }
}

void ViewCtrl::SetProjectionMatrix(){

  switch(prjtype){
    float x,y;
  case 0: //orthogonal
      {
        glm::mat4 orthov = glm::scale<float>(
            glm::ortho<float>(-mssh.wndw/2,mssh.wndw/2, -mssh.wndh/2,mssh.wndh/2, -300,300),
            {zoom,zoom,1});
        memcpy(PrjMtrxRender, glm::value_ptr(orthov),sizeof(PrjMtrxRender));
      }

      {
        x=(mssh.sx0-mssh.wndw/2)/zoom;
        y=(mssh.wndh/2-mssh.sy0)/zoom;
        glm::mat4 orthov = glm::scale<float>(
            glm::frustum<float>(-5+x,5+x,-5+y,5+y,-300,300),
                                            { zoom,zoom,1});
        memcpy(PrjMtrxSelect, glm::value_ptr(orthov),sizeof(PrjMtrxSelect));
      }

      break;
    
  case 1: //frustrum

    float k=2500; // distance from camera to the centre of the object

      {
        glm::mat4 orthov = glm::translate(glm::scale<float>(
            glm::frustum<float>(-mssh.wndw/2,mssh.wndw/2,-mssh.wndh/2,mssh.wndh/2,-300+k,300+k),
            { zoom*(k/(k-300)),zoom*(k/(k-300)),1}),
                                          {0,0,-k});
        memcpy(PrjMtrxRender, glm::value_ptr(orthov),sizeof(PrjMtrxRender));
      }

      {
        x=(mssh.sx0-mssh.wndw/2)/(zoom*(k/(k-300)));
        y=(mssh.wndh/2-mssh.sy0)/(zoom*(k/(k-300)));
        glm::mat4 orthov = glm::translate(glm::scale<float>(
            glm::frustum<float>(-5+x,5+x,-5+y,5+y,-300+k,300+k),
            { zoom*(k/(k-300)),zoom*(k/(k-300)),1}),
                                          {0,0,-k});
        memcpy(PrjMtrxSelect, glm::value_ptr(orthov),sizeof(PrjMtrxSelect));
      }

    break;
  }

  // calculate inverse matrix  
  memcpy(
    InvMVMat,
    glm::value_ptr(glm::make_mat4(PrjMtrxRender)*glm::inverse(glm::make_mat4(InitialModelMatrix))),
    sizeof(InvMVMat));
}

void ViewCtrl::hitscene(float &x, float &y, float &z){  

  SetProjectionMatrix();
  glReadPixels(x,mssh.wndh-y,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&z);
  //printf("before: px=%f py=%f pz=%f\n", x,y,z);
  BackScreenTransform(x,y,z);
  //printf("scene point:  px=%f py=%f pz=%f\n", x,y,z);
}

void ViewCtrl::BackScreenTransform(float &x, float &y, float &z){

  // see glspec14.pdf section 2.10.1 "Controling Viewport"

  float cx = 2.*float(x-mssh.wndw/2)/mssh.wndw;
  float cy = 2.*float(mssh.wndh-y-mssh.wndh/2)/mssh.wndh;
  float cz = z*2-1;

  // taking into consideration that 
  //      -1            -1            -1            -1
  // 1 = m (4,1)*x*w + m [4,2]*y*w + m [4,3]*z*w + m [4,4]*w
  // we have that w can be obtained as:
  //           -1          -1          -1          -1
  // w = 1 / (m (4,1)*x + m [4,2]*y + m [4,3]*z + m [4,4])
  // 

  double w = 1.0/(InvMVMat[3]*cx + InvMVMat[7]*cy +
		  InvMVMat[11]*cz + InvMVMat[15]);

  Ptn p = { cx*w, cy*w, cz*w };
  p = glMulMat(InvMVMat,p,w);
	  
  x = p.x; y = p.y;  z = p.z; 

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

  glRotatef(mssh.angle_r,mssh.norm_rx,mssh.norm_ry,mssh.norm_rz);
  
  // shift control
  glTranslated(mssh.shift_x,mssh.shift_y,mssh.shift_z);
  glMultMatrixf(InitialModelMatrix);

  glMatrixMode(GL_PROJECTION);
  GLint mode;
  glGetIntegerv(GL_RENDER_MODE,&mode);
  switch (mode){
  case GL_RENDER:
    glLoadMatrixf(PrjMtrxRender);
    break;
  case GL_SELECT:
    glLoadMatrixf(PrjMtrxSelect);
    break;
  }

}



// ZOOMING, ROTATION AND SHIFT PROCESSING

void ViewCtrl::Zoom(float r){
    if(r != 1.0)  zoom *= r;
    else          zoom = 1;
    SetProjectionMatrix();
}

void ViewCtrl::movestart(int oper, int x, int y){
  opercode=oper;
  switch(oper){
  case 1:
      //dv->drawsimple=drawsimple;
      mssh.StartMove(x,y);  
      break;
  case 2:
      //dv->drawsimple=drawsimple;
      mssh.StartRotate(x,y);
      break;
  case 3:
      //dv->drawsimple=drawsimple;
      zoom0=zoom; zoomy=y;
      break;
  }
}


void ViewCtrl::movecont(int x, int y){
  if(opercode==3){
    int dy = (zoomy-y);
    zoom = zoom0*(250+dy)/250.;
    SetProjectionMatrix();
  }
  else{
    mssh.ContinueMOper(x,y);
  }
}

void ViewCtrl::movestop(int x, int y){
  memcpy(InitialModelMatrix,
      glm::value_ptr(
           glm::rotate<float>(glm::mat4(1.0), mssh.angle_r*glm::pi<float>()/180., {mssh.norm_rx,mssh.norm_ry,mssh.norm_rz})*
           glm::translate<float>(glm::mat4(1.0),{mssh.shift_x,mssh.shift_y,mssh.shift_z})*
           glm::make_mat4(InitialModelMatrix)),
      sizeof(InitialModelMatrix));

//  auto m =
//        glm::translate<float>(
//        glm::rotate<float>(
//          glm::mat4(1.0),
//               glm::pi<float>()*mssh.angle_r/180., {mssh.norm_rx,mssh.norm_ry,mssh.norm_rz}),
//               {mssh.shift_x,mssh.shift_y,mssh.shift_z})*
//      glm::make_mat4(InitialModelMatrix);
//  memcpy(InitialModelMatrix, glm::value_ptr(m), sizeof(InitialModelMatrix));

  memcpy(InvMVMat, glm::value_ptr(glm::inverse(glm::make_mat4(InitialModelMatrix))), sizeof(InvMVMat));

  mssh.StopMOper();
  opercode=0;
}

void ViewCtrl::reset(){
  Zoom(1.0);
  int i;
  for(i=0; i<16; i++) InitialModelMatrix[i]=(i%5)?0.0:1.0;
  memcpy(InvMVMat,InitialModelMatrix,sizeof(InvMVMat));
  glInvMat(InvMVMat);
}


// HITS PROCESSING 
#ifdef off
void ViewCtrl::SelectObj2(int x, int y) { 
  mssh.sx0=x;mssh.sy0=y;
  SetProjectionMatrix();
  glSelectBuffer (1024, selectBuf); 
  glRenderMode (GL_SELECT); 
  glInitNames(); 
  //dv->Draw(0);
  hits = glRenderMode (GL_RENDER);
  if(hits==-1){
  	printf("ViewCtrl::SelectObj2 \"Selection Buffer overflow\"\n");
    hits=0;
  }
}

/* processHits prints out the contents of the * selection array. */ 
int ViewCtrl::ProcessHits2 (unsigned int stackdepth, unsigned int* stacknames) {

  int i;
  unsigned int j;
  float maxz1=1e10;
  GLuint maxname = 0;
  GLuint names, name, *ptr; 
  //printf ("hits = %d\n", hits); 
  ptr = (GLuint *) selectBuf; 
  for (i = 0; i < hits; i++) { 
    /* for each hit */ 
    names = *ptr; //printf (" number of names for this hit = %d\n", names); 
    ptr++; 
    float z1=(float) *ptr/0x7fffffff;
    //printf(" z1 is %g;", (float) *ptr/0x7fffffff); 
    ptr++; 
    //printf(" z2 is %g\n", (float) *ptr/0x7fffffff); 
    ptr++; 
    //printf (" names are "); 
    int count = stackdepth+1;
    int depthname;
    for (j = 0; j < names; j++) { 
      /* for each name */ 
      name = *ptr;
      if( j<stackdepth) {if(stacknames[j]==name) count--;}
      else if(j==stackdepth) { depthname=name; count--;}				 
      //printf ("%d ", name);       
      ptr++;
    }
    if(count==0 && z1<maxz1 ){
      maxname = depthname; maxz1=z1;    
    }  
    //printf ("\n"); 
  } 
  //printf("maxname=%i\n",maxname);
  return maxname;
}


int ViewCtrl::SelectObj(int x, int y){
  unsigned int stack[]={0};
  SelectObj2(x,y);
  return ProcessHits2(0, stack);
}
#endif


// class ViewCtrlEH
// ===================================================================


ViewCtrlEH::ViewCtrlEH(ViewCtrl*v):vc(v),state_drag(-1){}


void ViewCtrlEH::Handle(EventBall *eventball){
#ifdef off

  if(eventball->state(-34)){
    int x=eventball->x;
    int y=eventball->y;
    //glViewport(0,0,x,y);

    vc->mssh.wndw=x; vc->mssh.wndh=y;
    vc->SetProjectionMatrix();
  }
  else if(eventball->event(M_DRAG) && 
	  eventball->state(state_drag)){
    vc->movecont(eventball->x,eventball->y);
    vc->dv->redraw();
    eventball->stop();
  }
  else if(eventball->event(M_UP) &&
	  eventball->state(state_drag)){
    eventball->setstate(0);
    vc->movestop(eventball->x,eventball->y);
    eventball->stop();
  }
  else if(!eventball->state(0)){
    // move event futher down
    return;
  }
  else if(eventball->event(M_DOWN) && 
	  eventball->keys("c") && 
	  eventball->mbtn(1)){
    eventball->genstate(state_drag);
    vc->movestart(2,eventball->x,eventball->y);
    eventball->stop();
  }
  else if(eventball->event(M_DOWN) && 
	  eventball->keys("x") && 
	  eventball->mbtn(1)){
    eventball->genstate(state_drag);
    vc->movestart(1,eventball->x,eventball->y);
    eventball->stop();
  }
  else if(eventball->event(M_DOWN) && 
	  eventball->keys("z") && 
	  eventball->mbtn(1)){
    eventball->genstate(state_drag);
    vc->movestart(3,eventball->x,eventball->y);
    eventball->stop();
  }
  else if(eventball->event(M_DOWN) && 
	  eventball->keys("") && 
	  eventball->mbtn(1)){

  }
  else if(eventball->event(K_DOWN) &&
	  eventball->key(65451)){
    vc->Zoom(1.25);
    vc->dv->redraw();
    eventball->stop();
  }
  else if(eventball->event(K_DOWN) &&
	  eventball->key(65453)){	
    vc->Zoom(0.85);
    vc->dv->redraw();
    eventball->stop();
  }
  else if(eventball->event(K_DOWN) &&
	  eventball->keys("r")){
    vc->reset();
    vc->dv->redraw();
    eventball->stop();    
  }
  else if(eventball->event(K_DOWN) &&
	  eventball->keys("t")){	
    /*
      if(adisp.evthlist.modal) {
      if(adisp.evthlist.modal==mbeh){
      adisp.evthlist.remove(mbeh);
      dv.scene.Remove(mbeh->gl);
      //transform model from here
      delete mbeh; 
      adisp.evthlist.modal=mbeh=0;
      //wnd->redraw();
      dv.redraw();
      //evt=0;
      eventball->stop();
      }
      }
      else {
      mbeh = new ModelBldrEH;
      adisp.evthlist.modal=mbeh;
      adisp.evthlist.ins_head(mbeh);
      dv.scene.Add(mbeh->gl);
      //wnd->redraw();
      dv.redraw();
      //evt=0;
      eventball->stop();
      }
    */
  }
#endif //off
}    

