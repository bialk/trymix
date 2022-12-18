#ifndef imageandfft_h
#define imageandfft_h

#ifndef err_printf
#define err_printf(n) printf n
#endif

template<class T>
class rgbimg: public std::vector<T>{
public:
  int szx,szy;
};

template<class T>
int StorePPM(T *rgb, int szx, int szy, const char *name="test.ppm"){
  if(!rgb) {err_printf(("StorePPM: input array is zero\n")); return 1;}
  FILE *h = fopen(name,"wb");    
  if(!h) {err_printf(("StorePPM: can't open file (check the filename (\"%s\")\n",name)); return 1;}
  fprintf(h,"P6\n%i %i\n255\n",szx,szy);        
  int i,j;
  for(j=0;j<szy;j++)
    for(i=0;i<szx;i++){
      int idx = (szx*j+i)*3;
      unsigned char v[3]={rgb[idx],rgb[idx+1],rgb[idx+2]}; 
      fwrite(v,1,3,h);    
    }
  fclose(h);
  return 0;
}  


template<class T>
int LoadPPM(rgbimg<T> &rgb,const char *name="test.ppm"){
  if(!&rgb) {err_printf(("LoadPPM: input array is zero\n")); return 1;}
  FILE *h = fopen(name,"rb");    
  if(!h) {err_printf(("LoadPPM: can't open file (check the filename (\"%s\")\n",name)); return 1;}
  char s[255];
  //fscanf(h,"%s\n%i %i\n",s,&rgb.szx,&rgb.szy);
  
  fgets(s,255,h); s[2]=0;
  if(strcmp(s,"P6")){
    err_printf(("File format is not recognized\n"));
    return 0;
  }
  
  do fgets(s,255,h);  while(s[0]=='#');
  sscanf(s,"%i %i",&rgb.szx,&rgb.szy);

  int maxval;
  fscanf(h,"%i",&maxval);
  
  // scan until the data begins
  while( 1==fread(&s,1,1,h) && s[0]!='\n' ){};
  
  rgb.resize(rgb.szx*rgb.szy*3);
  int i,j;
  for(j=0;j<rgb.szy;j++)
    for(i=0;i<rgb.szx;i++){
      int idx = (rgb.szx*j+i)*3;
      if(maxval<256){
	unsigned char v[3];
	fread(v,1,3,h);	
	rgb[idx]=v[0]; rgb[idx+1]=v[1]; rgb[idx+2]=v[2]; 
      }
      else{
	unsigned short int v[3];
	fread(v,1,6,h);	
	rgb[idx]=v[0]; rgb[idx+1]=v[1]; rgb[idx+2]=v[2]; 
      }
    }
  fclose(h);
  return 0;
}  

template<class T>
void TorusShift(rgbimg<T> &rgb , int x, int y){
  int i,j;
  rgbimg<T>rgb2=rgb;
  for(j=0;j<rgb.szy;j++){
    for(i=0;i<rgb.szx;i++){
      int idx=(j*rgb.szx+i)*3;
      int idx2=(((j+y)%rgb.szy)*rgb.szx+(i+x)%rgb.szx)*3;
      rgb[idx2]=rgb2[idx];
      rgb[idx2+1]=rgb2[idx+1];
      rgb[idx2+2]=rgb2[idx+2];
    }
  }
}

#endif //image and fft
