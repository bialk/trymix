#ifndef eventballglut_h
#define eventballglut_h
#include "eventball.h"

class EventBallGlut: public EventBall{
public:
   void glutsetmodifiers();
   void keyboard_cb(unsigned char key, int vx, int vy);
   void keyboardup_cb(unsigned char key, int vx, int vy);
   void mouse_cb(int button, int state, int vx, int vy);
   void motion_cb(int vx, int vy);
};


#endif
