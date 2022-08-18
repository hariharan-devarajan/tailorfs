//
// Created by hariharan on 8/17/22.
//

#ifndef TAILORFS_UNIFYFS_FSVIEW_H
#define TAILORFS_UNIFYFS_FSVIEW_H

#include <tailorfs/core/datastructure.h>
#include <unifyfs/unifyfs_api.h>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#include "fsview.h"
namespace tailorfs {
class UnifyFSFSView : public FSView<UnifyFSInit, UnifyFSFinalize, UnifyFSOpen,
                                    UnifyFSClose, UnifyFSRead, UnifyFSWrite> {
 private:
  unifyfs_handle fshdl;
  fs::path unifyfs_namespace;
  std::unordered_map<unifyfs_gfid, std::vector<UnifyFSIORequest>> pending_req;

 public:
  UnifyFSFSView() = default;
  UnifyFSFSView(UnifyFSFSView& other) = default;
  UnifyFSFSView(UnifyFSFSView&& other) = default;
  UnifyFSFSView& operator=(const UnifyFSFSView& t) = default;

  TailorFSStatus initialize(UnifyFSInit& payload) override;
  TailorFSStatus open(UnifyFSOpen& payload) override;
  TailorFSStatus close(UnifyFSClose& payload) override;
  TailorFSStatus write(UnifyFSWrite& payload) override;
  TailorFSStatus read(UnifyFSRead& payload) override;
  TailorFSStatus finalize(UnifyFSFinalize& payload) override;
};
}  // namespace tailorfs

#endif  // TAILORFS_UNIFYFS_FSVIEW_H
