//---------------------------------------------------------------------------
#ifndef smooth_pro_scalH
#define smooth_pro_scalH
//---------------------------------------------------------------------------
template<class Type>
class SMOOTH_PROC_SCAL
{
private:
  Type w, w1;
public:
  void SetSmoothness(Type w_){w = w_; w1 = 1. / w_;}
  void StepTo(Type &newB, Type &newX, Type &newQ,
              Type curB, Type curX, Type curQ,
              Type oldB, Type oldX, Type oldQ);
  void StepTo(Type &newX, Type &newQ,
              Type curX, Type curQ,
              Type oldX, Type oldQ);
  void StepFrom(Type &newX, Type &newQ,
                Type curX, Type curQ,
                Type oldX, Type oldQ);
};
//---------------------------------------------------------------------------
#include "smooth_pro_scal.cpp"
//---------------------------------------------------------------------------
#endif
