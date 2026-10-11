#ifndef shapefromshade_h
#define shapefromshade_h

#include "mathlib/mathutl/LinSolver.h"

#include <array>
#include <cstdint>
#include <vector>

struct ShapeFromShadeStatistics {
  std::int64_t normalAndAlbedoMilliseconds = 0;
  std::int64_t matrixAssemblyMilliseconds = 0;
  std::int64_t linearSolveMilliseconds = 0;
  std::int64_t resultExtractionMilliseconds = 0;
};

struct DataExchangeBlock {
  DataExchangeBlock(int width, int height);

  int w;
  int h;
  std::vector<float> albedo;
  std::vector<float> data_norm;
  std::vector<float> data_attd;
};

class ShapeFromShade{
public:
  explicit ShapeFromShade(
    LinSolverKind solverKind = LinSolverKind::MT, int openClDeviceIndex = 0);

  float s[4][3]; // four sources of illumination
  float s_alb[4][3]; // four sources of illumination for albedo

  ShapeFromShadeStatistics build(
    DataExchangeBlock& data,
    const std::array<const unsigned char*, 4>& bgraImages);

private:
  LinSolverKind solverKind_;
  int openClDeviceIndex_ = 0;
};

#endif
