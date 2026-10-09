#include "lights.h"
#include "apputil/serializerV2.h"
#include "mathstruct.h"
#include "ModernOpenGLRenderer.h"
//#include "apputil/serializer.h"

#include <QMatrix4x4>
#include <QOpenGLFunctions>

// control elements definitions
//=============================================================

void Icon3DLight::TreeScan(TSOCntx *cntx) {
  if(cntx == &TSOCntx::TSO_Init){
  }
}

void Icon3DLight::Draw(DrawCntx *cntx) { 
  glScalef(30,30,30);
  glMultMatrixf(rot);
  glRotatef(90,1,0,0);

  GLfloat projectionValues[16];
  GLfloat modelViewValues[16];
  glGetFloatv(GL_PROJECTION_MATRIX, projectionValues);
  glGetFloatv(GL_MODELVIEW_MATRIX, modelViewValues);
  const QMatrix4x4 projection = QMatrix4x4(projectionValues).transposed();
  const QMatrix4x4 modelView = QMatrix4x4(modelViewValues).transposed();
  const float brightness = lights->g == this ? 1.0F : 0.5F;
  cntx->modernRenderer().drawLightIcon(
    projection, modelView, {brightness, brightness, brightness});
}


// Lights
//============================================================

Lights::Lights(){

  glic1.lights = this;
  glic2.lights = this;

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
    glic1.rot=rot1;
    glic2.rot=rot2;
  }
}


bool Lights::focus(EditViewObj* object){
  if(object == &glic1){
    g=&glic1;
  }
  else if(object == &glic2){
    g=&glic2;
  }
  else {
    g=nullptr;
  }
  return g != nullptr;
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
