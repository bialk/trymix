#ifndef lin_solver_eigen_h
#define lin_solver_eigen_h

#include "LinSolver.h"

#include <Eigen/Eigen>
#include <vector>

class LinSolverEigen final : public LinSolver {
public:
  std::vector<Eigen::Triplet<double>> mtxA;
  Eigen::VectorXd mtxY, mtxX;

  int neqns, nrhs, pivotingflag, seed, symmetryflag, type;

  LinSolverEigen();
  ~LinSolverEigen() override;

  void clear() override;
  void MtrxA(int rows, int cols, int ent) override;
  void A(int i, int j, float v) override;
  void MtrxB() override;
  void B(int i, float v) override;
  float X(int i) const override;
  void solve() override;
};

#endif
