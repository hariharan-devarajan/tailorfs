//
// Created by hariharan on 8/8/22.
//

#ifndef BRAHMA_STDIO_H
#define BRAHMA_STDIO_H

#include <cstdio>
#include "interface.h"
#include "brahma/interceptor.h"

namespace brahma {
class STDIO:public Interface {
 private:
  static std::shared_ptr<STDIO> my_instance;

 public:
  static std::shared_ptr<STDIO> get_instance() {
    if (my_instance == nullptr) {
      BRAHMA_LOGINFO("STDIO class not intercepted but used", "");
      my_instance = std::make_shared<STDIO>();
    }
    return my_instance;
  }
  STDIO():Interface(){}
  virtual ~STDIO() = default;
  static int set_instance(std::shared_ptr<STDIO> instance_i) {
    if (instance_i != nullptr) {
      my_instance = instance_i;
      return 0;
    } else {
      BRAHMA_LOGERROR("instance_i is not set", "");
      throw std::runtime_error("instance_i is not set");
    }
  }

  virtual FILE *fopen(const char *path, const char *mode);

  virtual FILE *fopen64(const char *path, const char *mode);

  virtual int fclose(FILE *fp);

  virtual size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

  virtual size_t fwrite(const void *ptr, size_t size, size_t nmemb,
                        FILE *stream);

  virtual long ftell(FILE *fp);

  virtual int fseek(FILE *stream, long offset, int whence);
};

}  // namespace brahma
GOTCHA_MACRO_TYPEDEF(fopen, FILE *, (const char *path, const char *mode),
                     (path, mode), brahma::STDIO);

GOTCHA_MACRO_TYPEDEF(fopen64, FILE *, (const char *path, const char *mode),
                     (path, mode), brahma::STDIO);

GOTCHA_MACRO_TYPEDEF(fclose, int, (FILE * fp), (fp), brahma::STDIO);

GOTCHA_MACRO_TYPEDEF(fread, size_t,
                     (void *ptr, size_t size, size_t nmemb, FILE *stream),
                     (ptr, size, nmemb, stream), brahma::STDIO);

GOTCHA_MACRO_TYPEDEF(fwrite, size_t,
                     (const void *ptr, size_t size, size_t nmemb, FILE *stream),
                     (ptr, size, nmemb, stream), brahma::STDIO);

GOTCHA_MACRO_TYPEDEF(ftell, long, (FILE * stream), (stream), brahma::STDIO);

GOTCHA_MACRO_TYPEDEF(fseek, int, (FILE * stream, long offset, int whence),
                     (stream, offset, whence), brahma::STDIO);
#endif  // BRAHMA_STDIO_H
