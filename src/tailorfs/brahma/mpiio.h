//
// Created by haridev on 8/29/22.
//

#ifndef TAILORFS_MPIIO_H
#define TAILORFS_MPIIO_H

#include <brahma/interface/mpiio.h>

#include <mimir/api/mimir_interceptor.h>
#include <tailorfs/core/fsview_manager.h>
#include <tailorfs/macro.h>
using namespace tailorfs;
namespace brahma {
class MPIIOTailorFS : public MPIIO {
 private:
  std::unordered_set<MPI_File> fds;
  std::unordered_map<MPI_File, FSID> fsid_map;
  std::unordered_map<FSID, std::pair<MPI_File, off_t>> mpiio_map;
  std::unordered_map<FSID, std::pair<unifyfs_gfid, off_t>> unifyfs_map;
  std::unordered_map<FSID, std::pair<FILE*, off_t>> stdio_map;
  std::unordered_map<FSID, std::pair<int, off_t>> posix_map;
  static std::shared_ptr<MPIIOTailorFS> instance;
  inline bool is_traced(MPI_File fd) {
    auto iter = fds.find(fd);
    if (iter != fds.end()) return true;
    return false;
  }
  inline void track_fd(MPI_File fd) { fds.emplace(fd); }
  inline void untrack_fd(MPI_File fd) { fds.erase(fd); }
  inline bool is_traced(const char* filename) {
    return utility->is_traced(filename, brahma::InterfaceType::INTERFACE_MPIIO);
  }
  inline void exclude_file(const char *filename) {
    utility->exclude_file(filename, brahma::InterfaceType::INTERFACE_MPIIO);
  }
  inline void include_file(const char *filename) {
    utility->include_file(filename, brahma::InterfaceType::INTERFACE_MPIIO);

  }
  inline void track_file(const char *filename) {
    utility->track_file(filename, brahma::InterfaceType::INTERFACE_MPIIO);
  }
  inline void untrack_file(const char *filename) {
    utility->untrack_file(filename, brahma::InterfaceType::INTERFACE_MPIIO);
  }
 public:
  MPIIOTailorFS() : MPIIO() {
    TAILORFS_LOGPRINT("MPIIO class intercepted", "");
    auto config = MIMIR_CONFIG();
    if (config->_current_process_index != -1) {
      auto app_intent = config->_app_repo[config->_current_process_index];
      for (auto element : app_intent._interfaces_used) {
        for (const auto& interface: element.second) {
          if (interface == mimir::InterfaceType::MPIIO) {
            track_file(config->_file_repo[element.first]._name.c_str());
          }
        }
      }
    }
  }
  ~MPIIOTailorFS() = default;
  static std::shared_ptr<MPIIOTailorFS> get_instance() {
    if (instance == nullptr) {
      instance = std::make_shared<MPIIOTailorFS>();
      MPIIO::set_instance(instance);
    }
    return instance;
  }
  int MPI_File_close(MPI_File *fh) override;
  int MPI_File_open(MPI_Comm comm, const char *filename, int amode,
                     MPI_Info info, MPI_File *fh) override;
  int MPI_File_read_at_all(MPI_File fh, MPI_Offset offset, void *buf,
                            int count, MPI_Datatype datatype,
                            MPI_Status *status) override;
  int MPI_File_write_at_all(MPI_File fh, MPI_Offset offset, const void *buf,
                             int count, MPI_Datatype datatype,
                             MPI_Status *status) override;

  /**
   * TODO(hari): add MPI_Get_count
   */

};
}

#endif  // TAILORFS_MPIIO_H
