//
// Created by hariharan on 8/21/22.
//

#include "stdio_fsview.h"

#include <fcntl.h>
#include <unistd.h>

#include <regex>

#include "tailorfs/error_code.h"
TailorFSStatus tailorfs::STDIOFSView::Initialize(tailorfs::STDIOInit& payload) {
  redirection = payload.redirection;
  return TAILORFS_SUCCESS;
}
TailorFSStatus tailorfs::STDIOFSView::Open(tailorfs::STDIOOpen& payload) {
  std::string filename = std::string(payload.filename);
  if (redirection.is_enabled) {
    filename = std::regex_replace(
        filename, std::regex(redirection.original_storage._mount_point),
        redirection.new_storage._mount_point);
  }
  FILE* fh = fopen(filename.c_str(), payload.mode);
  if (redirection.is_enabled) {
    redirect_map.insert_or_assign(
        payload.fh,
        std::pair<std::string, std::string>(payload.filename, filename));
  }
  return fh == nullptr ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::STDIOFSView::Close(tailorfs::STDIOClose& payload) {
  int status_orig;
  status_orig = fclose(payload.fh);
  if (redirection.is_enabled) {
    auto iter = redirect_map.find(payload.fh);
    if (iter != redirect_map.end()) {
      fs::copy_file(iter->second.second, fs::absolute(iter->second.first),
                    fs::copy_options::overwrite_existing);
    }
    fs::remove(iter->second.second);
  }
  return status_orig == 0 ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::STDIOFSView::Write(tailorfs::STDIOWrite& payload) {
  auto status = fseek(payload.fh, payload.offset, SEEK_SET);
  if (status == payload.offset) {
    payload.written_bytes =
        fwrite(payload.buf, sizeof(char), payload.size, payload.fh);
    return payload.written_bytes == payload.size ? TAILORFS_SUCCESS
                                                 : TAILORFS_FAILED;
  }
  return TAILORFS_FAILED;
}
TailorFSStatus tailorfs::STDIOFSView::Read(tailorfs::STDIORead& payload) {
  auto status = fseek(payload.fh, payload.offset, SEEK_SET);
  if (status == payload.offset) {
    payload.read_bytes =
        fread(payload.buf, sizeof(char), payload.size, payload.fh);
    return payload.read_bytes == payload.size ? TAILORFS_SUCCESS
                                              : TAILORFS_FAILED;
  }
  return TAILORFS_FAILED;
}
TailorFSStatus tailorfs::STDIOFSView::Finalize(
    tailorfs::STDIOFinalize& payload) {
  return TAILORFS_SUCCESS;
}
