#ifndef lin_solver_mt_h
#define lin_solver_mt_h

#include "LinSolver.h"

extern "C" {
#include <InpMtx.h>
}

class LinSolverMT final : public LinSolver {
public:

  InpMtx *mtxA ; 
  DenseMtx *mtxY, *mtxX; 
  int neqns, nrhs, pivotingflag, seed, symmetryflag, type; 

  LinSolverMT();
  ~LinSolverMT() override;

  void clear() override;

  void MtrxA(int rows,int cols, int ent) override;
  void A(int i,int j, float v) override;
  void MtrxB() override;
  void B(int i, float v) override;
  float X(int i) const override;
  void solve() override;
};


#endif
