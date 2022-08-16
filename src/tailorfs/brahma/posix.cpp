//
// Created by hariharan on 8/16/22.
//
#include <cpp-logger/logger.h>
#include <tailorfs/brahma/posix.h>

int brahma::POSIXTailorFS::open(const char *pathname, int flags, mode_t mode) {
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
int brahma::POSIXTailorFS::close(int fd) {
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
ssize_t brahma::POSIXTailorFS::write(int fd, const void *buf, size_t count) {
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
ssize_t brahma::POSIXTailorFS::read(int fd, void *buf, size_t count) {
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
off_t brahma::POSIXTailorFS::lseek(int fd, off_t offset, int whence) {
  BRAHMA_MAP_OR_FAIL(lseek);
  ssize_t ret = -1;
  if (is_traced(fd)) {
    ret = __real_lseek(fd, offset, whence);
    TAILORFS_LOGPRINT("lseek(%d, %ld, %d) = %d", fd, offset, whence, ret);
  } else {
    ret = __real_lseek(fd, offset, whence);
  }
  return ret;
}