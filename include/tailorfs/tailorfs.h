//
// Created by hariharan on 8/8/22.
//

#ifndef TAILORFS_TAILORFS_H
#define TAILORFS_TAILORFS_H

#include <brahma/brahma.h>
#include <mimir/mimir.h>
#include <tailorfs/brahma/posix.h>

void __attribute__((constructor)) tailorfs_init() {
  mimir_init_config();
  brahma_gotcha_wrap("tailorfs", 1);
  brahma::POSIXTest::get_instance();
}
void __attribute__((destructor)) tailorfs_finalize() {
  mimir_finalize_config();
}

#endif  // TAILORFS_TAILORFS_H
