//
// Created by hariharan on 8/17/22.
//

#ifndef TAILORFS_UNIFYFS_FSVIEW_H
#define TAILORFS_UNIFYFS_FSVIEW_H

#include <tailorfs/core/datastructure.h>

#include "fsview.h"
namespace tailorfs {
class UnifyFSFSView : public FSView<UnifyFSInit, UnifyFSFinalize, UnifyFSOpen,
                                    UnifyFSClose, UnifyFSRead, UnifyFSWrite> {
 public:
  UnifyFSFSView() = default;
  UnifyFSFSView(UnifyFSFSView& other) = default;
  UnifyFSFSView(UnifyFSFSView&& other) = default;
  UnifyFSFSView& operator=(const UnifyFSFSView& t) = default;

  TailorFSStatus initialize(UnifyFSInit& payload) override;
  TailorFSStatus open(UnifyFSOpen& payload) override;
  TailorFSStatus close(UnifyFSClose& payload) override;
  TailorFSStatus write(UnifyFSRead& payload) override;
  TailorFSStatus read(UnifyFSWrite& payload) override;
  TailorFSStatus finalize(UnifyFSFinalize& payload) override;
};
}  // namespace tailorfs

#endif  // TAILORFS_UNIFYFS_FSVIEW_H
