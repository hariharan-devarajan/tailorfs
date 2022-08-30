//
// Created by haridev on 8/29/22.
//

#include "mpiio.h"

#include <fcntl.h>

std::shared_ptr<brahma::MPIIOTailorFS> brahma::MPIIOTailorFS::instance = nullptr;
int brahma::MPIIOTailorFS::MPI_File_open(MPI_Comm comm, const char *filename,
                                          int amode, MPI_Info info,
                                          MPI_File *fh) {
  BRAHMA_MAP_OR_FAIL(MPI_File_open);
  int ret = -1;
  if (is_traced(filename)) {
    ret = __real_MPI_File_open(comm, filename, amode, info, fh);
    exclude_file(filename);
    FSID id;
    auto status = FSVIEW_MANAGER->get_fsview(filename, id);
    if (status == TAILORFS_SUCCESS) {
      fsid_map.insert_or_assign(*fh, id);
      if (id._type == FSViewType::MPIIO) {
        TAILORFS_LOGINFO("Opening file using MPIIO", "");
        MPIIOOpen args{};
        args.filename = filename;
        auto mpiio_fsview = MPIIOFSVIEW(id);
        args.flags = amode;
        args.communicator = comm;
        status = mpiio_fsview->Open(args);
        if (status == TAILORFS_SUCCESS) {
          mpiio_map.insert_or_assign(
              id, std::pair<MPI_File, off_t>(args.file_ptr, 0));
          track_fd(*fh);
          TAILORFS_LOGINFO("Opening file using MPIIO - success", "");
        } else {
          TAILORFS_LOGERROR("Opening file using MPIIO - failed", "");
        }
      } else if (id._type == FSViewType::STDIO) {
        TAILORFS_LOGINFO("Opening file using STDIO", "");
        if (comm == MPI_COMM_SELF) {
          STDIOOpen args{};
          args.filename = filename;
          auto fsview = STDIOFSVIEW(id);
          fsview->Convert(amode, args.mode);
          status = fsview->Open(args);
          if (status == TAILORFS_SUCCESS) {
            stdio_map.insert_or_assign(id,
                                       std::pair<FILE *, off_t>(args.fh, 0));
            track_fd(*fh);
            TAILORFS_LOGINFO("Opening file using STDIO - success", "");
          } else {
            TAILORFS_LOGERROR("Opening file using STDIO - failed", "");
          }
        }
      }  else if (id._type == FSViewType::POSIX) {
        TAILORFS_LOGINFO("Opening file using POSIX", "");
        if (comm == MPI_COMM_SELF) {
          POSIXOpen args{};
          args.filename = filename;
          auto fsview = POSIXFSVIEW(id);
          int flags = 0;
          if (amode & MPI_MODE_CREATE) flags |= O_CREAT;
          if (amode & MPI_MODE_RDONLY) flags |= O_RDONLY;
          if (amode & MPI_MODE_APPEND) flags |= O_APPEND;
          if (amode & MPI_MODE_EXCL) flags |= O_EXCL;
          if (amode & MPI_MODE_RDWR) flags |= O_RDWR;
          if (amode & MPI_MODE_WRONLY) flags |= O_WRONLY;
          args.mode = flags;
          status = fsview->Open(args);
          if (status == TAILORFS_SUCCESS) {
            posix_map.insert_or_assign(id,
                                       std::pair<int, off_t>(args.fd, 0));
            track_fd(*fh);
            TAILORFS_LOGINFO("Opening file using POSIX - success", "");
          } else {
            TAILORFS_LOGERROR("Opening file using POSIX - failed", "");
          }
        }
      } else if (id._type == tailorfs::FSViewType::UNIFYFS) {
        TAILORFS_LOGINFO("Opening file using UnifyFS", "");
        UnifyFSOpen args{};
        args.filename = filename;
        if (amode & MPI_MODE_CREATE) {
          args.is_create = true;
        } else {
          args.is_create = false;
        }
        args.flags = false;
        if (amode & MPI_MODE_RDONLY) {
          args.flags = args.flags & O_RDONLY;
        } else if (amode & O_WRONLY) {
          args.flags = args.flags & O_WRONLY;
        } else if (amode & O_RDWR) {
          args.flags = args.flags & O_RDWR;
        }
        auto fsview = UNIFYFSVIEW(id);
        status = fsview->Open(args);
        if (status == TAILORFS_SUCCESS) {
          unifyfs_map.insert_or_assign(
              id, std::pair<unifyfs_gfid, off_t>(args.gfid, 0));
          track_fd(*fh);
          TAILORFS_LOGINFO("Opening file using UnifyFS - success", "");
        } else {
          TAILORFS_LOGERROR("Opening file using UnifyFS - failed", "");
        }
      } else {
        TAILORFS_LOGERROR("Incorrect fsview type", "");
      }
    } else {
      TAILORFS_LOGERROR("get_fsview - failed", "");
      ret = __real_MPI_File_open(comm, filename, amode, info, fh);
    }
    include_file(filename);

  } else {
    ret = __real_MPI_File_open(comm, filename, amode, info, fh);
  }
  return ret;
}
int brahma::MPIIOTailorFS::MPI_File_close(MPI_File *fh) {
  BRAHMA_MAP_OR_FAIL(MPI_File_close);
  int ret = -1;
  if (is_traced(*fh)) {
    auto iter = fsid_map.find(*fh);
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
    ret = __real_MPI_File_close(fh);
    untrack_fd(*fh);
  } else {
    ret = __real_MPI_File_close(fh);
  }
  return ret;
}
int brahma::MPIIOTailorFS::MPI_File_read_at_all(MPI_File fh, MPI_Offset offset,
                                                 void *buf, int count,
                                                 MPI_Datatype datatype,
                                                 MPI_Status *status) {
  BRAHMA_MAP_OR_FAIL(MPI_File_read_at_all);
  ssize_t ret = -1;
  if (is_traced(fh)) {
    bool is_success = false;
    auto iter = fsid_map.find(fh);
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
            ret = 0;
            if (status != NULL){
              //FIXME(hari): status->_ucount = args.nbytes;
            }
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
      ret = __real_MPI_File_read_at_all(fh, offset, buf, count, datatype, status);
    }
  } else {
    ret = __real_MPI_File_read_at_all(fh, offset, buf, count, datatype, status);
  }
  return ret;
}
int brahma::MPIIOTailorFS::MPI_File_write_at_all(MPI_File fh,
                                                  MPI_Offset offset,
                                                  const void *buf, int count,
                                                  MPI_Datatype datatype,
                                                  MPI_Status *status) {
  BRAHMA_MAP_OR_FAIL(MPI_File_write_at_all);
  ssize_t ret = -1;
  if (is_traced(fh)) {
    bool is_success = false;
    auto iter = fsid_map.find(fh);
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
      }
      else if (iter->second._type == FSViewType::STDIO) {
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
      }
      else if (iter->second._type == FSViewType::POSIX) {
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
      }
      else if (iter->second._type == tailorfs::FSViewType::UNIFYFS) {
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
            ret = 0;
            if (status != NULL){
              //FIXME(hari): status->_ucount = args.nbytes;
            }
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
      ret = __real_MPI_File_write_at_all(fh, offset, buf, count, datatype, status);
    }
  } else {
    ret = __real_MPI_File_write_at_all(fh, offset, buf, count, datatype, status);
  }
  return ret;
}
