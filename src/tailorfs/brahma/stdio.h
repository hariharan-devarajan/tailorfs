//
// Created by hariharan on 8/16/22.
//

#ifndef TAILORFS_STDIO_H
#define TAILORFS_STDIO_H
#include <brahma/brahma.h>
#include <fcntl.h>
#include <mimir/mimir.h>

#include <memory>
#include <unordered_set>

#include "tailorfs/macro.h"

namespace brahma {
class STDIOTailorFS : public STDIO {
 private:
  typedef FILE *FilePointer;
  static std::shared_ptr<STDIOTailorFS> instance;
  std::unordered_set<FilePointer> fps;

  inline bool is_traced(const char* filename) {
    return utility->is_traced(filename, brahma::InterfaceType::INTERFACE_STDIO);
  }
  inline void exclude_file(const char *filename) {
    utility->exclude_file(filename, brahma::InterfaceType::INTERFACE_STDIO);
  }
  inline void include_file(const char *filename) {
    utility->include_file(filename, brahma::InterfaceType::INTERFACE_STDIO);

  }
  inline void track_file(const char *filename) {
    utility->track_file(filename, brahma::InterfaceType::INTERFACE_STDIO);
  }
  inline void untrack_file(const char *filename) {
    utility->untrack_file(filename, brahma::InterfaceType::INTERFACE_STDIO);
  }

  inline bool is_traced(const FilePointer fp) {
    auto iter = fps.find(fp);
    if (iter != fps.end()) return true;
    return false;
  }
  inline void track_fp(const FilePointer fp) { fps.emplace(fp); }
  inline void untrack_fp(const FilePointer fp) { fps.erase(fp); }

 public:
  STDIOTailorFS() : STDIO() {
    TAILORFS_LOGINFO("STDIO class intercepted", "");
    auto config = MIMIR_CONFIG();
    if (config->_current_process_index != -1) {
      auto app_intent = config->_app_repo[config->_current_process_index];
      auto selected_interface = mimir::InterfaceType::STDIO;
      int rank = 0;
      if(app_intent._is_mpi) {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      }
      bool is_used = true;
      auto interface_file_indices = std::unordered_set<mimir::FileIndex>();
      for (auto element : app_intent._interfaces_used) {
        for (const auto& interface: element.second) {
          if (interface == selected_interface) {
            interface_file_indices.emplace(element.first);
          }
        }
      }

      auto rank_file_indices = std::unordered_set<mimir::FileIndex>();
      for (auto element : app_intent._rank_file_dag.edges) {
        if (element.source == rank) rank_file_indices.emplace(element.destination);
      }
      for ( auto iter:rank_file_indices) {
        auto iterface_iter = interface_file_indices.find(iter);
        if (iterface_iter != interface_file_indices.end())
          track_file(config->_file_repo[iter]._name.c_str());
      }
    }
  }
  ~STDIOTailorFS() = default;
  static std::shared_ptr<STDIOTailorFS> get_instance() {
    if (instance == nullptr) {
      instance = std::make_shared<STDIOTailorFS>();
      STDIO::set_instance(instance);
    }
    return instance;
  }
  FILE *fopen(const char *path, const char *mode) override;
  FILE *fopen64(const char *path, const char *mode) override;
  int fclose(FILE *fp) override;
  size_t fread(void *ptr, size_t size, size_t nmemb, FILE *fp) override;
  size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *fp) override;
  long ftell(FILE *fp) override;
  int fseek(FILE *fp, long offset, int whence) override;
};

}  // namespace brahma
#endif  // TAILORFS_STDIO_H
