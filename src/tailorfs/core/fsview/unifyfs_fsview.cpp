//
// Created by hariharan on 8/17/22.
//

#include "unifyfs_fsview.h"

#include <fcntl.h>

#include "tailorfs/error_code.h"
#include "tailorfs/macro.h"
TailorFSStatus tailorfs::UnifyFSFSView::initialize(UnifyFSInit& payload) {
  int options_ct = 8;
  char unifyfs_consistency[32] = "LAMINATED";
  char client_fsync_persist[32] = "off";
  unifyfs_cfg_option* options = static_cast<unifyfs_cfg_option*>(
      calloc(options_ct, sizeof(unifyfs_cfg_option)));
  char logio_chunk_size[32];
  strcpy(logio_chunk_size,
         std::to_string(payload.feature.transfer_size).c_str());
  char logio_shmem_size[32];
  strcpy(logio_shmem_size,
         std::to_string(payload.feature.shm._capacity_mb).c_str());
  char logio_spill_dir[256];
  strcpy(logio_spill_dir, payload.feature.splill._mount_point.c_str());
  char logio_spill_size[32];
  strcpy(logio_spill_size,
         std::to_string(payload.feature.splill._capacity_mb).c_str());
  char client_local_extents[32];
  char client_node_local_extents[32];
  if (payload.feature.enable_read_optimization) {
    strcpy(client_local_extents, "on");
    strcpy(client_node_local_extents, "on");
  } else {
    strcpy(client_local_extents, "on");
    strcpy(client_node_local_extents, "on");
  }

  options[0] = {.opt_name = "unifyfs.consistency",
                .opt_value = unifyfs_consistency};
  options[1] = {.opt_name = "client.fsync_persist",
                .opt_value = client_fsync_persist};
  options[2] = {.opt_name = "logio.chunk_size", .opt_value = logio_chunk_size};
  options[3] = {.opt_name = "logio.shmem_size", .opt_value = logio_shmem_size};
  options[4] = {.opt_name = "logio.spill_dir", .opt_value = logio_spill_dir};
  options[5] = {.opt_name = "logio.spill_size", .opt_value = logio_spill_size};
  options[6] = {.opt_name = "client.local_extents",
                .opt_value = client_local_extents};
  options[7] = {.opt_name = "client.node_local_extents",
                .opt_value = client_node_local_extents};
  unifyfs_namespace = payload.feature.unifyfs_namespace;
  int rc = unifyfs_initialize(payload.feature.unifyfs_namespace, options,
                              options_ct, &fshdl);
  if (rc != UNIFYFS_SUCCESS) {
    TAILORFS_LOGERROR("unifyfs_initialize failed. rc %d, message %s", rc,
                      strerror(rc));
  }
  return (rc == UNIFYFS_SUCCESS) ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::UnifyFSFSView::open(UnifyFSOpen& payload) {
  int rc = UNIFYFS_SUCCESS;
  fs::path unify_filename = unifyfs_namespace / fs::path(payload.filename);
  if (payload.is_create) {
    rc = unifyfs_create(fshdl, payload.flags, unify_filename.c_str(),
                        &payload.gfid);
  } else {
    rc = unifyfs_open(fshdl, payload.flags, unify_filename.c_str(),
                      &payload.gfid);
  }
  if (payload.flags & O_RDONLY) {
    rc = unifyfs_laminate(fshdl, unify_filename.c_str());
  }
  return (rc == UNIFYFS_SUCCESS) ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::UnifyFSFSView::close(UnifyFSClose& payload) {
  auto iter = pending_req.find(payload.gfid);
  int rc = UNIFYFS_SUCCESS;
  if (iter != pending_req.end()) {
    /* wait for outstanding reqs */
    unifyfs_io_request reqs[iter->second.size()];
    int index = 0;
    for (const auto& item : iter->second) {
      reqs[index].offset = item.offset;
      reqs[index].nbytes = item.nbytes;
      reqs[index].gfid = item.gfid;
      reqs[index]._reqid = item._reqid;
      reqs[index].op = item.op;
      reqs[index].result = item.result;
      reqs[index].state = item.state;
    }
    int waitall = 1;
    rc = unifyfs_wait_io(fshdl, iter->second.size(), reqs, waitall);
    if (rc == UNIFYFS_SUCCESS) {
      for (size_t i = iter->second.size(); i > 0; i++) {
        if (reqs[i].result.error == 0) {
          iter->second.pop_back();
        } else {
          TAILORFS_LOGERROR("Error while waiting for op %d. rc %d, message %s",
                            reqs[i].op, rc, strerror(rc));
        }
      }
    }
  }
  return (rc == UNIFYFS_SUCCESS) ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::UnifyFSFSView::write(UnifyFSWrite& payload) {
  unifyfs_io_request write_req;
  write_req.op = UNIFYFS_IOREQ_OP_WRITE;
  write_req.gfid = payload.gfid;
  write_req.nbytes = payload.nbytes;
  write_req.offset = payload.offset;
  write_req.user_buf = payload.buf;
  int rc = unifyfs_dispatch_io(fshdl, 1, &write_req);
  if (rc == UNIFYFS_SUCCESS) {
    auto iter = pending_req.find(payload.gfid);
    std::vector<UnifyFSIORequest> reqs;
    if (iter == pending_req.end()) {
      reqs = std::vector<UnifyFSIORequest>();
    } else {
      reqs = iter->second;
    }
    reqs.emplace_back(write_req);
    pending_req.insert_or_assign(payload.gfid, reqs);
  } else {
    TAILORFS_LOGERROR("Error while writing. rc %d, message %s", rc,
                      strerror(rc));
  }
  return (rc == UNIFYFS_SUCCESS) ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::UnifyFSFSView::read(UnifyFSRead& payload) {
  unifyfs_io_request read_req;
  read_req.op = UNIFYFS_IOREQ_OP_WRITE;
  read_req.gfid = payload.gfid;
  read_req.nbytes = payload.nbytes;
  read_req.offset = payload.offset;
  read_req.user_buf = payload.buf;
  int rc = unifyfs_dispatch_io(fshdl, 1, &read_req);
  if (rc == UNIFYFS_SUCCESS) {
    int waitall = 1;
    rc = unifyfs_wait_io(fshdl, 1, &read_req, waitall);
    if (rc == UNIFYFS_SUCCESS) {
      if (read_req.result.error != 0) {
        TAILORFS_LOGERROR("Error while reading. rc %d, message %s",
                          read_req.result.error,
                          strerror(read_req.result.error));
      }
    }
  } else {
    TAILORFS_LOGERROR("Error reading writing. rc %d, message %s", rc,
                      strerror(rc));
  }
  return (rc == UNIFYFS_SUCCESS) ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::UnifyFSFSView::finalize(UnifyFSFinalize& payload) {
  int rc = unifyfs_finalize(fshdl);
  return (rc == UNIFYFS_SUCCESS) ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
