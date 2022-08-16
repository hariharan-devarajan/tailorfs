//
// Created by hariharan on 8/16/22.
//
#include <cpp-logger/logger.h>
#include <tailorfs/brahma/stdio.h>

FILE *brahma::STDIOTailorFS::fopen64(const char *path, const char *mode) {
  BRAHMA_MAP_OR_FAIL(fopen64);
  FILE *ret = nullptr;
  if (is_traced(path)) {
    ret = __real_fopen64(path, mode);
    TAILORFS_LOGPRINT("fopen64(%s, %s) = %d", path, mode, ret);
    track_fp(ret);
  } else {
    ret = __real_fopen64(path, mode);
  }
  return ret;
}
FILE *brahma::STDIOTailorFS::fopen(const char *path, const char *mode) {
  BRAHMA_MAP_OR_FAIL(fopen);
  FILE *ret = nullptr;
  if (is_traced(path)) {
    ret = __real_fopen(path, mode);
    TAILORFS_LOGPRINT("fopen(%s, %s) = %d", path, mode, ret);
    track_fp(ret);
  } else {
    ret = __real_fopen(path, mode);
  }
  return ret;
}
int brahma::STDIOTailorFS::fclose(FILE *fp) {
  BRAHMA_MAP_OR_FAIL(fclose);
  int ret = -1;
  if (is_traced(fp)) {
    ret = __real_fclose(fp);
    TAILORFS_LOGPRINT("fclose(%d) = %d", fp, ret);
    untrack_fp(fp);
  } else {
    ret = __real_fclose(fp);
  }
  return ret;
}
size_t brahma::STDIOTailorFS::fread(void *ptr, size_t size, size_t nmemb,
                                    FILE *fp) {
  BRAHMA_MAP_OR_FAIL(fread);
  size_t ret = -1;
  if (is_traced(fp)) {
    ret = __real_fread(ptr, size, nmemb, fp);
    TAILORFS_LOGPRINT("fread(ptr,%ld,%ld, %d) = %d", fp, size, nmemb, ret);
  } else {
    ret = __real_fread(ptr, size, nmemb, fp);
  }
  return ret;
}
size_t brahma::STDIOTailorFS::fwrite(const void *ptr, size_t size, size_t nmemb,
                                     FILE *fp) {
  BRAHMA_MAP_OR_FAIL(fwrite);
  size_t ret = -1;
  if (is_traced(fp)) {
    ret = __real_fwrite(ptr, size, nmemb, fp);
    TAILORFS_LOGPRINT("fwrite(ptr,%ld,%ld, %d) = %d", fp, size, nmemb, ret);
  } else {
    ret = __real_fwrite(ptr, size, nmemb, fp);
  }
  return ret;
}
long brahma::STDIOTailorFS::ftell(FILE *fp) {
  BRAHMA_MAP_OR_FAIL(ftell);
  long ret = -1;
  if (is_traced(fp)) {
    ret = __real_ftell(fp);
    TAILORFS_LOGPRINT("ftell(fp) = %d", fp, ret);
  } else {
    ret = __real_ftell(fp);
  }
  return ret;
}
int brahma::STDIOTailorFS::fseek(FILE *fp, long offset, int whence) {
  BRAHMA_MAP_OR_FAIL(fseek);
  int ret = -1;
  if (is_traced(fp)) {
    ret = __real_fseek(fp, offset, whence);
    TAILORFS_LOGPRINT("lseek(%d, %ld, %d) = %d", fp, offset, whence, ret);
  } else {
    ret = __real_fseek(fp, offset, whence);
  }
  return ret;
}