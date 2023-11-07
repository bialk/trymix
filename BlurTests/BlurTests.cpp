#include "BlurTests.h"

#include <QDebug>
#include <QImage>

#include <CL/cl.hpp>
#include <vector>
#include <thread>


//##################################################################################################
template<typename T>
void parallel(T worker)
{
//  TPMutex mutex{TPM};
//  auto locker = [&](auto closure){mutex.locked(TPMc closure);};

  size_t threads = std::thread::hardware_concurrency();
  std::vector<std::thread> workers;
  workers.reserve(threads);
  for(size_t i=0; i<threads; i++)
    workers.emplace_back([&]{worker(i);});
  for(auto& thread : workers)
    thread.join();
}


void fillChessBoard(float *img, int w, int h, int stride, int boxSize){
  std::atomic<size_t> row{0};
  w*=4;
  auto boxSize4 = boxSize*4;
  parallel([&](int i){
    for( auto r = row++; r<h; r = row++){
      auto rp = img + r*stride*4;
      for(int c=0; c < w; c+=4){
        auto fill = (c/boxSize4)%2 == (r/boxSize)%2;
        rp[c+0] = rp[c+1] = rp[c+2] = (fill? 1:0);
        rp[c+3] = 1;
      }
    }
  });
}

void blurTest(int radius, QImage& qimg)
{
  int const w=1024, h=1024;
  QImage input(w,h,QImage::Format_RGBA32FPx4);
  fillChessBoard((float*)input.bits(), w, h, w, 64);
  qimg = input;


  return;


  std::vector<cl::Platform> all_platforms;
      cl::Platform::get(&all_platforms);
      if(all_platforms.size()==0){
          qInfo() <<" No platforms found. Check OpenCL installation!\n";
      }
      cl::Platform default_platform=all_platforms[0];
      qInfo() << "Using platform: "<<default_platform.getInfo<CL_PLATFORM_NAME>()<<"\n";

      //get default device of the default platform
      std::vector<cl::Device> all_devices;
      default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
      if(all_devices.size()==0){
        qInfo() <<" No devices found. Check OpenCL installation!\n";
        exit(1);
      }
      cl::Device default_device=all_devices[0];
      qInfo() << "Using device: "<<default_device.getInfo<CL_DEVICE_NAME>()<<"\n";

      cl::Context context({default_device});

      //Next we need to create the program which we want to execute on our device:
      cl::Program::Sources sources;

      // kernel calculates for each element C=A+B
      std::string kernel_code =
R"V0G0N(
          void kernel simple_add(global const int* A, global const int* B, global int* C){
              C[get_global_id(0)]=A[get_global_id(0)]+B[get_global_id(0)];
          }
)V0G0N";

      //Next we need our kernel sources to build. We also check for the errors at building:

      sources.push_back({kernel_code.c_str(),kernel_code.length()});

      cl::Program program(context,sources);
      if(program.build({default_device})!=CL_SUCCESS){
        qInfo() <<" Error building: "<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device)<<"\n";
      }

      //For arrays A, B, C we need to allocate the space on the device:

      // create buffers on the device
      cl::Buffer buffer_A(context,CL_MEM_READ_WRITE,sizeof(int)*10);
      cl::Buffer buffer_B(context,CL_MEM_READ_WRITE,sizeof(int)*10);
      cl::Buffer buffer_C(context,CL_MEM_READ_WRITE,sizeof(int)*10);

      //Arrays will have 10 element. We want to calculate sum of next arrays (A, B).

      int A[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
      int B[] = {0, 1, 2, 0, 1, 2, 0, 1, 2, 0};

      //create queue to which we will push commands for the device.
      cl::CommandQueue queue(context,default_device);

      //write arrays A and B to the device
      queue.enqueueWriteBuffer(buffer_A,CL_TRUE,0,sizeof(int)*10,A);
      queue.enqueueWriteBuffer(buffer_B,CL_TRUE,0,sizeof(int)*10,B);


      //run the kernel
//      cl::KernelFunctor simple_add(cl::Kernel(program,"simple_add"),queue,cl::NullRange,cl::NDRange(10),cl::NullRange);
//      simple_add(buffer_A,buffer_B,buffer_C);

      cl::Kernel simple_add(program, "simple_add");
      simple_add.setArg(0, buffer_A);
      simple_add.setArg(1, buffer_B);
      simple_add.setArg(2, buffer_C);
      queue.enqueueNDRangeKernel(simple_add,cl::NullRange,cl::NDRange(10),cl::NullRange);
      queue.finish();

      //alternative way to run the kernel
      /*cl::Kernel kernel_add=cl::Kernel(program,"simple_add");
          kernel_add.setArg(0,buffer_A);
          kernel_add.setArg(1,buffer_B);
          kernel_add.setArg(2,buffer_C);
          queue.enqueueNDRangeKernel(kernel_add,cl::NullRange,cl::NDRange(10),cl::NullRange);
          queue.finish();*/

      int C[10];
      //read result C from the device to array C
      queue.enqueueReadBuffer(buffer_C,CL_TRUE,0,sizeof(int)*10,C);

      qInfo() <<" result: \n";
      for(int i=0;i<10;i++){
        qInfo() <<C[i]<<" ";
      }



}
