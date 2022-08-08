//
// Created by hariharan on 8/8/22.
//

#ifndef TAILORFS_POSIX_H
#define TAILORFS_POSIX_H

#include <brahma/brahma.h>
#include <fcntl.h>
#include <memory>

namespace brahma {
class POSIXTest: public POSIX {
 private:
  static std::shared_ptr<POSIXTest> instance;
 public:
  POSIXTest(){}
  static std::shared_ptr<POSIXTest> get_instance() {
    if (instance == nullptr) {
      instance = std::make_shared<POSIXTest>();
      POSIX::set_instance(instance);
    }
    return instance;
  }
  int open(const char *pathname, int flags, mode_t mode) override {
    auto open_wrappee = (open_fptr) gotcha_get_wrappee(open_handle);
    int result = open_wrappee(pathname, flags, mode);
    fprintf(stderr, "[TailorFS]\topen(%s, %d, %u) = %d\n",
            pathname, flags, (unsigned int) mode, result);
    return result;
  }
};
std::shared_ptr<POSIXTest> POSIXTest::instance = nullptr;
}
#endif  // TAILORFS_POSIX_H
