#include <sys/stat.h>
#include <string>

#include "dispview.h"
#include "toolspanel.h"
#include "viewctrl.h"
#include "lights.h"
//#include "viewsurf.h"
#include "imageplane.h"
//#include "matting.h"
#include "savejpeg.h"
#include "apputil/serializer.h"

#include <vector>

// test under git repository

// class GLScene
//=====================================================================

void GLScene::Add(EditViewObj* globj){
  iterator it = std::find(begin(), end(), globj);
  if(it!=end()){
    err_printf(("Register: EditViewObj object is already in the list\n"));
    return;
  }
  insert(end(),globj);
}

void GLScene::Remove(EditViewObj* globj){
  iterator it = std::find(begin(), end(), globj);
  if(it==end()){
    err_printf(("Remove: EditViewObj object was not found in the list\n"));
    return;
  }
  erase(it);  
}

// class DispView
//=====================================================================

DispView::DispView()
   : noredisplay(0),drawsimple(0)
   , toolpanel(new ToolPanel)
   , viewctrl(new ViewCtrl)
   , lights(new Lights)
   //, viewsurf(new ViewSurf)
   , imageplane(new ImagePlane)
   //, matting(new Matting)
   , glnamecount(0)
   , selectid(-1)
   , nsnap(0)
   , dispvieweh(this)
{
  SelectRst();
  //viewctrl->dv=this;
  //lights->dv=this;
  //viewsurf->dv=this;
  //imageplane->dv=this;
  //matting->dv=this;
  //toolpanel->dv=this;
}


DispView::~DispView(){}

void DispView::AskForData(Serializer *s){
  s->Item("ViewCtrl",Sync(viewctrl.get()));
  s->Item("Lights",Sync(lights.get()));
  //s->Item("ViewSurf",Sync(viewsurf.get()));
  s->Item("ImagePlane",Sync(imageplane.get()));
  //s->Item("Matting",Sync(matting.get()));
}

void DispView::TreeScan(TSOCntx *cntx){
  if(cntx==&TSOCntx::TSO_Init){

    // initialize event handlers queue
    /*
    evq_view.push(EvthList());
    evq_view.top().ins_top(&viewctrl->viewctrleh);
    evq_view.top().ins_top(&lights->lighteh);
    evq_view.top().ins_top(&dispvieweh);
    evq_view.top().ins_top(&evq_scene);
    evq_scene.push(EvthList());
    evq_scene.top().ins_top(&imageplane->eh);
    */

    // place scene objects to the draw queue
    //*
    scene.Add(viewctrl.get());
    scene.Add(lights.get());
    //scene.Add(viewsurf.get());
    scene.Add(imageplane.get());
    //scene.Add(matting.get());
    //*/
  }


  viewctrl->TreeScan(cntx);
  lights->TreeScan(cntx);
  toolpanel->TreeScan(cntx);
  //viewsurf->TreeScan(cntx);
  imageplane->TreeScan(cntx);
  //matting->TreeScan(cntx);
}


void DispView::Draw(DrawCntx *cntx){
  if(noredisplay){ noredisplay=0; return;  }
  redisplay=0;

  glClearDepth(1.0);
  if(viewctrl->background==0)
    glClearColor(.0, .0, .0, 0.0);
  else
    glClearColor(1.0, 1.0, 1.0, 0.0);  

  glClear(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT |
          GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE); //need to support lights
  glDepthFunc(GL_LEQUAL);

  glDisable(GL_CLIP_PLANE0);
  glDisable(GL_CLIP_PLANE1);
  glDisable(GL_CLIP_PLANE2);
  glDisable(GL_CLIP_PLANE3);
  glDisable(GL_CLIP_PLANE4);
  glDisable(GL_CLIP_PLANE5);

  // Default Settings
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_TEXTURE_2D);


  GLScene::iterator it;
  for(it=scene.begin();it!= scene.end();it++) (*it)->Draw(cntx);

  toolpanel->Draw(cntx);
}

GLuint DispView::GetNewName(){
  return ++glnamecount;
}


void DispView::SelectRst(){
  selectid=-1;
}

int DispView::SelectObj(int x, int y){
#ifdef off
  if(selectid==-1){
    selectid = viewctrl->SelectObj(x,y);
  }
  return selectid;
#endif
  return {};
}


void DispView::snapshot(){
  glPixelStorei(GL_PACK_ALIGNMENT,1);
  int x = viewctrl->mssh.wndw;
  int y = viewctrl->mssh.wndh;

  unsigned char *data = new unsigned char[x*y*3];
  glReadPixels( 0, 0, x,y, GL_RGB, GL_UNSIGNED_BYTE, data );
  char *buf = new char[x*3];
  char filename[20];  
  sprintf(filename,"image_%03li.jpeg",nsnap++);
  savejpeg(filename,data,x,y);
  delete data;
  delete buf;
}




// class DispViewEH
//==============================================================================


DispViewEH::DispViewEH(DispView *v):dv(v){}

void DispViewEH::Handle(EventBall *eventball){
  
  if(eventball->event(K_DOWN) &&
     eventball->keys("s")){
    //dv->snapshot();
    eventball->stop();
  }
  if(eventball->event(K_DOWN) &&
     eventball->kmod(KM_CTRL) &&
     eventball->keys("s")){
    //dv->snapshot();
    eventball->stop();
  }
  else if(eventball->event(K_DOWN) &&
	  (eventball->keys("q") || eventball->keys("Q") )){
    dv->quit();        
    eventball->stop();
  }
}

