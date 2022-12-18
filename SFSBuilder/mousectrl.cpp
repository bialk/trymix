#ifdef _WIN32
#include <windows.h>
#endif

#include <math.h>
#include "mathlib/mathutl/mymath.h"
#include "mousectrl.h"
#include "glhelper.h"


// MouseMoveAndShift
//=======================================================================
MouseMoveAndShift::MouseMoveAndShift():oper(0){
  shift_x = shift_y = shift_z = 0;

}

void MouseMoveAndShift::StartMove(int x, int y ){
  oper = 1; sx0 = x; sy0 = y;    
}

void MouseMoveAndShift::StartRotate(int x, int y){
  oper = 2; sx0 = x; sy0 = y;
}

void MouseMoveAndShift::StopMOper(){
  //glGetFloatv(GL_MODELVIEW_MATRIX,InitialModelMatrix);


  //inverse model-view matrix calculation
  //int i;
  //for(i=0; i<16; i++) InvMVMat.WriteElem(i%4,i/4, InitialModelMatrix[i]);
  //if( !InvMVMat.Inverse() ) 
  //  printf("Model-view matrix is singular (determinant is zero!)\n");

  shift_x = shift_y = shift_z = 0;
  angle_r = norm_rx = norm_ry = 0; norm_rz=1;
  oper = 0;
}


int MouseMoveAndShift::ContinueMOper(int x, int y){    
  
  float cx = 2.*float(x-wndw/2)/wndw;
  float cy = 2.*float(wndh-y-wndh/2)/wndh;
  float cz = 0;
  float cx0 = 2.*float(sx0-wndw/2)/wndw;
  float cy0 = 2.*float(wndh-sy0-wndh/2)/wndh;
  float cz0 = 0;

  Ptn p = {cx, cy, cz};
  Ptn p0 = {cx0, cy0, cz0};  
  switch(oper){
    double w,w0;
  case 1:
	  memcpy(mi,m,sizeof(mi));
	  glInvMat(mi);
	  w =  1.0/(mi[3]*p.x + mi[7]*p.y +
		  mi[11]*p.z + mi[15]);
	  w0 =  1.0/(mi[3]*p0.x + mi[7]*p0.y +
		  mi[11]*p0.z + mi[15]);
	  p = p*w;
	  p0 = p0*w0;
		p = glMulMat(mi,p,w);
		p0 = glMulMat(mi,p0,w0);
	  p=p-p0;
    shift_x = p.x;
    shift_y = p.y;
    shift_z = p.z;
    break;
  case 2:

    // free rotation control
    if(sx0 == x && sy0 == y) {angle_r = 0; break;}
    int windW = wndw/2;
    int windH = wndh/2;
    float x0=sx0-windW, y0=-(sy0-windH), z0=windW/4;
    float x1=x-windW, y1=-(y-windH), z1=windW/4;
    float n0=sqrt(x0*x0+y0*y0+z0*z0);
    float n1=sqrt(x1*x1+y1*y1+z1*z1);
    x0/=n0;y0/=n0;z0/=n0;
    x1/=n1;y1/=n1;z1/=n1;
    angle_r=acos(x0*x1+y0*y1+z0*z1)*180/M_PI;
    vnormal(norm_rx, norm_ry, norm_rz, x0, y0, z0, x1, y1, z1);    
    break;
  }
  return oper;
}

//=================================================================

void Ctrl3DRotate::setcenter(int x, int y, int z){
  cx=x; cy=y; cz=z;    
}

void Ctrl3DRotate::start(int x, int y, float *m_in){
  const float e[] ={ 1,0,0,0, 0,1,0,0,  0,0,1,0,  0,0,0,1};
  if(m_in)
    memcpy(m,m_in,sizeof(float)*16);
  else {
    memcpy(m,e,sizeof(float)*16);    
  }
  sx0 = x; sy0 = y;
}


void Ctrl3DRotate::drag(int x, int y){

  // free rotation control
  if(sx0 == x && sy0 == y) {angle_r = 0; return;}
  float x0=sx0-cx, y0=-(sy0-cy), z0=cz;
  float x1=x-cx, y1=-(y-cy), z1=cz;
  float n0=sqrt(x0*x0+y0*y0+z0*z0);
  float n1=sqrt(x1*x1+y1*y1+z1*z1);
  x0/=n0;y0/=n0;z0/=n0;
  x1/=n1;y1/=n1;z1/=n1;
  angle_r=acos(x0*x1+y0*y1+z0*z1)*180/M_PI;
  vnormal(norm_rx, norm_ry, norm_rz, x0, y0, z0, x1, y1, z1);
}

void Ctrl3DRotate::stop(){
  angle_r = norm_rx = norm_ry = 0; norm_rz=1;
}

void Ctrl3DRotate::drag(int x, int y, float *mout){
  drag(x,y);
  glMatrixMode(GL_MODELVIEW);   
  glPushMatrix();
  glLoadIdentity();
  //glRotatef(mssh.angle_r, mssh.norm_rx, mssh.norm_ry, mssh.norm_rz)
  glRotatef(angle_r, norm_rx, norm_ry, norm_rz);
  glMultMatrixf(m);
  glGetFloatv(GL_MODELVIEW_MATRIX,mout);
  glPopMatrix();
}
