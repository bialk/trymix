#include "lineqsol2.h"

/*--------------------------------------------------------------------*/


LinSolver2::LinSolver2()
{
   static bool once = true;
   if (once) {
      Eigen::initParallel();
      Eigen::setNbThreads(8);
      once = false;
   }
}

LinSolver2::~LinSolver2()
{
  clear();
}

void LinSolver2::clear(){
   mtxA.clear();
   mtxY.resize(0);
   mtxX.resize(0);
}

void LinSolver2::MtrxA(int nrow,int ncol, int nent)
{
  mtxA.clear();
  type=1; // real enries
  neqns = nrow; // number of equations
}

void LinSolver2::A(int i, int j, float v)
{
   mtxA.push_back({i,j,v});
}


void LinSolver2::MtrxB()
{
   mtxY.resize(neqns);
   mtxY.fill(0);
}

void LinSolver2::B(int irow, float v)
{
   mtxY[irow]+=v;
}


float LinSolver2::X(int irow)
{
   if(mtxX.size() > irow)
      return mtxX[irow];
   else {
      return 0.0f;
   }
}

void LinSolver2::solve()
{
   //Eigen::BiCGSTAB<Eigen::SparseMatrix<double>,Eigen::IncompleteLUT<double> > solver;

   //QR - not working for me (Am I doing something wrong?)
   //Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int> > solver;
   //Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::AMDOrdering<int> > solver;

   //Eigen::SparseLU<Eigen::SparseMatrix<double> > solver;
   Eigen::SimplicialLDLT<Eigen::SparseMatrix<double> > solver;

   Eigen::SparseMatrix<double,Eigen::RowMajor> mtxAaux(neqns,neqns);
   mtxAaux.setFromTriplets(mtxA.begin(),mtxA.end());

   //this lines was test for QR solver but without success
//   Eigen::SparseMatrix<double> mtxAaux2 = mtxAaux;
//   mtxAaux.makeCompressed();

   printf("step 1 passed!\n");
   solver.compute(mtxAaux);
   if(solver.info()!=Eigen::Success) {
      // decomposition failed
      return;
   }
   printf("step 2 passed!\n");
   mtxX = solver.solve(mtxY);
   if(solver.info()!=Eigen::Success) {
      // solving failed
      return;
   }
   printf("step 3 passed!\n");
}

