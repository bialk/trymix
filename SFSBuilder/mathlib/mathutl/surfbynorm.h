#ifndef surfbynorm_h
#define surfbynorm_h

#include "../lineqsolver/linsolver_cpp.h"
#include "../mathutl/meshandalg.h"


class SurfByNorm2{
public:
  // initial surface 
  Surf *surf;
  // bitmask of triangle role:
  //     bit  0x1 - boundary triangle
  std::vector<int> trgb;
  // distance from center for boundary triangles and 
  // distance of triangles after calculations
  std::vector<float> trgd;
  // normal vectors for each triangle
  std::vector<Ptn> trgn;
  // initial postition of the rays triangles
  std::vector<Ptn> p0;
  // initial postition of the rays of vertexes
  std::vector<Ptn> p;

  
  // array of vectors of triangle center
  std::vector<Ptn> v0;
  // array of vectors of triangle vertex
  std::vector<Ptn> v;

  // auxilary array pairs of vertex - triangle
  std::set<indx> pairs;

  class Solver: public  SparceSolver {
  public: 
    std::vector<float> *trgd;
    virtual void vectorx(int i, float a){
      // put here the code for accepting solution
      (*trgd)[i]=a;
    }
  } ssolver; 

  SurfByNorm2():surf(0){
    ssolver.aux_file_prefix="test";
    ssolver.aux_delete=0;
    ssolver.trgd=&trgd;
  }

  void Init(Surf *s){
    ssolver.init();
    surf=s;
    ssolver.sizem=surf->trg.size();
    trgn.resize(surf->trg.size());
    trgb.resize(surf->trg.size());
    trgd.resize(surf->trg.size());
    v0.resize(surf->trg.size());
    v.resize(surf->vtx.size());
    p0.resize(surf->trg.size());
    p.resize(surf->vtx.size());

    Ptn pnull={0,0,0};
    std::fill(p.begin(), p.end(), pnull);
    std::fill(v.begin(), v.end(), pnull);
    std::fill(p0.begin(), p0.end(), pnull);
    std::fill(v0.begin(), v0.end(), pnull);

    int i;
    Ptn x0,n;

    for(i=0;i<surf->trg.size();i++){
      Surf::TTrg &trg = surf->trg[i];
      
      Surf::TVtx &vtx1 = surf->vtx[trg.i1];
      Surf::TVtx &vtx2 = surf->vtx[trg.i2];
      Surf::TVtx &vtx3 = surf->vtx[trg.i3];
      vnormal2(n.x,n.y,n.z,
               vtx1.p.x*vtx1.d,vtx1.p.y*vtx1.d,vtx1.p.z*vtx1.d,
	       vtx2.p.x*vtx2.d,vtx2.p.y*vtx2.d,vtx2.p.z*vtx2.d,
               vtx3.p.x*vtx3.d,vtx3.p.y*vtx3.d,vtx3.p.z*vtx3.d );
      x0=vtx1.p+vtx2.p+vtx3.p;
      x0=x0*(1.0/x0.norma());      
      float d=vtx1.d*(n*vtx1.p)/(n*x0);
      trgn[i]=n;
      trgd[i]=d;
      trgb[i]=0;
      v0[i]=x0;

    }
  }

  void Solve(){
    int i;
    pairs.clear();
    indx ij;
    for(i=0;i<surf->trg.size();i++){      
      Surf::TTrg *trg = &surf->trg[i];
      ij.j=i;
      ij.i=trg->i1;  pairs.insert(ij);      
      ij.i=trg->i2;  pairs.insert(ij);      
      ij.i=trg->i3;  pairs.insert(ij);
    }

    std::set<indx>::iterator it;
    int vidx = pairs.begin()->i;
    int tcnt = 0;
    int tidx[120];
    for(it=pairs.begin();it!=pairs.end();it++){
      if(vidx==it->i) {
	tidx[tcnt]=it->j;
	tcnt++;
      }
      else{
	vtxproc(vidx,tcnt,tidx);
	vidx=it->i;
	tidx[0]=it->j;
	tcnt=1;	
      }
    }
    vtxproc(vidx,tcnt,tidx);
    ssolver.solve();

    // restore vertex coordinates
    vidx = pairs.begin()->i;
    tcnt = 0;
    float d=0;
    float weight=0;
    float n;
    Ptn ptmp;
    for(it=pairs.begin();it!=pairs.end();it++){
      if(vidx==it->i) {
	n=trgn[it->j].norma();
	//d+=n*trgd[it->j]*(trgn[it->j]*v0[it->j])/(trgn[it->j]*surf->vtx[it->i].p);
	d+= n*(trgd[it->j]*(trgn[it->j]*v0[it->j])/(trgn[it->j]*v[it->i]) - 
	       (p[it->i]-p0[it->j])*trgn[it->j]/(trgn[it->j]*v[it->i])); 
	weight+=n;
      }
      else{
	ptmp=v[vidx]*(d/weight)+p[vidx];
	surf->vtx[vidx].p=ptmp*(1./ptmp.norma());
	surf->vtx[vidx].d=ptmp.norma();//d/weight;//tcnt;
	vidx=it->i;
	n=trgn[it->j].norma()+0.01;
	//d=n*trgd[it->j]*(trgn[it->j]*v0[it->j])/(trgn[it->j]*surf->vtx[it->i].p);
	d=n * (trgd[it->j]*(trgn[it->j]*v0[it->j])/(trgn[it->j]*v[it->i])-
	       (p[it->i]-p0[it->j])*trgn[it->j]/(trgn[it->j]*v[it->i]));
	weight=n;
      }
    }    
    //surf->vtx[vidx].d=d/weight;
	ptmp=v[vidx]*(d/weight)+p[vidx];
	surf->vtx[vidx].p=ptmp*(1./ptmp.norma());
	surf->vtx[vidx].d=ptmp.norma();//d/weight;//tcnt;
  }

  void vtxproc(int vidx, int tctn, int *tidx){
    int i,j,m,n;    
    Ptn vi=v[vidx];
    Ptn pi=p[vidx];
    for(m=0;m<tctn;m++){      
      i=tidx[m];
      //boundary condition
      if( trgb[i] & 0x1 ) { ssolver.mtrk(i,i)+=1; ssolver.mtrb(i)+=trgd[i]; continue ;}
      double ai = (trgn[i]*v0[i])/(trgn[i]*vi);
      double bi = ((pi-p0[i])*trgn[i])/(trgn[i]*vi);      

      ssolver.mtrk(i,i)-=2*ai*ai;
      ssolver.mtrb(i)-=2*ai*bi;

      for(n=0;n<tctn;n++){
	j=tidx[n];
       double aj = (trgn[j]*v0[j])/(trgn[j]*vi);
       double bj = ((pi-p0[j])*trgn[j])/(trgn[j]*vi);
	//boundary condition
	if(trgb[j] & 0x1 ){ 
	  ssolver.mtrb(i)-=trgd[j]*(2.0/tctn) * ai * aj; 
	}
	else{
	  ssolver.mtrk(i,j)+=(2.0/tctn) * ai * aj;
	}
 	ssolver.mtrb(i)+=(2.0/tctn) * ai * bj;
      }
    }
  }
};



void testSurfByNorm2_t1(void){

  Surf surf;
  SurfByNorm2 sbn;

  SurfLoad()(&surf,"head_tdm02.srf");
  sbn.Init(&surf);

  int i;
  Ptn x0;
  for(i=0; i<sbn.trgn.size();i++){
    sbn.p0[i]=sbn.v0[i]*(23/sbn.v0[i].norma());
    x0=sbn.v0[i];
    if(0*x0.x+0*x0.y+1*x0.z>0.5){

      if(0*x0.x+0*x0.y+1*x0.z>0.8){
	if(0*x0.x+0*x0.y+1*x0.z<0.9){
	  x0.x=0; x0.y=0; x0.z=1;
	  //x0.x+=1.1*(float(rand())/RAND_MAX-0.5); x0.y+=1.1*(float(rand())/RAND_MAX-0.5); 
	}
      }
      sbn.trgn[i]=x0;
    }
    else {
      //xsbn.trgn[i]=n;
      //sbn.trgb[i]=0x1;
    }
  }
  sbn.trgb[0]=0x1;
  //this is initial compensation of the shift 
  sbn.trgd[0]-=23;


  for(i=0;i<sbn.p.size();i++){
    sbn.p[i]=surf.vtx[i].p*(30/surf.vtx[i].p.norma());
    sbn.v[i]=surf.vtx[i].p;
  }

  sbn.Solve();
  /*
  for(i=0;i<sbn.p.size();i++){
    surf.vtx[i].d+=(30/surf.vtx[i].p.norma());
  }
  */
  SurfStore()(&surf,"result-test-1.srf");  

}


void testSurfByNorm2_t2(void){

  Surf surf;

  //create rectangular flat surface
  
  int i,j;
  int n=200, m=400;
  surf.vtx.resize((n+1)*(m+1));
  surf.trg.resize(n*m*2);

  for(i=0;i<n;i++){
    for(j=0;j<m;j++){      
      
      int i0=n*j+i;

      int i1=(n+1)*j+i;
      int i2=(n+1)*j+(i+1);
      int i3=(n+1)*(j+1)+(i+1);
      int i4=(n+1)*(j+1)+i;

      Surf::TTrg &trg1 = surf.trg[i0*2];
      Surf::TTrg &trg2 = surf.trg[i0*2+1];

      trg1.i1=i1;
      trg1.i2=i2;
      trg1.i3=i3;
      trg2.i1=i1;
      trg2.i2=i3;
      trg2.i3=i4;


      Surf::TVtx &vtx1 = surf.vtx[i1];
      Surf::TVtx &vtx2 = surf.vtx[i2];
      Surf::TVtx &vtx3 = surf.vtx[i3];
      Surf::TVtx &vtx4 = surf.vtx[i4];

      vtx1.p.x=i; vtx1.p.y=j; vtx1.p.z=0; vtx1.d=1;  
      vtx2.p.x=i+1; vtx2.p.y=j; vtx2.p.z=0; vtx2.d=1;  
      vtx3.p.x=i+1; vtx3.p.y=j+1; vtx3.p.z=0; vtx3.d=1;  
      vtx4.p.x=i; vtx4.p.y=j+1; vtx4.p.z=0; vtx4.d=1;  

    }      
  }
  surf.cnt.x= surf.cnt.y=surf.cnt.z=0;

  //SurfStore()(&surf,"result-test-2.srf");   return;

  SurfByNorm2 sbn;
  sbn.Init(&surf);
  
  Ptn x0;
  for(i=0; i<sbn.trgn.size();i++){
    Surf::TTrg &trg = surf.trg[i];
    sbn.p0[i]=(surf.vtx[trg.i1].p+surf.vtx[trg.i2].p+surf.vtx[trg.i3].p)*(1./3);
    sbn.v0[i].x=0; sbn.v0[i].y=0; sbn.v0[i].z=1;

    float rx=sbn.p0[i].x-n/2, ry=sbn.p0[i].y-m/2;

    sbn.trgn[i].x=0;    sbn.trgn[i].y=0;    sbn.trgn[i].z=1;
    if(sqrt(rx*rx+ry*ry)<n/2.2){
      sbn.trgn[i].x=rx;    sbn.trgn[i].y=ry;    sbn.trgn[i].z=m/8;
      if(sqrt(rx*rx+ry*ry)<n/5.2) sbn.trgn[i].z=-m/8;
     }
    sbn.trgn[i]=sbn.trgn[i]*(1./sbn.trgn[i].norma());
    sbn.trgb[i]=0;
  }
  //i = n*m+n;
  //i =0;
  //sbn.trgb[i]=0x1;
  //sbn.trgd[i]=80;

  for(i=0;i<sbn.p.size();i++){
    sbn.p[i]=surf.vtx[i].p;
    sbn.v[i].x=0; sbn.v[i].y=0;  sbn.v[i].z=1;
  }

  sbn.Solve();
  double sum=0;
  for(i=0; i<sbn.trgn.size();i++)   sum+=sbn.trgd[i];
  printf("sum=%le\n",sum);

  SurfStore()(&surf,"result-test-2.srf");  

}


#endif
