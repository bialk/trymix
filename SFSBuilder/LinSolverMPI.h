#ifndef lin_solver_mpi_h
#define lin_solver_mpi_h

#include "LinSolver.h"

#include <vector>

extern "C" {
#include <spoolesMPI.h>
}

class LinSolverMPI final : public LinSolver {
public:
  // MPI must be initialized before construction. Every rank calls the same
  // methods; A() and B() distribute rows internally in round-robin order.
  explicit LinSolverMPI(MPI_Comm communicator = MPI_COMM_WORLD);
  ~LinSolverMPI() override;

  LinSolverMPI(const LinSolverMPI &) = delete;
  LinSolverMPI &operator=(const LinSolverMPI &) = delete;

  void clear() override;
  void MtrxA(int rows, int cols, int ent) override;
  void A(int i, int j, float value) override;
  void MtrxB() override;
  void B(int i, float value) override;
  float X(int i) const override;
  void solve() override;

private:
  void requireMPI() const;

  MPI_Comm communicator_;
  InpMtx *mtxA_ = nullptr;
  DenseMtx *mtxY_ = nullptr;
  std::vector<double> solution_;
  int neqns_ = 0;
  int nrhs_ = 1;
  int type_ = SPOOLES_REAL;
  int symmetryflag_ = SPOOLES_NONSYMMETRIC;
  int pivotingflag_ = SPOOLES_PIVOTING;
  int seed_ = 666666;
};

#endif
