//
// Created by hariharan on 8/17/22.
//

#ifndef TAILORFS_ENUMERATION_H
#define TAILORFS_ENUMERATION_H
#include <cstdint>
namespace tailorfs {
enum FSViewType : uint8_t { NONE = 0, MPIIO = 1, UNIFYFS = 2 };
}
#endif  // TAILORFS_ENUMERATION_H
