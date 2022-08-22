//
// Created by hariharan on 8/21/22.
//

#ifndef TAILORFS_POSIX_FSVIEW_H
#define TAILORFS_POSIX_FSVIEW_H
#include <tailorfs/core/datastructure.h>

#include <experimental/filesystem>

#include "fsview.h"
namespace fs = std::experimental::filesystem;
namespace tailorfs {
class POSIXFSView : public FSView<POSIXInit, POSIXFinalize, POSIXOpen,
                                  POSIXClose, POSIXRead, POSIXWrite> {
  RedirectFeature redirection;
  std::unordered_map<int, std::pair<std::string, std::string>> redirect_map;

 public:
  POSIXFSView() = default;
  POSIXFSView(POSIXFSView& other) = default;
  POSIXFSView(POSIXFSView&& other) = default;
  TailorFSStatus Initialize(POSIXInit& payload) override;
  TailorFSStatus Open(POSIXOpen& payload) override;
  TailorFSStatus Close(POSIXClose& payload) override;
  TailorFSStatus Write(POSIXWrite& payload) override;
  TailorFSStatus Read(POSIXRead& payload) override;
  TailorFSStatus Finalize(POSIXFinalize& payload) override;
};
}  // namespace tailorfs

#endif  // TAILORFS_POSIX_FSVIEW_H
