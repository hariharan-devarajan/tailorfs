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
    char* tailorfs_log_level = getenv("TAILORFS_LOG_LEVEL");
    if (tailorfs_log_level == nullptr) {
      TAILORFS_LOGGER->level = cpplogger::LoggerType::LOG_ERROR;
      TAILORFS_LOGPRINT("Enabling ERROR loggin", "");
    } else {
      if (strcmp(tailorfs_log_level , "INFO") == 0) {
        TAILORFS_LOGGER->level = cpplogger::LoggerType::LOG_INFO;
        TAILORFS_LOGPRINT("Enabling INFO loggin", "");
      } else if (strcmp(tailorfs_log_level , "DEBUG") == 0) {
        TAILORFS_LOGPRINT("Enabling DEBUG loggin", "");
        TAILORFS_LOGGER->level = cpplogger::LoggerType::LOG_WARN;
      }
    }
    char* tailorfs_debug = getenv("TAILORFS_DEBUG");
    if (tailorfs_debug != nullptr) {
      if (strcmp(tailorfs_debug , "1") == 0) {
        fprintf(stderr, "Connect to processes\n");
        fflush(stderr);
        getchar();
      }
    }
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