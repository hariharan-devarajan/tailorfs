//
// Created by hariharan on 8/17/22.
//

#ifndef TAILORFS_MPIIO_FSVIEW_H
#define TAILORFS_MPIIO_FSVIEW_H
#include <tailorfs/core/datastructure.h>

#include "fsview.h"
namespace tailorfs {
class MPIIOFSView : public FSView<MPIIOInit, MPIIOFinalize, MPIIOOpen,
                                  MPIIOClose, MPIIORead, MPIIOWrite> {
 public:
  MPIIOFSView() = default;
  MPIIOFSView(MPIIOFSView& other) = default;
  MPIIOFSView(MPIIOFSView&& other) = default;
  MPIIOFSView& operator=(const MPIIOFSView& t) = default;
  TailorFSStatus initialize(MPIIOInit& payload) override;
  TailorFSStatus open(MPIIOOpen& payload) override;
  TailorFSStatus close(MPIIOClose& payload) override;
  TailorFSStatus write(MPIIORead& payload) override;
  TailorFSStatus read(MPIIOWrite& payload) override;
  TailorFSStatus finalize(MPIIOFinalize& payload) override;
};
}  // namespace tailorfs
#endif  // TAILORFS_MPIIO_FSVIEW_H
