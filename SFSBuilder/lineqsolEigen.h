#ifndef lineqsol_eigen_h
#define lineqsol_eigen_h

#include <Eigen/Eigen>
#include <vector>

class LinSolverEigen {
public:
  std::vector<Eigen::Triplet<double>> mtxA;
  Eigen::VectorXd mtxY, mtxX;

  int neqns, nrhs, pivotingflag, seed, symmetryflag, type;

  LinSolverEigen();
  ~LinSolverEigen();

  void clear();
  void MtrxA(int rows, int cols, int ent);
  void A(int i, int j, float v);
  void MtrxB();
  void B(int i, float v);
  float X(int i);
  void solve();
};

#endif
