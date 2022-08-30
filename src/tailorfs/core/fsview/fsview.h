//
// Created by hariharan on 8/17/22.
//

#ifndef TAILORFS_FSVIEW_H
#define TAILORFS_FSVIEW_H
#include <brahma/interface/interface_utility.h>
#include <tailorfs/core/datastructure.h>
#include <tailorfs/core/typedef.h>

#include <type_traits>

#include <brahma/singleton.h>

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
  std::shared_ptr<brahma::InterfaceUtility> utility;
  FSView(){
    utility = brahma::Singleton<brahma::InterfaceUtility>::get_instance();
  }
  virtual TailorFSStatus Initialize(I& payload) = 0;
  virtual TailorFSStatus Open(O& payload) = 0;
  virtual TailorFSStatus Close(C& payload) = 0;
  virtual TailorFSStatus Write(W& payload) = 0;
  virtual TailorFSStatus Read(R& payload) = 0;
  virtual TailorFSStatus Finalize(F& payload) = 0;
};
}  // namespace tailorfs
#endif  // TAILORFS_FSVIEW_H
