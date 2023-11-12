#ifndef BLURTESTS_H
#define BLURTESTS_H

class QImage;

void blurTest(int width, int height, int radius, int chessBoxSize, QImage& qimg, bool opencl_bool);

#endif
