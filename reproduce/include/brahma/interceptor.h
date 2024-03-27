//
// Created by hariharan on 8/8/22.
//

#ifndef BRAHMA_INTERCEPTOR_H
#define BRAHMA_INTERCEPTOR_H

#include <cpp-logger/logger.h>
#include <gotcha/gotcha.h>

#include <memory>
#define BRAHMA_LOGGER cpplogger::Logger::Instance("BRAHMA")
#define BRAHMA_LOGINFO(format, ...) \
  BRAHMA_LOGGER->log(cpplogger::LOG_INFO, format, __VA_ARGS__);
#define BRAHMA_LOGWARN(format, ...) \
  BRAHMA_LOGGER->log(cpplogger::LOG_WARN, format, __VA_ARGS__);
#define BRAHMA_LOGERROR(format, ...) \
  BRAHMA_LOGGER->log(cpplogger::LOG_ERROR, format, __VA_ARGS__);
#define BRAHMA_LOGPRINT(format, ...) \
  BRAHMA_LOGGER->log(cpplogger::LOG_PRINT, format, __VA_ARGS__);

#define GOTCHA_BINDING_MACRO(fname)                                 \
  bindings[binding_index].name = #fname;                            \
  bindings[binding_index].wrapper_pointer = (void*)fname##_wrapper; \
  bindings[binding_index].function_handle = &fname##_handle;        \
  binding_index++;

#define GOTCHA_MACRO_TYPEDEF(name, ret, args, args_val, class_name) \
  typedef ret(*name##_fptr) args;                                   \
  inline ret name##_wrapper args {                                  \
    return class_name::get_instance()->name args_val;               \
  };                                                                \
  gotcha_wrappee_handle_t get_##name##_handle();
#define GOTCHA_MACRO(name, ret, args, args_val, class_name) \
  gotcha_wrappee_handle_t name##_handle;                    \
  gotcha_wrappee_handle_t get_##name##_handle() { return name##_handle; }

#define BRAHMA_WRAPPER(name) name##_wrapper;

#define BRAHMA_UNWRAPPED_FUNC(name, ret, args)                                 \
  BRAHMA_LOGINFO("[BRAHMA]\tFunction %s() not wrapped. Calling Original.\n",   \
                 #name);                                                       \
  name##_fptr name##_wrappee = (name##_fptr)gotcha_get_wrappee(name##_handle); \
  ret result = name##_wrappee args;

#define BRAHMA_MAP_OR_FAIL(func_)                               \
  auto __real_##func_ =                                         \
      (func_##_fptr)gotcha_get_wrappee(get_##func_##_handle()); \
  assert(__real_##func_ != NULL)

size_t count_posix();
size_t count_stdio();
size_t count_mpiio();
size_t count_mpi();
int update_posix(gotcha_binding_t*& bindings, size_t& binding_index);

int update_stdio(gotcha_binding_t*& bindings, size_t& binding_index);

int update_mpiio(gotcha_binding_t*& bindings, size_t& binding_index);
int update_mpi(gotcha_binding_t*& bindings, size_t& binding_index);

extern int brahma_bind_functions();
extern int brahma_get_binding(gotcha_binding_t*& bindings,
                              size_t& binding_count);

extern int free_bindings();

#endif  // BRAHMA_INTERCEPTOR_H
