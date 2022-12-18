#ifdef _WIN32
#include <windows.h>
#endif

#include <FL/gl.h>

#include "eventlog.h"
#include "mathstruct.h"


// D3DG1obj
//==============================================================================

void D3DG1obj::open(const char* filename){
  FILE *f = fopen(filename, "r");
  if (!f) { err_printf(("Can't open file \"%s\"\n",filename)); return; }
  char fcode[10];
  int nv=0;
  fscanf(f,"%10s", fcode);
  if ( strncmp(fcode,"3DG1",4) ) { 
    err_printf(("Wrong file format \"%s\" \n",filename));
    fclose(f);
  }
  fscanf(f,"%i",&nv);
  vtx.resize(nv);
  int i;
  for(i=0;i<nv;i++){
    fscanf(f,"%f %f %f ", &vtx[i].x, &vtx[i].y, &vtx[i].z);
  }
    
  while(!feof(f)){
    int k,c,i1,i2,i3,i4;
    fscanf(f,"%i",&k); 
    switch(k){
    case 3:
      fscanf(f,"%i %i %i %x", &i1, &i2, &i3, (unsigned int*)&c);
      triangle(i1,i2,i3,c);
      break;
    case 4: 
      fscanf(f,"%i %i %i %i %x",  &i1, &i2, &i3, &i4, (unsigned int*)&c);
      quadriple(i1,i2,i3,i4,c);
      break;
    }
  }
  fclose(f);
}

void D3DG1obj::triangle(int i1, int i2, int i3, int c){   
  glBegin(GL_TRIANGLES);
  Ptn &vtx1 = vtx[i1];  Ptn &vtx2 = vtx[i2]; Ptn &vtx3 = vtx[i3];
  float x,y,z;
  vnormal2(x,y,z,
	   vtx2.x,vtx2.y,vtx2.z,
	   vtx3.x,vtx3.y,vtx3.z,
	   vtx1.x,vtx1.y,vtx1.z );
  glNormal3f(x,y,z);         
  //glColor4f(rgba.r/555.,rgba.g/555.,rgba.b/555., 1);
  glVertex3f(vtx1.x,vtx1.y,vtx1.z);
  glVertex3f(vtx2.x,vtx2.y,vtx2.z);
  glVertex3f(vtx3.x,vtx3.y,vtx3.z);    
  glEnd();
}

void D3DG1obj::quadriple(int i1, int i2, int i3, int i4, int c){
  // this two lines for future
  //glBegin(GL_QUADS);
  //glEnd();
  triangle(i1,i2,i3,c);
  triangle(i3,i4,i1,c);
}

