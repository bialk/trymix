#include "BlurTests.h"

#include "GaussBlurEngine.h"
#include "BoxBlur.h"

#include <QDebug>
#include <QImage>
#include <QDateTime>

void fillChessBoard(float *img, int w, int h, int stride, int boxSize){
  std::atomic<size_t> row{0};
  auto w3 = w*3;
  auto boxSize3 = boxSize*3;
  parallel([&](size_t i){
    for( auto r = row++; r<h; r = row++){
      auto rp = img + r*stride*3;
      for(int c=0; c < w3; c+=3){
        auto fill = (c/boxSize3)%2 == (r/boxSize)%2;
        rp[c+0] = (fill? 1:0);
        rp[c+1] = rp[c+2] = (fill? 0:1);
      }
    }
  });
}

int blurTest(int width, int height, int radius, int chessBoxSize, QImage& qimg, bool opencl_bool)
{
  std::unique_ptr<float[]> testImageIn(new float[width*height*3]);  
  fillChessBoard(testImageIn.get(), width, height, width, chessBoxSize);

  static BlurTests::GaussBlurEngine gb;
  auto startTime = QDateTime::currentDateTime();
  if(opencl_bool){
    gb.doBlur(testImageIn.get(), width, height, radius);
    qInfo() << gb.getInfoString();
  }  
  else{
    std::unique_ptr<float[]> aux(new float[width*height*3]);
    gaussBlur_4_cpu(testImageIn.get(), aux.get(), width, height, radius);
    qInfo() << "Using CPU Blur";
  }
  auto msec_spent = startTime.msecsTo(QDateTime::currentDateTime());

  qimg = QImage(width,height,QImage::Format_RGBA32FPx4);
  float* qimgPtr = reinterpret_cast<float*>(qimg.bits());
  std::atomic<size_t> row{0};
  auto w4 =width*4;
  auto testImagePtr = testImageIn.get();
  parallel([&](auto /*threadNum*/){
    for( auto r = row++; r<height; r = row++){
      auto rdestp = qimgPtr + r*width*4;
      auto rsrcp  = testImagePtr + r*width*3;
      auto csrc = 0;
      for(int cdst=0; cdst < w4; cdst+=4, csrc += 3){
        rdestp[cdst+0] = rsrcp[csrc+0];
        rdestp[cdst+1] = rsrcp[csrc+1];
        rdestp[cdst+2] = rsrcp[csrc+2];
        rdestp[cdst+3] = 1.0;
      }
    }
  });
  return msec_spent;
}
