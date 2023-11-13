#pragma once

#include <string>
#include <memory>

//#define NO_OPENCL

namespace BlurTests
{

//##################################################################################################
class GaussBlurEngine{
public:
  GaussBlurEngine();
  ~GaussBlurEngine();

  void doBlur(float* scl, float* tcl, size_t w, size_t h, size_t r);

  std::string getErrorString();
  std::string getInfoString();

private:
  class GaussBlurAccelerator;
  std::unique_ptr<GaussBlurAccelerator> oclGaussBlur;
};

}
