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
  std::unordered_set<std::string> filenames;
  std::unordered_set<std::string> excluded_filenames;
  std::unordered_set<FileDescriptor> fds;
  std::unordered_map<FileDescriptor, FSID> fsid_map;
  std::unordered_map<FSID, std::pair<MPI_File, off_t>> mpiio_map;
  std::unordered_map<FSID, std::pair<unifyfs_gfid, off_t>> unifyfs_map;

  inline bool is_traced(const char *filename) {
    auto iter = excluded_filenames.find(filename);
    if (iter != excluded_filenames.end()) return false;
    iter = filenames.find(filename);
    if (iter != filenames.end()) return true;
    return false;
  }

  inline bool is_traced(const int fd) {
    auto iter = fds.find(fd);
    if (iter != fds.end()) return true;
    return false;
  }

  inline void track_file(const char *filename) { filenames.emplace(filename); }
  inline void untrack_file(const char *filename) { filenames.erase(filename); }
  inline void track_fd(const int fd) { fds.emplace(fd); }
  inline void untrack_fd(const int fd) { fds.erase(fd); }

 public:
  POSIXTailorFS() : POSIX(), filenames(), fds() {
    auto config = MIMIR_CONFIG();
    for (auto file : config->_file_repo) {
      track_file(file._name.c_str());
    }
  }
  ~POSIXTailorFS() override = default;
  static std::shared_ptr<POSIXTailorFS> get_instance() {
    if (instance == nullptr) {
      instance = std::make_shared<POSIXTailorFS>();
      POSIX::set_instance(instance);
    }
    return instance;
  }
  int open(const char *pathname, int flags, mode_t mode) override;
  int close(int fd) override;
  ssize_t write(int fd, const void *buf, size_t count) override;
  ssize_t read(int fd, void *buf, size_t count) override;
  off_t lseek(int fd, off_t offset, int whence) override;
};

std::shared_ptr<POSIXTailorFS> POSIXTailorFS::instance = nullptr;
}  // namespace brahma
#endif  // TAILORFS_POSIX_H
