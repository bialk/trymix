#ifndef lin_solver_amgcl_cg_h
#define lin_solver_amgcl_cg_h

#include "LinSolver.h"

#include <Eigen/SparseCore>
#include <vector>

enum class AmgclBackend {
  CpuOpenMP,
  OpenCL
};

enum class AmgclPrecision {
  Double,
  Float
};

class LinSolverAmgclCG final : public LinSolver {
public:
  explicit LinSolverAmgclCG(
    AmgclBackend backend, int openClDeviceIndex = 0,
    AmgclPrecision precision = AmgclPrecision::Double)
    : backend_(backend), openClDeviceIndex_(openClDeviceIndex),
      precision_(precision) {}

  void clear() override;
  void MtrxA(int rows, int cols, int estimatedEntries) override;
  void A(int row, int column, float value) override;
  void MtrxB() override;
  void B(int row, float value) override;
  float X(int row) const override;
  void solve() override;

private:
  AmgclBackend backend_;
  int openClDeviceIndex_ = 0;
  AmgclPrecision precision_ = AmgclPrecision::Double;
  int equationCount_ = 0;
  std::vector<Eigen::Triplet<double>> coefficients_;
  std::vector<double> rightHandSide_;
  std::vector<double> solution_;
};

#endif
