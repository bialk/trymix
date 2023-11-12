#include <CL/cl.hpp>

#include <thread>

//##################################################################################################
template<typename T>
void parallel(T worker)
{
  size_t threads = std::thread::hardware_concurrency();
  std::vector<std::thread> workers;
  workers.reserve(threads);
  for(size_t i=0; i<threads; i++)
    workers.emplace_back([&]{worker(i);});
  for(auto& thread : workers)
    thread.join();
}

namespace opencl_test
{

////##################################################################################################
void gaussBlur_4_cpu(float* scl, float* tcl, size_t w, size_t h, size_t r);

class GaussBlur_opencl{
public:
  GaussBlur_opencl();

  void gaussBlur_4_opencl(float* scl, float* tcl, size_t w, size_t h, size_t r);

  void gaussBlur_4_auto(float* scl, float* tcl, size_t w, size_t h, size_t r);

private:
  std::vector<cl::Platform> all_platforms;
  std::vector<cl::Device> all_devices;
  cl::Device default_device;
  cl::Context context;
  cl::Program program;
  cl::Kernel boxBlurH_4;
  cl::Kernel boxBlurT_4;
  cl::CommandQueue queue;
  std::string errorString;
};

//##################################################################################################
//void gaussBlur_4_opencl(float* scl, float* tcl, size_t w, size_t h, size_t r);

}
