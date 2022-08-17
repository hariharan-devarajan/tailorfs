//
// Created by hariharan on 8/16/22.
//

#ifndef TAILORFS_MACRO_H
#define TAILORFS_MACRO_H
#include <cpp-logger/logger.h>
#define TAILORFS_LOGGER cpplogger::Logger::Instance("TAILORFS")
#define TAILORFS_LOGINFO(format, ...) \
  TAILORFS_LOGGER->log(cpplogger::LOG_INFO, format, __VA_ARGS__);
#define TAILORFS_LOGWARN(format, ...) \
  TAILORFS_LOGGER->log(cpplogger::LOG_WARN, format, __VA_ARGS__);
#define TAILORFS_LOGERROR(format, ...) \
  TAILORFS_LOGGER->log(cpplogger::LOG_ERROR, format, __VA_ARGS__);
#define TAILORFS_LOGPRINT(format, ...) \
  TAILORFS_LOGGER->log(cpplogger::LOG_PRINT, format, __VA_ARGS__);
#endif  // TAILORFS_MACRO_H
