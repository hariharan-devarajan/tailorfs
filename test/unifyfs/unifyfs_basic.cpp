#include <catch_config.h>
#include <test_utils.h>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <fcntl.h>
#include <mpi.h>
#include <unifyfs/unifyfs_api.h>

/**
 * Test data structures
 */
namespace tailorfs::test {
struct Arguments {
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
         cl::Opt(args.request_size, "request_size")["-r"]["--request_size"](
             "Transfer size used for performing I/O") |
         cl::Opt(args.iteration,
                 "iteration")["-i"]["--iteration"]("Number of Iterations");
}

/**
 * Test cases
 */
void buffer_io(Timer &init_time, Timer &finalize_time, Timer &open_time,
               Timer &close_time, Timer &write_time, Timer &async_write_time,
               Timer &flush_time, fs::path &bb, fs::path &pfs,
               unifyfs_cfg_option *options, int options_ct) {
  int rank, comm_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  fs::path unifyfs_path = "/unifyfs1";
  const char* val = unifyfs_path.c_str();
  unifyfs_handle fshdl;
  init_time.resumeTime();
  int rc =
      unifyfs_initialize(val, options, options_ct, &fshdl);
  init_time.pauseTime();
  REQUIRE(rc == UNIFYFS_SUCCESS);
  fs::path unifyfs_filename = unifyfs_path / args.filename;
  unifyfs_gfid gfid;
  if (rank ==0) {
    int create_flags = 0;
    open_time.resumeTime();
    rc = unifyfs_create(fshdl, create_flags, unifyfs_filename.c_str(), &gfid);
    open_time.pauseTime();
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (rank !=0) {
    int access_flags = O_RDWR;
    open_time.resumeTime();
    rc = unifyfs_open(fshdl, access_flags, unifyfs_filename.c_str(), &gfid);
    open_time.pauseTime();
  }
  REQUIRE(rc == UNIFYFS_SUCCESS);
  REQUIRE(gfid != UNIFYFS_INVALID_GFID);

  int max_buff = 100;
  auto num_req_to_buf = args.iteration >= max_buff ? max_buff : args.iteration;

  auto num_iter = ceil(args.iteration / num_req_to_buf);

  for (int iter = 0; iter < num_iter; ++iter) {
      auto write_data = std::vector<char>(args.request_size * num_req_to_buf, 'w');
      unifyfs_io_request write_req[num_req_to_buf];
      int j = 0;
      for (int i = iter * num_req_to_buf; i < iter * num_req_to_buf + num_req_to_buf; ++i) {
        write_req[i].op = UNIFYFS_IOREQ_OP_WRITE;
        write_req[i].gfid = gfid;
        write_req[i].nbytes = args.request_size;
        write_req[i].offset = i * args.request_size + (rank * args.request_size * args.iteration);
        write_req[i].user_buf = write_data.data() + (j * args.request_size);
        j++;
      }
      write_time.resumeTime();
      async_write_time.resumeTime();
      rc = unifyfs_dispatch_io(fshdl, num_req_to_buf, write_req);
      async_write_time.pauseTime();
      if (rc == UNIFYFS_SUCCESS) {
        int waitall = 1;
        rc = unifyfs_wait_io(fshdl, num_req_to_buf, write_req, waitall);
        if (rc == UNIFYFS_SUCCESS) {
          for (size_t i = 0; i < num_req_to_buf; i++) {
            REQUIRE(write_req[i].result.error == 0);
            REQUIRE(write_req[i].result.count == args.request_size);
          }
        }
      }
      write_time.pauseTime();
  }
  fs::path pfs_filename = pfs / args.filename;
  unifyfs_transfer_request mv_req;
  mv_req.src_path = unifyfs_filename.c_str();
  mv_req.dst_path = pfs_filename.c_str();
  mv_req.mode = UNIFYFS_TRANSFER_MODE_MOVE;
  mv_req.use_parallel = 1;
  flush_time.resumeTime();
  rc = unifyfs_dispatch_transfer(fshdl, 1, &mv_req);
  flush_time.pauseTime();
  REQUIRE(rc == UNIFYFS_SUCCESS);
  if (rc == UNIFYFS_SUCCESS) {
    int waitall = 1;
    flush_time.resumeTime();
    rc = unifyfs_wait_transfer(fshdl, 1, &mv_req, waitall);
    flush_time.pauseTime();
    if (rc == UNIFYFS_SUCCESS) {
      for (int i = 0; i < (int)1; i++) {
        REQUIRE(mv_req.result.error == 0);
      }
    }
  }
  flush_time.pauseTime();
  finalize_time.resumeTime();
  rc = unifyfs_finalize(fshdl);
  finalize_time.pauseTime();
  REQUIRE(rc == UNIFYFS_SUCCESS);
}

TEST_CASE("Write-Only", "[type=write-only][optimization=buffered_write]") {
    int rank, comm_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  const char *PFS_VAR = std::getenv("PFS_PATH");
  const char *BB_VAR = std::getenv("BB_PATH");
  const char *SHM_VAR = std::getenv("SHM_PATH");
  REQUIRE(PFS_VAR != nullptr);
  REQUIRE(BB_VAR != nullptr);
  REQUIRE(SHM_VAR != nullptr);
  fs::path pfs = fs::path(PFS_VAR) / "unifyfs" / "data";
  fs::path bb = fs::path(BB_VAR) / "unifyfs" / "data";
  fs::path shm = fs::path(SHM_VAR) / "unifyfs" / "data";
  fs::create_directories(pfs);
  fs::create_directories(bb);
  fs::create_directories(shm);

  Timer init_time, finalize_time, open_time, close_time, write_time,
      async_write_time, flush_time;
  char usecase[256];
  SECTION("storage.pfs") {
    strcpy(usecase, "pfs");
    fs::path filename = pfs / args.filename;
    open_time.resumeTime();
    MPI_File fh_orig;
    int status_orig = MPI_File_open(MPI_COMM_WORLD, filename.c_str(), MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &fh_orig);
    open_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
    for (int i = 0; i < args.iteration; ++i) {
      auto write_data = std::vector<char>(args.request_size, 'w');
      write_time.resumeTime();
      MPI_Status stat_orig;
      auto ret_orig = MPI_File_write_at_all(fh_orig, rank * args.request_size * args.iteration,
                                            write_data.data(), args.request_size,
                                            MPI_CHAR, &stat_orig);
      int written_bytes;
      MPI_Get_count(&stat_orig, MPI_CHAR, &written_bytes);
      write_time.pauseTime();
      REQUIRE(written_bytes == args.request_size);
    }
    close_time.resumeTime();
    status_orig = MPI_File_close(&fh_orig);
    close_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
//    if (rank == 0){
//    printf(
//        "Timing\t\t,n\t\t,rq\t\t,init\t\t,fin\t\t,open\t\t,close\t\t,write\t\t,"
//        "awrite\t\t,flush\t\t,case\n");
//    }
  }
  SECTION("storage.unifyfs.buffer") {
    strcpy(usecase, "unifyfs.buffer");
    const int options_c = 6;
    unifyfs_cfg_option chk_size[options_c];
    size_t io_size = args.request_size * args.iteration * comm_size;

    char logio_chunk_size[256];
    strcpy(logio_chunk_size, std::to_string(args.request_size).c_str());
    char logio_shmem_size[256];
    strcpy(logio_shmem_size, std::to_string(64*1024*1024*1024LL/comm_size).c_str());
    char logio_spill_size[256];
    strcpy(logio_spill_size, std::to_string(io_size).c_str());
    chk_size[0] = {.opt_name = "unifyfs.consistency", .opt_value = "LAMINATED"};
    chk_size[1] = {.opt_name = "client.fsync_persist", .opt_value = "off"};
    chk_size[2] = {.opt_name = "logio.chunk_size", .opt_value = logio_chunk_size};
    chk_size[3] = {.opt_name = "logio.shmem_size", .opt_value = logio_shmem_size};
    chk_size[4] = {.opt_name = "logio.spill_dir", .opt_value = bb.c_str()};
    chk_size[5] = {.opt_name = "logio.spill_size", .opt_value = logio_spill_size};
    buffer_io(init_time, finalize_time, open_time, close_time, write_time,
              async_write_time, flush_time, bb, pfs, chk_size, options_c);
  }

  double total_init = 0.0, total_finalize = 0.0, total_open= 0.0,
            total_close = 0.0, total_awrite = 0.0, total_write = 0.0, total_flush = 0.0;
  auto init_a = init_time.getElapsedTime();
    auto finalize_a = finalize_time.getElapsedTime();
    auto open_a = open_time.getElapsedTime();
    auto close_a = close_time.getElapsedTime();
    auto awrite_a = async_write_time.getElapsedTime();
    auto write_a = write_time.getElapsedTime();
    auto flush_a = flush_time.getElapsedTime();
    MPI_Reduce(&init_a, &total_init, 1, MPI_DOUBLE, MPI_SUM, 0,
               MPI_COMM_WORLD);
    MPI_Reduce(&finalize_a, &total_finalize, 1, MPI_DOUBLE, MPI_SUM, 0,
               MPI_COMM_WORLD);
    MPI_Reduce(&open_a, &total_open, 1, MPI_DOUBLE, MPI_SUM, 0,
               MPI_COMM_WORLD);
    MPI_Reduce(&close_a, &total_close, 1, MPI_DOUBLE, MPI_SUM, 0,
               MPI_COMM_WORLD);
    MPI_Reduce(&awrite_a, &total_awrite, 1, MPI_DOUBLE, MPI_SUM, 0,
               MPI_COMM_WORLD);
    MPI_Reduce(&write_a, &total_write, 1, MPI_DOUBLE, MPI_SUM, 0,
               MPI_COMM_WORLD);
    MPI_Reduce(&flush_a, &total_flush, 1, MPI_DOUBLE, MPI_SUM, 0,
               MPI_COMM_WORLD);
    if (rank == 0){
      printf("Timing\t\t,%ld\t,%ld\t,%f\t,%f\t,%f\t,%f\t,%f\t,%f\t,%f,\t%s\n", args.iteration,
             args.request_size, total_init/comm_size,
             total_finalize/comm_size, total_open/comm_size,
             total_close/comm_size, total_awrite/comm_size,
             total_write/comm_size,total_flush/comm_size,
             usecase);
  }
    MPI_Barrier(MPI_COMM_WORLD);
}
