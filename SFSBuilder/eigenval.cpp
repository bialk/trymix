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

  SUBROUTINE SGESV( N, NRHS, A,	LDA, IPIV, B, LDB, INFO	)

      INTEGER	    INFO, LDA, LDB, N, NRHS
      INTEGER	    IPIV( * )
      REAL	    A( LDA, * ), B( LDB, * )


*
*  -- LAPACK driver routine (version 3.0) --
*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
*     Courant Institute, Argonne National Lab, and Rice University
*     June 30, 1999
*
*     .. Scalar Arguments ..
      CHARACTER          JOBZ, UPLO
      INTEGER            INFO, LDA, LWORK, N
*     ..
*     .. Array Arguments ..
      REAL               A( LDA, * ), W( * ), WORK( * )
*     ..
*
*  Purpose
*  =======
*
*  SSYEV computes all eigenvalues and, optionally, eigenvectors of a
*  real symmetric matrix A.
*
*  Arguments
*  =========
*
*  JOBZ    (input) CHARACTER*1
*          = 'N':  Compute eigenvalues only;
*          = 'V':  Compute eigenvalues and eigenvectors.
*
*  UPLO    (input) CHARACTER*1
*          = 'U':  Upper triangle of A is stored;
*          = 'L':  Lower triangle of A is stored.
*
*  N       (input) INTEGER
*          The order of the matrix A.  N >= 0.
*
*  A       (input/output) REAL array, dimension (LDA, N)
*          On entry, the symmetric matrix A.  If UPLO = 'U', the
*          leading N-by-N upper triangular part of A contains the
*          upper triangular part of the matrix A.  If UPLO = 'L',
*          the leading N-by-N lower triangular part of A contains
*          the lower triangular part of the matrix A.
*          On exit, if JOBZ = 'V', then if INFO = 0, A contains the
*          orthonormal eigenvectors of the matrix A.
*          If JOBZ = 'N', then on exit the lower triangle (if UPLO='L')
*          or the upper triangle (if UPLO='U') of A, including the
*          diagonal, is destroyed.
*
*  LDA     (input) INTEGER
*          The leading dimension of the array A.  LDA >= max(1,N).
*
*  W       (output) REAL array, dimension (N)
*          If INFO = 0, the eigenvalues in ascending order.
*
*  WORK    (workspace/output) REAL array, dimension (LWORK)
*          On exit, if INFO = 0, WORK(1) returns the optimal LWORK.
*
*  LWORK   (input) INTEGER
*          The length of the array WORK.  LWORK >= max(1,3*N-1).
*          For optimal efficiency, LWORK >= (NB+2)*N,
*          where NB is the blocksize for SSYTRD returned by ILAENV.
*
*          If LWORK = -1, then a workspace query is assumed; the routine
*          only calculates the optimal size of the WORK array, returns
*          this value as the first entry of the WORK array, and no error
*          message related to LWORK is issued by XERBLA.
*
*  INFO    (output) INTEGER
*          = 0:  successful exit
*          < 0:  if INFO = -i, the i-th argument had an illegal value
*          > 0:  if INFO = i, the algorithm failed to converge; i
*                off-diagonal elements of an intermediate tridiagonal
*                form did not converge to zero.
*
*  =====================================================================
*/

// definitions in C can define two methods all parameters are passed as address 

extern "C" { void ssyev_(char &jobz, char &uplo, int &n, float &a, int &lda, 
			 float &w, float  &work, int &lwork, int &info ); }


int Lapackcpp::EigenValue(int size, float *matrix, float *value, char jobz,char uplo){
  float *a = (float*)alloca((size*size*2+size)*sizeof(float));
  int lwork = 3*size-1;
  float *work = (float*)alloca(lwork*sizeof(float));
  int ok;
  /* to call a Fortran routine from C we */
  /* have to transform the matrix */
  int i,j;
  for (i=0; i<size; i++)
    for(j=0; j<size; j++) 
      a[j+size*i]=matrix[i+size*j];    
  ssyev_(jobz, uplo, size, a[0], size, value[0], work[0], lwork, ok);
  for (i=0; i<size; i++)
    for(j=0; j<size; j++) 
      matrix[i*size+j]=a[i*size+j];    
  return ok;
}
