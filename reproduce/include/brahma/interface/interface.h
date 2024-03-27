//
// Created by haridev on 8/28/22.
//

#ifndef TAILORFS_INTERFACE_H
#define TAILORFS_INTERFACE_H
#include <memory>
#include "interface_utility.h"
namespace brahma {
class Interface {
 protected:
  std::shared_ptr<InterfaceUtility> utility;
 public:
  Interface();
};
}
#endif  // TAILORFS_INTERFACE_H
