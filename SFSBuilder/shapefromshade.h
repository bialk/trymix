#ifndef shapefromshade_h
#define shapefromshade_h


class ShapeFromShade{
public:

  int w, h;

  float s[4][3]; // four sources of illumination
  float s_alb[4][3]; // four sources of illumination for albedo


  float* image[4];  //w*h*3
  float* albedo;  //w*h*3

  float* data_norm; //(w*h*3);
  float* data_attd; //(w*h);

  void build();
};

#endif
