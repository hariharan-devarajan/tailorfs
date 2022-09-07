//
// Created by hariharan on 8/17/22.
//

#ifndef TAILORFS_FSVIEW_MANAGER_H
#define TAILORFS_FSVIEW_MANAGER_H
#include <mimir/mimir.h>
#include <tailorfs/core/constant.h>
#include <tailorfs/core/fsview/fsview_factory.h>

#include <unordered_map>

#include "tailorfs/error_code.h"
#include "tailorfs/macro.h"

namespace tailorfs {
class FSViewManager {
 private:
  std::unordered_map<FSViewType, FSID> fsid_map;
  std::unordered_map<FileHash, FSID> fsview_map;
  mimir::Config* mimir_intent_conf;
  std::vector<mimir::Storage> storages;
  inline size_t get_fastest(size_t file_size_mb) {
    size_t fastest_index = 0;
    for (auto& storage : storages) {
      uint64_t capacity_per_process = storage._capacity_mb;
      if (!storage._is_shared) {
        capacity_per_process = storage._capacity_mb / mimir_intent_conf->_job_config._num_cores_per_node / mimir_intent_conf->_job_config._num_nodes;
      } else {
        capacity_per_process = storage._capacity_mb / mimir_intent_conf->_job_config._num_cores_per_node;
      }
      TAILORFS_LOGINFO("Capacity in %s is %ld file size %d used capacity", storage._mount_point.c_str(),capacity_per_process, file_size_mb, storage._used_capacity_mb);
      if (capacity_per_process - storage._used_capacity_mb >= file_size_mb) {
        storage._used_capacity_mb += file_size_mb;
        return fastest_index;
      }
      fastest_index++;
    }
    TAILORFS_LOGINFO("Returning index %d", fastest_index%storages.size());
    return fastest_index%storages.size();
  }

 public:
  FSViewManager()
      : fsid_map(), fsview_map(), storages(), mimir_intent_conf(nullptr) {
    TAILORFS_LOGINFO("Calling FSViewManager Constructor", "");
  }
  TailorFSStatus initialize();
  TailorFSStatus get_fsview(const std::string& filename, FSID& id);
  TailorFSStatus finalize();
};
}  // namespace tailorfs

#define FSVIEW_MANAGER \
  tailorfs::Singleton<tailorfs::FSViewManager>::get_instance()

#endif  // TAILORFS_FSVIEW_MANAGER_H
