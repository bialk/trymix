#ifndef lin_solver_h
#define lin_solver_h

enum class LinSolverKind {
  MT,
  Eigen,
  MPI
};

class LinSolver {
public:
  virtual ~LinSolver() = default;

  virtual void clear() = 0;
  virtual void MtrxA(int rows, int cols, int estimatedEntries) = 0;
  virtual void A(int row, int column, float value) = 0;
  virtual void MtrxB() = 0;
  virtual void B(int row, float value) = 0;
  virtual float X(int row) const = 0;
  virtual void solve() = 0;

protected:
  LinSolver() = default;
  LinSolver(const LinSolver &) = default;
  LinSolver &operator=(const LinSolver &) = default;
};

#endif
