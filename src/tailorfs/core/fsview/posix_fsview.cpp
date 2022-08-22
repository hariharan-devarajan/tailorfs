//
// Created by hariharan on 8/21/22.
//

#include "posix_fsview.h"

#include <fcntl.h>
#include <unistd.h>

#include <regex>

#include "tailorfs/error_code.h"
TailorFSStatus tailorfs::POSIXFSView::Initialize(tailorfs::POSIXInit& payload) {
  redirection = payload.redirection;
  return TAILORFS_SUCCESS;
}
TailorFSStatus tailorfs::POSIXFSView::Open(tailorfs::POSIXOpen& payload) {
  std::string filename = std::string(payload.filename);
  if (redirection.is_enabled) {
    filename = std::regex_replace(
        filename, std::regex(redirection.original_storage._mount_point),
        redirection.new_storage._mount_point);
    if (redirection.type == RedirectionType::PREFETCH ||
        redirection.type == RedirectionType::BOTH) {
      fs::copy_file(fs::absolute(payload.filename), fs::absolute(filename),
                    fs::copy_options::overwrite_existing);
    }
  }
  int fd = open(filename.c_str(), payload.flags, payload.mode);
  if (redirection.is_enabled) {
    redirect_map.insert_or_assign(
        payload.fd,
        std::pair<std::string, std::string>(payload.filename, filename));
  }
  return fd != -1 ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::POSIXFSView::Close(tailorfs::POSIXClose& payload) {
  int status_orig;
  status_orig = close(payload.fd);
  if (redirection.is_enabled) {
    auto iter = redirect_map.find(payload.fd);
    if (redirection.type == RedirectionType::FLUSH ||
        redirection.type == RedirectionType::BOTH) {
      if (iter != redirect_map.end()) {
        fs::copy_file(iter->second.second, fs::absolute(iter->second.first),
                      fs::copy_options::overwrite_existing);
      }
    }
    fs::remove(iter->second.second);
  }
  return status_orig == 0 ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::POSIXFSView::Write(tailorfs::POSIXWrite& payload) {
  auto status = lseek(payload.fd, payload.offset, SEEK_SET);
  if (status == 0) {
    payload.written_bytes = write(payload.fd, payload.buf, payload.size);
    return payload.written_bytes == payload.size ? TAILORFS_SUCCESS
                                                 : TAILORFS_FAILED;
  }
  return TAILORFS_FAILED;
}
TailorFSStatus tailorfs::POSIXFSView::Read(tailorfs::POSIXRead& payload) {
  auto status = lseek(payload.fd, payload.offset, SEEK_SET);
  if (status == 0) {
    payload.read_bytes = read(payload.fd, payload.buf, payload.size);
    return payload.read_bytes == payload.size ? TAILORFS_SUCCESS
                                              : TAILORFS_FAILED;
  }
  return TAILORFS_FAILED;
}
TailorFSStatus tailorfs::POSIXFSView::Finalize(
    tailorfs::POSIXFinalize& payload) {
  return TAILORFS_SUCCESS;
}
