#include "lineqsol.h"

extern "C" {
#include <spoolesMT.h>
#include <misc.h>
#include <SymbFac.h>
}

#define MT_SUPPORT

/*--------------------------------------------------------------------*/ 


LinSolver::LinSolver():mtxA(0),mtxX(0),mtxY(0){}

LinSolver::~LinSolver(){
  clear();
}

void LinSolver::clear(){
  if(mtxA)  { InpMtx_free(mtxA); mtxA=0;}
  if(mtxY)  { DenseMtx_free(mtxY); mtxY=0; }
  if(mtxX)  { DenseMtx_free(mtxX); mtxX=0; }  
}

void LinSolver::MtrxA(int nrow,int ncol, int nent){
  //nrow number of rows 
  //ncol number of cols 
  //nent numver of entries

  if(mtxA) InpMtx_free(mtxA);

  type=1; // real enries  

  neqns = nrow; // number of equations
  mtxA = InpMtx_new();
  // two last paramters is number of entries and rows(equations) 
  // defines estimated values for initial allocation if not knows may be
  // set to zero matrix will handle the size automatically
  InpMtx_init(mtxA, INPMTX_BY_ROWS, type, nent, neqns);
}

#include <set>

void LinSolver::A(int i, int j, float v){
   InpMtx_inputRealEntry(mtxA, i, j, v) ;
   // example for complex entry
   // InpMtx_inputComplexEntry(mtxA, irow, jcol, real, imag) ;
}


void LinSolver::MtrxB(){

  if(mtxY) DenseMtx_free(mtxY);

  nrhs=1; // number of righthand side entries
  mtxY = DenseMtx_new() ;   
  DenseMtx_init(mtxY, type, 0, 0, neqns, nrhs, 1, 1);
  DenseMtx_zero(mtxY); 
}
void LinSolver::B(int irow, float v){
  double v1;
  DenseMtx_realEntry (mtxY, irow, 0, &v1);
  DenseMtx_setRealEntry(mtxY, irow, 0, v1+v) ;
  //DenseMtx_setComplexEntry(mtxY, irow, jrhs, real, imag) ; 
}


float LinSolver::X(int irow){
  double v;
  DenseMtx_realEntry (mtxX, irow, 0, &v);
  //double complex;
  //DenseMtx_complexEntry (mtxX, irow, 0, &v,&img);
  return (float)v;
}


void LinSolver::solve() { 
  if(mtxX)  DenseMtx_free(mtxX);

  /*

  ------------------------------------------------------------------ 
  all-in-one program to solve A X = Y

  (3) form Graph object, order matrix and form front tree 
  (4) get the permutation, permute the front tree, matrix
  and right hand side and get the symbolic factorization 
  (5) initialize the front matrix object to hold the factor matrices 
  (6) compute the numeric factorization 
  (7) post-process the factor matrices 
  (8) compute the solution 
  (9) permute the solution into the original ordering

  created -- 98jun04, cca 
  ------------------------------------------------------------------ */ 


  Chv           *rootchv = 0; 
  ChvManager    *chvmanager = 0; 
  SubMtxManager *mtxmanager = 0 ; 
  FrontMtx      *frontmtx = 0; 
  ETree         *frontETree = 0; 
  Graph         *graph = 0; 
  IV            *newToOldIV = 0, *oldToNewIV=0 ; 
  IVL           *adjIVL=0, *symbfacIVL=0 ; 

  double droptol, tau; 
  double cpus[20] ;
  int stats[10] ;
  int error, msglvl, nedges;
  int *newToOld=0, *oldToNew=0 ; 


  //some default values
  droptol = 0.0; 
  tau = 100.; 

  // message level default (0)
  msglvl = 0 ;
  
  //msgFile 
  FILE *msgFile = stdout;

  // type -- type of entries" 
  // 1 (SPOOLES.REAL) -- real entries" 
  // 2 (SPOOLES.COMPLEX) -- complex entries" 
  type = 1; 

  // symmetryflag -- type of matrix 
  // 0 (SPOOLES.SYMMETRIC) -- symmetric entries 
  // 1 (SPOOLES.HERMITIAN) -- Hermitian entries 
  // 2 (SPOOLES.NONSYMMETRIC) -- nonsymmetric entries 
  symmetryflag = 2;

  // pivotingflag -- type of pivoting
  // 0 (SPOOLES.NO.PIVOTING) -- no pivoting used
  // 1 (SPOOLES.PIVOTING) -- pivoting used   
  pivotingflag = 1; 

  // seed -- random number seed, used for ordering
  seed = 666666;
  

  InpMtx_changeStorageMode(mtxA, INPMTX_BY_VECTORS) ;

  /*
    ------------------------------------------------- 
    STEP 3 : find a low-fill ordering 
    (1) create the Graph object 
    (2) order the graph using multiple minimum degree 
    ------------------------------------------------- 
  */ 
  graph = Graph_new() ; 
  adjIVL = InpMtx_fullAdjacency(mtxA) ; 
  nedges = IVL_tsize(adjIVL) ; 
  Graph_init2(graph, 0, neqns, 0, nedges, neqns, nedges, adjIVL, NULL, NULL) ; 
  if ( msglvl > 2 ) {
    //printf(msgFile, "\n\n graph of the input matrix") ; 
    //Graph_writeForHumanEye(graph, msgFile) ; 
    //fflush(msgFile) ; 
  } 
  frontETree = orderViaMMD(graph, seed, msglvl, msgFile) ; 
  if ( msglvl > 2 ) {
    //fprintf(msgFile, "\n\n front tree from ordering") ;
    //ETree_writeForHumanEye(frontETree, msgFile) ; 
    //fflush(msgFile) ; 
  } 
  /*--------------------------------------------------------------------*/ 
  printf("step 3 passed!\n");

  /*
    ---------------------------------------------------- 
    STEP 4: get the permutation, permute the front tree,
    permute the matrix and right hand side, and get the symbolic factorization 
    ---------------------------------------------------- 
  */ 
  oldToNewIV = ETree_oldToNewVtxPerm(frontETree) ; 
  oldToNew = IV_entries(oldToNewIV) ; 
  newToOldIV = ETree_newToOldVtxPerm(frontETree) ; 
  newToOld = IV_entries(newToOldIV) ;

  //28 SPOOLES 2.2 -- Solving Linear Systems April 29, 2002 

  ETree_permuteVertices(frontETree, oldToNewIV) ;
  InpMtx_permute(mtxA, oldToNew, oldToNew) ;
  if ( symmetryflag == SPOOLES_SYMMETRIC
       || symmetryflag == SPOOLES_HERMITIAN ) {
    InpMtx_mapToUpperTriangle(mtxA) ; 
  }
  InpMtx_changeCoordType(mtxA, INPMTX_BY_CHEVRONS) ;
  InpMtx_changeStorageMode(mtxA, INPMTX_BY_VECTORS) ; 
  DenseMtx_permuteRows(mtxY, oldToNewIV) ; 
  symbfacIVL = SymbFac_initFromInpMtx(frontETree, mtxA) ;
  if ( msglvl > 2 ) {
    /*
    fprintf(msgFile, "\n\n old-to-new permutation vector") ; 
    IV_writeForHumanEye(oldToNewIV, msgFile) ; 
    fprintf(msgFile, "\n\n new-to-old permutation vector") ; 
    IV_writeForHumanEye(newToOldIV, msgFile) ; 
    fprintf(msgFile, "\n\n front tree after permutation") ; 
    ETree_writeForHumanEye(frontETree, msgFile) ; 
    fprintf(msgFile, "\n\n input matrix after permutation") ; 
    InpMtx_writeForHumanEye(mtxA, msgFile) ; 
    fprintf(msgFile, "\n\n right hand side matrix after permutation") ; 
    DenseMtx_writeForHumanEye(mtxY, msgFile) ; 
    fprintf(msgFile, "\n\n symbolic factorization") ; 
    IVL_writeForHumanEye(symbfacIVL, msgFile) ; 
    fflush(msgFile) ; 
    */
  } 

  /*--------------------------------------------------------------------*/ 


  printf("step 4 passed!\n");

  /*   
       ------------------------------------------ 
       STEP 5: initialize the front matrix object 
       ------------------------------------------ 
  */ 

  frontmtx = FrontMtx_new() ; 
  mtxmanager = SubMtxManager_new() ; 
  SubMtxManager_init(mtxmanager, LOCK_IN_PROCESS, 0) ;
  FrontMtx_init(frontmtx, frontETree, symbfacIVL, type, symmetryflag,
        FRONTMTX_DENSE_FRONTS, pivotingflag, LOCK_IN_PROCESS, 0, NULL, mtxmanager, msglvl, msgFile) ;

  /*--------------------------------------------------------------------*/
  printf("step 5 passed!\n");

  /*
    ----------------------------------------- 
    STEP 6: compute the numeric factorization 
    ----------------------------------------- 
  */ 

  chvmanager = ChvManager_new() ; 
  ChvManager_init(chvmanager, LOCK_IN_PROCESS, 1) ;
  DVfill(20, cpus, 0.0) ;
  IVfill(10, stats, 0) ;
#ifdef MT_SUPPORT
  int nthread=8, nfront;
  if ( nthread > (nfront = FrontMtx_nfront(frontmtx)) ) {
    nthread = nfront ;
  }
  DV* cumopsDV = DV_new() ;
  DV_init(cumopsDV, nthread, NULL) ;
  IV *ownersIV = ETree_ddMap(frontETree, type, symmetryflag, cumopsDV, 1./(2.*nthread)) ;
  rootchv = FrontMtx_MT_factorInpMtx(frontmtx, mtxA, tau, droptol,chvmanager,
                                     ownersIV, 0,
                                     &error, cpus, stats, msglvl, msgFile) ;
#else
  rootchv = FrontMtx_factorInpMtx(frontmtx, mtxA, tau, droptol, chvmanager,
                                  &error, cpus, stats, msglvl, msgFile) ;
#endif
  ChvManager_free(chvmanager) ;
  InpMtx_free(mtxA) ; mtxA = 0;
 
  if ( msglvl > 2 ) {
    //April 29, 2002 SPOOLES 2.2 -- Solving Linear Systems 29

    //fprintf(msgFile, "\n\n factor matrix") ; 
    //FrontMtx_writeForHumanEye(frontmtx, msgFile) ; 
    //fflush(msgFile) ; 
  }
  if ( rootchv != NULL ) {
    //fprintf(msgFile, "\n\n matrix found to be singular\n") ; 
    //exit(-1) ; 
    return;
  } 
  if ( error >= 0 ) {
    //fprintf(msgFile, "\n\n error encountered at front %d", error) ; 
    //exit(-1) ; 
    return;
  }
  /*--------------------------------------------------------------------*/ 
  printf("step 6 passed!\n");

  /*
    -------------------------------------- 
    STEP 7: post-process the factorization 
    -------------------------------------- 
  */ 
  FrontMtx_postProcess(frontmtx, msglvl, msgFile) ; 
  if ( msglvl > 2 ) {
    //fprintf(msgFile, "\n\n factor matrix after post-processing") ; 
    //FrontMtx_writeForHumanEye(frontmtx, msgFile) ; fflush(msgFile) ; 
  } 
  /*--------------------------------------------------------------------*/ 
  printf("step 7 passed!\n");

  /*
    ------------------------------- 
    STEP 8: solve the linear system 
    ------------------------------- 
  */ 

  mtxX = DenseMtx_new();
  DenseMtx_init(mtxX, type, 0, 0, neqns, nrhs, 1, neqns);
  DenseMtx_zero(mtxX);

#ifdef MT_SUPPORT
  SolveMap *solvemap = SolveMap_new() ;
  SolveMap_ddMap(solvemap, symmetryflag, FrontMtx_upperBlockIVL(frontmtx),
                 FrontMtx_lowerBlockIVL(frontmtx), nthread, ownersIV,
                 FrontMtx_frontTree(frontmtx), seed, msglvl, msgFile);
  FrontMtx_MT_solve(frontmtx, mtxX, mtxY, mtxmanager, solvemap,
                    cpus, msglvl, msgFile);

  // release structures for MultiThread support
  SolveMap_free(solvemap);
  IV_free(ownersIV);
  DV_free(cumopsDV);
#else
  FrontMtx_solve(frontmtx, mtxX, mtxY, mtxmanager,
                 cpus, msglvl, msgFile);
#endif
  if ( msglvl > 2 ) {
    //fprintf(msgFile, "\n\n solution matrix in new ordering");
    //DenseMtx_writeForHumanEye(mtxX, msgFile);
    //fflush(msgFile);
  }
  /*--------------------------------------------------------------------*/ 
  printf("step 8 passed!\n");

  /*
    ------------------------------------------------------- 
    STEP 9: permute the solution into the original ordering 
    ------------------------------------------------------- */ 
  DenseMtx_permuteRows(mtxX, newToOldIV) ;
  if ( msglvl > 0 ) {
    //fprintf(msgFile, "\n\n solution matrix in original ordering") ; 
    //DenseMtx_writeForHumanEye(mtxX, msgFile) ; fflush(msgFile) ;
    //30 SPOOLES 2.2 -- Solving Linear Systems April 29, 2002 
  } 
 
  /*--------------------------------------------------------------------*/ 
  printf("step 9 passed!\n");

  /*
    ----------- 
    free memory 
    ----------- 
  */ 

  FrontMtx_free(frontmtx) ;
  // DenseMtx_free(mtxX) ; 
  DenseMtx_free(mtxY) ; mtxY=0; 
  IV_free(newToOldIV) ; 
  IV_free(oldToNewIV) ; 
  ETree_free(frontETree) ; 
  IVL_free(symbfacIVL) ; 
  SubMtxManager_free(mtxmanager) ; 
  Graph_free(graph) ; 


  /*--------------------------------------------------------------------*/ 
  return ; 
}
/*--------------------------------------------------------------------*/

