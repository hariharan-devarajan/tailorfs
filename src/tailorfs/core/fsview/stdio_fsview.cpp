//
// Created by hariharan on 8/21/22.
//

#include "stdio_fsview.h"

#include <fcntl.h>
#include <unistd.h>

#include <regex>

#include "tailorfs/error_code.h"
#include "tailorfs/macro.h"
TailorFSStatus tailorfs::STDIOFSView::Initialize(tailorfs::STDIOInit& payload) {
  redirection = payload.redirection;
  return TAILORFS_SUCCESS;
}
TailorFSStatus tailorfs::STDIOFSView::Open(tailorfs::STDIOOpen& payload) {
  TAILORFS_LOGINFO("tailorfs::STDIOFSView::Open for file %s with redirection to %s", payload.filename,
                    redirection.new_storage._mount_point.c_str());
  std::string filename = std::string(payload.filename);
  if (redirection.is_enabled) {
    if (redirection.original_storage == redirection.new_storage) redirection.is_enabled=false;
    else {
    filename = std::regex_replace(
        filename, std::regex(redirection.original_storage._mount_point),
        redirection.new_storage._mount_point);
    fs::create_directory(fs::path(filename).parent_path());
    if (redirection.type == RedirectionType::PREFETCH ||
        redirection.type == RedirectionType::BOTH) {
      fs::copy_file(fs::absolute(payload.filename), fs::absolute(filename),
                    fs::copy_options::update_existing);
    }
    }	
  }
  payload.fh = fopen(filename.c_str(), payload.mode);
  if (redirection.is_enabled) {
    redirect_map.insert_or_assign(
        payload.fh,
        std::pair<std::string, std::string>(payload.filename, filename));
  }
  return payload.fh != nullptr ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::STDIOFSView::Close(tailorfs::STDIOClose& payload) {
  int status_orig;
  status_orig = fclose(payload.fh);
  if (redirection.is_enabled) {
    auto iter = redirect_map.find(payload.fh);
    if (redirection.type == RedirectionType::FLUSH ||
        redirection.type == RedirectionType::BOTH) {
      if (iter != redirect_map.end()) {
        utility->exclude_file(fs::absolute(iter->second.first).c_str(),
                              brahma::InterfaceType::INTERFACE_POSIX);
        fs::copy_file(iter->second.second, fs::absolute(iter->second.first),
                      fs::copy_options::overwrite_existing);
        utility->include_file(fs::absolute(iter->second.first).c_str(),
                              brahma::InterfaceType::INTERFACE_POSIX);
      }
    }
    fs::remove(iter->second.second);
  }
  return status_orig == 0 ? TAILORFS_SUCCESS : TAILORFS_FAILED;
}
TailorFSStatus tailorfs::STDIOFSView::Write(tailorfs::STDIOWrite& payload) {
  auto status = fseek(payload.fh, payload.offset, SEEK_SET);
  if (status == 0) {
    payload.written_bytes =
        fwrite(payload.buf, sizeof(char), payload.size, payload.fh);
    return payload.written_bytes == payload.size ? TAILORFS_SUCCESS
                                                 : TAILORFS_FAILED;
  }
  return TAILORFS_FAILED;
}
TailorFSStatus tailorfs::STDIOFSView::Read(tailorfs::STDIORead& payload) {
  auto status = fseek(payload.fh, payload.offset, SEEK_SET);
  if (status == 0) {
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
TailorFSStatus tailorfs::STDIOFSView::Convert(int& flags, char*& mode) {
  mode = new char[3];
  if (flags & O_WRONLY) {
    mode[0] = 'w';
    mode[1] = '\0';
  } else if (flags & O_RDONLY) {
    mode[0] = 'r';
    mode[1] = '\0';
  }else if (flags & O_APPEND) {
    mode[0] = 'a';
    mode[1] = '\0';
  }else if (flags & O_RDWR) {
    mode[0] = 'w';
    mode[1] = '+';
  }
  if (flags & O_CREAT) {
    mode[1] = '+';
  }
  if (flags & O_EXCL) {
    mode[1] = '\0';
  }
  if (flags == 0) {
    mode[0] = 'r';
    mode[1] = '\0';
  }
  mode[2]='\0';
  return TAILORFS_SUCCESS;
}
