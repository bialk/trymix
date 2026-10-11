#include "LinSolverAmgclCG.h"
#include "apputil/OpenClDevices.h"

#include <amgcl/adapter/eigen.hpp>
#include <amgcl/amg.hpp>
#include <amgcl/backend/builtin.hpp>
#include <amgcl/backend/vexcl.hpp>
#include <amgcl/coarsening/smoothed_aggregation.hpp>
#include <amgcl/make_solver.hpp>
#include <amgcl/relaxation/spai0.hpp>
#include <amgcl/solver/cg.hpp>

#include <Eigen/Core>
#include <QDebug>

#include <cmath>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

namespace {
struct SolveResult {
  std::vector<double> solution;
  std::size_t iterations = 0;
  double estimatedError = 0.0;
  QString device;
};

template<class Backend>
using AmgclSolver = amgcl::make_solver<
  amgcl::amg<Backend, amgcl::coarsening::smoothed_aggregation,
    amgcl::relaxation::spai0>,
  amgcl::solver::cg<Backend>>;

SolveResult solveOnCpu(
  const Eigen::SparseMatrix<double, Eigen::RowMajor>& matrix,
  const std::vector<double>& rightHandSide)
{
  using Solver = AmgclSolver<amgcl::backend::builtin<double>>;
  typename Solver::params parameters;
  parameters.solver.tol = 1.0e-6;
  parameters.solver.maxiter = 2000;

  Solver solver(matrix, parameters);
  SolveResult result;
  result.solution.assign(rightHandSide.size(), 0.0);
  std::tie(result.iterations, result.estimatedError) =
    solver(rightHandSide, result.solution);
  result.device = QStringLiteral("CPU/OpenMP");
  return result;
}

template<class Scalar>
SolveResult solveOnOpenClImpl(
  const Eigen::SparseMatrix<double, Eigen::RowMajor>& matrix,
  const std::vector<double>& rightHandSide,
  int deviceIndex)
{
  const auto devices = apputil::enumerateOpenClDevices({true, false});
  if(deviceIndex < 0 || static_cast<std::size_t>(deviceIndex) >= devices.size())
    throw std::runtime_error("The selected OpenCL GPU is unavailable");
  const auto& selectedDevice = devices[static_cast<std::size_t>(deviceIndex)];
  if constexpr(std::is_same_v<Scalar, double>) {
    if(!selectedDevice.supportsDoublePrecision)
      throw std::runtime_error(
        "The selected OpenCL GPU does not support double precision");
  }

  const auto selectedDeviceFilter = [device = selectedDevice.device](
      const vex::backend::device& candidate) {
    return candidate() == device;
  };
  std::unique_ptr<vex::Context> context;
  context = std::make_unique<vex::Context>(selectedDeviceFilter);
  if(!*context)
    throw std::runtime_error(
      "The selected OpenCL GPU is unavailable for the requested precision");

  using Backend = amgcl::backend::vexcl<Scalar>;
  using Solver = AmgclSolver<Backend>;
  typename Solver::params parameters;
  parameters.solver.tol = 1.0e-6;
  parameters.solver.maxiter = 2000;
  typename Backend::params backendParameters;
  backendParameters.q = *context;

  const Eigen::SparseMatrix<Scalar, Eigen::RowMajor> scalarMatrix =
    matrix.template cast<Scalar>();
  const std::vector<Scalar> scalarRightHandSide(
    rightHandSide.begin(), rightHandSide.end());
  Solver solver(scalarMatrix, parameters, backendParameters);
  vex::vector<Scalar> deviceRightHandSide(*context, scalarRightHandSide);
  vex::vector<Scalar> deviceSolution(*context, rightHandSide.size());
  deviceSolution = 0.0;

  SolveResult result;
  std::tie(result.iterations, result.estimatedError) =
    solver(deviceRightHandSide, deviceSolution);
  std::vector<Scalar> scalarSolution(rightHandSide.size());
  vex::copy(deviceSolution, scalarSolution);
  result.solution.assign(scalarSolution.begin(), scalarSolution.end());

  std::ostringstream description;
  description << *context;
  result.device = QString::fromStdString(description.str()).trimmed();
  return result;
}

SolveResult solveOnOpenCl(
  const Eigen::SparseMatrix<double, Eigen::RowMajor>& matrix,
  const std::vector<double>& rightHandSide, int deviceIndex,
  AmgclPrecision precision)
{
  return precision == AmgclPrecision::Float
    ? solveOnOpenClImpl<float>(matrix, rightHandSide, deviceIndex)
    : solveOnOpenClImpl<double>(matrix, rightHandSide, deviceIndex);
}
}

void LinSolverAmgclCG::clear()
{
  coefficients_.clear();
  rightHandSide_.clear();
  solution_.clear();
  equationCount_ = 0;
}

void LinSolverAmgclCG::MtrxA(int rows, int, int estimatedEntries)
{
  equationCount_ = rows;
  coefficients_.clear();
  coefficients_.reserve(static_cast<std::size_t>(estimatedEntries));
}

void LinSolverAmgclCG::A(int row, int column, float value)
{
  coefficients_.emplace_back(row, column, value);
}

void LinSolverAmgclCG::MtrxB()
{
  rightHandSide_.assign(static_cast<std::size_t>(equationCount_), 0.0);
}

void LinSolverAmgclCG::B(int row, float value)
{
  rightHandSide_[static_cast<std::size_t>(row)] += value;
}

float LinSolverAmgclCG::X(int row) const
{
  return solution_.size() > static_cast<std::size_t>(row)
    ? static_cast<float>(solution_[static_cast<std::size_t>(row)])
    : 0.0F;
}

void LinSolverAmgclCG::solve()
{
  Eigen::SparseMatrix<double, Eigen::RowMajor> matrix(
    equationCount_, equationCount_);
  matrix.setFromTriplets(coefficients_.begin(), coefficients_.end());

  // ShapeFromShade assembles a negative-definite discrete Laplacian.
  matrix *= -1.0;
  for(double& value : rightHandSide_)
    value = -value;

  SolveResult result = backend_ == AmgclBackend::OpenCL
    ? solveOnOpenCl(
        matrix, rightHandSide_, openClDeviceIndex_, precision_)
    : solveOnCpu(matrix, rightHandSide_);
  solution_ = std::move(result.solution);

  const Eigen::Map<const Eigen::VectorXd> rightHandSide(
    rightHandSide_.data(), equationCount_);
  const Eigen::Map<const Eigen::VectorXd> solution(
    solution_.data(), equationCount_);
  const double rightHandSideNorm = rightHandSide.norm();
  const double residualNorm = (matrix * solution - rightHandSide).norm();
  const double relativeResidual = rightHandSideNorm > std::numeric_limits<double>::min()
    ? residualNorm / rightHandSideNorm
    : residualNorm;

  if(!std::isfinite(relativeResidual))
    throw std::runtime_error("AMGCL CG produced a non-finite residual");

  if(relativeResidual > 1.0e-6) {
    qWarning().noquote() << "AMGCL CG"
               << (precision_ == AmgclPrecision::Float ? "FP32" : "FP64")
               << "on" << result.device
               << "did not converge: iterations" << result.iterations
               << "estimated error" << result.estimatedError
               << "true relative residual" << relativeResidual
               << "-- using the approximate solution";
    return;
  }

  qInfo().noquote() << "AMGCL CG"
          << (precision_ == AmgclPrecision::Float ? "FP32" : "FP64")
          << "on" << result.device
          << ": iterations" << result.iterations
          << "estimated error" << result.estimatedError
          << "true relative residual" << relativeResidual;
}
