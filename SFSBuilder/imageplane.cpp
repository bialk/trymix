#include "imageplane.h"
#include <FreeImage.h>
#include <QOpenGLFunctions>
#include "apputil/serializer.h"
#include "glhelper.h"
#include "shapefromshade.h"
#include "mathlib/mathutl/mymath.h"
//#include "mathlib/mathutl/imageandfft.h"

ImagePlane::ImagePlane():
  w(0),h(0),
  curslot(0),
  image_mode(image_mode_image),
  shape_mode(shape_mode_off),
  edit_mode(edit_mode_off),
  cache_slot(-1),
  glsel_lightcur(0)
{
//    imagefname[0] = "D:/zero-devel/research/pmetrics/data/bej1x4.ppm";
//    imagefname[1] = "D:/zero-devel/research/pmetrics/data/bej2x4.ppm";
//    imagefname[2] = "D:/zero-devel/research/pmetrics/data/bej3x4.ppm";
//    imagefname[3] = "D:/zero-devel/research/pmetrics/data/bej4x4.ppm";
      imagefname[0] = "D:/zero-devel/research/pmetrics/data/bln1x4.ppm";
      imagefname[1] = "D:/zero-devel/research/pmetrics/data/bln2x4.ppm";
      imagefname[2] = "D:/zero-devel/research/pmetrics/data/bln3x4.ppm";
      imagefname[3] = "D:/zero-devel/research/pmetrics/data/bln4x4.ppm";

//    light_vect1=  -0.5   0.4    2
//    light_vect2=   0.5   0.4    2
//    light_vect3=   0.5  -0.4    2
//    light_vect4=  -0.5  -0.4    2

    lights[0][0]=-0.5f; lights[0][1]= 0.4f; lights[0][2]=2.f;
    lights[1][0]= 0.5f; lights[1][1]= 0.4f; lights[1][2]=2.f;
    lights[2][0]= 0.5f; lights[2][1]=-0.4f; lights[2][2]=2.f;
    lights[3][0]=-0.5f; lights[3][1]=-0.4f; lights[3][2]=2.f;

//    lights[0][0]=-1; lights[0][1]=-1; lights[0][2]=4.;
//    lights[1][0]= 1; lights[1][1]=-1; lights[1][2]=4.;
//    lights[2][0]= 1; lights[2][1]= 1; lights[2][2]=4.;
//    lights[3][0]=-1; lights[3][1]= 1; lights[3][2]=4.;
}

ImagePlane::~ImagePlane(){}


void ImagePlane::AskForData(Serializer *s){
  if(s->ss->storageid==SRLZ_LAYOUT){
  }
  if(s->ss->storageid==SRLZ_PROJECT){
    s->Item("imagefname",Sync(imagefname,4));
    s->Item("lights0",Sync(lights[0],3));
    s->Item("lights1",Sync(lights[1],3));
    s->Item("lights2",Sync(lights[2],3));
    s->Item("lights3",Sync(lights[3],3));
  } 
}

void ImagePlane::TreeScan(TSOCntx *cntx){ 
  if(cntx==&TSOCntx::TSO_Init){
  }
  else if(cntx==&TSOCntx::TSO_ProjectNew){
    img[0].clear();
    img[1].clear();
    img[2].clear();
    img[3].clear();
    curslot = 0;
    cache_slot = -1;
    imagefname[0].clear();
    imagefname[1].clear();
    imagefname[2].clear();
    imagefname[3].clear();
    //image_tex.clear();
    image_tile.clear();
    w=h=0;

    data_attd.clear();
    lights[0][0]=-1; lights[0][1]=-1; lights[0][2]=0;
    lights[1][0]= 1; lights[1][1]=-1; lights[1][2]=0;
    lights[2][0]= 1; lights[2][1]= 1; lights[2][2]=0;
    lights[3][0]=-1; lights[3][1]= 1; lights[3][2]=0;
  }
  else if(cntx==&TSOCntx::TSO_ProjectLoad){
    Open(0);
    Open(1);
    Open(2);
    Open(3);
   }
};


void ImagePlane::Open(int slot){
  if(cache_slot==slot)  cache_slot = -1;
  
  img[slot].clear();

  struct stat st;
  if(stat(imagefname[slot].c_str(),&st) ) {
    err_printf(("image file \"%s\"  not found",imagefname[slot].c_str()));
    return;
  }

  //Format Detecting
  //Automatocally detects the format(from over 20 formats!)
  FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(imagefname[slot].c_str(),0);
  
  if(formato == FIF_UNKNOWN) {
    err_printf(("image format unknown"));
    return;
  }

  //Image Loading
  FIBITMAP* imagen = FreeImage_Load(formato, imagefname[slot].c_str());
  FIBITMAP* temp = imagen;
  if(!imagen){
    err_printf(("image reading error"));
    return;
  }
  imagen = FreeImage_ConvertTo32Bits(imagen);
  if(!temp) {
    err_printf(("image reading error"));
    return;
  }
  FreeImage_Unload(temp);
	
  w = FreeImage_GetWidth(imagen);
  h = FreeImage_GetHeight(imagen);

  if(w*h>2048*2048*20) {
    err_printf(("image reading error - too big image size (%ix%i)",w,h));
    return;
  }
  if(0) { // debug code
    printf("load image (%ix%i) for slot %i\n", w, h, slot);
  }  
	

  img[slot].resize(4*w*h);
  GLubyte* img_ptr = &img[slot][0];
  char* pixeles = (char*)FreeImage_GetBits(imagen);
  //FreeImage loads in BGR format, so you need to swap some bytes(Or use GL_BGR).
  /*	
  for(int j= 0; j<w*h; j++){
    textura[j*4+0]= pixeles[j*4+2];
    textura[j*4+1]= pixeles[j*4+1];
    textura[j*4+2]= pixeles[j*4+0];
    textura[j*4+3]= pixeles[j*4+3];
  }
  */
  memcpy(img_ptr,pixeles,4*w*h);
  FreeImage_Unload(imagen);
}
	
void ImagePlane::LoadTxt(int slot){  
  if(img[slot].empty()) return;
  if( img[slot].size()!=w*h*4 ) {
    err_printf(("texture loading error"));
    return;    
  }

  /*
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, image_tex());
     
  // set texture parameters 
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, 
		  GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, 
		  GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, 
		  GL_TEXTURE_WRAP_S, GL_CLAMP); 
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, 
		  GL_TEXTURE_WRAP_T, GL_CLAMP); 
  // load rectangle texture
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_RGBA, w, h, 0, GL_BGRA,GL_UNSIGNED_BYTE,(GLvoid*)&img[slot][0] );
  */
  image_tile.LoadBGRA(&img[slot][0], w, h);
  
}

#if 0
class TaleTxt{
public:
  void Set(unsigned char* image);
  void RenderTriangle(float *vtx, float *txt);  
  void RenderQuadriplet(float *vtx, float *txt);  
};
#endif

void ImagePlane::Draw(DrawCntx *cntx){

  if(w==0 || h==0) return;

  // setup reflective surfaces
  { 
    GLfloat no_mat[] = { 0.0, 0.0, 0.0, 0.0 };
    
    GLfloat mat_emission[] = {1., 1., 1., 1.0}; 
    GLfloat mat_emissionhalf[] = {0.5, 0.5, 0.5, 1.0}; 

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, no_mat); 
    glMaterialfv(GL_BACK,  GL_DIFFUSE, no_mat); 
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, no_mat );
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, no_mat); 
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  no_mat); 
    glDisable(GL_COLOR_MATERIAL);
    if(0/*texture_on*/){
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_emission); 
    }else{  
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_emissionhalf);      
      //glColorMaterial(GL_FRONT_AND_BACK,GL_DIFFUSE);
      //glEnable(GL_COLOR_MATERIAL);
      //glColor3f(0.5,0.5,0.5);
    }
  }


  // draw image
  if(image_mode!=image_mode_off){

    if(cache_slot != curslot){
      cache_slot = curslot;
      LoadTxt(curslot);
    }

    glDisable(GL_LIGHTING);
    /*
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, image_tex());

    glColor4f(1,1,1,1);
    glBegin (GL_QUADS);  
    glTexCoord2f (0.0, 0.0);
    glVertex3f (0.0, 0.0, 0.0);
    glTexCoord2f (w, 0.0);
    glVertex3f (w, 0.0, 0.0);
    glTexCoord2f (w, h);
    glVertex3f (w, h, 0.0);
    glTexCoord2f (0.0, h);
    glVertex3f (0.0, h, 0.0);
    glEnd ();
    glDisable(GL_TEXTURE_RECTANGLE_ARB);
    */
    image_tile.Draw();
    glEnable(GL_LIGHTING);
  }

  glPushName(100);
  // draw shape
  if(shape_mode!=shape_mode_off){    
    if(!shape.id){
      glNewList(shape(),GL_COMPILE_AND_EXECUTE);
      if(data_attd.size()==w*h){
	int i,j;   
	for(i=1;i<h;i++){
	  glBegin(GL_QUAD_STRIP);
	  for(j=0;j<w;j++){
	    float x,y,z;
	    int ih=i;

	    if(j>0){
	      vnormal2(x,y,z,
		       j-1,i-1, data_attd[w*(ih-1)+j-1],
		       j,i-1, data_attd[w*(ih-1)+j],
		       j,i, data_attd[w*ih+j]
		       );
	      glNormal3f(x,y,z);


	      glColor3ubv(&img[0][(w*(ih-1)+j)*3]);
	      glVertex3f(j,i-1, data_attd[w*(ih-1)+j]);

	      vnormal2(x,y,z,
		       j,i-1, data_attd[w*(ih-1)+j],
		       j,i, data_attd[w*ih+j],
		       j-1,i, data_attd[w*ih+j-1]);	      
	      glNormal3f(x,y,z);
	      glColor3ubv(&img[0][(w*(ih)+j)*3]);
	      glVertex3f(j,i, data_attd[w*ih+j]);
	    }

	    if(j<w-1){
	      vnormal2(x,y,z,
		       j,i, data_attd[w*ih+j],
		       j,i-1, data_attd[w*(ih-1)+j],
		       j+1,i-1, data_attd[w*(ih-1)+j+1]);
	      glNormal3f(x,y,z);
	      glColor3ubv(&img[0][(w*(ih-1)+j)*3]);
	      glVertex3f(j,i-1, data_attd[w*(ih-1)+j]);

	      vnormal2(x,y,z,
		       j+1,i, data_attd[w*ih+j+1],
		       j,i, data_attd[w*ih+j],
		       j,i-1, data_attd[w*(ih-1)+j]);	      
	      glNormal3f(x,y,z);
	      glColor3ubv(&img[0][(w*(ih)+j)*3]);
	      glVertex3f(j,i, data_attd[w*ih+j]);
	    }

	  }
	  glEnd();
	}
      }
      glEndList();
    }
    else
      glCallList(shape());
  }
  glPopName();


  // drawing light control points
  if(image_mode!=image_mode_off && edit_mode == edit_mode_on){

    glDisable(GL_LIGHTING);  
    glEnable(GL_POINT_SMOOTH); 
    glPointSize(6);

    glColor3f(1,0,0);
    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex3f(w*0.5f,h*0.5f-3.f,0.f);
    glVertex3f(w*0.5f,h*0.5f+3.f,0.f);
    glVertex3f(w*0.5f-3.f,h*0.5f,0.f);
    glVertex3f(w*0.5f+3.f,h*0.5f,0.f);
    glEnd();

    int cursel=-1;
    int i;
    for(i=0;i<4;i++){
      if(glsel_lightcur==glsel_light[i]){
	glColor3f(1,1,0);
	cursel=i;
      }    
      else
	glColor3f(1,0,0);

      glPushName(glsel_light[i]);
      glBegin(GL_POINTS);
      glVertex3f(
		 (w+lights[i][0]*h)*0.5f,
		 (h+lights[i][1]*h)*0.5f,
		 0
		 );
      //glVertex3f(w,h,0);
      glEnd();
      glPopName();
    }

    if(cursel>=0){
      glColor3f(1,1,0);
      float dist = h*0.5*sqrt(lights[cursel][0]*lights[cursel][0]+
			      lights[cursel][1]*lights[cursel][1]);

      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glTranslatef(w*0.5,h*0.5,0);
      glScalef(dist,dist,1);
      if(!circle.id) {
	// drawing circle
	glNewList(circle(),GL_COMPILE_AND_EXECUTE);
	int i;
	glBegin(GL_LINE_STRIP);
	for(i=0;i<46;i++){
	  glVertex3f(sin(i*2*M_PI/45.f),cos(i*2*M_PI/45.f),0.f);
	}
	glEnd();
	glEndList();  
      }
      else{
	glCallList(circle());      
      }
      glPopMatrix();
    }

    glColor3f(1,1,1);
  }

};
  

void ImagePlane::Build(){
  ShapeFromShade sfs;
  sfs.w=w;
  sfs.h=h;
  memcpy(sfs.s,lights,sizeof(sfs.s));
  memcpy(sfs.s_alb,lights,sizeof(sfs.s));

  std::vector<float> image[4];
  image[0].resize(w*h*3);
  image[1].resize(w*h*3);
  image[2].resize(w*h*3);
  image[3].resize(w*h*3);
  std::vector<float> albedo(w*h*3);
  std::vector<float> data_norm(w*h*3);
  //std::vector<float> data_attd(w*h);
  data_attd.resize(w*h);

  sfs.image[0]=&image[0][0];
  sfs.image[1]=&image[1][0];
  sfs.image[2]=&image[2][0];
  sfs.image[3]=&image[3][0];
  sfs.albedo=&albedo[0];
  sfs.data_norm=&data_norm[0];
  sfs.data_attd=&data_attd[0];

  int i;
  for(i=0;i<w*h;++i){
    image[0][i*3+0]=float(img[0][i*4+2])/255.f;
    image[0][i*3+1]=float(img[0][i*4+1])/255.f;
    image[0][i*3+2]=float(img[0][i*4+0])/255.f;

    image[1][i*3+0]=float(img[1][i*4+2])/255.f;
    image[1][i*3+1]=float(img[1][i*4+1])/255.f;
    image[1][i*3+2]=float(img[1][i*4+0])/255.f;

    image[2][i*3+0]=float(img[2][i*4+2])/255.f;
    image[2][i*3+1]=float(img[2][i*4+1])/255.f;
    image[2][i*3+2]=float(img[2][i*4+0])/255.f;

    image[3][i*3+0]=float(img[3][i*4+2])/255.f;
    image[3][i*3+1]=float(img[3][i*4+1])/255.f;
    image[3][i*3+2]=float(img[3][i*4+0])/255.f;

  }
  sfs.build();
  shape.clear();
}


#ifdef off
// event handler
//=================================================
ImagePlaneEH::ImagePlaneEH(ImagePlane *v):
state_drag(-1),
ev(v)
{}

void ImagePlaneEH::Handle(EventBall *eventball){
  if(eventball->state(-34)){
  }
  else if(eventball->event(Q_ADD)){
    // event handler removed from the queue
    ev->edit_mode=ImagePlane::edit_mode_on;
    eventball->stop();
    //ev->dv->redraw();
    return;
  }
  else if(eventball->event(Q_DEL)){
    // event handler removed from the queue
    ev->edit_mode=ImagePlane::edit_mode_off;
    eventball->stop();
    //ev->dv->redraw();
    return;
  }
  else if(eventball->event(M_DRAG) && 
          eventball->state(state_drag)){
    float x=eventball->x, y=eventball->y, z=0;
    //ev->dv->viewctrl->hitscene(x,y,z);
    float h=ev->h/2;
    float w=ev->w/2;
    x-=w; y-=h;    
    float d = sqrt(x*x+y*y);
    if(d<h && fabs(z)<0.2f){
      z = sqrt(h*h - d*d);
    }

    int slot;
    if(ev->glsel_lightcur == ev->glsel_light[0]) slot = 0;
    if(ev->glsel_lightcur == ev->glsel_light[1]) slot = 1;
    if(ev->glsel_lightcur == ev->glsel_light[2]) slot = 2;
    if(ev->glsel_lightcur == ev->glsel_light[3]) slot = 3;
    
    x = ev->lights[slot][0]=x/h;
    y = ev->lights[slot][1]=y/h;
    z = ev->lights[slot][2]=z/h;
    //printf("x=%f y=%f z=%f\n",x,y,z);
   // SyncUI();
    //ev->dv->redraw();
    eventball->stop();
  }
  else if(eventball->event(M_UP) &&
          eventball->state(state_drag)){
    eventball->setstate(0);
    ev->glsel_lightcur=0;
    //ev->dv->redraw();
    eventball->stop();
  }
  else if(!eventball->state(0)){
    // move event futher down
    return;
  }
  else if(eventball->event(M_DOWN) && 
          eventball->keys("") && 
	  eventball->kmod(0) &&
          eventball->mbtn(1)){


#ifdef off
    int id =ev->dv->SelectObj(eventball->x,eventball->y);
    if(id == ev->glsel_light[0] ||
       id == ev->glsel_light[1] ||
       id == ev->glsel_light[2] ||
       id == ev->glsel_light[3]){
      if(ev->glsel_lightcur != id){
	eventball->genstate(state_drag);
	ev->glsel_lightcur=id;
	ev->curslot=id-ev->glsel_light[0];	
	ev->dv->redraw();
	SyncUI();
	eventball->stop();
      }
    }
#endif //off

  }
}
#endif
