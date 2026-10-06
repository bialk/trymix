#ifndef lineqsol_mt_h
#define lineqsol_mt_h

extern "C" {
#include <InpMtx.h>
}

class LinSolverMT {
public:

  InpMtx *mtxA ; 
  DenseMtx *mtxY, *mtxX; 
  int neqns, nrhs, pivotingflag, seed, symmetryflag, type; 

  LinSolverMT();
  ~LinSolverMT();

  void clear();

  void MtrxA(int rows,int cols, int ent);
  void A(int i,int j, float v);
  void MtrxB();
  void B(int i, float v);
  float X(int i);
  void solve();
};


#endif
