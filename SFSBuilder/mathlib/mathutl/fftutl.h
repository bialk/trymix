#ifndef fftutl.h
#define fftutl_h

class FFTShift{
public:
  double maxpeak;
  int maxx, maxy;

  fftw_complex *inl, *inr,*outr,*outl;
  fftw_plan  pfl,pfr,pbr;
  int nx,ny;

  FFTShift():inl(0),inr(0),outr(0),outl(0),pfl(0),pfr(0),pbr(0){}
  ~FFTShift(){Reset();}

  void Prepare(int x=720, int y=576){    
    Reset();
    nx = x; ny = y;
    inl = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nx * ny);
    inr = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nx * ny);
    outr = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nx * ny);
    outl = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nx * ny);
    pfl = fftw_plan_dft_2d(ny, nx, inl, outl, FFTW_FORWARD, FFTW_ESTIMATE);   
    pfr = fftw_plan_dft_2d(ny, nx, inr, outr, FFTW_FORWARD, FFTW_ESTIMATE);
    pbr = fftw_plan_dft_2d(ny, nx, outr, inr, FFTW_BACKWARD, FFTW_ESTIMATE);
  }

  void Reset(){
    fftw_free(inl); 
    fftw_free(inr);
    fftw_free(outr);
    fftw_free(outl);
    inl=inr=outr=outl=0; 
    fftw_destroy_plan(pfl);
    fftw_destroy_plan(pfr);
    fftw_destroy_plan(pbr);
    pfl=pfr=pbr=0;
  }


  void SetLImage(){
    fftw_execute(pfl);
  }


  void SetRFFT(){
    fftw_execute(pfr);
    int i,j;
    for(i=0;i<nx*ny;i++){
      double re= outl[i][0]*outr[i][0] - outl[i][1]*-outr[i][1];
      double im= outl[i][0]*-outr[i][1] + outl[i][1]*outr[i][0];
      outr[i][0]=re;outr[i][1]=im;      
    }
    fftw_execute(pbr);
  }

  void SetRImage(){
    fftw_execute(pfr);
    int i,j;
    for(i=0;i<nx*ny;i++){
      double re= outl[i][0]*outr[i][0] - outl[i][1]*-outr[i][1];
      double im= outl[i][0]*-outr[i][1] + outl[i][1]*outr[i][0];
      outr[i][0]=re;outr[i][1]=im;      
    }
    fftw_execute(pbr);

    maxpeak = sqrt(inr[0][0]*inr[0][0]+inr[0][1]*inr[0][1]);
    maxx=maxy=0;
    for(i=0;i<ny;i++){
      for(j=0;j<nx;j++){
	int idx = i*nx+j;
	double v = sqrt(inr[idx][0]*inr[idx][0]+inr[idx][1]*inr[idx][1]);
	if(v>maxpeak){
	  maxpeak = v; maxx=j;maxy=i;
	}
      }
    }
  }
};


class FFTFilter{
public:

  fftw_complex *out,*fout;
  double *in, *fin;
  fftw_plan  pff,pbw,pff2;
  int dx,nx,ny;

  //DFT 
  FFTFilter():out(0),in(0),fin(0),fout(0),pff(0),pbw(0),pff2(0){}
  ~FFTFilter(){Clear();}
  void Clear(){
    fftw_destroy_plan(pff);
    fftw_destroy_plan(pbw);
    fftw_destroy_plan(pff2);
    fftw_free(in); fftw_free(out);
    fftw_free(fin); fftw_free(fout);
  }
  void PrepareSize(int x, int y){
    Clear();
    nx=x; ny=y;
    dx=(nx/2+1);
    fin = (double*)fftw_malloc(sizeof(double) * nx*ny);
    fout = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * dx * ny);
    in = (double*)fftw_malloc(sizeof(double) * nx*ny);
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * dx * ny);
    pff2= fftw_plan_dft_r2c_2d(ny, nx, fin, fout, FFTW_ESTIMATE);    
    pff= fftw_plan_dft_r2c_2d(ny, nx, in, out, FFTW_ESTIMATE);    
    pbw= fftw_plan_dft_c2r_2d(ny, nx, out, in, FFTW_ESTIMATE);
  }
  void PrepareFilter(){
    fftw_execute(pff2);
  }
  void Apply(){
    double *in;
    fftw_execute(pff); /* repeat as needed */
    
    // filtration in frequency domain
    int i,j;
    for(i=0;i<ny;i++){
      // the last dimention from real to comples fft is (N/2+1) see docs
      for(j=0;j<dx;j++){ 
	int ind = i*dx+j;
	/*
	  if(sqrt(i*i+j*j)<80){
	  out[ind][0]=out[ind][1]=0;
	  }
	*/	
	double re= out[ind][0]*fout[ind][0] - out[ind][1]*fout[ind][1];
	double im= out[ind][0]*fout[ind][1] + out[ind][1]*fout[ind][0];
	out[ind][0]=re;out[ind][1]=im;
      }
    }  
  
    fftw_execute(pbw);
  }
  void test(unsigned char* rgbimg){
    PrepareSize(720,576);

    int i,j;
    double sum=0;
    for(j=0;j<ny;j++)
      for(i=0;i<nx;i++){
	int ind = j*nx+i;
	in[ind]=rgbimg[ind*3];
	double d = sqrt((j-ny/2)*(j-ny/2)+(i-nx/2)*(i-nx/2));
	float v = exp(-2*d);
	fin[ (j+ny/2)%ny*nx+(i+nx/2)%nx]= v;
	sum+=v;
      }
    for(i=0;i<nx*ny;i++) fin[i]/=sum;

    PrepareFilter();

    Apply();

    for(j=0;j<ny;j++)
      for(i=0;i<nx;i++){
	int ind = j*nx+i;
	float v=128+(rgbimg[ind*3]-in[ind]/(ny*nx))*10;
	if(v>255) v=255; if(v<0) v=0;
	rgbimg[ind*3]=rgbimg[ind*3+1]=rgbimg[ind*3+2]=v;
      }
  }

};



#endif
