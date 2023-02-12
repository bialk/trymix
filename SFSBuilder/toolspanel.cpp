#include "toolspanel.h"
#include "viewctrl.h"
#include "CommonComponents/drawContext.h"
#include "CommonComponents/CentralWidget.h"


void ToolPanel::Add(EditViewObj * ic){
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
//    switch (mode){
//      case GL_RENDER:
//        glOrtho(-cntx->w()/2,cntx->w()/2,-cntx->h()/2,cntx->h()/2,-300,300);
//        break;
//      case GL_SELECT:
//        x=(m_viewctrl->mssh.sx0 - cntx->w()/2);
//        y=(cntx->h()/2 - m_viewctrl->mssh.sy0);
//        glOrtho(-5+x,5+x,-5+y,5+y,-500,500);
//        break;
//    }

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    //glTranslatef((-m_viewctrl->mssh.wndw/2+100*(i+0.7)),(m_viewctrl->mssh.wndh/2-60),200);

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
    show=1;
  }
}


