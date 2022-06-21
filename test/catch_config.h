//
// Created by hariharan on 6/20/22.
//

#ifndef TAILORFS_CATCH_CONFIG_H
#define TAILORFS_CATCH_CONFIG_H

#define CATCH_CONFIG_RUNNER
#include <catch2/catch_all.hpp>
//#include <mpi.h>

namespace cl = Catch::Clara;

cl::Parser define_options();

int init(int* argc, char*** argv);
int finalize();

int main(int argc, char* argv[]) {
  Catch::Session session;
  auto cli = session.cli() | define_options();
  session.cli(cli);
  int returnCode = session.applyCommandLine(argc, argv);
  if (returnCode != 0) return returnCode;
  returnCode = init(&argc, &argv);
  if (returnCode != 0) return returnCode;
  int test_return_code = session.run();
  returnCode = finalize();
  if (returnCode != 0) return returnCode;
  exit(test_return_code);
}
#endif  // TAILORFS_CATCH_CONFIG_H
