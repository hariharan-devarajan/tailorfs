//
// Created by hariharan on 8/21/22.
//

#ifndef TAILORFS_PROCESS_STATE_H
#define TAILORFS_PROCESS_STATE_H
#include <mpi.h>
#include <tailorfs/core/singleton.h>
namespace tailorfs {
class ProcessState {
 public:
  int rank{}, comm_size{};
  ProcessState() {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  }
};
}  // namespace tailorfs
#define PROCESS_STATE \
  tailorfs::Singleton<tailorfs::ProcessState>::get_instance()

#endif  // TAILORFS_PROCESS_STATE_H
