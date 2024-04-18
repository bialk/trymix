#ifndef mathstruct_h
#define mathstruct_h
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#endif


#include "mathlib/mathutl/mymath.h"
#include "mathlib/mathutl/meshandalg.h"


//============================ Sphere point generator ================================

class SpherePointsGenerator;

class BoundFinder{
public:
  SpherePointsGenerator *spg;
  virtual void point(float x,float y,float z, int idx) = 0;
};

class SpherePointsGenerator{
public:
  int ngam, *nalf;
  float d, dgam, *dalf;
  float *dist;
  float cx,cy,cz;

  BoundFinder *bf;
  SpherePointsGenerator():nalf(0),dist(0),cx(0),cy(0),cz(0),bf(0){};
  ~SpherePointsGenerator(){Destroy();};
  virtual void point(float x,float y, float z){};
  
  void Destroy(void){
    delete nalf; nalf=0; 
    delete dist; dist=0;
  }
  int Init(float delta){
    d=delta;
    ngam = M_PI/d+1;
    dgam = M_PI/ngam;
    int i;
    Destroy();
    int n = 0;
    nalf = new int[ngam+1];    
    dalf = new float[ngam+1];
    for(i=0;i<ngam+1;i++){
      float r=fabs(cos(-M_PI/2+i*dgam));
      int k = 2*M_PI*r/d + 1;
      dalf[i]= 2*M_PI/k;
      n+=k;
      nalf[i]=n;
    }
    dist = new float[n];
    return ngam;
  }  
  void Circle(int n){
    float gam = -M_PI/2+n*dgam;
    float da = dalf[n];
    int i, imin = ((n<=0)?0:nalf[n-1]), imax = nalf[n];
    
    if(bf){
      for (i=imin; i<imax;i++){
	float alf = (i-imin)*da;
	bf->point(cos(gam)*cos(alf),cos(gam)*sin(alf),sin(gam), i);
      }
    }else{
      for (i=imin; i<imax;i++){
	float alf = (i-imin)*da;
	point(cos(gam)*cos(alf),cos(gam)*sin(alf),sin(gam));
      }
    }
  }

  void BuildBall(float delta = 0.05, float radius = 20){
    Init(delta);
    int i;
    for(i=0; i< nalf[ngam]; i++) dist[i]=radius;
  }

  void Store(const char* name = "./surf.dat"){
    int fd = open(name,
                  O_RDWR | O_CREAT | O_TRUNC | O_NOCTTY,
                  S_IREAD | S_IWRITE);
    write(fd, &d, sizeof(d));
    write(fd, &cx, sizeof(cx));
    write(fd, &cy, sizeof(cy));
    write(fd, &cz, sizeof(cz));
    
    write(fd, dist, sizeof(dist[0])*nalf[ngam]);
    close(fd);

  }
  void Load(const char* name = "./surf.dat"){
    int fd = open(name,O_RDONLY | O_NOCTTY);
    if(fd == -1) {
      BuildBall();
      return;
    }
    read(fd, &d, sizeof(d));
    read(fd, &cx, sizeof(cx));
    read(fd, &cy, sizeof(cy));
    read(fd, &cz, sizeof(cz));
    Init(d);
    read(fd, dist, sizeof(dist[0])*nalf[ngam]);    
    close(fd);
  }
};



class ClosedStripCreator{
public:
  int beg1,beg2,end1,end2; // range of poings in each boundary
  int f1,f2;
  
  virtual void SetEdge(int n1, int n2) = 0;// insert code to set edge between two points
  virtual float Dist(int n1, int n2) = 0;  // insert code to caclucate distance between two points
  inline void PreSetEdge(int n1,int n2){
    if (n1>=end1) n1=n1-end1+beg1;
    if (n2>=end2) n2=n2-end2+beg2;
    SetEdge(n1,n2);
  }
  inline float PreDist(int n1, int n2){
    if (n1>f1) return 1e10;
    if (n2>f2) return 1e10;
    if (n1>=end1) n1=n1-end1+beg1;
    if (n2>=end2) n2=n2-end2+beg2;
    return Dist(n1,n2);
  }
  void Run(){
    int c1 = beg1,c2 = beg2;
    float d = Dist(c1,c2);
    int i;
    // searching the closest node
    for(i=beg2+1;i<end2;i++) if( d > Dist(c1,i) ) { c2=i; d=Dist(c1,i); }
    f1=c1-beg1+end1; f2=c2-beg2+end2;

    // building the best points combination
    PreSetEdge(c1,c2);
    do{
      if (PreDist(c1+1,c2) <= PreDist(c1,c2+1)) c1++; else c2++;
      PreSetEdge(c1,c2);
    }while(c1!=f1 || c2!=f2);
    //set edges for 
    //SetEdge(end1,beg1-1);
    //for(i=beg1;i<end1-1;i++) SetEdge(i,i+1);
    //SetEdge(end2,beg2-1);
    //for(i=beg2;i<end2-1;i++) SetEdge(i,i+1);
  }
};

//============================= SURFACE STURCTURES AND ALGORITHMS ============================


class GenerateBall{
 private:
  class T: public BoundFinder{
  public:
    Surf *surf;
    class UseCSC: public ClosedStripCreator{
    public:
      Surf *surf;      
      //int beg1,beg2,end1,end2; // range of poings in each boundary
      Ptn ptt[10000]; int ptt1[10000];
      int np1,np2;
      virtual void SetEdge(int n1, int n2){
	if(np1==n1 && np2==n2) return;
	if(np1==-1){ np1=n1;np2=n2; return;}	  
	Surf::TEdg edg={ptt1[n1],ptt1[n2]};
	surf->edg.push_back(edg);
	if(np1==n1) { 
	  np1=np2;
	  edg.i1=ptt1[np2]; edg.i2=ptt1[n2];
	}
	else{
	  edg.i1=ptt1[np1]; edg.i2=ptt1[n1];
	}
	surf->edg.push_back(edg);
	Surf::TTrg trg={ptt1[np1],ptt1[n1],ptt1[n2]};
	surf->trg.push_back(trg);	
	np1=n1;np2=n2;
      }
    
      virtual float Dist(int n1, int n2){
	return (ptt[n1]-ptt[n2]).norma();
      }
    } ucsc;
    
  
    void point(float x,float y,float z, int idx){
      float d = spg->dist[idx];
      ucsc.ptt1[ucsc.end2]=idx;
      ucsc.ptt[ucsc.end2].x = x;
      ucsc.ptt[ucsc.end2].y = y;
      ucsc.ptt[ucsc.end2].z = z;      
      //Surf::TVtx vtx.p={ucsc.ptt[ucsc.end2],d};
      Surf::TVtx vtx;	vtx.p=ucsc.ptt[ucsc.end2]; vtx.d=d;

      surf->vtx[idx]=vtx;
      ucsc.end2++;
    };

    void Circle1(int n){      
      ucsc.beg1=ucsc.end2=0;
      spg->Circle(n);
      ucsc.end1=ucsc.beg2=ucsc.end2;
      spg->Circle(n+1);
      ucsc.np1=-1;
      ucsc.Run();
    }

  };

 public:

  void operator() (Surf *surf, float step=0.05, float dist=50){ 
    surf->clear();
    SpherePointsGenerator spg;
    spg.BuildBall(step, dist);  
    surf->vtx.resize(spg.nalf[spg.ngam]);
  
    T bf;
    spg.bf = &bf;
    bf.spg = &spg;
    bf.surf = bf.ucsc.surf=surf;
  
    //generate surface
    int k;
    int n = spg.ngam;
    for(k=0;k<n;k++) bf.Circle1(k);  
  }
};



//------------------------ Read 3D object from Blender ----------------------

class D3DG1obj{  
public:
  std::vector<Ptn> vtx;
  void open(const char* filename);
  virtual void triangle(int i1, int i2, int i3, int c);
  virtual void quadriple(int i1, int i2, int i3, int i4, int c);
};



#endif
