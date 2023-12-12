#include "lights.h"
#include "apputil/serializerV2.h"
#include "mathstruct.h"
//#include "apputil/serializer.h"

#include <QApplication>
#include <QOpenGLFunctions>

// control elements definitions
//=============================================================

// IconD3DG1
class IconD3DG1{
public:
  int state;
  int list;
  IconD3DG1():state(0){};
  void ExecOnce(){
    if(state) return;
    state ++;
    list = glGenLists(1);
    glNewList(list,GL_COMPILE);

    D3DG1obj().open((qApp->applicationDirPath() + "/torch.ico").toStdString().c_str());

    glEndList();
    return;
  }
};

void Icon3DLight::TreeScan(TSOCntx *cntx) {
  if(cntx == &TSOCntx::TSO_Init){
  }
}

void Icon3DLight::Draw(DrawCntx *cntx) { 
  cntx->trySetGLName(glsel_name);

  static IconD3DG1 torch;
  torch.ExecOnce();

  glPushAttrib(GL_ENABLE_BIT);
  glEnable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D); 
  
  { 
    GLfloat mat_zero[] = { 0.0, 0.0, 0.0, 0.0 };
    
    GLfloat mat_one[] = {1., 1., 1., 1.}; 
    GLfloat mat_half[] = {0.5, 0.5, 0.5, 1.}; 

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_zero);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_zero );
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_zero);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  mat_zero);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_zero);

    if(lights->selectid==glsel_name) {
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_one); 
    }else{
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_half);
    }
  } 

  glScalef(30,30,30);
  glMultMatrixf(rot);
  glRotatef(90,1,0,0);  
  glPushName(glsel_name);
  glCallList(torch.list);
  glPopName();
  glPopAttrib();
}


// Lights
//============================================================

Lights::Lights(){

  glic1.lights = this;
  glic2.lights = this;
  selectid=0;

  // parameter initialization
  show1 = 1;
  show2 = 1;
  rot1[0] = 1.0; rot1[1] = 0.0; rot1[2] = 0.0; rot1[3] = 0.0; 
  rot1[4] = 0.0; rot1[5] = 1.0; rot1[6] = 0.0; rot1[7] = 0.0; 
  rot1[8] = 0.0; rot1[9] = 0.0; rot1[10]= 1.0; rot1[11]= 0.0; 
  rot1[12]= 0.0; rot1[13]= 0.0; rot1[14]= 0.0; rot1[15]= 1.0;

  rot2[0] = 1.0; rot2[1] = 0.0; rot2[2] = 0.0; rot2[3] = 0.0; 
  rot2[4] = 0.0; rot2[5] = 1.0; rot2[6] = 0.0; rot2[7] = 0.0; 
  rot2[8] = 0.0; rot2[9] = 0.0; rot2[10]= 1.0; rot2[11]= 0.0; 
  rot2[12]= 0.0; rot2[13]= 0.0; rot2[14]= 0.0; rot2[15]= 1.0;
  
}

void Lights::Draw(DrawCntx *cntx){
  //light source 1  
  
  glEnable(GL_LIGHTING);

  glMatrixMode(GL_MODELVIEW);

  //GLfloat mat_shininess[] = { 50.0 }; 
  //glClearColor (0.0, 0.0, 0.0, 0.0); 

  //glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess); 
  GLfloat light_position[] = {0, 0, 1, 0.0 }; 
  GLfloat light_diffuse[] =  {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1. }; 
  GLfloat light_specular[] = { 1., 1., 1., 1.0 }; 

  //glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
  //glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
  //glLightfv(GL_LIGHT0, GL_POSITION, light_position);


  glPushMatrix();
  glShadeModel (GL_SMOOTH); 
  //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  glLoadMatrixf(rot1);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position); 
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  show1?glEnable(GL_LIGHT0):glDisable(GL_LIGHT0);

  glLoadMatrixf(rot2);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT1, GL_POSITION, light_position); 
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
  show2?glEnable(GL_LIGHT1):glDisable(GL_LIGHT1);
  glPopMatrix(); 
  //end of lite source
}


void Lights::AskForData(sV2::Serializer *s){
    s->SyncAs("show1",show1);
    s->SyncAs("show2",show2);
    s->SyncAs("rot1",rot1,16);
    s->SyncAs("rot2",rot2,16);
}

void Lights::TreeScan(TSOCntx *cntx){
  if(cntx==&TSOCntx::TSO_Init){
    //dv->reselect.connect(sigc::mem_fun(this,&Lights::reselect) );
    //dv->reselect = [=](){reselect();};
    glic1.rot=rot1;
    glic2.rot=rot2;
    //dv->toolpanel->Add(&glic1);
    //dv->toolpanel->Add(&glic2);
  }
//  glic1.TreeScan(cntx);
//  glic2.TreeScan(cntx);
}


void Lights::reselect(){
  selectid=0;
  g=0;
}

int Lights::isfocus(){
  return g!=nullptr;
}

void Lights::select(int id){
  //unsigned int stack[]={0};
  //int id = dv->viewctrl->ProcessHits2(0, stack);
  
  selectid=id;
  if(id==glic1.glsel_name ){
    g=&glic1;
  }
  else if(id==glic2.glsel_name ){ 
    g=&glic2;
  }
  else {
    g=nullptr;
  }
}

void Lights::lightrstart(int x, int y){
  if(g){
    mrot.setcenter(x, y, 20);
    mrot.start(x,y,g->rot);
  }
}

void Lights::lightrcont(int x, int y){
  if(g!=0) {
    mrot.drag(x,y,g->rot);
  }
}

#ifdef off
// class LightsEH
//=====================================================================

LightsEH::LightsEH(Lights *v):gl(v),state_drag(-1){}


int LightsEH::glName(int id){
  return (id==gl->glic1.glsel_name || id==gl->glic2.glsel_name);
}


void LightsEH::Handle(EventBall *eventball){
#ifdef off
  if(eventball->event(M_DRAG) &&
     eventball->state(state_drag)){
    gl->lightrcont(eventball->x,eventball->y);
    gl->dv->redraw();
    eventball->stop();
  }
  else if(eventball->event(M_UP) &&
	  eventball->state(state_drag)){
    gl->lightrstop();
    gl->dv->redraw();
    eventball->setstate(0);
    eventball->stop();
    gl->dv->reselect();
  }
  else if(!eventball->state(0)){
    // move event futher down
    return;
  }
  else if(eventball->event(M_DOWN) &&
	  eventball->keys("") &&
	  eventball->mbtn(1)){


    int id = gl->dv->SelectObj(eventball->x,eventball->y);
    if( glName(id) ) {
    
      gl->dv->reselect();
      gl->select();
      gl->lightrstart(eventball->x,eventball->y);
      eventball->genstate(state_drag);

      eventball->stop();
      gl->dv->redraw();

    }
  }
#endif
}

#endif
