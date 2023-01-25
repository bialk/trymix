#ifndef mousectrl_h
#define mousectrl_h

#include <glm/glm.hpp>

//------------------------ free rotation and shift control by mouse ---------------------

class MouseMoveAndShift{
public:

  // parameters of project transform 
  // and viewport transform
  int wndw,wndh;
  float mi[16]; //inverse projection matrix
  float* m; //projection matrix

  // mouse coord at the start operation
  int sx0,sy0;

  //output rotation increment
  float norm_rx, norm_ry, norm_rz, angle_r;
  //output shift increment
  float shift_x, shift_y, shift_z;

  
  inline static float rad(float g);

  MouseMoveAndShift();
  // state of operation
  int oper;

  void StartMove(int x, int y );
  void StartRotate(int x, int y);
  void StopMOper();
  int ContinueMOper(int x, int y);
};

class  Ctrl3DRotate{
 public:
  int   sx0,sy0;
  int cx,cy,cz;
  float norm_rx, norm_ry, norm_rz, angle_r;
  float m[16];

  void setcenter(int x, int y, int z);
  void start(int x, int y, float *mtrx = 0);
  void drag(int x, int y);
  void drag(int x, int y, float *m);
  void stop();
};

#endif
