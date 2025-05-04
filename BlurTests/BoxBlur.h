#pragma once

#include <vector>
#include <thread>

#include <glm/glm.hpp>

//##################################################################################################
std::vector<size_t> boxesForGauss(float sigma, size_t n);

//##################################################################################################
void gaussBlur_4_cpu(float* scl, float* tcl, size_t w, size_t h, size_t r);

