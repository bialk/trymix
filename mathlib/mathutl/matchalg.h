#ifndef matchalg_h
#define matchalg_h

#include <string.h>

class Match{
 public:

  float* wG; // weights
  signed char* dG; // directions

  virtual float Dist(long yl,long yr){
    return ((yl==yr)?1:0);
  };


  void Run(int lgl, int lgr, int *relPair, void *){
    // initialization of graph
    long lenl = lgl+1;
    long lenr = lgr+1;
    wG = new float[lenr*2];
    dG = new signed char[lenl*lenr];

    long j,i;

    wG[0] = 0; dG[0]=0;
    for(j = 1; j<lenr; j++) {
      wG[j] = wG[j-1] + Dist(-1,j-1);
      dG[j] = 0;
    }

    // forward
    for(i = 1; i<lenl; i++){
      //std::cout << "i:" << i << std::endl;
      wG[lenr]=wG[0] + Dist(i-1,-1);
      dG[i*lenr]=2;
      for(j = 1; j<lenr; j++){
	wG[lenr+j] = wG[j-1] + Dist(i-1,j-1); dG[i*lenr+j]=1;
	float v = wG[lenr+j-1] + Dist(-1,j-1);
	if(wG[lenr+j] < v) { wG[lenr+j] = v; dG[i*lenr+j]=0;}
	v = wG[j] + Dist(i-1,-1);				 
	if(wG[lenr+j] < v) { wG[lenr+j] = v; dG[i*lenr+j]=2;}
      }
      memcpy(wG,wG+lenr,lenr*sizeof(float));
    }
    // back
    i=lgl; j=lgr;
    while(i!=0 || j!=0){
      //std::cout << i << ":" << j << std::endl;
      switch(dG[i*lenr+j]){
      case 0: relPair[lgl+j-1] = -1; j--;
	break;
      case 1: relPair[i-1] = j-1; relPair[lgl+j-1] = i-1; j--;i--;
	break;
      case 2: relPair[i-1] = -1; i--;
	break;
      }
    }

    delete dG;
    delete wG;
  }
};

// ===================================================================================================

class Match2{
 public:

  float* wG; // weights
  signed char* dG; // directions
  float pnlt;

  virtual float Dist(long yl,long yr){ return 0;};

  void Run(int lgl, int lgr, int *relPair){
    // initialization of graph
    long lenl = lgl+1;
    long lenr = lgr+1;
    wG = new float[3*lenr*2];
    dG = new signed char[3*lenr*lenl];

    long j,i;

    wG[0] = 0;wG[1] = 0;wG[2] = 0;
    for(j = 1; j<lenr; j++) {
      wG[3*j+0] = 0; wG[3*j+1] = -1e10;	wG[3*j+2] = -1e10;
      dG[3*j+0] = 0; dG[3*j+1] = 0; dG[3*j+2] = 0;
    }

    float pnltl = pnlt;

    // forward
    for(i = 1; i<lenl; i++){
      //std::cout << "i:" << i << std::endl;
      wG[3*lenr+0]=-1e10; wG[3*lenr+1]=-1e10; wG[3*lenr+2] = 0;
      dG[3*lenr*i+0] = 2; dG[3*lenr*i+1] = 2; dG[3*lenr*i+2] = 2 ;

      for(j = 1; j<lenr; j++){
	float v;
	int ind=3*(lenr+j), indh=ind-3, indv = 3*j, indd=indv-3, inddg = 3*(lenr*i+j);

	wG[ind+0] = wG[indh+0]; dG[inddg+0] = 0;
	v = wG[indh+1] + pnltl;
	if(wG[ind+0] < v) { wG[ind+0] = v; dG[inddg+0] = 1;}
	v = wG[indh+2] + pnltl;
	if(wG[ind+0] < v) { wG[ind+0] = v; dG[inddg+0] = 2;}

	float dist = Dist(i-1,j-1);
	wG[ind+1] = wG[indd+0] + pnltl + dist; dG[inddg+1] = 0;
	v = wG[indd+1] + dist;
	if(wG[ind+1] < v) { wG[ind+1] = v; dG[inddg+1] = 1;}
	v = wG[indd+2] + pnltl + dist;
	if(wG[ind+1] < v) { wG[ind+1] = v; dG[inddg+1] = 2;}

	wG[ind+2] = wG[indv+0] + pnltl; dG[inddg+2] = 0;
	v = wG[indv+1] + pnltl;
	if(wG[ind+2] < v) { wG[ind+2] = v; dG[inddg+2] = 1;}
	v = wG[indv+2];
	if(wG[ind+2] < v) { wG[ind+2] = v; dG[inddg+2] = 2;}

      }
      memcpy(wG,wG+3*lenr,3*lenr*sizeof(float));
    }
    // back
    i=lgl; j=lgr;
    int maxi = 0;
    if(wG[3*(lenr+j)+maxi] < wG[3*(lenr+j)+1] ) maxi = 1;
    if(wG[3*(lenr+j)+maxi] < wG[3*(lenr+j)+2] ) maxi = 2;
		
    while(i!=0 || j!=0){
      //std::cout << i << ":" << j << ":" << maxi << std::endl;
      int maxi2 = dG[3*(i*lenr+j)+maxi];
      switch(maxi){
      case 0:  relPair[lgl+j-1] = -1; j--; 
	break;
      case 1:  relPair[i-1] = j-1; relPair[lgl+j-1] = i-1; j--;i--; 
	break;
      case 2:  relPair[i-1] = -1; i--;
	break;
      }			
      maxi = maxi2;
    }


    delete dG;
    delete wG;
  }
};


class Match03{
 public:

  float* wG; // weights
  signed char* dG; // directions

  virtual float Dist(long yl,long yr, long edge = 1){
    // 0 horizontal 
    // 1 diagonal
    // 2 vertical
    return ((yl==yr)?1:0);
  };


  void Run(int lgl, int lgr, int *relPair){
    // initialization of graph
    long lenl = lgl+1;
    long lenr = lgr+1;
    wG = new float[lenr*2];
    dG = new signed char[lenl*lenr];

    long j,i;

    wG[0] = 0; dG[0]=0;
    for(j = 1; j<lenr; j++) {
      wG[j] = wG[j-1] + Dist(i-1,j-1,0);
      dG[j] = 0;
    }

    // forward
    for(i = 1; i<lenl; i++){
      //std::cout << "i:" << i << std::endl;
      wG[lenr]=wG[0] + Dist(i-1,j-1,2);
      dG[i*lenr]=2;
      for(j = 1; j<lenr; j++){
	wG[lenr+j] = wG[j-1] + Dist(i-1,j-1,1); dG[i*lenr+j]=1;
	float v = wG[lenr+j-1] + Dist(i-1,j-1,0);
	if(wG[lenr+j] < v) { wG[lenr+j] = v; dG[i*lenr+j]=0;}
	v = wG[j] + Dist(i-1,j-1,2);				 
	if(wG[lenr+j] < v) { wG[lenr+j] = v; dG[i*lenr+j]=2;}
      }
      memcpy(wG,wG+lenr,lenr*sizeof(float));
    }
    // back
    i=lgl; j=lgr;
    while(i!=0 || j!=0){
      //std::cout << i << ":" << j << std::endl;
      switch(dG[i*lenr+j]){
      case 0: relPair[lgl+j-1] = -1; j--;
	break;
      case 1: relPair[i-1] = j-1; relPair[lgl+j-1] = i-1; j--;i--;
	break;
      case 2: relPair[i-1] = -1; i--;
	break;
      }
    }

    delete dG;
    delete wG;
  }
};


// ===================================================================================================

class DynOpt{
 public:
  virtual float fi(long x, long t){
    return 0;
  }
  virtual float g(long x, long x1){
    return 0;
  }
  virtual void rstep(long x, long t){		
  }
  void Run(long szx, long szt){
    long x,t,t1;
    float *j = (float*)alloca(szt*2*sizeof(float));
    long *tbl = (long*)alloca(szt*szx*sizeof(long));
    for(t=0;t<szt;t++){
      j[t] = fi(0,t);
    }

    for(x=1;x<szx;x++){
      for(t=0;t<szt;t++){
	float fiv = fi(x,t);
	float max = fiv + g(0,t) + j[0]; long arg =0;
	for(t1=1;t1<szt;t1++){
	  float v = fiv + g(t1,t) + j[t1];
	  if(max < v) { max = v; arg = t1; }
	}
	j[szt+t] = max; tbl[szt*x+t]=arg;
      }
      memcpy(j,j+szt,sizeof(float)*szt);
    }
    // reverse step		
    x--;
    long arg = 0; float max = j[0];
    for(t=1;t<szt;t++)	if(max < j[t]) { max = j[t]; arg = t; }
    rstep(x,arg);
    while(x>=1){
      arg = tbl[szt*x+arg]; x--;
      rstep(x,arg);
    }
  };	
};


class DynOpt2{
 public:
  long *tbl;
  float *j;
  long szt;
  virtual float fi(long x, long t){
    return 0;
  }
  virtual float g(long t_1, long t, long x){
    return 0;
  }
  virtual void rstep(long x, long t){		
  }
  void Run(long szx, long pszt){
    long x,t,t1;
    szt = pszt;
    j = (float*)alloca(szt*2*sizeof(float));
    tbl = (long*)alloca(szt*szx*sizeof(long));
    for(t=0;t<szt;t++){
      j[t] = fi(0,t);
    }

    for(x=1;x<szx;x++){
      for(t=0;t<szt;t++){
	float fiv = fi(x,t);
	float max = fiv + g(0,t,x) + j[0]; long arg =0;
	for(t1=1;t1<szt;t1++){
	  float v = fiv + g(t1,t,x) + j[t1];
	  if(max < v) { max = v; arg = t1; }
	}
	j[szt+t] = max; tbl[szt*x+t]=arg;
      }
      memcpy(j,j+szt,sizeof(float)*szt);
    }
    // reverse step		
    x--;
    long arg = 0; float max = j[0];
    for(t=1;t<szt;t++)	if(max < j[t]) { max = j[t]; arg = t; }
    rstep(x,arg);
    while(x>=1){
      arg = tbl[szt*x+arg];	x--;
      rstep(x,arg);
    }
  };	
};

/*
this optimization has no node function all operations are made in boud functions
this allowed to take into consideration the deformation in local operator
*/

class DynOpt3{
 public:
  long *tbl;
  float *j;
  long szt;

  virtual float g(long t_1, long t, long x){
    return 0;
  }
  virtual void rstep(long x, long t){		
  }
  void Run(long szx, long pszt){
    long x,t,t1;
    szt = pszt;
    j = (float*)alloca(szt*2*sizeof(float));
    tbl = (long*)alloca(szt*szx*sizeof(long));
    // variation of the first position
    for(t=0;t<szt;t++) j[t] = 0;

    for(x=1;x<szx;x++){
      for(t=0;t<szt;t++){
	float max = g(0,t,x) + j[0]; long arg =0;
	for(t1=1;t1<szt;t1++){
	  float v = g(t1,t,x) + j[t1];
	  if(max < v) { max = v; arg = t1; }
	}
	j[szt+t] = max; tbl[szt*x+t]=arg;
      }
      memcpy(j,j+szt,sizeof(float)*szt);
    }
    // reverse step		
    x--;
    long arg = 0; float max = j[0];
    for(t=1;t<szt;t++)	if(max < j[t]) { max = j[t]; arg = t; }
    rstep(x,arg);
    while(x>=1){
      arg = tbl[szt*x+arg];	x--;
      rstep(x,arg);
    }
  };	
};


#endif


