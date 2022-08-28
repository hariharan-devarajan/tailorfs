//
// Created by hariharan on 8/17/22.
//

#ifndef TAILORFS_DATASTRUCTURE_H
#define TAILORFS_DATASTRUCTURE_H
#include <mimir/common/data_structure.h>
#include <mpi.h>
#include <unifyfs/unifyfs_api.h>

#include "enumeration.h"
namespace tailorfs {

struct FSID {
  FSViewType _type;
  uint16_t _feature_hash;
  uint8_t _id;
  FSID() : _type(FSViewType::FS_NONE), _feature_hash(0), _id(0) {}
  FSID(const FSID& other)
      : _type(other._type),
        _feature_hash(other._feature_hash),
        _id(other._id) {}
  FSID(FSID&& other)
      : _type(other._type),
        _feature_hash(other._feature_hash),
        _id(other._id) {}
  FSID& operator=(const FSID& other) {
    _type = other._type;
    _feature_hash = other._feature_hash;
    _id = other._id;
    return *this;
  }
  bool operator==(const FSID& other) const {
    return _type == other._type && _feature_hash == other._feature_hash &&
           _id == other._id;
  }
};

struct RedirectFeature {
  RedirectionType type;
  bool is_enabled;
  mimir::Storage original_storage;
  mimir::Storage new_storage;
};

struct UnifyFSFeature {
  off_t transfer_size;
  mimir::Storage shm;
  mimir::Storage splill;
  bool is_shared;
  bool enable_read_optimization;
  char unifyfs_namespace[32];
  RedirectFeature redirection;
};

struct FSViewInit {};
struct FSViewFinalize {};
struct FSViewOpen {};
struct FSViewClose {};
struct FSViewRead {};
struct FSViewWrite {};

struct MPIIOInit : public FSViewInit {
  RedirectFeature redirection;
};
struct MPIIOFinalize : public FSViewFinalize {};
struct MPIIOOpen : public FSViewOpen {
  const char* filename;
  int flags;
  MPI_Comm communicator;
  MPI_File file_ptr;
};
struct MPIIOClose : public FSViewClose {
  MPI_File file_ptr;
};
struct MPIIORead : public FSViewRead {
  MPI_File file_ptr;
  MPI_Offset offset;
  int count;
  void* buf;
  MPI_Datatype type;
  int read_bytes;
};
struct MPIIOWrite : public FSViewWrite {
  MPI_File file_ptr;
  MPI_Offset offset;
  int count;
  void* buf;
  MPI_Datatype type;
  int written_bytes;
};

struct STDIOInit : public FSViewInit {
  RedirectFeature redirection;
};
struct STDIOFinalize : public FSViewFinalize {};
struct STDIOOpen : public FSViewOpen {
  const char* filename;
  char* mode;
  FILE* fh;
};
struct STDIOClose : public FSViewClose {
  FILE* fh;
};
struct STDIORead : public FSViewRead {
  FILE* fh;
  off_t offset;
  int size;
  void* buf;
  int read_bytes;
};
struct STDIOWrite : public FSViewWrite {
  FILE* fh;
  off_t offset;
  int size;
  void* buf;
  int written_bytes;
};
struct POSIXInit : public FSViewInit {
  RedirectFeature redirection;
};
struct POSIXFinalize : public FSViewFinalize {};
struct POSIXOpen : public FSViewOpen {
  const char* filename;
  int flags;
  int mode;
  int fd;
};
struct POSIXClose : public FSViewClose {
  int fd;
};
struct POSIXRead : public FSViewRead {
  int fd;
  off_t offset;
  int size;
  void* buf;
  int read_bytes;
};
struct POSIXWrite : public FSViewWrite {
  int fd;
  off_t offset;
  int size;
  void* buf;
  int written_bytes;
};

struct UnifyFSInit : public FSViewInit {
  UnifyFSFeature feature;
};
struct UnifyFSFinalize : public FSViewFinalize {};
struct UnifyFSOpen : public FSViewOpen {
  bool is_create;
  bool flags;
  const char* filename;
  unifyfs_gfid gfid;
};
struct UnifyFSClose : public FSViewClose {
  unifyfs_gfid gfid;
};
struct UnifyFSRead : public FSViewRead {
  unifyfs_gfid gfid;
  size_t nbytes;
  off_t offset;
  void* buf;
};
struct UnifyFSWrite : public FSViewWrite {
  unifyfs_gfid gfid;
  size_t nbytes;
  off_t offset;
  void* buf;
};

struct UnifyFSIORequest {
  size_t nbytes;
  off_t offset;
  unifyfs_gfid gfid;
  unifyfs_ioreq_op op;
  unifyfs_req_state state;
  unifyfs_ioreq_result result;
  int _reqid;
  UnifyFSIORequest()
      : nbytes(), offset(), gfid(), op(), state(), result(), _reqid() {}
  UnifyFSIORequest(const unifyfs_io_request& other)
      : nbytes(other.nbytes),
        offset(other.offset),
        gfid(other.gfid),
        op(other.op),
        state(other.state),
        result(other.result),
        _reqid(other._reqid) {}
  UnifyFSIORequest(const UnifyFSIORequest& other)
      : nbytes(other.nbytes),
        offset(other.offset),
        gfid(other.gfid),
        op(other.op),
        state(other.state),
        result(other.result),
        _reqid(other._reqid) {}
  UnifyFSIORequest(UnifyFSIORequest&& other)
      : nbytes(other.nbytes),
        offset(other.offset),
        gfid(other.gfid),
        op(other.op),
        state(other.state),
        result(other.result),
        _reqid(other._reqid) {}
  UnifyFSIORequest& operator=(const UnifyFSIORequest& other) {
    nbytes = other.nbytes;
    offset = other.offset;
    gfid = other.gfid;
    op = other.op;
    state = other.state;
    result = other.result;
    _reqid = other._reqid;
    return *this;
  }
  UnifyFSIORequest& operator=(const unifyfs_io_request& other) {
    nbytes = other.nbytes;
    offset = other.offset;
    gfid = other.gfid;
    op = other.op;
    state = other.state;
    result = other.result;
    _reqid = other._reqid;
    return *this;
  }
};
}  // namespace tailorfs

namespace std {
template <>
struct hash<tailorfs::RedirectFeature> {
  size_t operator()(const tailorfs::RedirectFeature& k) const {
    size_t hash_val = hash<bool>()(k.is_enabled);
    hash_val ^= hash<uint8_t>()(k.type);
    hash_val ^= hash<mimir::Storage>()(k.original_storage);
    hash_val ^= hash<mimir::Storage>()(k.new_storage);
    return hash_val;
  }
};

template <>
struct hash<tailorfs::UnifyFSFeature> {
  size_t operator()(const tailorfs::UnifyFSFeature& k) const {
    size_t hash_val = hash<bool>()(k.enable_read_optimization);
    hash_val ^= hash<mimir::Storage>()(k.shm);
    hash_val ^= hash<mimir::Storage>()(k.splill);
    hash_val ^= hash<off_t>()(k.transfer_size);
    hash_val ^= hash<bool>()(k.is_shared);
    hash_val ^= hash<tailorfs::RedirectFeature>()(k.redirection);
    return hash_val;
  }
};

template <>
struct hash<tailorfs::FSID> {
  size_t operator()(const tailorfs::FSID& k) const {
    size_t hash_val = hash<uint8_t>()(k._type);
    hash_val ^= hash<uint8_t>()(k._id);
    hash_val ^= hash<uint16_t>()(k._feature_hash);
    return hash_val;
  }
};

}  // namespace std

#endif  // TAILORFS_DATASTRUCTURE_H
