//---------------------------------------------------------------------------
// developed by Alex Kostin    March 11, 1999
//---------------------------------------------------------------------------
#include <stdio.h>
#include "smooth_vect.h"
//---------------------------------------------------------------------------
TSmoothVect::TSmoothVect(int smoothOrder)
{
  int dimMatr = smoothOrder + 1;
  U.SetSize(dimMatr, dimMatr);
  U_1.SetSize(dimMatr, dimMatr);
  H.SetSize(dimMatr, dimMatr);
  QT1.SetSize(dimMatr, dimMatr);
  QT2.SetSize(dimMatr, dimMatr);
  QT3.SetSize(dimMatr, dimMatr);
  XT1.SetSize(dimMatr, 1);
  XT2.SetSize(dimMatr, 1);
  XT3.SetSize(dimMatr, 1);
  
  Tmp1.SetSize(dimMatr, dimMatr);
  Tmp2.SetSize(dimMatr, dimMatr);
  TmpV1.SetSize(dimMatr, 1);
  TmpV2.SetSize(dimMatr, 1);

  M1.SetSize(dimMatr, dimMatr);
  M2.SetSize(dimMatr, dimMatr);
  V1.SetSize(dimMatr, 1);
  V2.SetSize(dimMatr, 1);

  Y.SetSize(dimMatr, 1);
}
//---------------------------------------------------------------------------
void TSmoothVect::SetMatrix(TMatrix &U_)
{
  U = U_;
  U_1 = U_;
  U_1.Inverse();
}
//---------------------------------------------------------------------------
void TSmoothVect::StepTo(TMatrix &newX, TMatrix &newQ,
                        TMatrix &curX, TMatrix &curQ,
                        TMatrix &oldX, TMatrix &oldQ, float norma)
{
  long double d;
  
  // ~     0   ~    ~        -1
  // Q  = Q  + Q   (Q   + U )   U
  //  i    i    i-1  i-1   i     i
  //                -----1-----
  //           --------2----------
  // -------------3---------------
  
  // 1
  Tmp1.madd(oldQ,U);  /* M1=U; M1*=norma; Tmp1.madd(oldQ,M1);*/
  d=Tmp1.Inverse();
  while(d==0)
  {
    printf("Error of inversing 1 \n");  
    getchar();
  }
  // 2
  Tmp2.mmul(oldQ,Tmp1);
  Tmp1.mmul(Tmp2,U);              /* Tmp1*=norma;*/
  // 3
  newQ.madd(curQ,Tmp1);

  // ~ -1 / 0  0   ~    ~        -1     ~   \
  // Q    |Q  X  + Q   (Q   + U )   U   X   |
  //  i   \ i  i    i-1  i-1   i     i   i-1/
  // --1-- --2--   -------------3-----------
  //       ---------------4-----------------
  // -------------------5-------------------
  
  // 1
  Tmp2=newQ;
  if (Tmp2.Inverse() != 0 || 1) {
    // 2
    TmpV1.mmul(curQ,curX);
    // 3
    TmpV2.mmul(Tmp1,oldX);
    // 4
    TmpV1.madd(TmpV2);
    newX.mmul(Tmp2,TmpV1);
  }
  else {
    // see documentation
    V1.mmul(Tmp1,oldX);
    
    V2.mmul(curQ,curX);
    V1.madd(V2);
    
    
    //M1=newQ; M1.Inverse();
    //V1.mmul(M1,V2);
    
    M1=newQ; M1.Transpose();
    V2.mmul(M1,V1);
    
    //Y=curX; V1=Y; V1*=1e-10; V2.madd(V1);
    M2.mmul(M1,newQ);
    //M2.addDiag(1e-10);
    if( M2.Inverse()==0){
      printf("Error of inversing 2\n");  
      getchar();
    }
    
    newX.mmul(M2,V2);
  }
};


//---------------------------------------------------------------------------
void TSmoothVect::Merge(TMatrix &newX, TMatrix   &newQ,
                        TMatrix &leftX, TMatrix  &leftQ,
                        TMatrix &curX, TMatrix   &curQ,
                        TMatrix &rightX, TMatrix &rightQ)
{
  long double d;

  // ^<->    ~<-     0    ~->
  // Q    = Q    + Q  +  Q
  //  i      i-1    i     i+1
  
  newQ=curQ; newQ.madd(leftQ); newQ.madd(rightQ);
  
  //  ^<->   /^<-> \-1 / ~<- ~<-     0  0    ~->  ~->  \
  //  X    = |Q    |   | Q   X    + Q  X  +  Q    X    |
  //   i     \ i   /   \  i-1 i-1    i  i     i+1  i+1 /
  //         ----1----
  //                   ---------------2-----------------
  //  ----------------------3---------------------------

  //1
  Tmp1=newQ; d = Tmp1.Inverse();
  while(d==0)
  {
    printf("Error of inversing 3 \n");  
    getchar();
  }
  //2
  TmpV1.mmul(leftQ,leftX);
  TmpV2.mmul(curQ,curX);
  TmpV1.madd(TmpV2);
  TmpV2.mmul(rightQ,rightX);
  TmpV1.madd(TmpV2);
  //3
  newX.mmul(Tmp1,TmpV1);
}


void TSmoothVect::StepFrom(TMatrix &newX, TMatrix &newQ,
                           TMatrix &curX, TMatrix &curQ,
                           TMatrix &oldX, TMatrix &oldQ, float norma)
{
  long double d;
  H.mmul(U_1, curQ); /*H*=1/norma;*/
  

  H.addDiag(1.);
  d = H.Inverse();
  while(d==0)
  {
    printf("Error of inversing 4 \n");  
    getchar();
  }

  XT1.msub(oldX,curX);
  XT2.mmul(H, XT1);
  newX.madd(curX, XT2);
  

  QT1=oldQ;
  d = QT1.Inverse();
  while(d==0)
  {
    printf("Error of inversing 5 \n");  
    getchar();
  }
  QT2.mtra(H);
  QT3.mmul(QT1, QT2);
  QT3.madd(U_1); /* M1=U_1; M1*=1/norma; QT3.madd(M1);*/
  newQ.mmul(H, QT3);
  
  d = newQ.Inverse();
  while(d==0)
  {
    printf("Error of inversing 6 \n");  
    getchar();
  }
}
