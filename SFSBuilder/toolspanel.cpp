#include "toolspanel.h"
#include "dispview.h"
#include "viewctrl.h"


void ToolPanel::Add(EditViewObj * ic){
  ic->dv=dv;
  iconvector.push_back(ic);
}

void ToolPanel::Draw(DrawCntx *cntx){
  if (!show) return;
  unsigned int i;
  for( i=0; i<iconvector.size();i++){
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    GLint mode;
    glGetIntegerv(GL_RENDER_MODE,&mode);  
    float x,y;
    switch (mode){ 
    case GL_RENDER:
      glOrtho(-viewctrl->mssh.wndw/2,viewctrl->mssh.wndw/2,-viewctrl->mssh.wndh/2,viewctrl->mssh.wndh/2,-300,300);
      break;
    case GL_SELECT:
      x=(viewctrl->mssh.sx0-viewctrl->mssh.wndw/2);
      y=(viewctrl->mssh.wndh/2-viewctrl->mssh.sy0);
      glOrtho(-5+x,5+x,-5+y,5+y,-500,500);
      break;
    }

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef((-viewctrl->mssh.wndw/2+50*(i+0.7)),(viewctrl->mssh.wndh/2-30),200);

    iconvector[i]->Draw(cntx);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
  }
}

void ToolPanel::AskForData(Serializer *s){}

void ToolPanel::TreeScan(TSOCntx *cntx){
  if(cntx==&TSOCntx::TSO_Init){
    viewctrl = dv->viewctrl.get();
    show=1;
  }
}


