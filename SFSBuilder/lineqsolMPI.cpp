#include "lineqsolMPI.h"

#include <stdexcept>

extern "C" {
#include <SymbFac.h>
#include <misc.h>
}

LinSolverMPI::LinSolverMPI(MPI_Comm communicator)
    : communicator_(communicator) {
  requireMPI();
}

LinSolverMPI::~LinSolverMPI() { clear(); }

void LinSolverMPI::requireMPI() const {
  int initialized = 0;
  int finalized = 0;
  MPI_Initialized(&initialized);
  if (initialized) {
    MPI_Finalized(&finalized);
  }
  if (!initialized || finalized) {
    throw std::runtime_error(
        "LinSolverMPI requires an initialized, non-finalized MPI runtime");
  }
}

void LinSolverMPI::clear() {
  if (mtxA_ != nullptr) {
    InpMtx_free(mtxA_);
    mtxA_ = nullptr;
  }
  if (mtxY_ != nullptr) {
    DenseMtx_free(mtxY_);
    mtxY_ = nullptr;
  }
  solution_.clear();
}

void LinSolverMPI::MtrxA(int rows, int cols, int ent) {
  requireMPI();
  if (rows != cols || rows <= 0 || ent < 0) {
    throw std::invalid_argument("LinSolverMPI requires a non-empty square matrix");
  }

  int rank = 0;
  int processCount = 0;
  MPI_Comm_rank(communicator_, &rank);
  MPI_Comm_size(communicator_, &processCount);
  if (processCount > rows) {
    throw std::invalid_argument(
        "LinSolverMPI requires at least one matrix row per MPI rank");
  }
  if (mtxA_ != nullptr) {
    InpMtx_free(mtxA_);
  }
  neqns_ = rows;
  solution_.clear();
  mtxA_ = InpMtx_new();
  const int estimatedLocalEntries = (ent + processCount - 1) / processCount;
  const int localRows = (rows - rank + processCount - 1) / processCount;
  InpMtx_init(mtxA_, INPMTX_BY_ROWS, type_, estimatedLocalEntries, localRows);
}

void LinSolverMPI::A(int i, int j, float value) {
  if (mtxA_ == nullptr) {
    throw std::logic_error("MtrxA() must be called before A()");
  }
  int rank = 0;
  int processCount = 0;
  MPI_Comm_rank(communicator_, &rank);
  MPI_Comm_size(communicator_, &processCount);
  if (i % processCount == rank) {
    InpMtx_inputRealEntry(mtxA_, i, j, value);
  }
}

void LinSolverMPI::MtrxB() {
  if (neqns_ <= 0) {
    throw std::logic_error("MtrxA() must be called before MtrxB()");
  }
  int rank = 0;
  int processCount = 0;
  MPI_Comm_rank(communicator_, &rank);
  MPI_Comm_size(communicator_, &processCount);
  if (mtxY_ != nullptr) {
    DenseMtx_free(mtxY_);
  }
  mtxY_ = DenseMtx_new();
  const int localRows = (neqns_ - rank + processCount - 1) / processCount;
  DenseMtx_init(mtxY_, type_, 0, 0, localRows, nrhs_, 1,
                localRows);
  int nrow = 0;
  int *rowind = nullptr;
  DenseMtx_rowIndices(mtxY_, &nrow, &rowind);
  IVramp(nrow, rowind, rank, processCount);
  DenseMtx_zero(mtxY_);
}

void LinSolverMPI::B(int i, float value) {
  if (mtxY_ == nullptr) {
    throw std::logic_error("MtrxB() must be called before B()");
  }
  int rank = 0;
  int processCount = 0;
  MPI_Comm_rank(communicator_, &rank);
  MPI_Comm_size(communicator_, &processCount);
  if (i % processCount == rank) {
    double oldValue = 0.0;
    const int localRow = i / processCount;
    DenseMtx_realEntry(mtxY_, localRow, 0, &oldValue);
    DenseMtx_setRealEntry(mtxY_, localRow, 0, oldValue + value);
  }
}

float LinSolverMPI::X(int i) const {
  if (i < 0 || i >= static_cast<int>(solution_.size())) {
    throw std::out_of_range("LinSolverMPI solution index is out of range");
  }
  return static_cast<float>(solution_[i]);
}

void LinSolverMPI::solve() {
  requireMPI();
  if (mtxA_ == nullptr || mtxY_ == nullptr) {
    throw std::logic_error("matrix and right-hand side must be initialized");
  }

  int rank = 0;
  int processCount = 0;
  MPI_Comm_rank(communicator_, &rank);
  MPI_Comm_size(communicator_, &processCount);

  int minEquations = 0;
  int maxEquations = 0;
  MPI_Allreduce(&neqns_, &minEquations, 1, MPI_INT, MPI_MIN, communicator_);
  MPI_Allreduce(&neqns_, &maxEquations, 1, MPI_INT, MPI_MAX, communicator_);
  if (minEquations != maxEquations) {
    throw std::runtime_error("all MPI ranks must configure the same matrix size");
  }

  FILE *messageFile = stdout;
  constexpr int messageLevel = 0;
  constexpr int lookahead = 0;
  constexpr double tau = 100.0;
  constexpr double dropTolerance = 0.0;
  int firstTag = 0;
  int error = -1;
  int stats[20];
  double cpus[20];
  IVzero(20, stats);
  DVzero(20, cpus);

  InpMtx_sortAndCompress(mtxA_);
  InpMtx_changeStorageMode(mtxA_, INPMTX_BY_VECTORS);

  IVL *adjacency = InpMtx_MPI_fullAdjacency(
      mtxA_, stats, messageLevel, messageFile, communicator_);
  Graph *graph = Graph_new();
  const int edgeCount = IVL_tsize(adjacency);
  Graph_init2(graph, 0, neqns_, 0, edgeCount, neqns_, edgeCount, adjacency,
              nullptr, nullptr);
  ETree *frontTree = orderViaMMD(graph, seed_ + rank, messageLevel, messageFile);
  Graph_free(graph);

  double *operationCounts = DVinit(processCount, 0.0);
  operationCounts[rank] = ETree_nFactorOps(frontTree, type_, symmetryflag_);
  MPI_Allgather(&operationCounts[rank], 1, MPI_DOUBLE, operationCounts, 1,
                MPI_DOUBLE, communicator_);
  int orderingRoot = 0;
  DVmin(processCount, operationCounts, &orderingRoot);
  DVfree(operationCounts);
  frontTree = ETree_MPI_Bcast(frontTree, orderingRoot, messageLevel,
                              messageFile, communicator_);

  IV *oldToNew = ETree_oldToNewVtxPerm(frontTree);
  IV *newToOld = ETree_newToOldVtxPerm(frontTree);
  ETree_permuteVertices(frontTree, oldToNew);
  InpMtx_permute(mtxA_, IV_entries(oldToNew), IV_entries(oldToNew));
  InpMtx_changeCoordType(mtxA_, INPMTX_BY_CHEVRONS);
  InpMtx_changeStorageMode(mtxA_, INPMTX_BY_VECTORS);
  DenseMtx_permuteRows(mtxY_, oldToNew);

  DV *cumulativeOperations = DV_new();
  DV_init(cumulativeOperations, processCount, nullptr);
  IV *owners = ETree_ddMap(frontTree, type_, symmetryflag_,
                           cumulativeOperations, 1.0 / (2.0 * processCount));
  DV_free(cumulativeOperations);
  IV *vertexMap = IV_new();
  IV_init(vertexMap, neqns_, nullptr);
  IVgather(neqns_, IV_entries(vertexMap), IV_entries(owners),
           ETree_vtxToFront(frontTree));

  InpMtx *distributedA = InpMtx_MPI_split(
      mtxA_, vertexMap, stats, messageLevel, messageFile, firstTag,
      communicator_);
  ++firstTag;
  InpMtx_free(mtxA_);
  mtxA_ = distributedA;
  InpMtx_changeStorageMode(mtxA_, INPMTX_BY_VECTORS);

  DenseMtx *distributedY = DenseMtx_MPI_splitByRows(
      mtxY_, vertexMap, stats, messageLevel, messageFile, firstTag,
      communicator_);
  DenseMtx_free(mtxY_);
  mtxY_ = distributedY;
  firstTag += processCount;

  IVL *symbolic = SymbFac_MPI_initFromInpMtx(
      frontTree, owners, mtxA_, stats, messageLevel, messageFile, firstTag,
      communicator_);
  firstTag += frontTree->nfront;

  SubMtxManager *matrixManager = SubMtxManager_new();
  SubMtxManager_init(matrixManager, NO_LOCK, 0);
  FrontMtx *frontMatrix = FrontMtx_new();
  FrontMtx_init(frontMatrix, frontTree, symbolic, type_, symmetryflag_,
                FRONTMTX_DENSE_FRONTS, pivotingflag_, NO_LOCK, rank, owners,
                matrixManager, messageLevel, messageFile);

  ChvManager *chvManager = ChvManager_new();
  ChvManager_init(chvManager, NO_LOCK, 0);
  Chv *rootChv = FrontMtx_MPI_factorInpMtx(
      frontMatrix, mtxA_, tau, dropTolerance, chvManager, owners, lookahead,
      &error, cpus, stats, messageLevel, messageFile, firstTag, communicator_);
  ChvManager_free(chvManager);
  firstTag += 3 * frontTree->nfront + 2;
  const int localFactorizationFailure = rootChv != nullptr || error >= 0;
  int factorizationFailure = 0;
  MPI_Allreduce(&localFactorizationFailure, &factorizationFailure, 1, MPI_INT,
                MPI_MAX, communicator_);
  if (factorizationFailure != 0) {
    throw std::runtime_error("SPOOLES MPI factorization failed");
  }

  FrontMtx_MPI_postProcess(frontMatrix, owners, stats, messageLevel,
                           messageFile, firstTag, communicator_);
  firstTag += 5 * processCount;

  SolveMap *solveMap = SolveMap_new();
  SolveMap_ddMap(solveMap, symmetryflag_, FrontMtx_upperBlockIVL(frontMatrix),
                 FrontMtx_lowerBlockIVL(frontMatrix), processCount, owners,
                 FrontMtx_frontTree(frontMatrix), seed_, messageLevel,
                 messageFile);
  FrontMtx_MPI_split(frontMatrix, solveMap, stats, messageLevel, messageFile,
                     firstTag, communicator_);

  if (FRONTMTX_IS_PIVOTING(frontMatrix)) {
    IV *rowMap = FrontMtx_MPI_rowmapIV(frontMatrix, owners, messageLevel,
                                       messageFile, communicator_);
    DenseMtx *pivotedY = DenseMtx_MPI_splitByRows(
        mtxY_, rowMap, stats, messageLevel, messageFile, firstTag,
        communicator_);
    DenseMtx_free(mtxY_);
    mtxY_ = pivotedY;
    IV_free(rowMap);
  }

  IV *ownedColumns = FrontMtx_ownedColumnsIV(
      frontMatrix, rank, owners, messageLevel, messageFile);
  const int localColumnCount = IV_size(ownedColumns);
  DenseMtx *localX = DenseMtx_new();
  DenseMtx_init(localX, type_, 0, 0, localColumnCount, nrhs_, 1,
                localColumnCount);
  if (localColumnCount > 0) {
    int localRows = 0;
    int *rowIndices = nullptr;
    DenseMtx_rowIndices(localX, &localRows, &rowIndices);
    IVcopy(localColumnCount, rowIndices, IV_entries(ownedColumns));
  }

  SubMtxManager *solveManager = SubMtxManager_new();
  SubMtxManager_init(solveManager, NO_LOCK, 0);
  FrontMtx_MPI_solve(frontMatrix, localX, mtxY_, solveManager, solveMap, cpus,
                     stats, messageLevel, messageFile, firstTag,
                     communicator_);
  SubMtxManager_free(solveManager);

  DenseMtx_permuteRows(localX, newToOld);
  IV_fill(vertexMap, 0);
  ++firstTag;
  DenseMtx *gatheredX = DenseMtx_MPI_splitByRows(
      localX, vertexMap, stats, messageLevel, messageFile, firstTag,
      communicator_);
  DenseMtx_free(localX);

  solution_.assign(neqns_, 0.0);
  if (rank == 0) {
    int gatheredRows = 0;
    int *rowIndices = nullptr;
    DenseMtx_rowIndices(gatheredX, &gatheredRows, &rowIndices);
    for (int localRow = 0; localRow < gatheredRows; ++localRow) {
      double value = 0.0;
      DenseMtx_realEntry(gatheredX, localRow, 0, &value);
      solution_[rowIndices[localRow]] = value;
    }
  }
  MPI_Bcast(solution_.data(), neqns_, MPI_DOUBLE, 0, communicator_);

  DenseMtx_free(gatheredX);
  IV_free(ownedColumns);
  SolveMap_free(solveMap);
  FrontMtx_free(frontMatrix);
  SubMtxManager_free(matrixManager);
  IVL_free(symbolic);
  IV_free(vertexMap);
  IV_free(owners);
  IV_free(newToOld);
  IV_free(oldToNew);
  ETree_free(frontTree);
}
