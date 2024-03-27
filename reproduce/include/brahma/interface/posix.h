//
// Created by hariharan on 8/8/22.
//

#ifndef BRAHMA_POSIX_H
#define BRAHMA_POSIX_H

#include <unistd.h>

#include <cstdlib>

#include "brahma/interceptor.h"
#include "interface.h"

namespace brahma {
class POSIX :public Interface {
 private:
  static std::shared_ptr<POSIX> my_instance;

 public:
  static std::shared_ptr<POSIX> get_instance() {
    if (my_instance == nullptr) {
      BRAHMA_LOGINFO("POSIX class not intercepted but used", "");
      my_instance = std::make_shared<POSIX>();
    }
    return my_instance;
  }
  POSIX():Interface(){}
  virtual ~POSIX() = default;
  static int set_instance(std::shared_ptr<POSIX> instance_i) {
    if (instance_i != nullptr) {
      my_instance = instance_i;
      return 0;
    } else {
      BRAHMA_LOGERROR("instance_i is not set", "");
      throw std::runtime_error("instance_i is not set");
    }
  }

  virtual int open(const char *pathname, int flags, mode_t mode);

  virtual int creat64(const char *path, mode_t mode);

  virtual int open64(const char *path, int flags, mode_t mode);

  virtual int close(int fd);

  virtual ssize_t write(int fd, const void *buf, size_t count);

  virtual ssize_t read(int fd, void *buf, size_t count);

  virtual off_t lseek(int fd, off_t offset, int whence);

  virtual off64_t lseek64(int fd, off64_t offset, int whence);

  virtual ssize_t pread(int fd, void *buf, size_t count, off_t offset);

  virtual ssize_t pread64(int fd, void *buf, size_t count, off64_t offset);

  virtual ssize_t pwrite(int fd, const void *buf, size_t count, off64_t offset);

  virtual ssize_t pwrite64(int fd, const void *buf, size_t count,
                           off64_t offset);

  virtual int fsync(int fd);

  virtual int fdatasync(int fd);
};

}  // namespace brahma

GOTCHA_MACRO_TYPEDEF(open, int, (const char *pathname, int flags, mode_t mode),
                     (pathname, flags, mode), brahma::POSIX);

GOTCHA_MACRO_TYPEDEF(creat64, int, (const char *path, mode_t mode),
                     (path, mode), brahma::POSIX);

GOTCHA_MACRO_TYPEDEF(open64, int, (const char *path, int flags, mode_t mode),
                     (path, flags, mode), brahma::POSIX);

GOTCHA_MACRO_TYPEDEF(close, int, (int fd), (fd), brahma::POSIX);

GOTCHA_MACRO_TYPEDEF(write, ssize_t, (int fd, const void *buf, size_t count),
                     (fd, buf, count), brahma::POSIX);

GOTCHA_MACRO_TYPEDEF(read, ssize_t, (int fd, void *buf, size_t count),
                     (fd, buf, count), brahma::POSIX);

GOTCHA_MACRO_TYPEDEF(lseek, off_t, (int fd, off_t offset, int whence),
                     (fd, offset, whence), brahma::POSIX);

GOTCHA_MACRO_TYPEDEF(lseek64, off64_t, (int fd, off64_t offset, int whence),
                     (fd, offset, whence), brahma::POSIX);

GOTCHA_MACRO_TYPEDEF(pread, ssize_t,
                     (int fd, void *buf, size_t count, off_t offset),
                     (fd, buf, count, offset), brahma::POSIX);

GOTCHA_MACRO_TYPEDEF(pread64, ssize_t,
                     (int fd, void *buf, size_t count, off64_t offset),
                     (fd, buf, count, offset), brahma::POSIX);

GOTCHA_MACRO_TYPEDEF(pwrite, ssize_t,
                     (int fd, const void *buf, size_t count, off_t offset),
                     (fd, buf, count, offset), brahma::POSIX);

GOTCHA_MACRO_TYPEDEF(pwrite64, ssize_t,
                     (int fd, const void *buf, size_t count, off64_t offset),
                     (fd, buf, count, offset), brahma::POSIX);

GOTCHA_MACRO_TYPEDEF(fsync, int, (int fd), (fd), brahma::POSIX);

GOTCHA_MACRO_TYPEDEF(fdatasync, int, (int fd), (fd), brahma::POSIX);

#endif  // BRAHMA_POSIX_H
