#ifndef raster3d_h
#define raster3d_h
#include "meshandalg.h"


#ifndef err_printf
#define err_printf(n) printf n
#endif

// basic triangle rasterisation

class BaseTrgRaster{
private:
  float k_y,k_x;

  inline void zkoef(const Ptn &a, const Ptn &b, const Ptn &c){
    // calculation of coefficient for z - calculation
    // this equation was obtained from maple as solution of system equation
      k_y = (-b.x*c.z+a.x*c.z-a.x*b.z-c.x*a.z+c.x*b.z+b.x*a.z)/
	(a.x*c.y-a.x*b.y-b.x*c.y-c.x*a.y+c.x*b.y+b.x*a.y); 
      k_x = (-a.y*c.z+b.y*c.z-b.y*a.z+a.z*c.y-b.z*c.y+b.z*a.y)/
	(a.x*c.y-a.x*b.y-b.x*c.y-c.x*a.y+c.x*b.y+b.x*a.y);      
  }

  inline float zval(const Ptn &p,const Ptn &a, const Ptn &b, const Ptn &c){
    // z- distance calculation
    // this equation was obtained from maple as solution of system equation
    k_y = (-b.x*c.z+a.x*c.z-a.x*b.z-c.x*a.z+c.x*b.z+b.x*a.z)/
      (a.x*c.y-a.x*b.y-b.x*c.y-c.x*a.y+c.x*b.y+b.x*a.y); 
    k_x = (-a.y*c.z+b.y*c.z-b.y*a.z+a.z*c.y-b.z*c.y+b.z*a.y)/
      (a.x*c.y-a.x*b.y-b.x*c.y-c.x*a.y+c.x*b.y+b.x*a.y);      
    return (p.x-b.x)*k_x+(p.y-b.y)*k_y+b.z;
  }
  
public:
  int szX, szY;

  void traster(Ptn p1, Ptn p2, Ptn p3,int indx){
    
    if(p1.x > p2.x) std::swap(p1,p2);
    if(p2.x > p3.x) std::swap(p2,p3);
    if(p1.x > p2.x) std::swap(p1,p2);      
    //triangle is out of the screen
    if(p3.x < 0) return;

    zkoef(p1,p2,p3);
    int j,k;    
    // triangle rasterization
    float k1 = p2.x-p1.x;
    float k2 = p3.x-p2.x;
    int jb=max(p1.x+1,0);
    int je=min(p3.x,szX-1);
    for(j=jb;j<=je;j++){
      float y1 = p1.y+(j-p1.x)*(p3.y-p1.y)/(p3.x-p1.x);
      float y2;
      if(j<=p2.x && k1)
	y2 = p1.y+(j-p1.x)*(p2.y-p1.y)/k1;
      else
	y2 = p2.y+(j-p2.x)*(p3.y-p2.y)/k2;
      if(y1>y2) std::swap(y1,y2);

      // this part of triangle is out of the screen
      if(y2<0) continue;

      int kb=max(y1+1,0);
      int ke=min(y2,szY-1);
      for(k=kb;k<=ke;k++){  
	Ptn p; p.x=j; p.y=k;
	//p.z = zval(p,p1,p2,p3); // old method of z calculation
	p.z = (p.x-p2.x)*k_x+(p.y-p2.y)*k_y+p2.z;
	setptn(p.x,p.y,p.z,indx);
      }
    }      
  }

  virtual void setptn(int x, int y, float z, int n){
    // n - index of triangle
    // x,y, screen coordinates of projected point, 
    // z - depth coordinate for projected plane
  }
};

// basic surface rasterisation

class SurfaceRaster: public BaseTrgRaster{
 protected:
  Ptn norm;

 public:
  Surf *surf;
  float M[4*4]; //model view matrix
  float P[4*4]; //projection matrix

  //coordinates, normals and indices of 
  //triangle during rasterization
  Ptn pp1,pp2,pp3;
  Ptn nn1,nn2,nn3;
  int i1,i2,i3;

  //smothing normals array support
  int is_normsmooth; //1 - smooth normals, 0 - do not smooth normals
  std::vector<Ptn> smthnorms;

  SurfaceRaster():is_normsmooth(0){}

  void BuildNorms(){
    Ptn null={0,0,0};
    smthnorms.resize(surf->vtx.size());
    std::fill(smthnorms.begin(),smthnorms.end(),null);

    int sz = surf->trg.size();
    int i;
    for(i=0;i<sz;i++){
      Surf::TTrg &trg = surf->trg[i];
      Surf::TVtx &vtx1 = surf->vtx[trg.i1];
      Surf::TVtx &vtx2 = surf->vtx[trg.i2];
      Surf::TVtx &vtx3 = surf->vtx[trg.i3];
      Ptn p1 = vtx1.p * vtx1.d + surf->cnt;
      Ptn p2 = vtx2.p * vtx2.d + surf->cnt;
      Ptn p3 = vtx3.p * vtx3.d + surf->cnt;

      pp1.x=(M[0]*p1.x+M[1]*p1.y+M[2]*p1.z+M[3]);
      pp1.y=(M[4]*p1.x+M[5]*p1.y+M[6]*p1.z+M[7]);
      pp1.z=(M[8]*p1.x+M[9]*p1.y+M[10]*p1.z+M[11]);

      pp2.x=(M[0]*p2.x+M[1]*p2.y+M[2]*p2.z+M[3]);
      pp2.y=(M[4]*p2.x+M[5]*p2.y+M[6]*p2.z+M[7]);
      pp2.z=(M[8]*p2.x+M[9]*p2.y+M[10]*p2.z+M[11]);

      pp3.x=(M[0]*p3.x+M[1]*p3.y+M[2]*p3.z+M[3]);
      pp3.y=(M[4]*p3.x+M[5]*p3.y+M[6]*p3.z+M[7]);
      pp3.z=(M[8]*p3.x+M[9]*p3.y+M[10]*p3.z+M[11]);

      vnormal2(norm.x,norm.y,norm.z,
	       pp1.x,pp1.y,pp1.z,
	       pp2.x,pp2.y,pp2.z,
	       pp3.x,pp3.y,pp3.z);

      Ptn *p = &smthnorms[trg.i1]; *p=*p+norm;
      p = &smthnorms[trg.i2]; *p=*p+norm;
      p = &smthnorms[trg.i3]; *p=*p+norm;
    }
    sz = surf->vtx.size();
    for(i=0;i<sz;i++){
      Ptn &p = smthnorms[i];
      p=p*(1.0/p.norma());
    }
  }

  void Rasterize(){
    
    if(surf){
      if(is_normsmooth) BuildNorms();
      int sz = surf->trg.size();
      int i;
      for(i=0;i<sz;i++){
	Surf::TTrg &trg = surf->trg[i];
	Surf::TVtx &vtx1 = surf->vtx[i1=trg.i1];
	Surf::TVtx &vtx2 = surf->vtx[i2=trg.i2];
	Surf::TVtx &vtx3 = surf->vtx[i3=trg.i3];
	Ptn p1 = vtx1.p * vtx1.d + surf->cnt;
	Ptn p2 = vtx2.p * vtx2.d + surf->cnt;
	Ptn p3 = vtx3.p * vtx3.d + surf->cnt;

	pp1.x=(M[0]*p1.x+M[1]*p1.y+M[2]*p1.z+M[3]);
	pp1.y=(M[4]*p1.x+M[5]*p1.y+M[6]*p1.z+M[7]);
	pp1.z=(M[8]*p1.x+M[9]*p1.y+M[10]*p1.z+M[11]);

	pp2.x=(M[0]*p2.x+M[1]*p2.y+M[2]*p2.z+M[3]);
	pp2.y=(M[4]*p2.x+M[5]*p2.y+M[6]*p2.z+M[7]);
	pp2.z=(M[8]*p2.x+M[9]*p2.y+M[10]*p2.z+M[11]);

	pp3.x=(M[0]*p3.x+M[1]*p3.y+M[2]*p3.z+M[3]);
	pp3.y=(M[4]*p3.x+M[5]*p3.y+M[6]*p3.z+M[7]);
	pp3.z=(M[8]*p3.x+M[9]*p3.y+M[10]*p3.z+M[11]);

	if(is_normsmooth) { 
	  nn1 = smthnorms[trg.i1];
	  nn2 = smthnorms[trg.i2];
	  nn3 = smthnorms[trg.i3];
	}
	else{
	  vnormal2(norm.x,norm.y,norm.z,
		   pp1.x,pp1.y,pp1.z,
		   pp2.x,pp2.y,pp2.z,
		   pp3.x,pp3.y,pp3.z);

	  nn1 = nn2 = nn3 = norm;
	}

	Ptn p;
	float w = P[12]*pp1.x+P[13]*pp1.y+P[14]*pp1.z+P[15];
	p.x=(P[0]*pp1.x+P[1]*pp1.y+P[2]*pp1.z+P[3])/w;
	p.y=(P[4]*pp1.x+P[5]*pp1.y+P[6]*pp1.z+P[7])/w;
	p.z=(P[8]*pp1.x+P[9]*pp1.y+P[10]*pp1.z+P[11])/w;
	pp1=p;

	w = P[12]*pp2.x+P[13]*pp2.y+P[14]*pp2.z+P[15];
	p.x=(P[0]*pp2.x+P[1]*pp2.y+P[2]*pp2.z+P[3])/w;
	p.y=(P[4]*pp2.x+P[5]*pp2.y+P[6]*pp2.z+P[7])/w;
	p.z=(P[8]*pp2.x+P[9]*pp2.y+P[10]*pp2.z+P[11])/w;
	pp2=p;

	w = P[12]*pp3.x+P[13]*pp3.y+P[14]*pp3.z+P[15];
	p.x=(P[0]*pp3.x+P[1]*pp3.y+P[2]*pp3.z+P[3])/w;
	p.y=(P[4]*pp3.x+P[5]*pp3.y+P[6]*pp3.z+P[7])/w;
	p.z=(P[8]*pp3.x+P[9]*pp3.y+P[10]*pp3.z+P[11])/w;
	pp3=p;

	traster(pp1,pp2,pp3,i);
      }
    }
  } 

  void setptn(int x, int y, float z, int n){
    //virtual functon add your code for child class
  }
};

// matrix transformation

void MTranslat(float *M, float x, float y, float z){
  float s[16]= { 1, 0, 0, x, 
		 0, 1, 0, y,
		 0, 0, 1, z,
                 0, 0, 0, 1};
  float m[16];
  int i,j,k;
  std::copy(M,M+16,m);
  for(i=0;i<4;i++)
    for(j=0;j<4;j++){
      M[i*4+j]=0;
      for(k=0;k<4;k++)
	M[i*4+j]+=s[i*4+k]*m[k*4+j];
    }
}

void MScale(float *M, float x, float y, float z){
  float s[16]= { x, 0, 0, 0, 
		 0, y, 0, 0,
		 0, 0, z, 0,
                 0, 0, 0, 1};
  float m[16];
  int i,j,k;
  std::copy(M,M+16,m);
  for(i=0;i<4;i++)
    for(j=0;j<4;j++){
      M[i*4+j]=0;
      for(k=0;k<4;k++)
	M[i*4+j]+=s[i*4+k]*m[k*4+j];
    }
}

void MRotate(float *M, float fi, float x, float y, float z){
  float norma = sqrt(x*x+y*y+z*z);
  x/=norma;  y/=norma;  z/=norma;
  float s[16]= { 0, -z,  y, 0, 
		 z,  0, -x, 0,
		 -y,  x,  0, 0,
                 0,  0,  0, 0};
  float uut[16]= { x*x, x*y, x*z, 0,
		   y*x, y*y, y*z, 0,
		   z*x, z*y, z*z, 0,
                   0,   0,   0,   0}; 
  float I[16] =  { 1, 0, 0, 0, 
		   0, 1, 0, 0,
		   0, 0, 1, 0, 
                   0, 0, 0, 0};
  float r[16],m[16];
  float cosfi=cos(fi), sinfi=sin(fi);
  int i,j,k;
  std::copy(M,M+16,m);
  for(i=0;i<16;i++)
    r[i]=uut[i]+cosfi*(I[i]-uut[i])+sinfi*s[i];    
  r[15]=1;

  for(i=0;i<4;i++)
    for(j=0;j<4;j++){
      M[i*4+j]=0;
      for(k=0;k<4;k++)
	M[i*4+j]+=r[i*4+k]*m[k*4+j];
    }
}

void MFrustum(float *M, float l, float r, float b, float t, float n, float f){
  
  float s[16]={2*n/(r-l), 0,         (r+l)/(r-l),    0,
               0,         2*n/(t-b), (t+b)/(t-b),    0,
               0,         0,          (f+n)/(f-n), 2*f*n/(f-n),
               0,         0,          -1,            0};
  
  float m[16];
  std::copy(M,M+16,m);

  int i,j,k;
  for(i=0;i<4;i++)
    for(j=0;j<4;j++){
      M[i*4+j]=0;
      for(k=0;k<4;k++)
	M[i*4+j]+=s[i*4+k]*m[k*4+j];
    }
}

int MStore(float *M, const char* name="matix_m.txt"){
    FILE *f = fopen(name,"w+");
    if(f==0){
      err_printf(("MStore:Can't open file: %s",name));
      return 1;
    }
    fprintf(f,"%f %f %f %f\n",M[0],M[1],M[2],M[3]);
    fprintf(f,"%f %f %f %f\n",M[4],M[5],M[6],M[7]);
    fprintf(f,"%f %f %f %f\n",M[8],M[9],M[10],M[11]);
    fprintf(f,"%f %f %f %f\n",M[12],M[13],M[14],M[15]);
    fclose(f);
    return 0;
}

int MLoad(float *M, const char* name){
    FILE *f = fopen(name,"r");
    if(f==0){
      err_printf(("MStore:Can't open file: %s",name));
      return 1;
    }
    int n=0;
    n+=fscanf(f,"%f %f %f %f\n",&M[0],&M[1],&M[2],&M[3]);
    n+=fscanf(f,"%f %f %f %f\n",&M[4],&M[5],&M[6],&M[7]);
    n+=fscanf(f,"%f %f %f %f\n",&M[8],&M[9],&M[10],&M[11]);
    n+=fscanf(f,"%f %f %f %f\n",&M[12],&M[13],&M[14],&M[15]);
    if(n!=16){
      err_printf(("MLoad:Warnings! Only %i parameters read. (Must be 16).",n));
      return 1;
    }
    fclose(f);  
    return 0;
}


#endif
// END OF 3D SURFACE RASTERIZATION MODULE 
