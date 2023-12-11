#pragma once

#include <vector>
#include <thread>

#include <glm/glm.hpp>

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

//##################################################################################################
std::vector<size_t> boxesForGauss(float sigma, size_t n);

//##################################################################################################
void gaussBlur_4_cpu(float* scl, float* tcl, size_t w, size_t h, size_t r);

