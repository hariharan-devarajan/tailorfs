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

  TailorFSStatus Initialize(UnifyFSInit& payload) override;
  TailorFSStatus Open(UnifyFSOpen& payload) override;
  TailorFSStatus Close(UnifyFSClose& payload) override;
  TailorFSStatus Write(UnifyFSWrite& payload) override;
  TailorFSStatus Read(UnifyFSRead& payload) override;
  TailorFSStatus Finalize(UnifyFSFinalize& payload) override;
};
}  // namespace tailorfs

#endif  // TAILORFS_UNIFYFS_FSVIEW_H
