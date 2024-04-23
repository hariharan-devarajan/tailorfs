//
// Created by hariharan on 8/8/22.
//

#ifndef TAILORFS_POSIX_H
#define TAILORFS_POSIX_H

#include <brahma/brahma.h>
#include <fcntl.h>
#include <mimir/mimir.h>
#include <tailorfs/core/datastructure.h>
#include <tailorfs/macro.h>

#include <memory>
#include <unordered_set>

using namespace tailorfs;
namespace brahma {
class POSIXTailorFS : public POSIX {
 private:
  typedef int FileDescriptor;
  static std::shared_ptr<POSIXTailorFS> instance;
  std::unordered_set<FileDescriptor> fds;
  std::unordered_map<FileDescriptor, FSID> fsid_map;
  std::unordered_map<FSID, std::pair<MPI_File, off_t>> mpiio_map;
  std::unordered_map<FSID, std::pair<unifyfs_gfid, off_t>> unifyfs_map;
  std::unordered_map<FSID, std::pair<FILE*, off_t>> stdio_map;
  std::unordered_map<FSID, std::pair<FileDescriptor, off_t>> posix_map;
  inline bool is_traced(const char* filename) {
    return utility->is_traced(filename, brahma::InterfaceType::INTERFACE_POSIX);
  }
  inline void exclude_file(const char *filename) {
    utility->exclude_file(filename, brahma::InterfaceType::INTERFACE_POSIX);
  }
  inline void include_file(const char *filename) {
    utility->include_file(filename, brahma::InterfaceType::INTERFACE_POSIX);

  }
  inline void track_file(const char *filename) {
    utility->track_file(filename, brahma::InterfaceType::INTERFACE_POSIX);
  }
  inline void untrack_file(const char *filename) {
    utility->untrack_file(filename, brahma::InterfaceType::INTERFACE_POSIX);
  }
  inline bool is_traced(const int fd) {
    auto iter = fds.find(fd);
    if (iter != fds.end()) return true;
    return false;
  }
  inline void track_fd(const int fd) { fds.emplace(fd); }
  inline void untrack_fd(const int fd) { fds.erase(fd); }

 public:
  POSIXTailorFS() : POSIX(),  fds() {
    TAILORFS_LOGINFO("POSIX class intercepted", "");
    auto config = MIMIR_CONFIG();
    if (config->_current_process_index != -1) {
      auto app_intent = config->_app_repo[config->_current_process_index];
      auto selected_interface = mimir::InterfaceType::POSIX;
      int rank = 0;
      if(app_intent._is_mpi) {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      }
      bool is_used = true;
      auto interface_file_indices = std::unordered_set<mimir::FileIndex>();
      for (auto element : app_intent._interfaces_used) {
        for (const auto& interface: element.second) {
          if (interface == selected_interface) {
            interface_file_indices.emplace(element.first);
          }
        }
      }

      auto rank_file_indices = std::unordered_set<mimir::FileIndex>();
      for (auto element : app_intent._rank_file_dag.edges) {
        if (element.source == rank) rank_file_indices.emplace(element.destination);
      }
      for ( auto iter:rank_file_indices) {
        auto iterface_iter = interface_file_indices.find(iter);
        if (iterface_iter != interface_file_indices.end())
          track_file(config->_file_repo[iter]._name.c_str());
      }
    }
  }
  ~POSIXTailorFS() {}
  static std::shared_ptr<POSIXTailorFS> get_instance() {
    if (instance == nullptr) {
      instance = std::make_shared<POSIXTailorFS>();
      POSIX::set_instance(instance);
    }
    return instance;
  }
  int open(const char *pathname, int flags, ...) override;
  int close(int fd) override;
  ssize_t write(int fd, const void *buf, size_t count) override;
  ssize_t read(int fd, void *buf, size_t count) override;
  off_t lseek(int fd, off_t offset, int whence) override;
};

}  // namespace brahma
#endif  // TAILORFS_POSIX_H
