//
// Created by hariharan on 8/8/22.
//

#ifndef TAILORFS_TAILORFS_H
#define TAILORFS_TAILORFS_H

#include <brahma/brahma.h>
#include <mimir/mimir.h>
#include <tailorfs/brahma/posix.h>
#include <tailorfs/brahma/stdio.h>
#include <tailorfs/macro.h>

#include "tailorfs/core/fsview_manager.h"
#include "tailorfs/core/singleton.h"
bool is_init();
void set_init(bool _init);


extern void __attribute__ ((constructor)) tailorfs_init(void);
extern void __attribute__ ((destructor)) tailorfs_fini(void);


#endif  // TAILORFS_TAILORFS_H
