//
// Created by hariharan on 8/17/22.
//

#include "mpiio_fsview.h"

#include <fcntl.h>

#include <regex>

#include "tailorfs/error_code.h"
TailorFSStatus tailorfs::MPIIOFSView::initialize(MPIIOInit& payload) {
  feature = payload.feature;
  return TAILORFS_SUCCESS;
}
TailorFSStatus tailorfs::MPIIOFSView::open(MPIIOOpen& payload) {
  std::string filename = std::string(payload.filename);
  if (feature.redirection.redirection) {
    filename = std::regex_replace(
        filename, std::regex(feature.redirection.original_storage._mount_point),
        feature.redirection.new_storage._mount_point);
  }
  int status_orig =
      MPI_File_open(payload.communicator, filename.c_str(), payload.flags,
                    MPI_INFO_NULL, &payload.file_ptr);
  if (feature.redirection.redirection) {
    redirect_map.insert_or_assign(
        payload.file_ptr,
        std::pair<std::string, std::string>(payload.filename, filename));
  }
  return status_orig == 0 ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::MPIIOFSView::close(MPIIOClose& payload) {
  int status_orig;
  if (feature.redirection.redirection) {
    auto iter = redirect_map.find(payload.file_ptr);
    if (iter != redirect_map.end()) {
      status_orig = MPI_File_close(&payload.file_ptr);
      fs::copy_file(iter->second.second, fs::absolute(iter->second.first),
                    fs::copy_options::overwrite_existing);
    } else {
      status_orig = MPI_File_close(&payload.file_ptr);
    }
    fs::remove(iter->second.second);
  }
  return status_orig == 0 ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::MPIIOFSView::write(MPIIOWrite& payload) {
  MPI_Status stat_orig;
  MPI_File_write_at_all(payload.file_ptr, payload.offset, payload.buf,
                        payload.count, payload.type, &stat_orig);
  MPI_Get_count(&stat_orig, MPI_CHAR, &payload.written_bytes);
  return payload.written_bytes == payload.count ? TAILORFS_SUCCESS
                                                : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::MPIIOFSView::read(MPIIORead& payload) {
  MPI_Status stat_orig;
  MPI_File_read_at_all(payload.file_ptr, payload.offset, payload.buf,
                       payload.count, payload.type, &stat_orig);
  MPI_Get_count(&stat_orig, MPI_CHAR, &payload.read_bytes);
  return payload.read_bytes == payload.count ? TAILORFS_SUCCESS
                                             : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::MPIIOFSView::finalize(MPIIOFinalize& payload) {
  return 0;
}
TailorFSStatus tailorfs::MPIIOFSView::convert(int flags, int& mpi_flags) {
  mpi_flags = 0;
  if (flags & O_RDWR) {
    mpi_flags = mpi_flags | MPI_MODE_RDWR;
  }
  if (flags & O_RDONLY) {
    mpi_flags = mpi_flags | MPI_MODE_RDONLY;
  }
  if (flags & O_WRONLY) {
    mpi_flags = mpi_flags | MPI_MODE_WRONLY;
  }
  if (flags & O_EXCL) {
    mpi_flags = mpi_flags | MPI_MODE_EXCL;
  }
  if (flags & O_CREAT) {
    mpi_flags = mpi_flags | MPI_MODE_CREATE;
  }
  if (flags & O_APPEND) {
    mpi_flags = mpi_flags | MPI_MODE_APPEND;
  }
  return TAILORFS_SUCCESS;
}
