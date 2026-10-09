#include "MpiRuntime.h"

#include <mpi.h>

#include <cstdio>

MpiRuntime::MpiRuntime(int &argc, char **&argv)
{
  int initialized = 0;
  MPI_Initialized(&initialized);
  if (initialized) {
    active_ = true;
    return;
  }

  int provided = MPI_THREAD_SINGLE;
  const int error =
      MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
  if (error != MPI_SUCCESS || provided < MPI_THREAD_SERIALIZED) {
    std::fprintf(stderr,
                 "Unable to initialize MPI with serialized thread support.\n");
    return;
  }

  active_ = true;
  ownsRuntime_ = true;
}

MpiRuntime::~MpiRuntime()
{
  int finalized = 0;
  MPI_Finalized(&finalized);
  if (ownsRuntime_ && !finalized) {
    MPI_Finalize();
  }
}

bool MpiRuntime::active() const { return active_; }
