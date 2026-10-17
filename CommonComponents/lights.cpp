#include "lights.h"
#include "apputil/serializerV2.h"
#include "OpenGLRenderer.h"
//#include "apputil/serializer.h"

#include <QMatrix4x4>

// control elements definitions
//=============================================================

void Icon3DLight::TreeScan(TSOCntx *cntx) {
  if(cntx == &TSOCntx::TSO_Init){
  }
}

void Icon3DLight::Draw(DrawCntx* cntx)
{
  const float brightness = lights->g == this ? 1.0F : 0.5F;
  cntx->openGLRenderer().drawLightIcon(
    cntx->projectionMatrix(), cntx->modelViewMatrix(),
    {brightness, brightness, brightness});
}

void Icon3DLight::DrawWithMatrices(
    DrawCntx* cntx, const QMatrix4x4& projection,
    const QMatrix4x4& modelView)
{
  QMatrix4x4 iconModelView = modelView;
  iconModelView.scale(30.0F);
  iconModelView *= QMatrix4x4(rot).transposed();
  iconModelView.rotate(90.0F, 1.0F, 0.0F, 0.0F);
  const float brightness = lights->g == this ? 1.0F : 0.5F;
  cntx->openGLRenderer().drawLightIcon(
    projection, iconModelView, {brightness, brightness, brightness});
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
  cntx->openGLRenderer().setDirectionalLights(
    {rot1[8], rot1[9], rot1[10]}, show1 != 0,
    {rot2[8], rot2[9], rot2[10]}, show2 != 0);
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
