//  mat_pro.h
//---------------------------------------------------------------------------
template<class Type>
void SMOOTH_PROC_SCAL<Type>::StepTo(Type &newB, Type &newX, Type &newQ,
                        Type curB, Type curX, Type curQ,
                        Type oldB, Type oldX, Type oldQ)
{
  Type temp = 1. / (1. / oldQ + w1);
  newQ = curQ + temp;
  newX = (curQ * curX + temp * oldX) / newQ;
  newB = curB + curQ * (curX - newX) * curX +
         oldB + temp * (oldX - newX) * oldX;
}
//---------------------------------------------------------------------------
template<class Type>
void SMOOTH_PROC_SCAL<Type>::StepTo(Type &newX, Type &newQ,
                        Type curX, Type curQ,
                        Type oldX, Type oldQ)
{
  Type temp = 1. / (1. / oldQ + w1);
  newQ = curQ + temp;
  newX = (curQ * curX + temp * oldX) / newQ;
}
//---------------------------------------------------------------------------
template<class Type>
void SMOOTH_PROC_SCAL<Type>::StepFrom(Type &newX, Type &newQ,
                          Type curX, Type curQ,
                          Type oldX, Type oldQ)
{
  Type h = 1. / (curQ * w1 + 1.);
  newX = (1. - h) * curX + h * oldX;
  newQ = 1. / (h * h / oldQ + 1. / (curQ + w));
}
//---------------------------------------------------------------------------

