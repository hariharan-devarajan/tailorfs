//
// Created by hariharan on 8/17/22.
//

#ifndef TAILORFS_FSVIEW_H
#define TAILORFS_FSVIEW_H
#include <tailorfs/core/datastructure.h>
#include <tailorfs/core/typedef.h>

#include <type_traits>

namespace tailorfs {
template <
    typename I, typename F, typename O, typename C, typename R, typename W,
    typename std::enable_if<std::is_base_of<FSViewInit, I>::value>::type* =
        nullptr,
    typename std::enable_if<std::is_base_of<FSViewFinalize, F>::value>::type* =
        nullptr,
    typename std::enable_if<std::is_base_of<FSViewOpen, O>::value>::type* =
        nullptr,
    typename std::enable_if<std::is_base_of<FSViewClose, C>::value>::type* =
        nullptr,
    typename std::enable_if<std::is_base_of<FSViewRead, R>::value>::type* =
        nullptr,
    typename std::enable_if<std::is_base_of<FSViewWrite, W>::value>::type* =
        nullptr>
class FSView {
 public:
  virtual TailorFSStatus initialize(I& payload) = 0;
  virtual TailorFSStatus open(O& payload) = 0;
  virtual TailorFSStatus close(C& payload) = 0;
  virtual TailorFSStatus write(R& payload) = 0;
  virtual TailorFSStatus read(W& payload) = 0;
  virtual TailorFSStatus finalize(F& payload) = 0;
};
}  // namespace tailorfs
#endif  // TAILORFS_FSVIEW_H
