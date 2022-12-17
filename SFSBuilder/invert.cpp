#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

#ifdef _WIN32
#include <io.h>
#define O_NOCTTY O_BINARY
#include <malloc.h>
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <vector>
#include <set>
#include "./lapackcpp.h"

/*
      SUBROUTINE SGETRI( N, A, LDA, IPIV, WORK, LWORK, INFO )
*
*  -- LAPACK routine (version 3.1) --
*     Univ. of Tennessee, Univ. of California Berkeley and NAG Ltd..
*     November 2006
*
*     .. Scalar Arguments ..
      INTEGER            INFO, LDA, LWORK, N
*     ..
*     .. Array Arguments ..
      INTEGER            IPIV( * )
      REAL               A( LDA, * ), WORK( * )
*     ..
*
*  Purpose
*  =======
*
*  SGETRI computes the inverse of a matrix using the LU factorization
*  computed by SGETRF.
*
*  This method inverts U and then computes inv(A) by solving the system
*  inv(A)*L = inv(U) for inv(A).
*
*  Arguments
*  =========
*
*  N       (input) INTEGER
*          The order of the matrix A.  N >= 0.
*
*  A       (input/output) REAL array, dimension (LDA,N)
*          On entry, the factors L and U from the factorization
*          A = P*L*U as computed by SGETRF.
*          On exit, if INFO = 0, the inverse of the original matrix A.
*
*  LDA     (input) INTEGER
*          The leading dimension of the array A.  LDA >= max(1,N).
*
*  IPIV    (input) INTEGER array, dimension (N)
*          The pivot indices from SGETRF; for 1<=i<=N, row i of the
*          matrix was interchanged with row IPIV(i).
*
*  WORK    (workspace/output) REAL array, dimension (MAX(1,LWORK))
*          On exit, if INFO=0, then WORK(1) returns the optimal LWORK.
*
*  LWORK   (input) INTEGER
*          The dimension of the array WORK.  LWORK >= max(1,N).
*          For optimal performance LWORK >= N*NB, where NB is
*          the optimal blocksize returned by ILAENV.
*
*          If LWORK = -1, then a workspace query is assumed; the routine
*          only calculates the optimal size of the WORK array, returns
*          this value as the first entry of the WORK array, and no error
*          message related to LWORK is issued by XERBLA.
*
*  INFO    (output) INTEGER
*          = 0:  successful exit
*          < 0:  if INFO = -i, the i-th argument had an illegal value
*          > 0:  if INFO = i, U(i,i) is exactly zero; the matrix is
*                singular and its inverse could not be computed.
*
*  =====================================================================


      SUBROUTINE SGETRI( N, A, LDA, IPIV, WORK, LWORK, INFO )
*
*  -- LAPACK routine (version 3.1) --
*     Univ. of Tennessee, Univ. of California Berkeley and NAG Ltd..
*     November 2006
*
*     .. Scalar Arguments ..
      INTEGER            INFO, LDA, LWORK, N
*     ..
*     .. Array Arguments ..
      INTEGER            IPIV( * )
      REAL               A( LDA, * ), WORK( * )
*     ..

*/

/*

     SUBROUTINE SGETRF( M, N, A, LDA, IPIV, INFO )
*
*  -- LAPACK routine (version 3.1) --
*     Univ. of Tennessee, Univ. of California Berkeley and NAG Ltd..
*     November 2006
*
*     .. Scalar Arguments ..
      INTEGER            INFO, LDA, M, N
*     ..
*     .. Array Arguments ..
      INTEGER            IPIV( * )
      REAL               A( LDA, * )
*     ..
*
*/

extern "C" { 
void sgetrf_(int &m, int &n, float &a, int &lda, int &ipiv, int &info); 

void sgetri_(int &n, float &a, int &lda, int &ipiv, 
	     float &work, int &lwork, int &info); 

void dgetrf_(int &m, int &n, double &a, int &lda, int &ipiv, int &info); 

void dgetri_(int &n, double &a, int &lda, int &ipiv, 
	     double &work, int &lwork, int &info); 

}



int Lapackcpp::Invert( int m, float *a){
  int info;
  int lda = m;

  int *ipiv = (int*)alloca(sizeof(int)*m);
  sgetrf_(m, m, *a, lda, *ipiv, info); 

  int lwork=-1; float tmp;
  sgetri_(m, *a, lda, *ipiv, tmp, lwork, info );
  lwork=(int)tmp;
  if(info!=0){
    printf("error: info=%i lwork=%i\n",info,lwork);
    return info;
  }
  float *work = (float*)alloca(sizeof(float)*lwork);  
  sgetri_(m, *a, lda, *ipiv, *work, lwork, info );

  return info;
}

int Lapackcpp::Invert( int m, double *a){
  int info;
  int lda = m;

  int *ipiv = (int*)alloca(sizeof(int)*m);
  dgetrf_(m, m, *a, lda, *ipiv, info); 

  int lwork=-1; double tmp;
  dgetri_(m, *a, lda, *ipiv, tmp, lwork, info );
  lwork=(int)tmp;
  if(info!=0){
    printf("error: info=%i lwork=%i\n",info,lwork);
    return info;
  }
  double *work = (double*)alloca(sizeof(double)*lwork);  
  dgetri_(m, *a, lda, *ipiv, *work, lwork, info );

  return info;
}
