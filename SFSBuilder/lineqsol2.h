#ifndef lineqsol_h
#define lineqsol_h

#include <Eigen/Eigen>
#include <map>

class LinSolver2{
public:

//  InpMtx *mtxA ;
//  DenseMtx *mtxY, *mtxX;

  //Eigen::SparseMatrix<double> *mtxA;
  std::vector<Eigen::Triplet<double>> mtxA;
  // fill A
  Eigen::VectorXd mtxY, mtxX;

  int neqns, nrhs, pivotingflag, seed, symmetryflag, type; 

  LinSolver2();
  ~LinSolver2();

  void clear();

  void MtrxA(int rows,int cols, int ent);
  void A(int i,int j, float v);
  void MtrxB();
  void B(int i, float v);
  float X(int i);
  void solve();
};


#endif
