#ifndef lin_solver_eigen_cg_h
#define lin_solver_eigen_cg_h

#include "LinSolver.h"

#include <Eigen/Core>
#include <Eigen/SparseCore>
#include <vector>

class LinSolverEigenCG final : public LinSolver {
public:
  void clear() override;
  void MtrxA(int rows, int cols, int estimatedEntries) override;
  void A(int row, int column, float value) override;
  void MtrxB() override;
  void B(int row, float value) override;
  float X(int row) const override;
  void solve() override;

private:
  int equationCount_ = 0;
  std::vector<Eigen::Triplet<double>> coefficients_;
  Eigen::VectorXd rightHandSide_;
  Eigen::VectorXd solution_;
};

#endif
