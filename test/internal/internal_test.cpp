//
// Created by haridev on 9/8/22.
//
#include <catch_config.h>
#include <mpi.h>
#include <test_utils.h>
#include <tailorfs/tailorfs.h>

#include <experimental/filesystem>
#include <fstream>

namespace tailorfs::test {}

namespace fs = std::experimental::filesystem;
namespace tt = tailorfs::test;

/**
 * Test data structures
 */
namespace tailorfs::test {
struct Arguments {
  int num_of_ops = 1000;
  int debug = false;
};
struct Info {
  int rank, comm_size;
};
}// namespace tailorfs::test
tailorfs::test::Arguments args;
tailorfs::test::Info info;

int init(int *argc, char ***argv) {
  MPI_Init(argc, argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &info.rank);
  MPI_Comm_size(MPI_COMM_WORLD, &info.comm_size);
  char *tailorfs_debug = getenv("TAILORFS_DEBUG");
  if (tailorfs_debug != nullptr) {
    if (strcmp(tailorfs_debug, "1") == 0 && info.rank == 0) {
      std::string sp;
      std::ifstream("/proc/self/cmdline") >> sp;
      std::replace(sp.begin(), sp.end() - 1, '\000', ' ');
      fprintf(stderr, "Connect to pid %d %s\n", getpid(), sp.c_str());
      fflush(stderr);
      getchar();
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  return 0;
}

int finalize() {
  MPI_Finalize();
  return 0;
}
cl::Parser define_options() {
  auto opts = cl::Opt(args.num_of_ops, "num_of_ops")["--num_of_ops"](
      "num of ops.") |
              cl::Opt(args.debug, "debug")["--debug"]("Enable Debug");
  return opts;
}

/**
 * Helper functions
 */
 int pretest() { return 0;}
 int posttest() {return 0;}

#define init_fini(type, timer) \
  FSID timer##_id;  \
  timer##_id._type = FSViewType::type; \
  timer##_id._id = id_val++; \
  type##Init timer##_init_args; \
  timer##_init_time.resumeTime(); \
  type##FSVIEW(timer##_id)->Initialize(timer##_init_args); \
  timer##_init_time.pauseTime(); \
  type##Finalize timer##_fini_args; \
  timer##_fin_time.resumeTime(); \
  type##FSVIEW(timer##_id)->Finalize(timer##_fini_args); \
  timer##_fin_time.pauseTime();
 TEST_CASE("anatomy", CONVERT_VAL(num_of_ops, args.num_of_ops)) {
   pretest();
   Timer tfs_init_time, tfs_fin_time;
   Timer mpiio_init_time, mpiio_fin_time;
   Timer posix_init_time, posix_fin_time;
   Timer stdio_init_time, stdio_fin_time;
   tfs_init_time.resumeTime();
   tfs_init();
   tfs_init_time.pauseTime();
   uint8_t id_val = 0;
   for (int iteration =0; iteration < args.num_of_ops; ++iteration) {
     init_fini(MPIIO, mpiio);
     init_fini(POSIX, posix);
     init_fini(STDIO, stdio);
    }
    tfs_fin_time.resumeTime();
    tfs_finalize();
    tfs_fin_time.pauseTime();
    AGGREGATE_TIME(tfs_init);
    AGGREGATE_TIME(tfs_fin);
    AGGREGATE_TIME(mpiio_init);
    AGGREGATE_TIME(mpiio_fin);
    AGGREGATE_TIME(posix_init);
    AGGREGATE_TIME(posix_fin);
    AGGREGATE_TIME(stdio_init);
    AGGREGATE_TIME(stdio_fin);
    if (info.rank == 0) {
      PRINT_MSG(
          "%10s,%10d,"
          "%10.6f,%10.6f,"
          "%10.6f,%10.6f,"
          "%10.6f,%10.6f,"
          "%10.6f,%10.6f\n",
          "Timing", args.num_of_ops,
          total_tfs_init / info.comm_size, total_tfs_fin / info.comm_size,
          total_mpiio_init / info.comm_size, total_mpiio_fin / info.comm_size,
          total_posix_init / info.comm_size, total_posix_fin / info.comm_size,
          total_stdio_init / info.comm_size, total_stdio_fin / info.comm_size);
    }
  posttest();
 }