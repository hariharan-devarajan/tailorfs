//
// Created by hariharan on 8/17/22.
//

#ifndef TAILORFS_FSVIEW_FACTORY_H
#define TAILORFS_FSVIEW_FACTORY_H
#include <tailorfs/core/enumeration.h>
#include <tailorfs/core/fsview/fsview.h>
#include <tailorfs/core/fsview/mpiio_fsview.h>
#include <tailorfs/core/fsview/unifyfs_fsview.h>
#include <tailorfs/core/singleton.h>

#include <memory>
#include <unordered_map>

#include "posix_fsview.h"
#include "stdio_fsview.h"
#include "tailorfs/error_code.h"
namespace tailorfs {
class FSViewFactory {
 private:
  std::unordered_map<FSID, std::shared_ptr<MPIIOFSView>> mpiio_map;
  std::unordered_map<FSID, std::shared_ptr<POSIXFSView>> posix_map;
  std::unordered_map<FSID, std::shared_ptr<STDIOFSView>> stdio_map;
  std::unordered_map<FSID, std::shared_ptr<UnifyFSFSView>> unifyfs_map;

 public:
  FSViewFactory() : mpiio_map(), unifyfs_map() {}
  TailorFSStatus finalize() {
    for (auto& item : mpiio_map) {
      item.second.reset();
    }
    mpiio_map.clear();
    for (auto& item : unifyfs_map) {
      item.second.reset();
    }
    unifyfs_map.clear();
    return TAILORFS_SUCCESS;
  }
  template <typename T, typename = typename std::enable_if<
                            std::is_same<T, STDIOFSView>::value>::type>
  std::shared_ptr<STDIOFSView> get_fsview(const FSID& id) {
    std::shared_ptr<STDIOFSView> instance;
    auto iter = stdio_map.find(id);
    if (iter == stdio_map.end()) {
      instance = std::make_shared<STDIOFSView>();
      stdio_map.emplace(id, instance);
    } else
      instance = iter->second;
    return instance;
  }
  template <typename T, typename = typename std::enable_if<
                            std::is_same<T, POSIXFSView>::value>::type>
  std::shared_ptr<POSIXFSView> get_fsview(const FSID& id) {
    std::shared_ptr<POSIXFSView> instance;
    auto iter = posix_map.find(id);
    if (iter == posix_map.end()) {
      instance = std::make_shared<POSIXFSView>();
      posix_map.emplace(id, instance);
    } else
      instance = iter->second;
    return instance;
  }
  template <typename T, typename = typename std::enable_if<
                            std::is_same<T, MPIIOFSView>::value>::type>
  std::shared_ptr<MPIIOFSView> get_fsview(const FSID& id) {
    std::shared_ptr<MPIIOFSView> instance;
    auto iter = mpiio_map.find(id);
    if (iter == mpiio_map.end()) {
      instance = std::make_shared<MPIIOFSView>();
      mpiio_map.emplace(id, instance);
    } else
      instance = iter->second;
    return instance;
  }
  template <typename T, typename = typename std::enable_if<
                            std::is_same<T, UnifyFSFSView>::value>::type>
  std::shared_ptr<UnifyFSFSView> get_fsview(const FSID& id) {
    std::shared_ptr<UnifyFSFSView> instance;
    auto iter = unifyfs_map.find(id);
    if (iter == unifyfs_map.end()) {
      instance = std::make_shared<UnifyFSFSView>();
      unifyfs_map.emplace(id, instance);
    } else
      instance = iter->second;
    return instance;
  }
};
}  // namespace tailorfs
#define FSVIEW_FACTORY \
  tailorfs::Singleton<tailorfs::FSViewFactory>::get_instance()

#define POSIXFSVIEW(id) FSVIEW_FACTORY->get_fsview<tailorfs::POSIXFSView>(id)
#define STDIOFSVIEW(id) FSVIEW_FACTORY->get_fsview<tailorfs::STDIOFSView>(id)
#define MPIIOFSVIEW(id) FSVIEW_FACTORY->get_fsview<tailorfs::MPIIOFSView>(id)
#define UNIFYFSVIEW(id) FSVIEW_FACTORY->get_fsview<tailorfs::UnifyFSFSView>(id)

#endif  // TAILORFS_FSVIEW_FACTORY_H
