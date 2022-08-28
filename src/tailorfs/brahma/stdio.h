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
  std::unordered_set<std::string> filenames;
  std::unordered_set<FilePointer> fps;

  inline bool is_traced(const char *filename) {
    auto iter = filenames.find(filename);
    if (iter != filenames.end()) return true;
    return false;
  }

  inline bool is_traced(const FilePointer fp) {
    auto iter = fps.find(fp);
    if (iter != fps.end()) return true;
    return false;
  }

  inline void track_file(const char *filename) { filenames.emplace(filename); }
  inline void untrack_file(const char *filename) { filenames.erase(filename); }
  inline void track_fp(const FilePointer fp) { fps.emplace(fp); }
  inline void untrack_fp(const FilePointer fp) { fps.erase(fp); }

 public:
  STDIOTailorFS() : STDIO() {
    auto config = MIMIR_CONFIG();
    for (auto file : config->_file_repo) {
      track_file(file._name.c_str());
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
