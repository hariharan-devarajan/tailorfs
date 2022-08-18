//
// Created by hariharan on 8/17/22.
//

#ifndef TAILORFS_MPIIO_FSVIEW_H
#define TAILORFS_MPIIO_FSVIEW_H
#include <tailorfs/core/datastructure.h>

#include <experimental/filesystem>

#include "fsview.h"
namespace fs = std::experimental::filesystem;
namespace tailorfs {
class MPIIOFSView : public FSView<MPIIOInit, MPIIOFinalize, MPIIOOpen,
                                  MPIIOClose, MPIIORead, MPIIOWrite> {
 private:
  MPIIOFeature feature;
  std::unordered_map<MPI_File, std::pair<std::string, std::string>>
      redirect_map;

 public:
  MPIIOFSView() = default;
  MPIIOFSView(MPIIOFSView& other) = default;
  MPIIOFSView(MPIIOFSView&& other) = default;
  MPIIOFSView& operator=(const MPIIOFSView& t) = default;
  TailorFSStatus initialize(MPIIOInit& payload) override;
  TailorFSStatus open(MPIIOOpen& payload) override;
  TailorFSStatus close(MPIIOClose& payload) override;
  TailorFSStatus write(MPIIOWrite& payload) override;
  TailorFSStatus read(MPIIORead& payload) override;
  TailorFSStatus finalize(MPIIOFinalize& payload) override;
  TailorFSStatus convert(int flags, int& mpi_flags);
};
}  // namespace tailorfs
#endif  // TAILORFS_MPIIO_FSVIEW_H
