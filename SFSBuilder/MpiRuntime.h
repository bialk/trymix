#ifndef mpi_runtime_h
#define mpi_runtime_h

class MpiRuntime {
public:
  MpiRuntime(int &argc, char **&argv);
  ~MpiRuntime();

  MpiRuntime(const MpiRuntime &) = delete;
  MpiRuntime &operator=(const MpiRuntime &) = delete;

  bool active() const;

private:
  bool active_ = false;
  bool ownsRuntime_ = false;
};

#endif
