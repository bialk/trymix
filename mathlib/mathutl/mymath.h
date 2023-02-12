#ifndef mymath_h
#define mymath_h

#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "../matpro/matrix.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif


#ifdef blinov
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#endif

//inline long round(float x){	return int((x>0)?(x+0.5f):(x-0.5f));	}

#ifndef INF
#define INF 1e20
#endif

class NDistr{
public:
	TMatrix b,a,r1,r2,r3,r4;
	double koef, lnkoef;
	long n;
	void Set(long nn, float* aa, float* bb){
		n = nn;	b.SetSize(n,n); a.SetSize(1,n);
		r1.SetSize(1,n); r2.SetSize(n,1); r3.SetSize(1,n); r4.SetSize(1,1);
		long i;
		for(i = 0; i<n*n; i++) b[i] = bb[i];
		for(i = 0; i<n; i++) a[i] = aa[i];
		if(n==2){
		std::cout << b[0] <<  " " << b[1] << std::endl;
		std::cout  <<  b[2] << " " << b[3] << std::endl;
		}
		koef = b.Inverse();		
		if(koef==0) {std::cout << "d:=0" << std::endl;}
		koef = 1./(pow(2*M_PI,n/2.)*sqrt(koef));
		lnkoef = log(koef);
	}
	inline double Val(float* x){
		long i;
		for(i = 0; i < n; i++) r2[i] = r1[i] = a[i]-x[i];
		r3.mmul(r1,b);
		r4.mmul(r3,r2);
		return koef*exp(-0.5*r4[0]);
	}
	inline double ValLn(float* x){
		long i;
		for(i = 0; i<n; i++) r2[i] = r1[i] = a[i]-x[i];
		r3.mmul(r1,b);
		r4.mmul(r3,r2);
		return lnkoef - 0.5*r4[0];
	}
};

class NDistrScal{
public:
	double a,b, koef, lnkoef;
	void Set(float mean, float sigm){
		a = mean; b = -0.5/sigm;
		koef = 1./sqrt(2*M_PI*sigm);
		lnkoef = log(koef);
	}
    inline double Val(double x){
        return x-=a, koef*exp(b*x*x);
	}
    inline double ValLn(double x){
        return x-=a,lnkoef + b*x*x;
	}
};


inline void vnormal(float &x, float &y, float &z, 
	       float ux, float uy, float uz, 
	       float vx, float vy, float vz){
  x = uy*vz-uz*vy;
  y = uz*vx-ux*vz;
  z = ux*vy-uy*vx;
  float d = sqrt(x*x+y*y+z*z);
  x/=d;  y/=d;  z/=d;
}

inline void vnormal2(float &x, float &y, float &z, 
	       float x1, float y1, float z1, 
	       float x2, float y2, float z2, 
	       float x3, float y3, float z3){
  float ux = x2-x1; 
  float uy = y2-y1; 
  float uz = z2-z1; 
  float vx = x3-x1; 
  float vy = y3-y1; 
  float vz = z3-z1; 

  x = uy*vz-uz*vy;
  y = uz*vx-ux*vz;
  z = ux*vy-uy*vx;
  float d = sqrt(x*x+y*y+z*z);
  x/=d;  y/=d;  z/=d;
}

class NormOfSurf{
public:
  void Build(int rangex, int rangey){
    float n[3]; int ix,iy;
    // generating of normals in the cetral part of surface
    for(ix=1;ix<rangex-1;ix++){
      for(iy=1;iy<rangey-1;iy++){
	float v1 = Get(ix+1,iy) - Get(ix-1,iy);
	float v2 = Get(ix,iy+1) - Get(ix,iy-1);
	vnormal(n[0],n[1],n[2],2,0,v1,0,2,v2);
	Set(ix,iy,n);
      }
    }

    //generating of normals on top-bottom boundaries
    for(ix=1;ix<rangex-1;ix++){
      iy = 0;
      float v1 = Get(ix+1,iy) - Get(ix-1,iy);
      float v2 = Get(ix,iy+1) - Get(ix,iy);
      vnormal(n[0],n[1],n[2],2,0,v1,0,2,v2);
      Set(ix,iy,n);

      iy = rangey-1;
      v1 = Get(ix+1,iy) - Get(ix-1,iy);
      v2 = Get(ix,iy) - Get(ix,iy-1);
      vnormal(n[0],n[1],n[2],2,0,v1,0,2,v2);
      Set(ix,iy,n);
    }

    //generating of normals on left-right boundaries
    for(iy=1;iy<rangey-1;iy++){
      ix = 0;
      float v1 = Get(ix+1,iy) - Get(ix,iy);
      float v2 = Get(ix,iy+1) - Get(ix,iy-1);
      vnormal(n[0],n[1],n[2],2,0,v1,0,2,v2);
      Set(ix,iy,n);

      ix = rangex-1;
      v1 = Get(ix,iy) - Get(ix-1,iy);
      v2 = Get(ix,iy+1) - Get(ix,iy-1);
      vnormal(n[0],n[1],n[2],2,0,v1,0,2,v2);
      Set(ix,iy,n);
    }

    //generating of normals in corners 
    float v = Get(0,0);
    float v1 = Get(1,0)-v;
    float v2 = Get(0,1)-v;
    vnormal(n[0],n[1],n[2],1,0,v1,0,1,v2);
    Set(0,0,n);

    v = Get(rangex-1,0);
    v1 = Get(rangex-1,1)-v;
    v2 = Get(rangex-2,0)-v;
    vnormal(n[0],n[1],n[2],0,1,v1,-1,0,v2);
    Set(rangex-1,0,n);

    v = Get(rangex-1,rangey-1);
    v1 = Get(rangex-2,rangey-1)-v;
    v2 = Get(rangex-1,rangey-2)-v;
    vnormal(n[0],n[1],n[2],-1,0,v1,0,-1,v2);
    Set(rangex-1,rangey-1,n);

    v = Get(0,rangey-1);
    v1 = Get(0,rangey-2)-v;
    v2 = Get(1,rangey-1)-v;
    vnormal(n[0],n[1],n[2],0,-1,v1,1,0,v2);
    Set(0,rangey-1,n);
  }
  virtual float Get(int x, int y) = 0;  
  virtual void  Set(int x, int y, float n[3]) = 0;  
};


//========================== LINEAR INTERPOLATOR ===============================

class LinItp{
  // scalar linear approximation 
  // coefficient of linear function are in a,b

 public:
  float a,b,r;
  float xx,fx,x,f,ff;
  float n;  
  void init(){
    xx=fx=x=f=n=0;    
  }
  void add(float _f, float _x){
    xx+=_x*_x; fx+=_x*_f; x+=_x;  f+=_f; ff+=_f*_f;
    
    n+=1;
  }
  void sub(float _f, float _x){
    xx-=_x*_x; fx-=_x*_f; x-=_x;  f-=_f; ff-=_f*_f;
    n-=1;    
  }
  void solve(){
    float d = (xx-x*x/n);
    if(d==0){
      a=0; b=0;
    }
    else{
      a=(fx-f*x/n)/d;
      b=(f-a*x)/n;
    }
    r = ff - 2*b*f - 2*a*fx + a*a*xx + 2*a*b*x + b*b*n;
  }
  void test(){
    //test example
    float ix;
    init();
    for(ix=-20;ix<=1000;ix+=1){
      float y = ix*4-12+float(rand())/RAND_MAX-0.5f;
      add(y,ix);
    }
    solve();
    printf("a=%f, b=%f\n",a,b);
  }
};

class QuadItpl{
public:
  double n,x,xx,xxx,xxxx,g,gx,gxx;
  double a,b,x0;
  void reset(){    
    a=b=x0=n=x=xx=xxx=xxxx=g=gx=gxx=0;
  }
  void add(float _x,float _g){
    x    += _x;
    xx   += _x*_x;
    xxx  += _x*_x*_x;
    xxxx += _x*_x*_x*_x;
    g    += _g;
    gx   += _g*_x;
    gxx  += _g*_x*_x;
    n+=1;
  }
  void finish(){
    //(-x^2*xxxx-xx^3+2*xx*x*xxx-xxx^2*n+xx*n*xxxx);
    double d=(-x*x*xxxx-xx*xx*xx+2*xx*x*xxx-xxx*xxx*n+xx*n*xxxx);
    if(d==0) return;
    //b = (-x*g*xxxx+xxx*xx*g+x*xx*gxx-gx*xx^2+n*gx*xxxx-n*xxx*gxx)/d;
    b = (-x*g*xxxx+xxx*xx*g+x*xx*gxx-gx*xx*xx+n*gx*xxxx-n*xxx*gxx)/d;
    //a = -(xx^2*g+x^2*gxx-xx*x*gx-xxx*x*g-xx*n*gxx+xxx*n*gx)/d;
    a = -(xx*xx*g+x*x*gxx-xx*x*gx-xxx*x*g-xx*n*gxx+xxx*n*gx)/d;
    //c = -(-g*xx*xxxx+g*xxx^2+xx^2*gxx-xx*xxx*gx+x*gx*xxxx-x*xxx*gxx)/d;
    double c = -(-g*xx*xxxx+g*xxx*xxx+xx*xx*gxx-xx*xxx*gx+x*gx*xxxx-x*xxx*gxx)/d;
    if(a==0) return;
    x0 = -b/(2*a); b = (4*c*a-b*b)/(4*a);
  }
  void test(){
    reset();
    add(-2+12,20+2);
    add(0+12,0+2);
    add(2+12,20+2);
    finish();   
    printf("a=%f, b=%f, x0=%f\n",a,b,x0);
  }
};

#endif // mymath_h
