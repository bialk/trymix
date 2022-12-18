//---------------------------------------------------------------------------
// developed by Alex Kostin    March 11, 1999
//---------------------------------------------------------------------------
#include <stdio.h>
#include "smooth_matrix.h"
//---------------------------------------------------------------------------
MAT_SMOOTH::MAT_SMOOTH(int smoothOrder)
{
  int dimMatr = smoothOrder + 1;
  A.SetSize(dimMatr, dimMatr);
  AT.SetSize(dimMatr, dimMatr);
  U.SetSize(dimMatr, dimMatr);
  U_1.SetSize(dimMatr, dimMatr);
  H.SetSize(dimMatr, dimMatr);
  QT1.SetSize(dimMatr, dimMatr);
  QT2.SetSize(dimMatr, dimMatr);
  QT3.SetSize(dimMatr, dimMatr);
  XT1.SetSize(dimMatr, 1);
  XT2.SetSize(dimMatr, 1);
  XT3.SetSize(dimMatr, 1);
}
//---------------------------------------------------------------------------
void MAT_SMOOTH::SetMatrix(TMatrix &U_, TMatrix &A_)
{
  A = A_;
  AT.mtra(A);
  U = U_;
  U_1 = U_;
  U_1.Inverse();
}
//---------------------------------------------------------------------------
void MAT_SMOOTH::StepTo(TMatrix &newX, TMatrix &newQ,
                        TMatrix &curX, TMatrix &curQ,
                        TMatrix &oldX, TMatrix &oldQ)
{
  long double d;
  QT1.madd(oldQ, U_1);
  d = QT1.Inverse();
  while(d==0)
  {
    printf("1 \n");  
    QT1.addDiag(1e-5);
    QT1.Inverse();
  }

  QT2.mmul(AT, QT1);
  QT3.mmul(QT2, A);
  QT1 = curQ;
  d = QT1.Inverse();
  while(d==0)
  {
    printf("2 \n");  
    QT1.addDiag(1e-5);
    d = QT1.Inverse();
  }

  newQ.madd(QT1, QT3);

  XT1.mmul(QT1, curX);
  XT2.mmul(QT2, oldX);
  XT3.madd(XT1, XT2);
  d = newQ.Inverse();
  while(d==0)
  {
    printf("3 \n");  
    newQ.addDiag(1e-5);
    newQ.Inverse();
  }

  newX.mmul(newQ, XT3);
};
//---------------------------------------------------------------------------
void MAT_SMOOTH::StepFrom(TMatrix &newX, TMatrix &newQ,
                          TMatrix &curX, TMatrix &curQ,
                          TMatrix &oldX, TMatrix &oldQ)
{
  long double d;
  QT1 = curQ;
  d = QT1.Inverse();
  while(d==0)
  {
    printf("4 \n");  
    QT1.addDiag(1e-5);
    QT1.Inverse();
  }
  H.mmul(U_1, QT1);
  H.addDiag(1.);
  d = H.Inverse();
  while(d==0)
  {
    printf("5 \n");  
    H.addDiag(1e-5);
    H.Inverse();
  }

  XT1.mmul(A, oldX);
  XT2.msub(XT1, curX);
  XT3.mmul(H, XT2);
  newX.madd(curX, XT3);

  QT1.mmul(A, oldQ);
  QT2.mmul(QT1, AT);
  QT1.mtra(H);
  QT3.mmul(QT2, QT1);
  QT1.madd(QT3, U_1);
  newQ.mmul(H, QT1);
}
//---------------------------------------------------------------------------

