//
// Created by hariharan on 8/17/22.
//

#include "fsview_manager.h"
TailorFSStatus tailorfs::FSViewManager::initialize() {
  mimir_intent_conf = MIMIR_CONFIG();
  storages = mimir_intent_conf->_job_config._devices;
  mimir::FileIndex file_index = 0;
  for (const auto& file_intent : mimir_intent_conf->_file_repo) {
    FileHash file_hash = std::hash<std::string>()(file_intent._name);
    auto interfaces_used = std::unordered_set<mimir::InterfaceType>();
    for (const auto& app_intent : mimir_intent_conf->_app_repo) {
      std::copy(app_intent._interfaces_used.begin(),
                app_intent._interfaces_used.end(),
                std::inserter(interfaces_used, interfaces_used.end()));
    }
    auto iter = interfaces_used.find(mimir::InterfaceType::MPIIO);
    if (iter != interfaces_used.end()) {
      TAILORFS_LOGINFO("file %s uses MPIIO", file_intent._name.c_str());
      off_t mean_write_transfer_size =
          (file_intent._write_distribution._0_4kb * 2 * KB) +
          (file_intent._write_distribution._4_64kb * (4 + 64) / 2 * KB) +
          (file_intent._write_distribution._64kb_1mb * (64 + 1024) / 2 * KB) +
          (file_intent._write_distribution._1mb_16mb * (1 + 16) / 2 * MB) +
          (file_intent._write_distribution._16mb * 16 * MB);
      off_t mean_read_transfer_size =
          (file_intent._read_distribution._0_4kb * 2 * KB) +
          (file_intent._read_distribution._4_64kb * (4 + 64) / 2 * KB) +
          (file_intent._read_distribution._64kb_1mb * (64 + 1024) / 2 * KB) +
          (file_intent._read_distribution._1mb_16mb * (1 + 16) / 2 * MB) +
          (file_intent._read_distribution._16mb * 16 * MB);
      off_t mean_transfer_size = 0;
      int count = 0;
      if (mean_write_transfer_size > 0) {
        mean_transfer_size += mean_write_transfer_size;
        count++;
      }
      if (mean_read_transfer_size > 0) {
        mean_transfer_size += mean_read_transfer_size;
        count++;
      }
      if (count > 1) mean_transfer_size = mean_transfer_size / count;

      UnifyFSInit init_args;
      init_args.feature.transfer_size = mean_transfer_size;
      init_args.feature.enable_read_optimization = mean_read_transfer_size > 0;
      auto left_shm_size =
          storages[0]._capacity_mb - storages[0]._used_capacity_mb;
      if (left_shm_size > 0) {
        left_shm_size = left_shm_size > file_intent._size_mb
                            ? file_intent._size_mb
                            : left_shm_size;
      } else {
        left_shm_size = 0;
      }
      init_args.feature.shm._capacity_mb = left_shm_size;
      auto spill_storage = storages[1];
      auto left_spill_size =
          spill_storage._capacity_mb - spill_storage._used_capacity_mb;
      if (left_spill_size > 0) {
        left_spill_size = left_spill_size > file_intent._size_mb
                              ? file_intent._size_mb
                              : left_spill_size;
        init_args.feature.splill._capacity_mb = left_spill_size;
        init_args.feature.splill._mount_point = spill_storage._mount_point;
      } else {
        if (storages.size() > 2) {
          spill_storage = storages[2];
          auto left_spill_size =
              spill_storage._capacity_mb - spill_storage._used_capacity_mb;
          if (left_spill_size > 0) {
            left_spill_size = left_spill_size > file_intent._size_mb
                                  ? file_intent._size_mb
                                  : left_spill_size;
          }
          init_args.feature.splill._capacity_mb = left_spill_size;
          init_args.feature.splill._mount_point = spill_storage._mount_point;
        }
      }
      auto iter = fsid_map.find(FSViewType::UNIFYFS);
      FSID id;
      if (iter == fsid_map.end()) {
        id._type = FSViewType::UNIFYFS;
        id._id = 0;
      } else {
        id = iter->second;
        id._id++;
      }
      strcpy(init_args.feature.unifyfs_namespace,
             std::to_string(id._id).c_str());
      id._feature_hash = std::hash<UnifyFSFeature>()(init_args.feature);
      UNIFYFSVIEW(id)->Initialize(init_args);
      fsid_map.insert_or_assign(FSViewType::UNIFYFS, id);
      fsview_map.insert_or_assign(file_hash, id);
    } else {
      size_t file_sharing_count = 0;
      for (const auto& app_intent : mimir_intent_conf->_app_repo) {
        auto iter = app_intent._rank_file_dag.files.find(file_index);
        if (iter != app_intent._rank_file_dag.files.end()) {
          for (const auto& edge : app_intent._rank_file_dag.edges) {
            if (edge.source == file_index) {
              file_sharing_count++;
              if (file_sharing_count > 1) break;
            }
          }
          if (file_sharing_count > 1) break;
        }
      }
      if (file_sharing_count == 1) {
        TAILORFS_LOGINFO("file %s is FPP", file_intent._name.c_str());
        auto iter = fsid_map.find(FSViewType::MPIIO);
        FSID id;
        if (iter == fsid_map.end()) {
          id._type = FSViewType::MPIIO;
          id._id = 0;
        } else {
          id = iter->second;
          id._id++;
        }
        auto fastest_storage_index = get_fastest(file_intent._size_mb);
        MPIIOInit init_args;
        if (file_intent._current_device != fastest_storage_index) {
          init_args.redirection.is_enabled = true;
          init_args.redirection.original_storage =
              storages[file_intent._current_device];
          init_args.redirection.new_storage = storages[fastest_storage_index];
        }
        id._feature_hash = std::hash<RedirectFeature>()(init_args.redirection);
        MPIIOFSVIEW(id)->Initialize(init_args);
        fsid_map.insert_or_assign(FSViewType::MPIIO, id);
        fsview_map.insert_or_assign(file_hash, id);
        auto size = fsview_map.size();
      } else if (file_sharing_count == 2) {
        TAILORFS_LOGINFO("file %s is shared", file_intent._name.c_str());
        auto iter = fsid_map.find(FSViewType::MPIIO);
        FSID id;
        if (iter == fsid_map.end()) {
          id._type = FSViewType::MPIIO;
          id._id = 0;
        } else {
          id = iter->second;
          id._id++;
        }
        MPIIOInit init_args;
        init_args.redirection.is_enabled = false;
        id._feature_hash = std::hash<RedirectFeature>()(init_args.redirection);
        MPIIOFSVIEW(id)->Initialize(init_args);
        fsid_map.insert_or_assign(FSViewType::MPIIO, id);
        fsview_map.insert_or_assign(file_hash, id);
      }
    }
    file_index++;
  }
  return TAILORFS_SUCCESS;
}
TailorFSStatus tailorfs::FSViewManager::get_fsview(const std::string& filename,
                                                   tailorfs::FSID& id) {
  FileHash file_hash = std::hash<std::string>()(filename);
  auto iter = fsview_map.find(file_hash);
  if (iter != fsview_map.end())
    id = iter->second;
  else
    return TAILORFS_NOT_FOUND;
  return TAILORFS_SUCCESS;
}
TailorFSStatus tailorfs::FSViewManager::finalize() {
  for (auto& item : fsview_map) {
    if (item.second._type == tailorfs::FSViewType::MPIIO) {
      MPIIOFinalize arg;
      MPIIOFSVIEW(item.second)->Finalize(arg);
    } else if (item.second._type == tailorfs::FSViewType::UNIFYFS) {
      UnifyFSFinalize arg;
      UNIFYFSVIEW(item.second)->Finalize(arg);
    }
  }
  fsview_map.clear();
  fsid_map.clear();
  FSVIEW_FACTORY->finalize();
  return TAILORFS_SUCCESS;
}
