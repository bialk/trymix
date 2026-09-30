#include "BlurTests.h"

#include "GaussBlurEngine.h"
#include "BoxBlur.h"
#include "apputil/parallelWithBarrier.h"
#include "apputil/fillChessBoard.h"

#include <QDebug>
#include <QImage>
#include <QDateTime>

int blurTest(int width, int height, int radius, int chessBoxSize, QImage& qimg, bool opencl_bool)
{
  const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
  std::unique_ptr<float[]> testImageIn(new float[pixelCount * 3]);
  fillChessBoard(testImageIn.get(), width, height, width, chessBoxSize);

  static BlurTests::GaussBlurEngine gb;
  auto startTime = QDateTime::currentDateTime();
  if(opencl_bool){
    gb.doBlur(testImageIn.get(), width, height, radius);
    qInfo() << gb.getInfoString();
  }  
  else{
    std::unique_ptr<float[]> aux(new float[pixelCount * 3]);
    gaussBlur_4_cpu(testImageIn.get(), aux.get(), width, height, radius);
    qInfo() << "Using CPU Blur";
  }
  auto msec_spent = startTime.msecsTo(QDateTime::currentDateTime());

  qimg = QImage(width,height,QImage::Format_RGBA32FPx4);
  float* qimgPtr = reinterpret_cast<float*>(qimg.bits());
  std::atomic<size_t> row{0};
  const size_t w4 = static_cast<size_t>(width) * 4;
  auto testImagePtr = testImageIn.get();
  parallelWithRunLoop([&](auto /*threadTotal*/, auto /*threadNum*/, auto& /*bwc*/){
    for( auto r = row++; r<height; r = row++){
      auto rdestp = qimgPtr + r * static_cast<size_t>(width) * 4;
      auto rsrcp  = testImagePtr + r * static_cast<size_t>(width) * 3;
      size_t csrc = 0;
      for(size_t cdst = 0; cdst < w4; cdst += 4, csrc += 3){
        rdestp[cdst+0] = rsrcp[csrc+0];
        rdestp[cdst+1] = rsrcp[csrc+1];
        rdestp[cdst+2] = rsrcp[csrc+2];
        rdestp[cdst+3] = 1.0;
      }
    }
  });
  return msec_spent;
}
