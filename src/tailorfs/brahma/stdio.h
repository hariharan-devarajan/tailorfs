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
    TAILORFS_LOGPRINT("STDIO class intercepted", "");
    auto config = MIMIR_CONFIG();
    if (config->_current_process_index != -1) {
      auto app_intent = config->_app_repo[config->_current_process_index];
      for (auto element : app_intent._interfaces_used) {
        for (const auto& interface: element.second) {
          if (interface == mimir::InterfaceType::STDIO) {
            track_file(config->_file_repo[element.first]._name.c_str());
          }
        }
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
