//
// Created by hariharan on 8/16/22.
//
#include <cpp-logger/logger.h>
#include <tailorfs/brahma/posix.h>

#include "tailorfs/core/fsview_manager.h"

std::shared_ptr<brahma::POSIXTailorFS> brahma::POSIXTailorFS::instance = nullptr;
int brahma::POSIXTailorFS::open(const char *pathname, int flags, mode_t mode) {
  BRAHMA_MAP_OR_FAIL(open);
  int ret = -1;
  if (is_traced(pathname)) {
    ret = __real_open(pathname, flags, mode);
    exclude_file(pathname);
    FSID id;
    auto status = FSVIEW_MANAGER->get_fsview(pathname, id);
    if (status == TAILORFS_SUCCESS) {
      fsid_map.insert_or_assign(ret, id);
      if (id._type == FSViewType::MPIIO) {
        TAILORFS_LOGINFO("Opening file using MPIIO", "");
        MPIIOOpen args{};
        args.filename = pathname;
        auto mpiio_fsview = MPIIOFSVIEW(id);
        mpiio_fsview->Convert(flags, args.flags);
        args.communicator = MPI_COMM_SELF;
        status = mpiio_fsview->Open(args);
        if (status == TAILORFS_SUCCESS) {
          mpiio_map.insert_or_assign(
              id, std::pair<MPI_File, off_t>(args.file_ptr, 0));
          track_fd(ret);
          TAILORFS_LOGINFO("Opening file using MPIIO - success", "");
        } else {
          TAILORFS_LOGERROR("Opening file using MPIIO - failed", "");
        }
      } else if (id._type == FSViewType::STDIO) {
        TAILORFS_LOGINFO("Opening file using STDIO", "");
        STDIOOpen args{};
        args.filename = pathname;
        auto fsview = STDIOFSVIEW(id);
        fsview->Convert(flags, args.mode);
        status = fsview->Open(args);
        if (status == TAILORFS_SUCCESS) {
          stdio_map.insert_or_assign(
              id, std::pair<FILE*, off_t>(args.fh, 0));
          track_fd(ret);
          TAILORFS_LOGINFO("Opening file using STDIO - success", "");
        } else {
          TAILORFS_LOGERROR("Opening file using STDIO - failed", "");
        }
      } else if (id._type == FSViewType::POSIX) {
        TAILORFS_LOGINFO("Opening file using POSIX", "");
        POSIXOpen args{};
        args.filename = pathname;
        auto fsview = POSIXFSVIEW(id);
        args.mode = flags;
        status = fsview->Open(args);
        if (status == TAILORFS_SUCCESS) {
          posix_map.insert_or_assign(
              id, std::pair<int, off_t>(args.fd, 0));
          track_fd(ret);
          TAILORFS_LOGINFO("Opening file using POSIX - success", "");
        } else {
          TAILORFS_LOGERROR("Opening file using POSIX - failed", "");
        }
      } else if (id._type == tailorfs::FSViewType::UNIFYFS) {
        TAILORFS_LOGINFO("Opening file using UnifyFS", "");
        UnifyFSOpen args{};
        args.filename = pathname;
        if (flags & O_CREAT) {
          args.is_create = true;
        } else {
          args.is_create = false;
        }
        args.flags = false;
        if (flags & O_RDONLY) {
          args.flags = args.flags & O_RDONLY;
        } else if (flags & O_WRONLY) {
          args.flags = args.flags & O_WRONLY;
        } else if (flags & O_RDWR) {
          args.flags = args.flags & O_RDWR;
        }
        auto fsview = UNIFYFSVIEW(id);
        status = fsview->Open(args);
        if (status == TAILORFS_SUCCESS) {
          unifyfs_map.insert_or_assign(
              id, std::pair<unifyfs_gfid, off_t>(args.gfid, 0));
          track_fd(ret);
          TAILORFS_LOGINFO("Opening file using UnifyFS - success", "");
        } else {
          TAILORFS_LOGERROR("Opening file using UnifyFS - failed", "");
        }
      } else {
        TAILORFS_LOGERROR("Incorrect fsview type", "");
      }
    } else {
      TAILORFS_LOGERROR("get_fsview - failed", "");
      ret = __real_open(pathname, flags, mode);
    }
    include_file(pathname);

  } else {
    ret = __real_open(pathname, flags, mode);
  }
  return ret;
}
int brahma::POSIXTailorFS::close(int fd) {
  BRAHMA_MAP_OR_FAIL(close);
  int ret = -1;
  if (is_traced(fd)) {
    auto iter = fsid_map.find(fd);
    if (iter != fsid_map.end()) {
      if (iter->second._type == FSViewType::MPIIO) {
        auto mpiio_iter = mpiio_map.find(iter->second);
        if (mpiio_iter != mpiio_map.end()) {
          TAILORFS_LOGINFO("Closing file using MPIIO", "");
          MPIIOClose args{};
          args.file_ptr = mpiio_iter->second.first;
          auto mpiio_fsview = MPIIOFSVIEW(iter->second);
          TailorFSStatus status = mpiio_fsview->Close(args);
          if (status == TAILORFS_SUCCESS) {
            mpiio_map.erase(iter->second);
            TAILORFS_LOGINFO("Closing file using MPIIO - success", "");
          } else {
            TAILORFS_LOGERROR("Closing file using MPIIO - failed", "");
          }
        }
      } else if (iter->second._type == FSViewType::STDIO) {
        auto stdio_iter = stdio_map.find(iter->second);
        if (stdio_iter != stdio_map.end()) {
          TAILORFS_LOGINFO("Closing file using STDIO", "");
          STDIOClose args{};
          args.fh = stdio_iter->second.first;
          auto fsview = STDIOFSVIEW(iter->second);
          TailorFSStatus status = fsview->Close(args);
          if (status == TAILORFS_SUCCESS) {
            stdio_map.erase(iter->second);
            TAILORFS_LOGINFO("Closing file using STDIO - success", "");
          } else {
            TAILORFS_LOGERROR("Closing file using STDIO - failed", "");
          }
        }
      } else if (iter->second._type == FSViewType::POSIX) {
        auto posix_iter = posix_map.find(iter->second);
        if (posix_iter != posix_map.end()) {
          TAILORFS_LOGINFO("Closing file using POSIX", "");
          POSIXClose args{};
          args.fd = posix_iter->second.first;
          auto fsview = POSIXFSVIEW(iter->second);
          TailorFSStatus status = fsview->Close(args);
          if (status == TAILORFS_SUCCESS) {
            stdio_map.erase(iter->second);
            TAILORFS_LOGINFO("Closing file using POSIX - success", "");
          } else {
            TAILORFS_LOGERROR("Closing file using POSIX - failed", "");
          }
        }
      } else if (iter->second._type == tailorfs::FSViewType::UNIFYFS) {
        auto unifyfs_iter = unifyfs_map.find(iter->second);
        if (unifyfs_iter != unifyfs_map.end()) {
          TAILORFS_LOGINFO("Closing file using UnifyFS", "");
          UnifyFSClose args{};
          args.gfid = unifyfs_iter->second.first;
          auto fsview = UNIFYFSVIEW(iter->second);
          TailorFSStatus status = fsview->Close(args);
          if (status == TAILORFS_SUCCESS) {
            unifyfs_map.erase(iter->second);
            TAILORFS_LOGINFO("Closing file using UnifyFS - success", "");
          } else {
            TAILORFS_LOGERROR("Closing file using UnifyFS - failed", "");
          }
        }
      } else {
        TAILORFS_LOGERROR("Incorrect fsview type", "");
      }
    }
    ret = __real_close(fd);
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
    bool is_success = false;
    auto iter = fsid_map.find(fd);
    if (iter != fsid_map.end()) {
      if (iter->second._type == FSViewType::MPIIO) {
        auto mpiio_iter = mpiio_map.find(iter->second);
        if (mpiio_iter != mpiio_map.end()) {
          TAILORFS_LOGINFO("Writing file using MPIIO", "");
          MPIIOWrite args{};
          args.file_ptr = mpiio_iter->second.first;
          args.offset = mpiio_iter->second.second;
          args.buf = (void *)buf;
          args.count = count;
          args.type = MPI_CHAR;
          auto mpiio_fsview = MPIIOFSVIEW(iter->second);
          TailorFSStatus status = mpiio_fsview->Write(args);
          if (status == TAILORFS_SUCCESS) {
            mpiio_iter->second.second += args.written_bytes;
            mpiio_map.insert_or_assign(iter->second, mpiio_iter->second);
            is_success = true;
            ret = args.written_bytes;
            TAILORFS_LOGINFO("Writing file using MPIIO - success", "");
          } else {
            TAILORFS_LOGERROR("Writing file using MPIIO - failed", "");
          }
        }
      } else if (iter->second._type == FSViewType::STDIO) {
        auto stdio_iter = stdio_map.find(iter->second);
        if (stdio_iter != stdio_map.end()) {
          TAILORFS_LOGINFO("Writing file using STDIO", "");
          STDIOWrite args{};
          args.fh = stdio_iter->second.first;
          args.offset = stdio_iter->second.second;
          args.buf = (void *)buf;
          args.size = count;
          auto fsview = STDIOFSVIEW(iter->second);
          TailorFSStatus status = fsview->Write(args);
          if (status == TAILORFS_SUCCESS) {
            stdio_iter->second.second += args.written_bytes;
            stdio_map.insert_or_assign(iter->second, stdio_iter->second);
            is_success = true;
            ret = args.written_bytes;
            TAILORFS_LOGINFO("Writing file using STDIO - success", "");
          } else {
            TAILORFS_LOGERROR("Writing file using STDIO - failed", "");
          }
        }
      } else if (iter->second._type == FSViewType::POSIX) {
        auto posix_iter = posix_map.find(iter->second);
        if (posix_iter != posix_map.end()) {
          TAILORFS_LOGINFO("Writing file using POSIX", "");
          POSIXWrite args{};
          args.fd = posix_iter->second.first;
          args.offset = posix_iter->second.second;
          args.buf = (void *)buf;
          args.size = count;
          auto fsview = POSIXFSVIEW(iter->second);
          TailorFSStatus status = fsview->Write(args);
          if (status == TAILORFS_SUCCESS) {
            posix_iter->second.second += args.written_bytes;
            posix_map.insert_or_assign(iter->second, posix_iter->second);
            is_success = true;
            ret = args.written_bytes;
            TAILORFS_LOGINFO("Writing file using POSIX - success", "");
          } else {
            TAILORFS_LOGERROR("Writing file using POSIX - failed", "");
          }
        }
      } else if (iter->second._type == tailorfs::FSViewType::UNIFYFS) {
        auto unifyfs_iter = unifyfs_map.find(iter->second);
        if (unifyfs_iter != unifyfs_map.end()) {
          TAILORFS_LOGINFO("Writing file using UnifyFS", "");
          UnifyFSWrite args{};
          args.gfid = unifyfs_iter->second.first;
          args.buf = (void *)buf;
          args.offset = unifyfs_iter->second.second;
          args.nbytes = count;
          auto fsview = UNIFYFSVIEW(iter->second);
          TailorFSStatus status = fsview->Write(args);
          if (status == TAILORFS_SUCCESS) {
            unifyfs_iter->second.second += args.nbytes;
            unifyfs_map.insert_or_assign(iter->second, unifyfs_iter->second);
            is_success = true;
            ret = args.nbytes;
            TAILORFS_LOGINFO("Writing file using UnifyFS - success", "");
          } else {
            TAILORFS_LOGERROR("Writing file using UnifyFS - failed", "");
          }
        }
      } else {
        TAILORFS_LOGERROR("Incorrect fsview type", "");
      }

    }
    if (!is_success) {
      ret = __real_write(fd, buf, count);
    }
  } else {
    ret = __real_write(fd, buf, count);
  }
  return ret;
}
ssize_t brahma::POSIXTailorFS::read(int fd, void *buf, size_t count) {
  BRAHMA_MAP_OR_FAIL(read);
  ssize_t ret = -1;
  if (is_traced(fd)) {
    bool is_success = false;
    auto iter = fsid_map.find(fd);
    if (iter != fsid_map.end()) {
      if (iter->second._type == FSViewType::MPIIO) {
        auto mpiio_iter = mpiio_map.find(iter->second);
        if (mpiio_iter != mpiio_map.end()) {
          TAILORFS_LOGINFO("Reading file using MPIIO", "");
          MPIIORead args{};
          args.file_ptr = mpiio_iter->second.first;
          args.offset = mpiio_iter->second.second;
          args.buf = (void *)buf;
          args.count = count;
          args.type = MPI_CHAR;
          auto mpiio_fsview = MPIIOFSVIEW(iter->second);
          TailorFSStatus status = mpiio_fsview->Read(args);
          if (status == TAILORFS_SUCCESS) {
            mpiio_iter->second.second += args.read_bytes;
            mpiio_map.insert_or_assign(iter->second, mpiio_iter->second);
            is_success = true;
            ret = args.read_bytes;
            TAILORFS_LOGINFO("Reading file using MPIIO - success", "");
          } else {
            TAILORFS_LOGERROR("Reading file using MPIIO - failed", "");
          }
        }
      } else if (iter->second._type == FSViewType::POSIX) {
        auto posix_iter = posix_map.find(iter->second);
        if (posix_iter != posix_map.end()) {
          TAILORFS_LOGINFO("Reading file using POSIX", "");
          POSIXRead args{};
          args.fd = posix_iter->second.first;
          args.offset = posix_iter->second.second;
          args.buf = (void *)buf;
          args.size = count;
          auto fsview = POSIXFSVIEW(iter->second);
          TailorFSStatus status = fsview->Read(args);
          if (status == TAILORFS_SUCCESS) {
            posix_iter->second.second += args.read_bytes;
            posix_map.insert_or_assign(iter->second, posix_iter->second);
            is_success = true;
            ret = args.read_bytes;
            TAILORFS_LOGINFO("Reading file using POSIX - success", "");
          } else {
            TAILORFS_LOGERROR("Reading file using POSIX - failed", "");
          }
        }
      } else if (iter->second._type == FSViewType::STDIO) {
        auto stdio_iter = stdio_map.find(iter->second);
        if (stdio_iter != stdio_map.end()) {
          TAILORFS_LOGINFO("Reading file using STDIO", "");
          STDIORead args{};
          args.fh = stdio_iter->second.first;
          args.offset = stdio_iter->second.second;
          args.buf = (void *)buf;
          args.size = count;
          auto fsview = STDIOFSVIEW(iter->second);
          TailorFSStatus status = fsview->Read(args);
          if (status == TAILORFS_SUCCESS) {
            stdio_iter->second.second += args.read_bytes;
            stdio_map.insert_or_assign(iter->second, stdio_iter->second);
            is_success = true;
            ret = args.read_bytes;
            TAILORFS_LOGINFO("Reading file using STDIO - success", "");
          } else {
            TAILORFS_LOGERROR("Reading file using STDIO - failed", "");
          }
        }
      } else if (iter->second._type == tailorfs::FSViewType::UNIFYFS) {
        auto unifyfs_iter = unifyfs_map.find(iter->second);
        if (unifyfs_iter != unifyfs_map.end()) {
          TAILORFS_LOGINFO("Reading file using UnifyFS", "");
          UnifyFSRead args{};
          args.gfid = unifyfs_iter->second.first;
          args.buf = (void *)buf;
          args.offset = unifyfs_iter->second.second;
          args.nbytes = count;
          auto fsview = UNIFYFSVIEW(iter->second);
          TailorFSStatus status = fsview->Read(args);
          if (status == TAILORFS_SUCCESS) {
            unifyfs_iter->second.second += args.nbytes;
            unifyfs_map.insert_or_assign(iter->second, unifyfs_iter->second);
            is_success = true;
            ret = args.nbytes;
            TAILORFS_LOGINFO("Reading file using UnifyFS - success", "");
          } else {
            TAILORFS_LOGERROR("Reading file using UnifyFS - failed", "");
          }
        }
      } else {
        TAILORFS_LOGERROR("Incorrect fsview type", "");
      }

    }
    if (!is_success) {
      ret = __real_read(fd, buf, count);
    }
  } else {
    ret = __real_read(fd, buf, count);
  }
  return ret;
}
off_t brahma::POSIXTailorFS::lseek(int fd, off_t offset, int whence) {
  BRAHMA_MAP_OR_FAIL(lseek);
  ssize_t ret = -1;
  if (is_traced(fd)) {
    bool is_success = false;
    auto iter = fsid_map.find(fd);
    if (iter != fsid_map.end()) {
      if (iter->second._type == FSViewType::MPIIO) {
        auto mpiio_iter = mpiio_map.find(iter->second);
        if (mpiio_iter != mpiio_map.end()) {
          TAILORFS_LOGINFO("lseek file using MPIIO", "");
          switch (whence) {
            case SEEK_SET: {
              mpiio_iter->second.second = offset;
              ret = mpiio_iter->second.second;
              break;
            }
            case SEEK_CUR: {
              mpiio_iter->second.second += offset;
              ret = mpiio_iter->second.second;
              break;
            }
            default: {
              TAILORFS_LOGERROR("Unsupported Seek End", "");
              break;
            }
          }
          mpiio_map.insert_or_assign(iter->second, mpiio_iter->second);
          is_success = true;
        } else {
          TAILORFS_LOGERROR("lseek file using MPIIO - failed", "");
        }
      }  else if (iter->second._type == FSViewType::STDIO) {
        auto stdio_iter = stdio_map.find(iter->second);
        if (stdio_iter != stdio_map.end()) {
          TAILORFS_LOGINFO("lseek file using STDIO", "");
          switch (whence) {
            case SEEK_SET: {
              stdio_iter->second.second = offset;
              ret = stdio_iter->second.second;
              break;
            }
            case SEEK_CUR: {
              stdio_iter->second.second += offset;
              ret = stdio_iter->second.second;
              break;
            }
            default: {
              TAILORFS_LOGERROR("Unsupported Seek End", "");
              break;
            }
          }
          stdio_map.insert_or_assign(iter->second, stdio_iter->second);
          is_success = true;
        } else {
          TAILORFS_LOGERROR("lseek file using STDIO - failed", "");
        }
      }  else if (iter->second._type == FSViewType::POSIX) {
        auto posix_iter = posix_map.find(iter->second);
        if (posix_iter != posix_map.end()) {
          TAILORFS_LOGINFO("lseek file using POSIX", "");
          switch (whence) {
            case SEEK_SET: {
              posix_iter->second.second = offset;
              ret = posix_iter->second.second;
              break;
            }
            case SEEK_CUR: {
              posix_iter->second.second += offset;
              ret = posix_iter->second.second;
              break;
            }
            default: {
              TAILORFS_LOGERROR("Unsupported Seek End", "");
              break;
            }
          }
          posix_map.insert_or_assign(iter->second, posix_iter->second);
          is_success = true;
        } else {
          TAILORFS_LOGERROR("lseek file using STDIO - failed", "");
        }
      } else if (iter->second._type == FSViewType::UNIFYFS) {
        auto unifyfs_iter = unifyfs_map.find(iter->second);
        if (unifyfs_iter != unifyfs_map.end()) {
          TAILORFS_LOGINFO("lssek file using UnifyFS", "");
          switch (whence) {
            case SEEK_SET: {
              unifyfs_iter->second.second = offset;
              ret = unifyfs_iter->second.second;
              break;
            }
            case SEEK_CUR: {
              unifyfs_iter->second.second += offset;
              ret = unifyfs_iter->second.second;
              break;
            }
            default: {
              TAILORFS_LOGERROR("Unsupported Seek End", "");
              break;
            }
          }
          unifyfs_map.insert_or_assign(iter->second, unifyfs_iter->second);
          is_success = true;
        } else {
          TAILORFS_LOGERROR("lseek file using UnifyFS - failed", "");
        }
      } else {
        TAILORFS_LOGERROR("Incorrect fsview type", "");
      }
    }
    if (!is_success) {
      ret = __real_lseek(fd, offset, whence);
      TAILORFS_LOGPRINT("lseek(%d, %ld, %d) = %d", fd, offset, whence, ret);
    }
  } else {
    ret = __real_lseek(fd, offset, whence);
  }
  return ret;
}