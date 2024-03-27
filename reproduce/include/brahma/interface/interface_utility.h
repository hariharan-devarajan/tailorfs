//
// Created by haridev on 8/28/22.
//

#ifndef TAILORFS_INTERFACE_UTILITY_H
#define TAILORFS_INTERFACE_UTILITY_H
#include <string>
#include <unordered_set>

#include <brahma/enumeration.h>
#include <unordered_map>
namespace brahma {
class InterfaceUtility {

  std::unordered_map<brahma::InterfaceType, std::unordered_set<std::string>> filenames;
  std::unordered_map<brahma::InterfaceType, std::unordered_set<std::string>> excluded_filenames;
 public:
  InterfaceUtility():filenames(),excluded_filenames(){}
  inline void exclude_file(const char *filename, brahma::InterfaceType interface) {
    auto interface_iter = excluded_filenames.find(interface);
    std::unordered_set<std::string> excludes_files;
    if (interface_iter == excluded_filenames.end()) {
      excludes_files = std::unordered_set<std::string>();
    }else {
      excludes_files = interface_iter->second;
    }
    excludes_files.emplace(filename);
    excluded_filenames.insert_or_assign(interface, excludes_files);
  }
  inline void include_file(const char *filename,
                           brahma::InterfaceType interface) {
    auto interface_iter = excluded_filenames.find(interface);
    if (interface_iter != excluded_filenames.end()) {
      interface_iter->second.erase(filename);
      excluded_filenames.insert_or_assign(interface, interface_iter->second);
    }
  }
  inline bool is_traced(const char *filename, brahma::InterfaceType interface) {
    auto interface_iter = excluded_filenames.find(interface);
    if (interface_iter != excluded_filenames.end()) {
      auto iter = interface_iter->second.find(filename);
      if (iter != interface_iter->second.end()) return false;
    }
    interface_iter = filenames.find(interface);
    if (interface_iter != filenames.end()) {
      auto iter = interface_iter->second.find(filename);
      if (iter != interface_iter->second.end()) return true;
    }
    return false;
  }
  inline void track_file(const char *filename, brahma::InterfaceType interface) {
    auto interface_iter = filenames.find(interface);
    std::unordered_set<std::string> track_files;
    if (interface_iter == filenames.end()) {
      track_files = std::unordered_set<std::string>();
    }else {
      track_files = interface_iter->second;
      filenames.erase(interface_iter);
    }
    track_files.emplace(filename);
    filenames.emplace(interface, track_files);
  }
  inline void untrack_file(const char *filename,
                           brahma::InterfaceType interface) {
    auto interface_iter = filenames.find(interface);
    if (interface_iter != filenames.end()) {
      interface_iter->second.erase(filename);
      filenames.insert_or_assign(interface, interface_iter->second);
    }
  }
};
}
#endif  // TAILORFS_INTERFACE_UTILITY_H
