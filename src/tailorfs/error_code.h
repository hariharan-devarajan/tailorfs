//
// Created by haridev on 2/15/22.
//

#ifndef TAILORFS_ERROR_CODE_H
#define TAILORFS_ERROR_CODE_H
#include <tailorfs/core/typedef.h>

namespace tailorfs {
typedef enum ErrorCode : TailorFSStatus {
  TAILORFS_SUCCESS = 0,
  TAILORFS_NOT_FOUND = -1,
  TAILORFS_FAILED = -2
} ErrorCode;
}
#endif  // TAILORFS_ERROR_CODE_H
