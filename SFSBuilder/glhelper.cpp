//#ifdef _WIN32
//#include "windows.h"
//#endif

//#include <FL/gl.h>
//#include <GL/glu.h>

#include "glhelper.h"
#include "mathlib/lapackcpp/lapackcpp.h"
#include "mathlib/mathutl/mymath.h"
#include "mathlib/mathutl/imageandfft.h"

void CompoundTexture::Before(){
  glPushAttrib(GL_ENABLE_BIT);
  glEnable(GL_TEXTURE_2D); 
}
void CompoundTexture::After(){
  glPopAttrib();
}

void CompoundTexture::SetTexture(float *rgb,int rgbx, int rgby, int brd){
  border=brd;
  szx=rgbx;
  szy=rgby;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtxs);
  // the smalest fragment can be used is 64 the bigest is depends from hardware. 
  // the nearest fragments of power of 2 is used to cover whole texture with overlap
  // i.e. if the texture size 150x240 then separate frament size is 128x128 (nearest power of 2 to 150)
  maxtxs=std::max<int>(64,std::min<int>(maxtxs,2<<int(log((float)std::min<int>(rgby,rgbx))/log(2.0f))));
  // printf("%i\n",maxtxs);
  maxtxs=maxtxs-2*border;    
  std::vector<float> bff((maxtxs+2*border)*(maxtxs+2*border)*3);
  rgbxa=rgbx/maxtxs+((rgbx%maxtxs)?1:0);
  rgbya=rgby/maxtxs+((rgby%maxtxs)?1:0);
  int i,j;
  for(i=0;i<rgbxa;i++){
    for(j=0;j<rgbya;j++){	
      int ix,iy;
      for(ix=0;ix<(maxtxs+border*2);ix++){
	for(iy=0;iy<(maxtxs+border*2);iy++){
	  int imgx=(i*maxtxs+ix-border);
	  int imgy=(j*maxtxs+iy-border);
	  float zero[] =  {0.7,0.7,0.7};
	  float *v = zero;
	  if( imgx >= 0 && imgx < rgbx && imgy >= 0 && imgy < rgby )
	    v = &rgb[(imgy*rgbx + imgx)*3];
	  bff[(iy*(maxtxs+border*2)+ix)*3] = v[0];
	  bff[(iy*(maxtxs+border*2)+ix)*3+1]= v[1];
	  bff[(iy*(maxtxs+border*2)+ix)*3+2] = v[2];

	}
      }
	
      glGenTextures(1, &texNames[rgbya*i+j]); 
      glBindTexture(GL_TEXTURE_2D, texNames[rgbya*i+j]);
      glPixelStorei(GL_PACK_ALIGNMENT,1);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      // The mode GL_TEXTURE_ENV_COLOR should be used next. These values are used 
      // only if the GL_BLEND texture function has been specified as well.
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, GL_DECAL);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxtxs+2*border, maxtxs+2*border, 
		   0, GL_RGB, GL_FLOAT, &bff[0]);
    } 
  }
  txx=-1;
}

void CompoundTexture::SetTestTexture(int szx, int szy){
  int i,j;
  std::vector<float> img(szx*szy*3);
  for(j=0;j<szy;j++){
    for(i=0;i<szx;i++){
      img[(j*szx+i)*3+0]=((((i/8)%2)==((j/8)%2))?1:0);
      img[(j*szx+i)*3+1]=((((i/8)%2)==((j/8)%2))?0:1);
      img[(j*szx+i)*3+2]=((((i/8)%2)==((j/8)%2))?0.5:0.5);
    }
  }
  SetTexture(&img[0],szx,szy,3);
}

void CompoundTexture::MapTriangle(float x, float y){
  int newtxx = x/maxtxs;
  int newtxy = y/maxtxs;
  // calculate fragment;

  if(x<-border || x>=szx+border || y<-border || y>=szy+border) newtxx=newtxy=-1;

  if(newtxx!=txx || newtxy!=txy){
    txx=newtxx; txy=newtxy;
    if(txx>=0 && txx<rgbxa && txy>=0 && txy<rgbya)
      glBindTexture(GL_TEXTURE_2D, texNames[txx*rgbya+txy]);
    else {
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
}


void CompoundTexture::Map(float x, float y){
  glTexCoord2f((x-txx*maxtxs+border)/(maxtxs+2*border),		 
	       (y-txy*maxtxs+border)/(maxtxs+2*border));
}







  void Texture::Begin(){  cmpdtxtr.Before(); }
  void Texture::End(){  cmpdtxtr.After(); }
  void Texture::MapTriangle(Ptn x1, Ptn x2, Ptn x3){
    float xx=ma[0][0]*x1.x+ma[0][1]*x1.y+ma[0][2]*x1.z+ma[0][3]+
      ma[0][0]*x2.x+ma[0][1]*x2.y+ma[0][2]*x2.z+ma[0][3]+
      ma[0][0]*x3.x+ma[0][1]*x3.y+ma[0][2]*x3.z+ma[0][3];
    float yy=ma[1][0]*x1.x+ma[1][1]*x1.y+ma[1][2]*x1.z+ma[1][3]+
      ma[1][0]*x2.x+ma[1][1]*x2.y+ma[1][2]*x2.z+ma[1][3]+
      ma[1][0]*x3.x+ma[1][1]*x3.y+ma[1][2]*x3.z+ma[1][3];
    cmpdtxtr.MapTriangle(xx/3,yy/3);
  }
  
  void Texture::Map(float x, float y, float z){
    float xx=ma[0][0]*x+ma[0][1]*y+ma[0][2]*z+ma[0][3];
    float yy=ma[1][0]*x+ma[1][1]*y+ma[1][2]*z+ma[1][3];
    cmpdtxtr.Map(xx,yy);
  }

  void Texture::MapTriangle2d(float x, float y){    
    cmpdtxtr.MapTriangle(x,cmpdtxtr.szy-y);
  }
  void Texture::Map2d(float x, float y){
    cmpdtxtr.Map(x,cmpdtxtr.szy-y);
  }


  int Texture::Load(const char *name){
    rgbimg<float> rgb;
    if(LoadPPM(rgb,name)) return 1;
    unsigned int i;
    for(i=0;i<rgb.size();i++)  rgb[i]/=255;
    cmpdtxtr.SetTexture(&rgb[0],rgb.szx,rgb.szy,4);
    return 0;
  }










  int Surf2GL::SetTxtr(Surf *surface,Texture *texture, MapTxtrCoord *maptexture){
    surf=surface;
    txtr=texture;
    maptxtr=maptexture;
    if(txtr==0 || maptexture==0) { txtr=0; return 0; }
    if(surf->vtx.size()!=maptxtr->arrfxy.size()){
      err_printf(("Surf2GL: Surface does not match to texture mapping array!\n"));
      txtr=0;
      return 0;
    }
    return 1;
  }
  void Surf2GL::SmoothNorms(){
    unsigned int k,n;
    norms.resize(surf->vtx.size());
    Ptn null = {0,0,0};
    std::fill(norms.begin(), norms.end(), null);
    n = surf->trg.size();  
    for(k=0;k<n;k++){
      Surf::TTrg &trg = surf->trg[k];
      Surf::TVtx vtx1 = surf->vtx[trg.i1];
      Surf::TVtx vtx2 = surf->vtx[trg.i2];
      Surf::TVtx vtx3 = surf->vtx[trg.i3];
      vtx1.p=vtx1.p*vtx1.d+surf->cnt;
      vtx2.p=vtx2.p*vtx2.d+surf->cnt;
      vtx3.p=vtx3.p*vtx3.d+surf->cnt;

      float x,y,z;
      vnormal2(x,y,z,
	       vtx2.p.x,vtx2.p.y,vtx2.p.z,
	       vtx3.p.x,vtx3.p.y,vtx3.p.z,
	       vtx1.p.x,vtx1.p.y,vtx1.p.z );

      Ptn *p = &norms[trg.i1];
      p->x+=x; p->y+=y; p->z+=z;
      p = &norms[trg.i2];
      p->x+=x; p->y+=y; p->z+=z;
      p = &norms[trg.i3];
      p->x+=x; p->y+=y; p->z+=z;
    }
  
    for(k=0;k<norms.size();k++) {
      Ptn &p=norms[k];
      p=p*(1./p.norma());
    }
  }
  
  void Surf2GL::operator()(){

    if(txtr) txtr->Begin();
    int k, n=surf->trg.size();
    
    MapTxtrCoord::TxtrCoord tc[3];
    
    for(k=0;k<n;k++) {    
      Surf::TTrg &trg = surf->trg[k];
      Surf::TVtx vtx1 = surf->vtx[trg.i1];
      Surf::TVtx vtx2 = surf->vtx[trg.i2];
      Surf::TVtx vtx3 = surf->vtx[trg.i3];
      vtx1.p=vtx1.p*vtx1.d+surf->cnt;
      vtx2.p=vtx2.p*vtx2.d+surf->cnt;
      vtx3.p=vtx3.p*vtx3.d+surf->cnt;

      float x,y,z;
      if(norms.size()==0){    
	vnormal2(x,y,z,
		 vtx2.p.x,vtx2.p.y,vtx2.p.z,
		 vtx3.p.x,vtx3.p.y,vtx3.p.z,
		 vtx1.p.x,vtx1.p.y,vtx1.p.z );
	glNormal3f(x,y,z);
      }
      
      if(txtr){
	tc[0] = *maptxtr->GetXY(trg.i1);
	tc[1] = *maptxtr->GetXY(trg.i2);
	tc[2] = *maptxtr->GetXY(trg.i3);
	if(tc[0].x == -1 || tc[1].x == -1 || tc[2].x ==-1){
	  txtr->MapTriangle2d(-1.0,-1.0);
	  tc[0].x=tc[1].x=tc[2].x=tc[0].y=tc[1].y=tc[2].y=-1;
	}
	else
	  txtr->MapTriangle2d((tc[0].x+tc[1].x+tc[2].x)/3,(tc[0].y+tc[1].y+tc[2].y)/3);
      }

      glBegin(GL_TRIANGLES);
      if(norms.size()){ Ptn *p=&norms[trg.i1];	glNormal3f(p->x,p->y,p->z); }
      if(txtr) txtr->Map2d(tc[0].x,tc[0].y); 
      glVertex3f(vtx1.p.x,vtx1.p.y,vtx1.p.z);

      if(norms.size()){ Ptn *p=&norms[trg.i2];	glNormal3f(p->x,p->y,p->z); }
      if(txtr) txtr->Map2d(tc[1].x,tc[1].y); 
      glVertex3f(vtx2.p.x,vtx2.p.y,vtx2.p.z);

      if(norms.size()){ Ptn *p=&norms[trg.i3];	glNormal3f(p->x,p->y,p->z); }
      if(txtr) txtr->Map2d(tc[2].x,tc[2].y); 
      glVertex3f(vtx3.p.x,vtx3.p.y,vtx3.p.z);
      glEnd();
    }
    if(txtr) txtr->End();
  }















int Surf2GL2::SetTxtr(Surf *surface,Texture *texture, SurfTxtr *surftx){
  surf=surface;
  txtr=texture;
  stxr=surftx;
  maptxtr=0;//maptexture;
  if(txtr==0 || stxr==0) { txtr=0; return 0; }
  if(surf->trg.size()!=stxr->trg.size()){
    err_printf(("Surf2GL: Surface does not match to texture mapping array!\n"));
    txtr=0;
    return 0;
  }
  return 1;
}

  void Surf2GL2::operator()(){

    if(txtr) txtr->Begin();
    int k, n=surf->trg.size();
    
    //MapTxtrCoord::TxtrCoord tc[3];
    SurfTxtr::Ttx tc[3];
    
    for(k=0;k<n;k++) {    
      Surf::TTrg &trg = surf->trg[k];
      Surf::TVtx vtx1 = surf->vtx[trg.i1];
      Surf::TVtx vtx2 = surf->vtx[trg.i2];
      Surf::TVtx vtx3 = surf->vtx[trg.i3];
      vtx1.p=vtx1.p*vtx1.d+surf->cnt;
      vtx2.p=vtx2.p*vtx2.d+surf->cnt;
      vtx3.p=vtx3.p*vtx3.d+surf->cnt;

      float x,y,z;
      if(norms.size()==0){    
	vnormal2(x,y,z,
		 vtx2.p.x,vtx2.p.y,vtx2.p.z,
		 vtx3.p.x,vtx3.p.y,vtx3.p.z,
		 vtx1.p.x,vtx1.p.y,vtx1.p.z );
	glNormal3f(x,y,z);
      }   

      if(txtr){

	Surf::TTrg &ttrg = stxr->trg[k];

	tc[0] = stxr->tx[ttrg.i1];
	tc[1] = stxr->tx[ttrg.i2];
	tc[2] = stxr->tx[ttrg.i3];

	if(tc[0].x == -1 || tc[1].x == -1 || tc[2].x ==-1){
	  txtr->MapTriangle2d(-1.0,-1.0);
	  tc[0].x=tc[1].x=tc[2].x=tc[0].y=tc[1].y=tc[2].y=-1;
	}
	else
	  tc[0].x*=txtr->cmpdtxtr.szx;  tc[0].y*=txtr->cmpdtxtr.szy;
	  tc[1].x*=txtr->cmpdtxtr.szx;  tc[1].y*=txtr->cmpdtxtr.szy;
	  tc[2].x*=txtr->cmpdtxtr.szx;  tc[2].y*=txtr->cmpdtxtr.szy;
	  txtr->MapTriangle2d((tc[0].x+tc[1].x+tc[2].x)/3,(tc[0].y+tc[1].y+tc[2].y)/3);
      }

      glBegin(GL_TRIANGLES);
      if(norms.size()){ Ptn *p=&norms[trg.i1];	glNormal3f(p->x,p->y,p->z); }
      if(txtr) txtr->Map2d(tc[0].x,tc[0].y); 
      glVertex3f(vtx1.p.x,vtx1.p.y,vtx1.p.z);

      if(norms.size()){ Ptn *p=&norms[trg.i2];	glNormal3f(p->x,p->y,p->z); }
      if(txtr) txtr->Map2d(tc[1].x,tc[1].y); 
      glVertex3f(vtx2.p.x,vtx2.p.y,vtx2.p.z);

      if(norms.size()){ Ptn *p=&norms[trg.i3];	glNormal3f(p->x,p->y,p->z); }
      if(txtr) txtr->Map2d(tc[2].x,tc[2].y); 
      glVertex3f(vtx3.p.x,vtx3.p.y,vtx3.p.z);
      glEnd();
    }
    if(txtr) txtr->End();
  }





































/*
float glInvMat(float *m){
  int i;
  TMatrix InvMVMat;  InvMVMat.SetSize(4,4);
  for(i=0; i<16; i++) InvMVMat.WriteElem(i%4,i/4,m[i]);
  InvMVMat.Inverse();
  for(i=0; i<16; i++) m[i]=InvMVMat.ReadElem(i%4,i/4);
  return 0;
} 
*/


float glInvMat(float *m){
  return Lapackcpp().Invert(4,m);
}


Ptn glMulMat(float *m, Ptn p, float w){
    Ptn pout = {
	m[0]*p.x+m[4]*p.y+m[8]*p.z+m[12]*w,
        m[1]*p.x+m[5]*p.y+m[9]*p.z+m[13]*w,
        m[2]*p.x+m[6]*p.y+m[10]*p.z+m[14]*w,
    };
    return pout;
}

Ptn glMulMatLeft(Ptn p, float *m, float w){
    Ptn pout = {
      m[0]*p.x+m[1]*p.y+m[2]*p.z+m[3]*w,
      m[4]*p.x+m[5]*p.y+m[6]*p.z+m[7]*w,
      m[8]*p.x+m[9]*p.y+m[10]*p.z+m[11]*w
    };
    return pout;
}

void print_matrixf(float *m,int row, int col){
  int i,j;
  for(i=0;i<row;i++){
    for(j=0;j<col;j++){
       printf("%f ",m[i+j*row]);
    }
    printf("\n");
  }
  printf("\n");
}

void print_matrixd(double *m,int row, int col){
  int i,j;
  for(i=0;i<row;i++){
    for(j=0;j<col;j++){
       printf("%f ",m[i+j*row]);
    }
    printf("\n");
  }
  printf("\n");
}
