//
// Created by hariharan on 8/8/22.
//

#ifndef TAILORFS_TAILORFS_H
#define TAILORFS_TAILORFS_H

#include <brahma/brahma.h>
#include <mimir/mimir.h>
#include <tailorfs/brahma/posix.h>
#include <tailorfs/brahma/stdio.h>
#include <tailorfs/macro.h>

inline void tfs_init() {
  init_mimir();
  mimir_init_config();
  insert_loaded_intents();
  brahma_gotcha_wrap("tailorfs", 1);
  brahma::POSIXTailorFS::get_instance();
  brahma::STDIOTailorFS::get_instance();
}
inline void tfs_finalize() {
  free_bindings();
  remove_loaded_intents();
  mimir_finalize_config();
  finalize_mimir();
}

void __attribute__((constructor)) tailorfs_init() { tfs_init(); }
void __attribute__((destructor)) tailorfs_finalize() { tfs_finalize(); }

#endif  // TAILORFS_TAILORFS_H
