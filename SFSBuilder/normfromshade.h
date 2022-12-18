#ifndef normfromshade_h
#define normfromshade_h

#include <algorithm>
#include <math.h>
#include "mathlib/lapackcpp/lapackcpp.h"

//======================================================================

class NormFromShade{
public:
  Lapackcpp lapack;

  float s[3*4]; // 4 sources of lights    

  void Init(float *sl, int nl){
    std::copy(sl,sl+12,s);
    define_MtxCalc(mc,float,4*4*10);
    float sum[4];
    mc.start(); (mc(s,1,3)*mc(s,3,1)).finish(sum+0);
    mc.start(); (mc(s+3,1,3)*mc(s+3,3,1)).finish(sum+1);
    mc.start(); (mc(s+6,1,3)*mc(s+6,3,1)).finish(sum+2);
    mc.start(); (mc(s+9,1,3)*mc(s+9,3,1)).finish(sum+3);
    sum[0]=1./sqrt(sum[0]); sum[1]=1./sqrt(sum[1]);
    sum[2]=1./sqrt(sum[2]); sum[3]=1./sqrt(sum[3]);
    mc.start(); (mc(s,3,4)*mc.diag(4,4,sum)).finish(s);
  }


  // pure calculating albedo w/out highlights analysing
  void Norm(float *i, float *normal, float *albedo){
    double a[]={ s[0],s[3],s[6],s[9],
		 s[1],s[4],s[7],s[10],
		 s[2],s[5],s[8],s[11] };

    double b[]={ i[0]+i[1]+i[2],
		 i[3]+i[4]+i[5],
		 i[6]+i[7]+i[8],
		 i[9]+i[10]+i[11] };

    /*
    //a = (double*)alloca(m*n*sizeof(double));
    //b = (double*)alloca(m*sizeof(double));

    double a[]={7, 3, 9, 3.2,
    6, 5, 8, 7, 
    12, 4, 1, 5};

    double b[]={ 97, 42, 39, 54};


    // minimize || B - A*X ||
    Lapackcpp().LeastSquare(4,3,a,b);

    printf("%f %f %f\n",b[0],b[1],b[2]);
    */

    lapack.LeastSquare(4,3,a,b);    

    normal[0]=b[0];  normal[1]=b[1];   normal[2]=b[2];

    float sum = sqrt(b[0]*b[0]+b[1]*b[1]+b[2]*b[2]);      
    float sumalf = ( (s[0]*b[0]+s[1]*b[1]+s[2]*b[2])+
		     (s[3]*b[0]+s[4]*b[1]+s[5]*b[2])+
		     (s[6]*b[0]+s[7]*b[1]+s[8]*b[2])+
		     (s[9]*b[0]+s[10]*b[1]+s[11]*b[2])
		     )/sum;


    albedo[0]=(i[0]+i[3]+i[6]+i[9])/sumalf;
    albedo[1]=(i[1]+i[4]+i[7]+i[10])/sumalf;
    albedo[2]=(i[2]+i[5]+i[8]+i[11])/sumalf;
  }

  // highlights analyzing version 2
  void Norm2(float *i, float *normal, float *albedo){
    
    float * ia[]={i,i+3,i+6,i+9};
    float * sa[]={s,s+3,s+6,s+9};

    float va[]={ia[0][0]+ia[0][1]+ia[0][2],
		ia[1][0]+ia[1][1]+ia[1][2],
		ia[2][0]+ia[2][1]+ia[2][2],
		ia[3][0]+ia[3][1]+ia[3][2]};



    // calculation highlights in picture
    // 1. looking for two most bright pixels between four images
    // 2. calculate difference between two most bright pixels
    // 3. if this difference is more then threshold then throw out the
    //    most bright pixels from calculations.


    int k=0, kmax=0;
    for(k=1;k<4;k++) if(va[kmax]<va[k]) kmax=k;

    k=0; int kmax2=(kmax+1)%4;
    for(k=0;k<4;k++) if(va[kmax2]<va[k] && kmax!=k ) kmax2=k;


    if(((ia[kmax][0]+ia[kmax][1]+ia[kmax][2])-
	(ia[kmax2][0]+ia[kmax2][1]+ia[kmax2][2]))/3>100){
      ia[kmax][0]=ia[kmax][1]=ia[kmax][2]=0;
    }

    k=0;
    int kmin=0;
    for(k=1;k<4;k++) if(va[kmin]>va[k]) kmin=k;

    std::swap(ia[kmin],ia[3]);
    std::swap(sa[kmin],sa[3]);
    std::swap(va[kmin],va[3]);


    if(va[0]<va[1]) {
      std::swap(ia[0],ia[1]);
      std::swap(sa[0],sa[1]);
      std::swap(va[0],va[1]);
    }
    if(va[1]<va[2]) {
      std::swap(ia[1],ia[2]);
      std::swap(sa[1],sa[2]);
      std::swap(va[1],va[2]);
    }

    if(va[0]<va[1]) {
      std::swap(ia[0],ia[1]);
      std::swap(sa[0],sa[1]);
      std::swap(va[0],va[1]);
    }
    if(va[1]<va[2]) {
      std::swap(ia[1],ia[2]);
      std::swap(sa[1],sa[2]);
      std::swap(va[1],va[2]);
    }


    double bb[] = { ia[0][0]+ia[0][1]+ia[0][2], 
		    ia[1][0]+ia[1][1]+ia[1][2], 
		    ia[2][0]+ia[2][1]+ia[2][2] };
    double *b=bb;
      
    // array is rewritten after each calculation
    double a[]={ sa[0][0], sa[1][0], sa[2][0], 
		 sa[0][1], sa[1][1], sa[2][1], 
		 sa[0][2], sa[1][2], sa[2][2] };

    lapack.LeastSquare(3,3,a,b);    
    normal[0]=b[0];
    normal[1]=b[1];
    normal[2]=b[2];

    float sum = sqrt(b[0]*b[0]+b[1]*b[1]+b[2]*b[2]);      
   

    float sumalf = (   (sa[0][0]*b[0]+sa[0][1]*b[1]+sa[0][2]*b[2])
		       +(sa[1][0]*b[0]+sa[1][1]*b[1]+sa[1][2]*b[2])
		       +(sa[2][0]*b[0]+sa[2][1]*b[1]+sa[2][2]*b[2])
		       )/sum;


    // estimation of the albedo using all three pictures
    //
    albedo[0]=1.3*(ia[0][0]+ia[1][0]+ia[2][0])/(sumalf);
    albedo[1]=1.3*(ia[0][1]+ia[1][1]+ia[2][1])/(sumalf);
    albedo[2]=1.3*(ia[0][2]+ia[1][2]+ia[2][2])/(sumalf);

    // we may estimate albedo just from one picture
    // as the britness was saturated the best strategy
    // is choose the most dark picture (the color in this case will be
    // more realistic) 	
    /*
      int p=1;	
      out[0]=1.3*sum*(ia[p][0])/(sa[p][0]*b[0]+sa[p][1]*b[1]+sa[p][2]*b[2]);
      out[1]=1.3*sum*(ia[p][1])/(sa[p][0]*b[0]+sa[p][1]*b[1]+sa[p][2]*b[2]);
      out[2]=1.3*sum*(ia[p][2])/(sa[p][0]*b[0]+sa[p][1]*b[1]+sa[p][2]*b[2]);
    */

    if(albedo[0]>255) albedo[0]=255;
    if(albedo[1]>255) albedo[1]=255;
    if(albedo[2]>255) albedo[2]=255;
	
    if(albedo[0] < 0 || albedo[1] < 0 || albedo[2] < 0)
      printf("less then zero");
  }  


  // highlights analyzing version 3
  void Norm3(float *ilm, float *normal, float *albedo, int ix, int iy, int szx, int szy){


    float * ia[]={ilm,ilm+3,ilm+6,ilm+9};
    float * sa[]={s,s+3,s+6,s+9};
    

    //float * ia[]={i1,i2,i3,i4};

    float va[]={ia[0][0]+ia[0][1]+ia[0][2],
		ia[1][0]+ia[1][1]+ia[1][2],
		ia[2][0]+ia[2][1]+ia[2][2],
		ia[3][0]+ia[3][1]+ia[3][2]};


    // calculation highlights in picture
    // 1. looking for two most bright pixels between four images
    // 2. calculate difference between two most bright pixels
    // 3. if this difference is more then threshold then throw out the
    //    most bright pixels from calculations.

    int k=0, kmax=0;
    for(k=1;k<4;k++) if(va[kmax]<va[k]) kmax=k;

    float aw[9],w[3];
    int i;
    for(i=0;i<9;i++) aw[i]=0;
    float sumw = 0;
    for(i=0;i<4;i++){
      //if(i==kmax) continue;
      float *iw=ia[i];
      aw[0]+=iw[0]*iw[0];  aw[1]+=iw[0]*iw[1];  aw[2]+=iw[0]*iw[2];
      aw[3]+=iw[1]*iw[0];  aw[4]+=iw[1]*iw[1];  aw[5]+=iw[1]*iw[2];
      aw[6]+=iw[2]*iw[0];  aw[7]+=iw[2]*iw[1];  aw[8]+=iw[2]*iw[2];
      //sumw+=1;
    }
    //for(i=0;i<9;i++) a[i]/=sum;
    lapack.EigenValue(3, aw, w);

    float cr=aw[6], cg=aw[7], cb=aw[8];
    //n2.x=a[3]; n2.y=a[4]; n2.z=a[5];
    //d = (w[2]-w[1])/(w[2]+w[1]);
    float krgb = sqrt(cr*cr+cg*cg+cb*cb);
    //cr/=krgb; cg/=krgb; cb/=krgb;

    double bb[] = { cr*ilm[0]+cg*ilm[1]+cb*ilm[2], 
		    cr*ilm[3]+cg*ilm[4]+cb*ilm[5], 
		    cr*ilm[6]+cg*ilm[7]+cb*ilm[8],
		    cr*ilm[9]+cg*ilm[10]+cb*ilm[11]};    
    double *b=bb;
      
    // array is rewritten after each calculation
    double a[]={ s[0], s[3], s[6], s[9],
		 s[1], s[4], s[7], s[10],
		 s[2], s[5], s[8], s[11]};

    lapack.LeastSquare(4,3,a,b);    

    normal[0]=b[0];

    //b[2]+=0.01;

    float sum = sqrt(b[0]*b[0]+b[1]*b[1]+b[2]*b[2]);      
    float sumalf = ( (s[0]*b[0]+s[1]*b[1]+s[2]*b[2])+
		     (s[3]*b[0]+s[4]*b[1]+s[5]*b[2])+
		     (s[6]*b[0]+s[7]*b[1]+s[8]*b[2])+
		     (s[9]*b[0]+s[10]*b[1]+s[11]*b[2])
		     )/sum;


    float albd = 0;

    if(ix<=szx/2 && iy>szy/2) {
      i=1;
      albd = sum*(cr*ia[i][0]+cg*ia[i][1]+cb*ia[i][2])/(s[i*3]*b[0]+s[i*3+1]*b[1]+s[i*3+2]*b[2]);
    }
    if(ix>szx/2 && iy>szy/2) {
      i=1;
      albd += sum*(cr*ia[i][0]+cg*ia[i][1]+cb*ia[i][2])/(s[i*3+0]*b[0]+s[i*3+1]*b[1]+s[i*3+2]*b[2]);
    }
    if(ix>szx/2 && iy<=szy/2) {
      i=2;
      albd += sum*(cr*ia[i][0]+cg*ia[i][1]+cb*ia[i][2])/(s[i*3+0]*b[0]+s[i*3+1]*b[1]+s[i*3+2]*b[2]);
    }
    if(ix<=szx/2 && iy<=szy/2) { 
      i=3;
      albd += sum*(cr*ia[i][0]+cg*ia[i][1]+cb*ia[i][2])/(s[i*3+0]*b[0]+s[i*3+1]*b[1]+s[i*3+2]*b[2]);
    }

    albd/=1;


    albedo[0]=cr*albd;
    albedo[1]=cg*albd;
    albedo[2]=cb*albd;

    if(albedo[0]>255) albedo[0]=255;
    if(albedo[1]>255) albedo[1]=255;
    if(albedo[2]>255) albedo[2]=255;
  }

  void Norm_without_one_light(int s_out, float *i, float *normal, float *albedo){

    define_MtxCalc(mc,float,4*4*10);
    

    double  a[9];
    float b[9],af[9];

    int k,j;
    for(j=0,k=0;k<4;k++){
      if(k==s_out) continue;	
      af[j+0]=a[j+0]  =s[k*3+0]; af[j+3]=a[j+3]  =s[k*3+1]; af[j+6]=a[j+6]  =s[k*3+2];
      b[j*3+0]=i[k*3+0]; b[j*3+1]=i[k*3+1]; b[j*3+2]=i[k*3+2];
      j++;
    }

    float c[9];
    mc.start(); (mc(b,3,3)*mc(b,3,3).tran()).finish(c);
    float eig[3];
    Lapackcpp().EigenValue(3,c,eig);

    double bd[3];
    mc.start(); (mc(b,3,3).tran()*mc(c+6,3,1)).finish(bd);
    float r[3] = {bd[0],bd[1],bd[2]};
    
    Lapackcpp().LeastSquare(3,3,a,bd,1);     
    normal[0]=bd[0];  normal[1]=bd[1];   normal[2]=bd[2];
    
    float sum = sqrt(bd[0]*bd[0]+bd[1]*bd[1]+bd[2]*bd[2]);      

    //if(bd[2]<0) printf("bd2 less than zero\n");

    
    /*    
	  // an attempt to use the brightest light to estimate albedo
	  float rmax=b[0];
	  sum=r[0]/b[0];

	  if(rmax<b[1]){
	  sum=r[1]/b[1];
	  rmax=b[1];
	  }
	  if(rmax<b[2]){
	  sum=r[2]/b[2];
	  rmax=b[2];
	  }
    */

    sum/=2.;
    albedo[0]=c[6]*sum;
    albedo[1]=c[7]*sum;
    albedo[2]=c[8]*sum;
    return;     
  }


  //Svetlana's algorithm
  void Norm4(float *i, float *normal, float *albedo){

    define_MtxCalc(mc,float,4*4*10);

    double  a[]={s[0],s[3],s[6],s[9],
		 s[1],s[4],s[7],s[10],
		 s[2],s[5],s[8],s[11] };

    float b[]={ i[0],i[1],i[2],
		i[3],i[4],i[5],
		i[6],i[7],i[8],
		i[9],i[10],i[11] };


    // chromacity calculation
    float c[9];
    mc.start(); (mc(b,3,4)*mc(b,3,4).tran()).finish(c);
    float eig[3];
    Lapackcpp().EigenValue(3,c,eig);

    double bd[12];
    mc.start(); (mc(b,3,4).tran()*mc(c+6,3,1)).finish(bd); 
    float r[4] = {bd[0],bd[1],bd[2],bd[4]};

    /*
    //  example of using LeastSquare method call
    //a = (double*)alloca(m*n*sizeof(double));
    //b = (double*)alloca(m*sizeof(double));

    double a[]={7, 3, 9, 3.2,
    6, 5, 8, 7, 
    12, 4, 1, 5};

    double b[]={ 97, 42, 39, 54};


    // minimize || B - A*X ||
    Lapackcpp().LeastSquare(4,3,a,b);

    printf("%f %f %f\n",b[0],b[1],b[2]);
    */    

    
    Lapackcpp().LeastSquare(4,3,a,bd,1);
    b[0]=bd[0]; b[1]=bd[1]; b[2]=bd[2]; b[3]=bd[3];

    mc.start(); (mc(s,3,4).tran()*mc(b,3,1)-mc(r,4,1)).finish(b);
    mc.start(); (mc(b,1,4)*mc(b,4,1)).finish(b);
    float eps=sqrt(b[0]);

    // find darkest and brightest pixel as candidate for highlight or shadow
    int imin=0,imax=0,k;
    for(k=1;k<4;k++){
      if( r[imax] < r[k]) imax=k;
      if( r[imin] > r[k]) imin=k;
    }
    

    // if error to big remove pixel
    if(eps>40){
      // highlight detection if brighness of the brightest pixel at least twice higher 
      // then summary intensity of all pixels then such a pixel is considered as a highighted one
      if((i[imax*3]+i[imax*3+1]+i[imax*3+2])
	 /(i[0]+i[1]+i[2]+i[3]+i[4]+i[5]+i[6]+i[7]+i[8]+i[9]+i[10]+i[11])<0.5){
	Norm_without_one_light(imin,i,normal,albedo);
      }
      else {
	Norm_without_one_light(imax,i,normal,albedo);
      }
      
    }
    else {

    
      normal[0]=bd[0];  normal[1]=bd[1];   normal[2]=bd[2];

      //if(bd[2]<0) printf("bd2 less than zero\n");
      float sum = sqrt(bd[0]*bd[0]+bd[1]*bd[1]+bd[2]*bd[2]);      
      sum/=2.;
      albedo[0]=c[6]*sum;
      albedo[1]=c[7]*sum;
      albedo[2]=c[8]*sum;
    }    
  }

};

#endif
