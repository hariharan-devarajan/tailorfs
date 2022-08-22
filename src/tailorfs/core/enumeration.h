//
// Created by hariharan on 8/17/22.
//

#ifndef TAILORFS_ENUMERATION_H
#define TAILORFS_ENUMERATION_H
#include <cstdint>
namespace tailorfs {
enum FSViewType : uint8_t { FS_NONE = 0, MPIIO = 1, UNIFYFS = 2 };
enum RedirectionType : uint8_t {
  REDIRECT_NONE = 0,
  PREFETCH = 1,
  FLUSH = 2,
  BOTH = 3
};
}  // namespace tailorfs
#endif  // TAILORFS_ENUMERATION_H
