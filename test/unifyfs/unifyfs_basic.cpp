#include <catch_config.h>
#include <test_utils.h>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <mpi.h>
#include <unifyfs/unifyfs_api.h>

/**
 * Test data structures
 */
namespace tailorfs::test {
struct Arguments {
  fs::path pfs = "/home/haridev/pfs";
  fs::path shm = "/dev/shm/haridev";
  std::string filename = "test.dat";
  size_t request_size = 65536;
  size_t iteration = 64;
};
}  // namespace tailorfs::test

tailorfs::test::Arguments args;

/**
 * Overridden methods for catch
 */

int init(int* argc, char*** argv) {
  MPI_Init(argc, argv);
  return 0;
}
int finalize() {
  MPI_Finalize();
  return 0;
}

cl::Parser define_options() {
  return cl::Opt(args.filename, "filename")["-f"]["--filename"](
             "Filename to be use for I/O.") |
         cl::Opt(args.pfs, "pfs")["-p"]["--pfs"](
             "Directory used for performing I/O (default pfs)") |
         cl::Opt(args.shm, "shm")["-s"]["--shm"](
             "Directory used for performing I/O (default shm)") |
         cl::Opt(args.request_size, "request_size")["-r"]["--request_size"](
             "Transfer size used for performing I/O") |
         cl::Opt(args.iteration,
                 "iteration")["-i"]["--iteration"]("Number of Iterations");
}

/**
 * Test cases
 */
TEST_CASE("basic", "[type=basic]") {
  fs::create_directories("./ssd");
  fs::create_directories("./pfs");
  int my_rank, comm_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  int n_configs = 1;
  unifyfs_cfg_option chk_size = { .opt_name = "logio.chunk_size",
                                 .opt_value = "32768" };

  const char* unifyfs_prefix = "/unifyfs1";
  unifyfs_handle fshdl = UNIFYFS_INVALID_HANDLE;
  int rc = unifyfs_initialize(unifyfs_prefix, &chk_size, n_configs, &fshdl);
  REQUIRE(rc == UNIFYFS_SUCCESS);
  const char* filename = "/unifyfs1/file.0";
  int create_flags = 0;
  unifyfs_gfid gfid = UNIFYFS_INVALID_GFID;
  rc = unifyfs_create(fshdl, create_flags, filename, &gfid);
  REQUIRE(rc == UNIFYFS_SUCCESS);
  REQUIRE(gfid != UNIFYFS_INVALID_GFID);
  const char* destfs_prefix = "./pfs";
  size_t n_files = 1;
  const int PATHLEN_MAX=256;
  unifyfs_transfer_request my_reqs[n_files];
  char src_file[PATHLEN_MAX];
  char dst_file[PATHLEN_MAX];
  for (int i = 0; i < (int)n_files; i++) {
    snprintf(src_file, sizeof(src_file), "%s/file.%d", unifyfs_prefix, i);
    snprintf(dst_file, sizeof(src_file), "%s/file.%d", destfs_prefix, i);
    my_reqs[i].src_path = strdup(src_file);
    my_reqs[i].dst_path = strdup(dst_file);
    my_reqs[i].mode = UNIFYFS_TRANSFER_MODE_MOVE;
    my_reqs[i].use_parallel = 1;
  }

  rc = unifyfs_dispatch_transfer(fshdl, n_files, my_reqs);
  REQUIRE(rc == UNIFYFS_SUCCESS);
  if (rc == UNIFYFS_SUCCESS) {
    int waitall = 1;
    rc = unifyfs_wait_transfer(fshdl, n_files, my_reqs, waitall);
    if (rc == UNIFYFS_SUCCESS) {
      for (int i = 0; i < (int)n_files; i++) {
        REQUIRE(my_reqs[i].result.error == 0);
      }
    }
  }
  rc = unifyfs_finalize(fshdl);
  REQUIRE(rc == UNIFYFS_SUCCESS);
}

