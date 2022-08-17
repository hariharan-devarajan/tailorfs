//
// Created by hariharan on 8/17/22.
//

#include "mpiio_fsview.h"
TailorFSStatus tailorfs::MPIIOFSView::initialize(MPIIOInit& payload) {
  return 0;
}
TailorFSStatus tailorfs::MPIIOFSView::open(MPIIOOpen& payload) { return 0; }
TailorFSStatus tailorfs::MPIIOFSView::close(MPIIOClose& payload) { return 0; }
TailorFSStatus tailorfs::MPIIOFSView::write(MPIIORead& payload) { return 0; }
TailorFSStatus tailorfs::MPIIOFSView::read(MPIIOWrite& payload) { return 0; }
TailorFSStatus tailorfs::MPIIOFSView::finalize(MPIIOFinalize& payload) {
  return 0;
}
