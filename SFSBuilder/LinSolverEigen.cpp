#include "LinSolverEigen.h"

LinSolverEigen::LinSolverEigen()
{
  static bool once = true;
  if (once) {
    Eigen::initParallel();
    Eigen::setNbThreads(8);
    once = false;
  }
}

LinSolverEigen::~LinSolverEigen() { clear(); }

void LinSolverEigen::clear()
{
  mtxA.clear();
  mtxY.resize(0);
  mtxX.resize(0);
}

void LinSolverEigen::MtrxA(int nrow, int, int)
{
  mtxA.clear();
  type = 1;
  neqns = nrow;
}

void LinSolverEigen::A(int i, int j, float v)
{
  mtxA.push_back({i, j, v});
}

void LinSolverEigen::MtrxB()
{
  mtxY.resize(neqns);
  mtxY.fill(0);
}

void LinSolverEigen::B(int irow, float v) { mtxY[irow] += v; }

float LinSolverEigen::X(int irow) const
{
  return mtxX.size() > irow ? static_cast<float>(mtxX[irow]) : 0.0f;
}

void LinSolverEigen::solve()
{
  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
  Eigen::SparseMatrix<double, Eigen::RowMajor> matrix(neqns, neqns);
  matrix.setFromTriplets(mtxA.begin(), mtxA.end());

  solver.compute(matrix);
  if (solver.info() != Eigen::Success) {
    return;
  }

  mtxX = solver.solve(mtxY);
}
