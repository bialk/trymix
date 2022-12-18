//---------------------------------------------------------------------------
// developed by Alex Kostin    March 11, 1999
//---------------------------------------------------------------------------
#ifndef smooth_vectH
#define smooth_vectH
#include "matrix.h"
//---------------------------------------------------------------------------
class TSmoothVect
{
private:
  TMatrix A, AT, U, U_1, H, QT1, QT2, QT3, XT1, XT2, XT3;
  TMatrix Tmp1,Tmp2,TmpV1,TmpV2;
  TMatrix M1,M2,V1,V2,Y;
public:
  TSmoothVect(int smoothOrder);
  ~TSmoothVect(){};
  void SetMatrix(TMatrix &U_);
  void StepTo(TMatrix &newX, TMatrix &newQ,
              TMatrix &curX, TMatrix &curQ,
              TMatrix &oldX, TMatrix &oldQ,float norma);
  void Merge(TMatrix &newX, TMatrix &newQ,
             TMatrix &leftX, TMatrix &leftQ,
             TMatrix &curX, TMatrix &curQ,
             TMatrix &rightX, TMatrix &rightQ);
  void StepFrom(TMatrix &newX, TMatrix &newQ,
                TMatrix &curX, TMatrix &curQ,
                TMatrix &oldX, TMatrix &oldQ,float norma);
};
//---------------------------------------------------------------------------
#endif
