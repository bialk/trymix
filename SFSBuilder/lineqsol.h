#ifndef lineqsol_h
#define lineqsol_h

extern "C" {
#include <InpMtx.h>
}

class LinSolver{
public:

  InpMtx *mtxA ; 
  DenseMtx *mtxY, *mtxX; 
  int neqns, nrhs, pivotingflag, seed, symmetryflag, type; 

  LinSolver();
  ~LinSolver();

  void clear();

  void MtrxA(int rows,int cols, int ent);
  void A(int i,int j, float v);
  void MtrxB();
  void B(int i, float v);
  float X(int i);
  void solve();
};


#endif
