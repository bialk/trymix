#ifndef meshandalg_h
#define meshandalg_h

#include <algorithm>
#include <vector>
#include <set>
#include <fstream>
#include <math.h>

#ifdef _WIN32
//#include <io.h>
//#include <stdlib.h>
#include <malloc.h>
#define bit_vector vector<char>
#define O_NOCTTY O_BINARY
#else
//#include <unistd.h>
//#include <alloca.h>
#define bit_vector vector<char>
#endif
#include <string.h>


#ifndef err_printf
#define err_printf(n) printf n
#endif

//========================== POINT IN 3D SPACE ===============================

  class Ptn{
  public:
    float x,y,z;
    
    Ptn operator-(const Ptn &r) const {
      Ptn p = {x-r.x, y-r.y, z-r.z};
      return p; 
    }
    Ptn operator+(const Ptn &r) const {
      Ptn p = {x+r.x, y+r.y, z+r.z};
      return p; 
    }
    float operator*(const Ptn &r) const {
      return x*r.x+y*r.y+z*r.z;
    }
    Ptn operator*(const float &r) const {
      Ptn p={x*r,y*r,z*r};
      return p;
    }
    Ptn operator=(const float &r) {
      x=y=z=r;
      return *this;
    }
    float norma() const {
      return sqrt(x*x+y*y+z*z);
    }
    float cos(const Ptn &r) const {
      return (x*r.x + y*r.y + z*r.z)/(norma()*r.norma());
    }
    float *asfloat(float *v){
      v[0]=x; v[1]=y; v[2]=z;
      return v;
    }
    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
    {
        ar & x;
        ar & y;
        ar & z;
    }
}; 


//==================== SURFACE STURCTURES AND ALGORITHMS =====================


class Surf{ 
 public:
    struct TEdg {
      int i1,i2;
      template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
      {
	ar & i1 & i2;
      }
    };
    struct TTrg {
      int i1,i2,i3;
      template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
      {
	ar & i1 & i2 & i3;
      }
    };
    struct TVtx {
      Ptn p; float d;
      template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
      {
	ar & p & d;
      }
    };

    Ptn cnt;
    std::vector<TEdg> edg;
    std::vector<TTrg> trg;
    std::vector<TVtx> vtx;
    void clear(){
      edg.clear(); 
      trg.clear();
      vtx.clear();
    }

    template<class Archive>
      void serialize(Archive & ar, const unsigned int version)
      {
	ar & edg & trg& vtx ;
      }
};

class SurfStore{
public:
  int operator()(Surf *surf, const char *name="test.srf"){
    std::ofstream f(name);
    if(! f.good()) {
      err_printf(("Can't open file of surface (check the name: \"%s\") \n",name)); return 1; 
    }
    f << surf->cnt.x << "\t" << surf->cnt.y << "\t" << surf->cnt.z << std::endl;
    f << surf->vtx.size() << "\t" << surf->edg.size() << "\t" << surf->trg.size() << std::endl;
    size_t i;
    for(i=0;i<surf->vtx.size();i++)
      f << surf->vtx[i].p.x << "\t" <<  surf->vtx[i].p.y << "\t"   
	<< surf->vtx[i].p.z << "\t" <<  surf->vtx[i].d << std::endl;
    for(i=0;i<surf->edg.size();i++)
      f << surf->edg[i].i1 << "\t" <<  surf->edg[i].i2 << std::endl;
    for(i=0;i<surf->trg.size();i++)
      f << surf->trg[i].i1 << "\t" <<  surf->trg[i].i2 << "\t" << surf->trg[i].i3 << std::endl;
    f.close();    
    return 0;
  }
};

class SurfLoad{
public:
  int operator()(Surf *surf, const char *name="test.srf"){
    std::ifstream f(name);
    if(!f.good() || f.eof()) {
      err_printf(("Can't open file of surface (check the name: \"%s\") \n",name)); return 1; 
    }
    surf->clear();    
    size_t i, sz;
    f >> surf->cnt.x >> surf->cnt.y >> surf->cnt.z;
    f >> sz; surf->vtx.resize(sz);
    f >> sz; surf->edg.resize(sz);
    f >> sz; surf->trg.resize(sz);
    for(i=0;i<surf->vtx.size() && !f.eof();i++)
      f >> surf->vtx[i].p.x >>  surf->vtx[i].p.y >>  surf->vtx[i].p.z >>  surf->vtx[i].d;
    for(i=0;i<surf->edg.size() && !f.eof();i++)
      f >> surf->edg[i].i1 >>  surf->edg[i].i2;
    for(i=0;i<surf->trg.size() && !f.eof();i++)
      f >> surf->trg[i].i1 >>  surf->trg[i].i2 >> surf->trg[i].i3;
    return 0;
  }
};


class SurfLoadGTS{
 private:
  int t(int i1,int i2,int i3,int i4){
    if(i1==i3 || i1==i4) return i1;
    if(i2==i3 || i2==i4) return i2;
  }

 public:
  void operator()(const char* name,Surf *surf, float diam = 50){
    std::ifstream f(name); 
    surf->clear();
    int i,nvtx,ntrg,nedg;
    
    if(f.good() && !f.eof()){      
      f >> nvtx >> nedg >> ntrg;
      
      for(i=0;i<nvtx;i++){
	Surf::TVtx vtx;
	f >> vtx.p.x >> vtx.p.y >> vtx.p.z;
	vtx.d=vtx.p.norma();
    vtx.p=vtx.p*(1.0f/vtx.d);
	vtx.d*=diam;
	surf->vtx.push_back(vtx);	
      }
      for(i=0;i<nedg;i++){
	Surf::TEdg edg;
	f >> edg.i1 >> edg.i2;
	edg.i1--; edg.i2--;
	surf->edg.push_back(edg);	
      }
      for(i=0;i<ntrg;i++){
	Surf::TTrg trg;
	int i1,i2,i3;
	f >> i1 >> i2 >> i3;
	i1--;i2--;i3--;	
	
	trg.i1 = t(surf->edg[i1].i1, surf->edg[i1].i2, surf->edg[i2].i1, surf->edg[i2].i2);
	trg.i2 = t(surf->edg[i2].i1, surf->edg[i2].i2, surf->edg[i3].i1, surf->edg[i3].i2);
	trg.i3 = t(surf->edg[i3].i1, surf->edg[i3].i2, surf->edg[i1].i1, surf->edg[i1].i2);
	
	surf->trg.push_back(trg);	
      }
    }    
  }
};




class SurfTxtr{
public:
  class Ttx{
  public:
    float x,y;
  };
  std::vector<Surf::TTrg> trg;
  std::vector<Ttx> tx;
  void clear(){
    trg.clear();
    tx.clear();
  }
};

inline int SurfTxtrStore(SurfTxtr *stxt, const char * filename){

  FILE *f=fopen(filename,"w");
  if(!f){
    err_printf(("Can't open texture file '%s' \n",filename));
    return 1;
  }
  fprintf(f,"%lu %lu\n",stxt->trg.size(),stxt->tx.size());

  std::vector<Surf::TTrg>::iterator it;
  for(it=stxt->trg.begin();it!=stxt->trg.end();it++)
    fprintf(f,"%i %i %i\n",it->i1,it->i2,it->i3);

  std::vector<SurfTxtr::Ttx>::iterator it1;
  for(it1=stxt->tx.begin();it1!=stxt->tx.end();it1++)
    fprintf(f,"%f %f\n",it1->x,it1->y);
  fclose(f);
  return 0;
}

inline int SurfTxtrLoad(SurfTxtr *stxt, const char *filename){
  FILE *f=fopen(filename,"r");
  if(!f){
    err_printf(("Can't open texture file '%s' \n",filename));
    return 1;
  }

  stxt->clear();
  char line[1024],test[1024], *s;
  fgets(line,1024,f); strcpy(test,line);
  s=strtok(line," "); if(!s) { err_printf(("Format error in line\"%s\" \n",test)); return 1; }
  stxt->trg.resize(atoi(s));
  s=strtok(0," \n"); if(!s) { err_printf(("Format error in line\"%s\" \n",test)); return 1; }
  stxt->tx.resize(atoi(s));

  unsigned int i;
  for(i=0;i<stxt->trg.size();i++){
    Surf::TTrg &trg = stxt->trg[i];
    fgets(line,1024,f); strcpy(test,line);
    s=strtok(line," "); if(!s) { err_printf(("Format error in line\"%s\" \n",test)); return 1; }
    trg.i1=atoi(s);
    s=strtok(0," "); if(!s) { err_printf(("Format error in line\"%s\" \n",test)); return 1; }
    trg.i2=atoi(s);
    s=strtok(0," \n");  if(!s){ err_printf(("Format error in line\"%s\" \n",test)); return 1; }
    trg.i3=atoi(s);
  }

  for(i=0;i<stxt->tx.size();i++){
    SurfTxtr::Ttx &tx = stxt->tx[i];
    fgets(line,1024,f); strcpy(test,line);
    s=strtok(line," "); if(!s) { err_printf(("Format error in line\"%s\" \n",test)); return 1; }
    tx.x=atof(s);
    s=strtok(0," "); if(!s) { err_printf(("Format error in line\"%s\" \n",test)); return 1; }
    tx.y=atof(s);
  }
  fclose(f);
  return 0;
}


inline char *mystrtok_r(char* s, char const *brk, char** r){
  if(s==0) s=*r;

  int p=strcspn(s,brk);
  if(s[p]!=0){
    s[p]=0;
    *r=s+p+1;
  } else {
    *r=s+p;
  }
  return s;
}


inline int SurfLoad_WaveFrontObj(Surf *surf, SurfTxtr *stxr, const char *filename){

  FILE *f=fopen(filename,"r");
  if(!f){
    printf("Can't open file '%s' \n",filename);
    return 1;
  }

  surf->clear();
  if(stxr)
    stxr->clear();
    


  char line[1024], *s,*r;
  while(fgets(line,1024,f)){
    char test[1024];
    strcpy(test,line);

    s=strtok(line," "); if(!s) { err_printf(("line format error: %s\n",test)); return 1; };

    if(strcmp(s,"v")==0){
      Surf::TVtx vtx; vtx.d=1;
      s=strtok(0," "); if(!s) { err_printf(("line format error: %s\n",test)); return 1; };
      vtx.p.x=atof(s);
      s=strtok(0," "); if(!s) { err_printf(("line format error: %s\n",test)); return 1; };
      vtx.p.y=atof(s);
      s=strtok(0," "); if(!s) { err_printf(("line format error: %s\n",test)); return 1; };
      vtx.p.z=atof(s);
      surf->vtx.push_back(vtx);
    }
    else if(strcmp(s,"vt")==0){
      SurfTxtr::Ttx tx;
      s=strtok(0," "); if(!s) { err_printf(("line format error: %s\n",test)); return 1; };
      tx.x=atof(s);
      s=strtok(0," "); if(!s) { err_printf(("line format error: %s\n",test)); return 1; };
      tx.y=atof(s);
      if(stxr) 
	stxr->tx.push_back(tx);
    }
    else if(strcmp(s,"f")==0){

      Surf::TTrg t;
      Surf::TTrg tt;

      s=strtok(0," \n");  if(!s) { err_printf(("line format error: %s\n",test)); return 1; }
      s=mystrtok_r(s,"/",&r);
      t.i1=atoi(s)-1;
      s=mystrtok_r(0,"/",&r);
      tt.i1=atoi(s)-1;
      s=mystrtok_r(0,"/",&r);

      s=strtok(0," \n");  if(!s) { err_printf(("line format error: %s\n",test)); return 1; }
      s=mystrtok_r(s,"/",&r);
      t.i2=atoi(s)-1;
      s=mystrtok_r(0,"/",&r);
      tt.i2=atoi(s)-1;
      s=mystrtok_r(0,"/",&r);

      s=strtok(0," \n");  if(!s) { err_printf(("line format error: %s\n",test)); return 1; }
      s=mystrtok_r(s,"/",&r);
      t.i3=atoi(s)-1;
      s=mystrtok_r(0,"/",&r);
      tt.i3=atoi(s)-1;
      s=mystrtok_r(0,"/",&r);

      surf->trg.push_back(t);
      if(stxr)
	stxr->trg.push_back(tt);
    }
  }
  surf->cnt.x=  surf->cnt.y=  surf->cnt.z=0;
  fclose(f);
  return 0;
}






class MeshPlusResample{
public:
  Surf *dst;
  Surf *src;
  int n;
  int vlastpos;
  std::vector<int> ijmap;
  std::vector<int> ijmap2;

  int intidx(int i,int j){
    int k=n-3; i--; j--;
    return vlastpos+(k*(k+1)-(k-i)*(k-i+1))/2 + j;
  }

  void reset(){
    ijmap.clear();
    ijmap2.clear();      
  }

  int idxget(int a, int b, int c, int i, int j){
    int k = n-i-j-1;
    int w = (i?0:1) + (j?0:1) +(k?0:1);
    int ii,p1,p2, *p;
    switch (w){
    case 2: return (i?1:0)*b + (j?1:0)*c +(k?1:0)*a;
    case 1: 
      if(!k) { 
	if(b>c) { std::swap(b,c); std::swap(i,j); }
	p1=b; p2=c; ii=j;
      } else if(!j) { 
	if(a>b) { std::swap(a,b); std::swap(i,k); }
	p1=a; p2=b; ii=i;
      } else if(!i) { 
	if(a>c) { std::swap(a,c); std::swap(j,k); }
	p1=a; p2=c; ii=j;
      }
      p = &ijmap2[ijmap[p1]];  for(;*p!=p2;p+=2);
      return p[1]+ii-1;
    case 0: 
      return intidx(i,j);
    }
  }

  void edgvtx(int a, int b){
    if(a > b) 	std::swap(a,b);
    if(a==b)
      printf("a=b\n");
    int *ij = &ijmap2[ijmap[a]];
    while(*ij!=-1 && *ij!=b) ij+=2;
    if(*ij==-1) {
      *ij=b;
      ij[2]=-1;
      ij[1]=vlastpos;
      vlastpos+=n-2;
      Surf::TVtx &vtx1 = src->vtx[a];
      Surf::TVtx &vtx2 = src->vtx[b];
      Ptn p1 = vtx1.p*vtx1.d;
      Ptn p2 = vtx2.p*vtx2.d;
      int i;
      for(i=1;i<n-1;i++){
	Surf::TVtx vtx;
	Ptn p = (p2*i+p1*(n-1-i))*(1.0/float(n-1));
	vtx.d = p.norma();
	vtx.p = p*(1.0/vtx.d); 
	//test string (to see the new parts surface)
	//vtx.d += 1;
	dst->vtx.push_back(vtx);
      }
    }	    
  }


  void operator()(Surf *_dst,Surf *_src,int _n){
    dst=_dst; src=_src; n=_n;
    int i,j,k,sz;

    // prepare structure for index transformation
    ijmap.resize(src->vtx.size());
    sz = src->vtx.size();
    for(i=0;i<sz;i++)  ijmap[i]=0;
    sz = src->trg.size();
    for(i=0;i<sz;i++){
      Surf::TTrg &trg = src->trg[i];
      if(trg.i1 < trg.i2 ) ijmap[trg.i1]++;
      else                 ijmap[trg.i2]++;
      if(trg.i2 < trg.i3 ) ijmap[trg.i2]++;
      else                 ijmap[trg.i3]++;
      if(trg.i3 < trg.i1 ) ijmap[trg.i3]++;
      else                 ijmap[trg.i1]++;
    }
    sz = src->vtx.size();
    int suml=0;
    for(i=0;i<sz;i++){
      int l = ijmap[i]*2+1;
      ijmap[i] = suml; 
      suml+=l;     
    }
    ijmap2.resize(suml);
    for(i=0;i<sz;i++)  ijmap2[ijmap[i]]=-1;

    // clear resulting array
    dst->clear();

    //insert initial vertexes 
    dst->vtx = src->vtx;
    vlastpos = sz;

    // insert vertexes on edges
    sz = src->trg.size();
    for(i=0;i<sz;i++){
      Surf::TTrg &trg = src->trg[i];
      edgvtx(trg.i1, trg.i2);
      edgvtx(trg.i2, trg.i3);
      edgvtx(trg.i3, trg.i1);
    }
    

    // insert vertices inside triangles
    sz=src->trg.size();
    float w = 1.0/(n-1);
    for(i=0;i<sz;i++){
      Surf::TTrg *trg = &src->trg[i];
      Surf::TVtx &vtx1 = src->vtx[trg->i1];
      Surf::TVtx &vtx2 = src->vtx[trg->i2];
      Surf::TVtx &vtx3 = src->vtx[trg->i3];
      Ptn p1 = vtx1.p*vtx1.d;
      Ptn p2 = vtx2.p*vtx2.d;
      Ptn p3 = vtx3.p*vtx3.d;
      
      for(j=1;j<n;j++){
	for(k=0;k<n-j;k++){
	  int l = n-j-k-1;

	  int a = idxget(trg->i1,trg->i2,trg->i3,j,k);
	  if(a >= vlastpos) { 
	    Ptn p = (p1*l + p2*j  + p3*k)*w;
	    Surf::TVtx vtx; 
	    vtx.d=p.norma();
	    vtx.p=p*(1.0/vtx.d);	  
	    //test string (to see the new parts surface)
	    //vtx.d+=1;
	    dst->vtx.push_back(vtx);
	  }
	  Surf::TTrg t;
	  if(k!=0){
	    t.i1 = a;
	    t.i2 = idxget(trg->i1,trg->i2,trg->i3,j-1,k);
	    t.i3 = idxget(trg->i1,trg->i2,trg->i3,j,k-1);
	    dst->trg.push_back(t);
	  }
	  t.i1 = a;
	  t.i2 = idxget(trg->i1,trg->i2,trg->i3,j-1,k+1);
	  t.i3 = idxget(trg->i1,trg->i2,trg->i3,j-1,k);	    
	  dst->trg.push_back(t);
	}
      }
      vlastpos=dst->vtx.size();
    }
    reset();
  }
};

//========================== 3DMESH AND TOPOLOGY =============================

class Tpg{
public:
  std::vector<int> itpg;
  std::vector<int> tpg;
  inline int *operator[](int i){
    return &tpg[itpg[i]];
  }
  void clear(){
    itpg.clear();
    tpg.clear();
  }
};

class GenerateTpg{
public:
  Surf *surf;
  Tpg *tpg;  
  std::vector<int> ipairs;
  std::vector<int> pairs;
  int idx;
  void operator()(Tpg *_tpg, Surf *_surf){
    surf=_surf; tpg=_tpg;
    int i,sz=surf->trg.size();
    pairinit();
    for(i=0;i<sz;i++){
      Surf::TTrg *trg = &surf->trg[i];
      pairins(trg->i1,trg->i2,trg->i3);
      pairins(trg->i2,trg->i3,trg->i1);
      pairins(trg->i3,trg->i1,trg->i2);
    }
    sz=surf->vtx.size();
    for(i=0;i<sz;i++){
      pairsort(i);
    }  
    pairs.clear();
    ipairs.clear();
  }
  void pairsort(int i){
    int *p = &pairs[ipairs[i]];
    int &n = *p; p++;
    int j,k;
    tpg->itpg[i]=idx;
    int* fan=(int*)alloca(n*3*sizeof(int)),nfan=0;
    for(j=1;j<n;j++){    
      for(k=j;k<n;k++){   
	if(p[j*2-1]==p[k*2]){
	  std::swap(p[j*2],p[k*2]);
	  std::swap(p[j*2+1],p[k*2+1]);
	  break;
	}
	if(p[j*2-1]==p[k*2+1]){
	  std::swap(p[j*2],p[k*2+1]);
	  std::swap(p[j*2+1],p[k*2]);
	  break;
	}
      }
    }
    for(j=0;j<n;j++){
      int k1=(2*j+1)%(2*n);
      int k2=(2*j+2)%(2*n);
      
      if(p[k1]==p[k2]){
	fan[nfan++]=p[k1];
      }else {
	fan[nfan++]=p[k1];
	fan[nfan++]=i;
	fan[nfan++]=p[k2];
      }
    }
    tpg->tpg.push_back(nfan); idx++;
    for(j=0;j<nfan;j++){
      tpg->tpg.push_back(fan[j]); idx++;
    }

  }
  void pairinit(){
    unsigned int i;
    unsigned int sz=surf->vtx.size();
    ipairs.resize(surf->vtx.size());
    for(i=0;i<sz;i++) ipairs[i]=0;

    sz=surf->trg.size();
    for(i=0;i<sz;i++){
      Surf::TTrg *trg = &surf->trg[i];
      ipairs[trg->i1]++;
      ipairs[trg->i2]++;
      ipairs[trg->i3]++;
    }

    sz=surf->vtx.size();
    unsigned int sumidx=0;
    for(i=0;i<surf->vtx.size();i++){
      int n = ipairs[i];
      ipairs[i]=sumidx; 
      sumidx+=n*2+1;
    }

    pairs.resize(sumidx);
    tpg->clear();
    tpg->itpg.resize(surf->vtx.size());
    idx=0;
  }
  void pairins(int i1, int i2, int i3){
    int *p = &pairs[ipairs[i1]];
    int &n = *p; p++;
    int i;
    for(i=0;i<n;i++,p+=2){
      if(p[0]==i2 && p[1]== i3 && p[0]==i2 && p[1]== i3) return;
    }

    p[0]=i2;p[1]=i3; n++;
  }
};


#define   eofft  -1

class WaveFront{
public:
  int buf[200000];
  int *ft1, *ft2; 
  int pft1,pft2, szpft1;
  Surf *surf;
  Tpg *tpg;
  int iter;
  
  WaveFront():ft1(buf),ft2(buf+100000){}

  virtual void draw(int i1,int i2, int i3){
    /*
    Surf::TVtx vtx1 = surf->vtx[i1];
    Surf::TVtx vtx2 = surf->vtx[i2];
    Surf::TVtx vtx3 = surf->vtx[i3];

    float x,y,z;
    vnormal2(x,y,z,
	     vtx2.p.x,vtx2.p.y,vtx2.p.z,
	     vtx3.p.x,vtx3.p.y,vtx3.p.z,
	     vtx1.p.x,vtx1.p.y,vtx1.p.z );
    glNormal3f(x,y,z);
    glVertex3f(vtx1.p.x,vtx1.p.y,vtx1.p.z);
    glVertex3f(vtx2.p.x,vtx2.p.y,vtx2.p.z);
    glVertex3f(vtx3.p.x,vtx3.p.y,vtx3.p.z);
    */
  }

  void move(){
    int i;
    int sz = surf->vtx.size();
    std::bit_vector color(sz);
    std::vector<short int> closed(sz);

    for(i=0;i<sz;i++) {
      color[i]=0; closed[i]=0;
    }

    for(pft1=0;ft1[pft1]!=eofft;pft1++){
      color[ft1[pft1]]=true;
    }
    szpft1=--pft1;
    color[ft2[0]]=true;
    int finish;
    do{
      finish=0;
      pft2=0;
      for(pft1=0;ft1[pft1+1]!=eofft;){
	//one step of front wave
	int i1 = ft2[pft2], i2 = ft1[pft1], i3 = ft1[pft1+1];

	//if(--iter <= 0) 
	//  return; 
	//if(iter==1)
	//  int stop=10;

	if(i1==i3){
	  pft1++;
	  pft2--;
	  closed[i3]--; //if(closed[i3]==0) closed[i3]=0;
	  if(pft2<0) {pft2=0; ft2[0]=ft1[szpft1--];ft1[szpft1+1]=eofft;}
	  finish++;
	} else	if(!closed[i2]){	  
	  //  -c-
	  int *p=(*tpg)[i2];	  
	  int np = *p; p++;
	  for(i=0;p[i]!=i1;i++);
	  for(i++;p[i%np]!=i3;i++){
	    //all new vertexes here
	    int ii= p[i%np];
	    if(color[ii]) 
	      closed[ii]++;	      
	    else 
	      color[ii]=true;
	    ft2[++pft2]=ii;
	    draw(i2,p[(i-1)%np],p[i%np]);
	    finish++;
	  }
	  draw(i2,p[(i-1)%np],i3);
	  pft1++;	  
      }	else if(closed[i1] && closed[i3]){	
	  // z-z-z
	  ft2[++pft2]=i2;
	  pft1++;
	}else{
	  // others
	  ft2[++pft2]=i2;
	  pft1++;
	}
      }
      szpft1=pft2;
      ft1[0]=ft1[pft1]; ft2[++pft2]=eofft;
      pft2=0; pft1=0;
      std::swap(ft1,ft2);
    }while(finish!=0);
  }
};


class SelectVolume{
public:
  typedef std::vector<float> bnd;
 
  inline float zval(const Ptn &p,const Ptn &a, const Ptn &b, const Ptn &c){
    float k_y = (-b.x*c.z+a.x*c.z-a.x*b.z-c.x*a.z+c.x*b.z+b.x*a.z)/
      (a.x*c.y-a.x*b.y-b.x*c.y-c.x*a.y+c.x*b.y+b.x*a.y); 
    float  k_x = (-a.y*c.z+b.y*c.z-b.y*a.z+a.z*c.y-b.z*c.y+b.z*a.y)/
      (a.x*c.y-a.x*b.y-b.x*c.y-c.x*a.y+c.x*b.y+b.x*a.y);
    return (p.x-b.x)*k_x+(p.y-b.y)*k_y+b.z;
  }

  void operator()(int szX, int szY, int szZ, Surf *surf=0){
    std::vector<bnd> bndflat(szX*szY);
    int i;
    if(surf){
      int sz = surf->trg.size();
      for(i=0;i<sz;i++){
	Surf::TTrg &trg = surf->trg[i];
	Surf::TVtx &vtx1 = surf->vtx[trg.i1];
	Surf::TVtx &vtx2 = surf->vtx[trg.i2];
	Surf::TVtx &vtx3 = surf->vtx[trg.i3];
	Ptn p1 = vtx1.p * vtx1.d + surf->cnt;
	Ptn p2 = vtx2.p * vtx2.d + surf->cnt;
	Ptn p3 = vtx3.p * vtx3.d + surf->cnt;
   //Ptn pp1 = p1; pp1.z=0;
   //Ptn pp2 = p2; pp2.z=0;
   //Ptn pp3 = p3; pp3.z=0;

	//float nx,ny,nz;
	//vnormal2(nx,nz,ny,
	//       p1.x,p1.y,p1.z,
	//       p2.x,p2.y,p2.z,
	//       p3.x,p3.y,p3.z );

	if(p1.x > p2.x) std::swap(p1,p2);
	if(p2.x > p3.x) std::swap(p2,p3);
	if(p1.x > p2.x) std::swap(p1,p2);      

	int j,k;
	// triangle rasterization
	for(j=p1.x+1;j<=p3.x;j++){
	  if(j<0 || j>=szX) continue;
	  float y1 = p1.y+(j-p1.x)*(p3.y-p1.y)/(p3.x-p1.x);
	  float y2;
	  if(j<p2.x)
	    y2 = p1.y+(j-p1.x)*(p2.y-p1.y)/(p2.x-p1.x);
	  else
	    y2 = p2.y+(j-p2.x)*(p3.y-p2.y)/(p3.x-p2.x);
	  if(y1>y2) std::swap(y1,y2);
	    
	  for(k=y1+1;k<=y2;k++){	  
	    if(k<0 || k>=szY) continue;
	    Ptn p; p.x=j; p.y=k; 
	    float z = zval(p,p1,p2,p3);	  
	    //boundary membering
	    bndflat[j*szY+k].push_back(z);
	  }
	}      
      }
    }


    for(i=0;i<szX*szY;i++){
      if(bndflat[i].size()%2!=0) 
	printf("Not even number of boundaries (size=%i)\n",(int)bndflat[i].size());
      std::sort(bndflat[i].begin(),bndflat[i].end());      
      //printf("%i: ",i);
      unsigned int j;
      float prev=0;
      for(j=0;j<bndflat[i].size();j++){
	//printf("%f ",bndflat[i][j]);	
	scan(i/szY,i%szY,prev,bndflat[i][j],j%2); 
	prev=bndflat[i][j];
      }
      scan(i/szY,i%szY,prev,szZ,j%2);
      //printf("\n");
    }
  }

  virtual void scan(int x, int y, float zb, float ze, int inside){
    /*
      glBegin(GL_QUADS);
      if(inside){
      drawBnd(x-0.5,y-0.5,zb,
      x+0.5,y+0.5,ze);
      }    
      glEnd();
    */
  } 

  /*
    void drawBnd( float x1, float y1, float z1, float x2, float y2, float z2){
    glNormal3f(-1,0,0);
    
    glNormal3f(0,0,-1);
    glVertex3f(x1,y1,z1);
    glVertex3f(x1,y2,z1);
    glVertex3f(x2,y2,z1);
    glVertex3f(x2,y1,z1);

    glNormal3f(0,0,1);
    glVertex3f(x1,y1,z2);
    glVertex3f(x1,y2,z2);
    glVertex3f(x2,y2,z2);
    glVertex3f(x2,y1,z2);
    } 
  */
};

class MapTxtrCoord{
public:  
  struct TxtrCoord{ float x, y;};
  std::vector<TxtrCoord> arrfxy;  
  TxtrCoord *GetXY(int i){    
    return &arrfxy[i];
  }
  int Load(const char *fn = 0){
    if(!fn)  { err_printf(("MapTxtCoord.Load: file name is zero\n")); return 1; }
    std::ifstream f(fn);
    if( !f.good() ) { err_printf(("MapTxtCoord.Load: Can't open file '%s'\n",fn)); return 1; }
    int i, n;
    f >> n;
    arrfxy.resize(n);
    for(i=0;i<n;i++){
      TxtrCoord &fxy = arrfxy[i];
      f >> fxy.x >> fxy.y;
    }
    return 0;
  }
  int Store(const char *fn = 0){
    if(!fn)  { err_printf(("MapTxtCoord.Store: file name is zero\n")); return 1; }
    std::ofstream f(fn);
    if( !f.good() ) { err_printf(("MapTxtCoord.Store: Can't open file '%s'\n",fn)); return 1; }
    int i, n = arrfxy.size();
    f << n << std::endl;
    for(i=0;i<n;i++){
      TxtrCoord &fxy = arrfxy[i];
      f << fxy.x << " " << fxy.y << std::endl;
    }
    return 0;
  }
};


#endif // math3d_h
