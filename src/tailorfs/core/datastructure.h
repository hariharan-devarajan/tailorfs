//
// Created by hariharan on 8/17/22.
//

#ifndef TAILORFS_DATASTRUCTURE_H
#define TAILORFS_DATASTRUCTURE_H
#include <mimir/common/data_structure.h>

#include "enumeration.h"
namespace tailorfs {

struct FSID {
  FSViewType _type;
  uint16_t _feature_hash;
  uint8_t _id;
  FSID() : _type(FSViewType::NONE), _feature_hash(0), _id(0) {}
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
  bool redirection;
  mimir::Storage storage;
};

struct MPIIOFeature {
  RedirectFeature redirection;
};

struct UnifyFSFeature {
  off_t transfer_size;
  mimir::Storage shm;
  mimir::Storage splill;
  bool enable_read_optimization;
};

struct FSViewInit {};
struct FSViewFinalize {};
struct FSViewOpen {};
struct FSViewClose {};
struct FSViewRead {};
struct FSViewWrite {};

struct MPIIOInit : public FSViewInit {
  MPIIOFeature feature;
};
struct MPIIOFinalize : public FSViewFinalize {};
struct MPIIOOpen : public FSViewOpen {};
struct MPIIOClose : public FSViewClose {};
struct MPIIORead : public FSViewRead {};
struct MPIIOWrite : public FSViewWrite {};

struct UnifyFSInit : public FSViewInit {
  UnifyFSFeature feature;
};
struct UnifyFSFinalize : public FSViewFinalize {};
struct UnifyFSOpen : public FSViewOpen {};
struct UnifyFSClose : public FSViewClose {};
struct UnifyFSRead : public FSViewRead {};
struct UnifyFSWrite : public FSViewWrite {};
}  // namespace tailorfs

namespace std {
template <>
struct hash<tailorfs::RedirectFeature> {
  size_t operator()(const tailorfs::RedirectFeature& k) const {
    size_t hash_val = hash<bool>()(k.redirection);
    hash_val ^= hash<mimir::Storage>()(k.storage);
    return hash_val;
  }
};
template <>
struct hash<tailorfs::MPIIOFeature> {
  size_t operator()(const tailorfs::MPIIOFeature& k) const {
    size_t hash_val = hash<tailorfs::RedirectFeature>()(k.redirection);
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
