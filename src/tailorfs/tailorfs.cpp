//
// Created by haridev on 8/27/22.
//
#include <tailorfs/tailorfs.h>
#include <tailorfs/brahma/mpi.h>
namespace tailorfs {
bool init = false;
}

bool is_init() {return tailorfs::init;}
void set_init(bool _init) { tailorfs::init = _init;}

void tfs_init(bool is_mpi) {
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
      if (!is_mpi) init_mimir();
      mimir_init_config(is_mpi);
      insert_loaded_intents();
      if (!is_mpi) brahma_gotcha_wrap("tailorfs", 1);
      FSVIEW_MANAGER->initialize();
      set_init(true);
    }
}
void tfs_finalize(bool is_mpi) {
  if (is_init()) {
    if (is_mpi) FSVIEW_MANAGER->finalize();
    if (is_mpi) FSVIEW_MANAGER.reset();
    if (!is_mpi) free_bindings();
    if (is_mpi) remove_loaded_intents();
    if (is_mpi) mimir_finalize_config();
    if (!is_mpi) finalize_mimir();
    if (!is_mpi) set_init(false);
  }
}
void tailorfs_init(void) {
  char* tailorfs_debug = getenv("TAILORFS_DEBUG");
  if (tailorfs_debug != nullptr) {
    if (strcmp(tailorfs_debug , "1") == 0) {
      std::string sp;
      std::ifstream("/proc/self/cmdline") >> sp;
      std::replace( sp.begin(), sp.end() - 1, '\000', ' ');
      fprintf(stderr, "Connect to pid %d %s\n", getpid(), sp.c_str());
      fflush(stderr);
      getchar();
    }
  }
  char* is_non_mpi = getenv("TAILORFS_NON_MPI");
  if (is_non_mpi != nullptr && strcmp(is_non_mpi , "1") == 0) {
    TAILORFS_LOGINFO("TAILORFS constructor for Non MPI\n","");
    tfs_init();
  } else {
    TAILORFS_LOGINFO("TAILORFS constructor for MPI\n","");
    init_mimir();
    brahma_gotcha_wrap("tailorfs", 1);
    brahma::MPITailorFS::get_instance();
  }
}
void tailorfs_fini(void) {
  tfs_finalize();
}