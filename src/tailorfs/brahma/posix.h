//
// Created by hariharan on 8/8/22.
//

#ifndef TAILORFS_POSIX_H
#define TAILORFS_POSIX_H

#include <brahma/brahma.h>
#include <fcntl.h>

#include <memory>

#include "tailorfs/macro.h"

namespace brahma {
class POSIXTest : public POSIX {
 private:
  typedef int FileDescriptor;
  static std::shared_ptr<POSIXTest> instance;
  std::unordered_set<std::string> filenames;
  std::unordered_set<FileDescriptor> fds;

  inline bool is_traced(const char *filename) {
    auto iter = filenames.find(filename);
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
  POSIXTest() {
    auto config = MIMIR_CONFIG();
    for (auto file : config->_file_repo) {
      track_file(file._name.c_str());
    }
  }
  static std::shared_ptr<POSIXTest> get_instance() {
    if (instance == nullptr) {
      instance = std::make_shared<POSIXTest>();
      POSIX::set_instance(instance);
    }
    return instance;
  }
  int open(const char *pathname, int flags, mode_t mode) override {
    BRAHMA_MAP_OR_FAIL(open);
    int ret = -1;
    if (is_traced(pathname)) {
      ret = __real_open(pathname, flags, mode);
      TAILORFS_LOGPRINT("open(%s, %d, %u) = %d", pathname, flags,
                        (unsigned int)mode, ret);
      track_fd(ret);
    } else {
      ret = __real_open(pathname, flags, mode);
    }
    return ret;
  }
  int close(int fd) override {
    BRAHMA_MAP_OR_FAIL(close);
    int ret = -1;
    if (is_traced(fd)) {
      ret = __real_close(fd);
      TAILORFS_LOGPRINT("close(%d) = %d", fd, ret);
      untrack_fd(ret);
    } else {
      ret = __real_close(fd);
    }
    return ret;
  }
  ssize_t write(int fd, const void *buf, size_t count) override {
    BRAHMA_MAP_OR_FAIL(write);
    ssize_t ret = -1;
    if (is_traced(fd)) {
      ret = __real_write(fd, buf, count);
      TAILORFS_LOGPRINT("write(%d, buf, %d) = %d", fd, count, ret);
    } else {
      ret = __real_write(fd, buf, count);
    }
    return ret;
  }
  ssize_t read(int fd, void *buf, size_t count) override {
    BRAHMA_MAP_OR_FAIL(read);
    ssize_t ret = -1;
    if (is_traced(fd)) {
      ret = __real_read(fd, buf, count);
      TAILORFS_LOGPRINT("write(%d, buf, %d) = %d", fd, count, ret);
    } else {
      ret = __real_read(fd, buf, count);
    }
    return ret;
  }
};
}  // namespace brahma
#endif  // TAILORFS_POSIX_H
