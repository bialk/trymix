#include "shapefromshade.h"
#include "normfromshade.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <map>
//#include <unistd.h> // is definition of function sleep()

#define SOLVER2_OFF

#ifdef SOLVER2
   #include "lineqsol2.h"
#else
   #include "lineqsol.h"
#endif

//======================================================================

void ShapeFromShade::build(){

   if(false){
#ifdef SOLVER2
   LinSolver2 lsvr;
#else
   LinSolver lsvr;
#endif

   int sizem = h*w;
   lsvr.MtrxA(sizem,sizem,sizem*3);
   lsvr.MtrxB();
   lsvr.solve();

   return;
   }

  // calculating normals and albedo from shadow
  int ix,iy;
  {
    NormFromShade  nfs, nfs_alb;
    nfs.Init(&s[0][0],4);
    nfs_alb.Init(&s_alb[0][0],4);

    // scanning pixels and calculating albedo
    for(ix=0;ix<w;ix++){
      for(iy=0;iy<h;iy++){

	int iy2 = iy; //h-iy-1; // image may be vertically fliped
	// reading image pixels
	float *i1= &image[0][(w*iy2+ix)*3];
	float *i2= &image[1][(w*iy2+ix)*3];
	float *i3= &image[2][(w*iy2+ix)*3];
	float *i4= &image[3][(w*iy2+ix)*3];
	
	float *albd= &albedo[(w*iy2+ix)*3];

	float ilm[] = { i1[0],i1[1],i1[2], 
			i2[0],i2[1],i2[2], 
			i3[0],i3[1],i3[2],
			i4[0],i4[1],i4[2] };	  

	float norm[3];	

	// normal and albedo estimation
	{
	  nfs.Norm4(ilm,norm,albd);
	  float sum=sqrt(norm[0]*norm[0]+norm[1]*norm[1]+norm[2]*norm[2]);
	  if(sum==0) { sum=1;}

	  // writting normal into array
	  float *out = data_norm + (w*iy+ix)*3;
	  out[0]=norm[0]/sum;
	  out[1]=norm[1]/sum;
	  out[2]=norm[2]/sum;
	  data_attd[w*iy+ix]=0;
	}
      }
    }
  }
  


  // assembling matrices A & B

#ifdef SOLVER2
   LinSolver2 lsvr;
#else
   LinSolver lsvr;
#endif
  int sizem = h*w;
  lsvr.MtrxA(sizem,sizem,sizem*3);
  lsvr.MtrxB();

  class TCntrElem{
  public:
#ifdef SOLVER2
   TCntrElem(LinSolver2& lsvr, int w, int h, float * const data)
#else
   TCntrElem(LinSolver& lsvr, int w, int h, float * const data)
#endif
          :m_lsvr(lsvr)
          ,m_w(w)
          ,m_h(h)
          ,m_d(data)
      {}
      void add2mtrx(int x, int y){
          int i,j;
          int x1,x2,y1,y2;
          for(i=0;i<4;i++){
              x1 = i/2; y1 = i & 0x1;
              int idx1=(x+x1)*m_h+(y+y1); // '-1' the matrix has size (szy*szx - 1)
              float z  = GetElem(x+x1,y+y1,2);
              if(z<0.1) z=0.1;
              float gx = -GetElem(x+x1,y+y1,0)/z;
              float gy = -GetElem(x+x1,y+y1,1)/z;
              float sum = ((0.5-x1)*gx+(0.5-y1)*gy)*3;
              m_lsvr.A(idx1,idx1,-3.0);
              for(j=0;j<4;j++){
                  if(i==j) continue;
                  x2 = j/2; y2 = j & 0x1;
                  int idx2=(x+x2)*m_h+(y+y2);
                  m_lsvr.A(idx1,idx2,1.0);
                  z  = GetElem(x+x2,y+y2,2);  if(z<0.1) z=0.1;
                  gx = -GetElem(x+x2,y+y2,0)/z;
                  gy = -GetElem(x+x2,y+y2,1)/z;
                  sum -= ((0.5-x2)*gx+(0.5-y2)*gy);
              }
              if(sum!=0.0)
                  m_lsvr.B(idx1,sum);
          }
      }
  private:
#ifdef SOLVER2
      LinSolver2& m_lsvr;
#else
      LinSolver& m_lsvr;
#endif
      int const m_h;
      int const m_w;
      float * const m_d;
      float GetElem(int x, int y, int z){
          return m_d[(m_w*y+x)*3+z];
      }

  } cntrelem(lsvr,w,h,data_norm);

  //scanning all internal nodes
  for(ix=0;ix<w-1;ix++){
    for(iy=0;iy<h-1;iy++){
      cntrelem.add2mtrx(ix,iy);
    }
  }
  // add boundary condition
  lsvr.A(0,0,2.0);

  // solving system of linear equations
  lsvr.solve();

  //get result back into attitude data set
  for(ix=0;ix<w;ix++)
    for(iy=0;iy<h;iy++){
      float a;
      a = lsvr.X(h*ix+iy);
      data_attd[w*iy+ix]=a;
    }
}
