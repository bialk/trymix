#include "LinSolverEigenCG.h"

#include <Eigen/IterativeLinearSolvers>
#include <stdexcept>
#include <string>

void LinSolverEigenCG::clear()
{
  coefficients_.clear();
  rightHandSide_.resize(0);
  solution_.resize(0);
  equationCount_ = 0;
}

void LinSolverEigenCG::MtrxA(int rows, int, int estimatedEntries)
{
  equationCount_ = rows;
  coefficients_.clear();
  coefficients_.reserve(static_cast<std::size_t>(estimatedEntries));
}

void LinSolverEigenCG::A(int row, int column, float value)
{
  coefficients_.emplace_back(row, column, value);
}

void LinSolverEigenCG::MtrxB()
{
  rightHandSide_ = Eigen::VectorXd::Zero(equationCount_);
}

void LinSolverEigenCG::B(int row, float value)
{
  rightHandSide_[row] += value;
}

float LinSolverEigenCG::X(int row) const
{
  return solution_.size() > row ? static_cast<float>(solution_[row]) : 0.0F;
}

void LinSolverEigenCG::solve()
{
  Eigen::SparseMatrix<double> matrix(equationCount_, equationCount_);
  matrix.setFromTriplets(coefficients_.begin(), coefficients_.end());

  // ShapeFromShade assembles a negative-definite discrete Laplacian.
  // Negating both sides produces the positive-definite system expected by CG.
  matrix *= -1.0;
  const Eigen::VectorXd rightHandSide = -rightHandSide_;

  Eigen::ConjugateGradient<
    Eigen::SparseMatrix<double>, Eigen::Lower | Eigen::Upper,
  Eigen::IncompleteCholesky<double>> solver;
  solver.setTolerance(1.0e-6);
  solver.setMaxIterations(2000);
  solver.compute(matrix);
  if(solver.info() != Eigen::Success)
    throw std::runtime_error("Eigen CG preconditioner construction failed");

  solution_ = solver.solve(rightHandSide);
  if(solver.info() != Eigen::Success) {
    throw std::runtime_error(
      "Eigen CG failed to converge after "
      + std::to_string(solver.iterations()) + " iterations; estimated error "
      + std::to_string(solver.error()));
  }
}
