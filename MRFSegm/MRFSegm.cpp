#include "MRFSegm.h"

#include "MRFSegm.h"
#include "apputil/parallelWithBarrier.h"
#include "apputil/fillChessBoard.h"

#include <QDebug>
#include <QImage>
#include <QDateTime>

class FilterGrid {
public:
  FilterGrid(int w, int h, int n);
  ~FilterGrid()=default;

  void estimate(int w, int h, std::vector<float> const& weightField, std::vector<float> rbgImage);

private:
  std::vector<float> w; // parameters
  std::vector<int> indices; // mask indices
};

void FilterGrid::estimate(int w, int h,
                          std::vector<float> const& weightField,
                          std::vector<float> rbgImage)
{
  //collecting matrix
}


int MRFSegm_Test(int width, int height, int radius, int chessBoxSize, QImage& qimg, bool opencl_bool)
{
  auto startTime = QDateTime::currentDateTime();
  auto msec_spent = startTime.msecsTo(QDateTime::currentDateTime());

  std::unique_ptr<float[]> testImageIn(new float[width*height*3]);
  fillChessBoard(testImageIn.get(), width, height, width, chessBoxSize);

#ifdef off
  static MRFSegm::GaussBlurEngine gb;
  if(opencl_bool){
    gb.doBlur(testImageIn.get(), width, height, radius);
    qInfo() << gb.getInfoString();
  }
  else{
    std::unique_ptr<float[]> aux(new float[width*height*3]);
    gaussBlur_4_cpu(testImageIn.get(), aux.get(), width, height, radius);
    qInfo() << "Using CPU Blur";
  }
#endif

  qimg = QImage(width,height,QImage::Format_RGBA32FPx4);
  float* qimgPtr = reinterpret_cast<float*>(qimg.bits());
  std::atomic<size_t> row{0};
  auto w4 =width*4;
  auto testImagePtr = testImageIn.get();
  parallelWithRunLoop([&](auto /*threadTotal*/, auto /*threadNum*/, auto& /*bwc*/){
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
