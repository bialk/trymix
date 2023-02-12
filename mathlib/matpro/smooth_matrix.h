//---------------------------------------------------------------------------
// developed by Alex Kostin    March 11, 1999
//---------------------------------------------------------------------------
#ifndef smooth_matrixH
#define smooth_matrixH
#include "matrix.h"
//---------------------------------------------------------------------------
class MAT_SMOOTH
{
private:
  TMatrix A, AT, U, U_1, H, QT1, QT2, QT3, XT1, XT2, XT3;
public:
  MAT_SMOOTH(int smoothOrder);
  ~MAT_SMOOTH(){};
  void SetMatrix(TMatrix &U_, TMatrix &A_);
  void StepTo(TMatrix &newX, TMatrix &newQ,
              TMatrix &curX, TMatrix &curQ,
              TMatrix &oldX, TMatrix &oldQ);
  void StepFrom(TMatrix &newX, TMatrix &newQ,
                TMatrix &curX, TMatrix &curQ,
                TMatrix &oldX, TMatrix &oldQ);
};
//---------------------------------------------------------------------------
#endif
