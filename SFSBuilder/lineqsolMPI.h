#ifndef lineqsol_mpi_h
#define lineqsol_mpi_h

#include <vector>

extern "C" {
#include <spoolesMPI.h>
}

class LinSolverMPI {
public:
  // MPI must be initialized before construction. Every rank calls the same
  // methods; A() and B() distribute rows internally in round-robin order.
  explicit LinSolverMPI(MPI_Comm communicator = MPI_COMM_WORLD);
  ~LinSolverMPI();

  LinSolverMPI(const LinSolverMPI &) = delete;
  LinSolverMPI &operator=(const LinSolverMPI &) = delete;

  void clear();
  void MtrxA(int rows, int cols, int ent);
  void A(int i, int j, float value);
  void MtrxB();
  void B(int i, float value);
  float X(int i) const;
  void solve();

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
