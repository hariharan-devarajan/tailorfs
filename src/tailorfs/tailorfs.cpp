//
// Created by haridev on 8/27/22.
//
#include <tailorfs/tailorfs.h>

namespace tailorfs {
bool init = false;
}

bool is_init() {return tailorfs::init;}
void set_init(bool _init) { tailorfs::init = _init;}

inline void tfs_init() {
  if (!is_init()) {
    init_mimir();
    mimir_init_config();
    insert_loaded_intents();
    brahma_gotcha_wrap("tailorfs", 1);
    FSVIEW_MANAGER->initialize();
    set_init(true);
  }
}
inline void tfs_finalize() {
  if (is_init()) {
    FSVIEW_MANAGER->finalize();
    FSVIEW_MANAGER.reset();
    free_bindings();
    remove_loaded_intents();
    mimir_finalize_config();
    finalize_mimir();
    set_init(false);
  }
}
void tailorfs_init(void) {
  TAILORFS_LOGPRINT("TAILORFS constructor\n","");
  tfs_init();
}
void tailorfs_fini(void) {
  TAILORFS_LOGPRINT("TAILORFS destructor\n","");
  tfs_finalize();
}