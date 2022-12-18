#include "eventballglut.h"
#include <GL/glut.h>


void EventBallGlut::glutsetmodifiers(){
   keymod=0;
   if(GLUT_ACTIVE_SHIFT==glutGetModifiers())
      keymod|=KM_SHIFT;
   if(GLUT_ACTIVE_CTRL==glutGetModifiers())
      keymod|=KM_CTRL;
   if(GLUT_ACTIVE_ALT==glutGetModifiers())
      keymod|=KM_ALT;
}

void EventBallGlut::keyboard_cb(unsigned char key, int vx, int vy){
   retcode=EB_RET_NONE;
   glutsetmodifiers();
   x=vx; y=vy;
   addkey(key);
   lastev=K_DOWN;
}

void EventBallGlut::keyboardup_cb(unsigned char key, int vx, int vy){
   retcode=EB_RET_NONE;
   glutsetmodifiers();
   x=vx; y=vy;
   rmkey(key);
   lastev=K_UP;
}

void EventBallGlut::mouse_cb(int button, int state, int vx, int vy){
   retcode=EB_RET_NONE;
   glutsetmodifiers();
   x=vx; y=vy;
   int key=0;
   switch(button){
         case GLUT_LEFT_BUTTON: key=1;  break;
         case GLUT_MIDDLE_BUTTON: key=2;  break;
         case GLUT_RIGHT_BUTTON: key=4;  break;
   }
   if(state==GLUT_UP){
      lastev=M_UP;
      msbtn &= ~key;
   }
   else if(state==GLUT_DOWN){
      lastev=M_DOWN;
      msbtn |= key;
   }
}

void EventBallGlut::motion_cb(int vx, int vy){
   retcode=EB_RET_NONE;
   x=vx; y=vy;
   lastev=M_DRAG;
}
