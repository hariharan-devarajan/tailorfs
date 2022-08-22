//
// Created by hariharan on 8/21/22.
//

#ifndef TAILORFS_STDIO_FSVIEW_H
#define TAILORFS_STDIO_FSVIEW_H

#include <tailorfs/core/datastructure.h>

#include <experimental/filesystem>

#include "fsview.h"
namespace fs = std::experimental::filesystem;
namespace tailorfs {
class STDIOFSView : public FSView<STDIOInit, STDIOFinalize, STDIOOpen,
                                  STDIOClose, STDIORead, STDIOWrite> {
  RedirectFeature redirection;
  std::unordered_map<FILE*, std::pair<std::string, std::string>> redirect_map;

 public:
  STDIOFSView() = default;
  STDIOFSView(STDIOFSView& other) = default;
  STDIOFSView(STDIOFSView&& other) = default;
  TailorFSStatus Initialize(STDIOInit& payload) override;
  TailorFSStatus Open(STDIOOpen& payload) override;
  TailorFSStatus Close(STDIOClose& payload) override;
  TailorFSStatus Write(STDIOWrite& payload) override;
  TailorFSStatus Read(STDIORead& payload) override;
  TailorFSStatus Finalize(STDIOFinalize& payload) override;
};
}  // namespace tailorfs

#endif  // TAILORFS_STDIO_FSVIEW_H
