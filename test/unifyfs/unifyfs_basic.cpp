#include <catch_config.h>
#include <test_utils.h>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <fcntl.h>
#include <mpi.h>
#include <unifyfs.h>
#include <unifyfs/unifyfs_api.h>

#include <tailorfs/unifyfs-stage-transfer.cpp>

/**
 * Test data structures
 */
namespace tailorfs::test {
struct Arguments {
  std::string filename = "test.dat";
  size_t request_size = 65536;
  size_t iteration = 64;
  int ranks_per_node = 1;
};
}  // namespace tailorfs::test

tailorfs::test::Arguments args;

/**
 * Overridden methods for catch
 */

int init(int *argc, char ***argv) {
  MPI_Init(argc, argv);
  int rank, comm_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  //    if(rank == 0) {
  //        fprintf(stderr, "attach to processes\n");
  //        getchar();
  //    }
  MPI_Barrier(MPI_COMM_WORLD);
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
                 "iteration")["-i"]["--iteration"]("Number of Iterations") |
         cl::Opt(args.ranks_per_node, "rpn")["-n"]["--rpn"]("Ranks per node");
}

/**
 * Test cases
 */
TEST_CASE("Write-Only", "[type=write-only][optimization=buffered_write]") {
  int rank, comm_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  const char *PFS_VAR = std::getenv("pfs");
  const char *BB_VAR = std::getenv("BBPATH");
  const char *SHM_VAR = "/dev/shm";
  char host[256];
  gethostname(host, 256);
  //fprintf(stderr, "BBPATH %s on hostname %s on rank %d\n", BB_VAR, host, rank);
  REQUIRE(PFS_VAR != nullptr);
  REQUIRE(BB_VAR != nullptr);
  REQUIRE(SHM_VAR != nullptr);
  args.filename = args.filename + "_" + std::to_string(comm_size);
  fs::path pfs = fs::path(PFS_VAR) / "unifyfs" / "data";
  fs::path bb = fs::path(BB_VAR) / "unifyfs" / "data";
  fs::path shm = fs::path(SHM_VAR) / "unifyfs" / "data";
  if (rank == 0) {
    fs::remove_all(pfs);
    fs::create_directories(pfs);
  }
  if (rank % args.ranks_per_node == 0) {
    fprintf(stderr, "rank %d on node %s with BB %s\n", rank, host, bb.string().c_str());
    fs::remove_all(bb);
    fs::remove_all(shm);
    fs::create_directories(bb);
    fs::create_directories(shm);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  REQUIRE(fs::exists(bb));
  REQUIRE(fs::exists(shm));
  REQUIRE(fs::exists(pfs));
  Timer init_time, finalize_time, open_time, close_time, write_time,
      async_write_time, flush_time;
  char usecase[256];
  SECTION("storage.pfs") {
    strcpy(usecase, "pfs");
    fs::path filename = pfs / args.filename;
    open_time.resumeTime();
    MPI_File fh_orig;
    int status_orig =
        MPI_File_open(MPI_COMM_WORLD, filename.c_str(),
                      MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &fh_orig);
    open_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
    for (int i = 0; i < args.iteration; ++i) {
      auto write_data = std::vector<char>(args.request_size, 'w');
      write_time.resumeTime();
      MPI_Status stat_orig;
      off_t base_offset = (off_t)rank * args.request_size * args.iteration;
      off_t relative_offset = i * args.request_size;
      auto ret_orig = MPI_File_write_at_all(
          fh_orig, base_offset + relative_offset, write_data.data(),
          args.request_size, MPI_CHAR, &stat_orig);
      int written_bytes;
      MPI_Get_count(&stat_orig, MPI_CHAR, &written_bytes);
      write_time.pauseTime();
      REQUIRE(written_bytes == args.request_size);
    }
    close_time.resumeTime();
    status_orig = MPI_File_close(&fh_orig);
    close_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
  }
  SECTION("storage.unifyfs.buffer") {
    strcpy(usecase, "unifyfs.buffer");
    const int options_ct = 6;
    unifyfs_cfg_option options[options_ct];
    unsigned long io_size = 512 * 1024 * 1024 * 1024LL / args.ranks_per_node;

    char logio_chunk_size[256];
    strcpy(logio_chunk_size, std::to_string(args.request_size).c_str());
    char logio_shmem_size[256];
    strcpy(logio_shmem_size,
           std::to_string(64 * 1024 * 1024 * 1024LL / args.ranks_per_node)
               .c_str());
    char logio_spill_size[256];
    strcpy(logio_spill_size, std::to_string(io_size).c_str());
    char logio_spill_dir[256];
    strcpy(logio_spill_dir, bb.string().c_str());
    options[0] = {.opt_name = "unifyfs.consistency",
                  .opt_value = "LAMINATED"};
    options[1] = {.opt_name = "client.fsync_persist",
                  .opt_value = "off"};
    options[2] = {.opt_name = "logio.chunk_size",
                  .opt_value = logio_chunk_size};
    options[3] = {.opt_name = "logio.shmem_size",
                  .opt_value = logio_shmem_size};
    options[4] = {.opt_name = "logio.spill_dir",
                  .opt_value = logio_spill_dir};
    options[5] = {.opt_name = "logio.spill_size",
                  .opt_value = logio_spill_size};
    fs::path unifyfs_path = "/unifyfs1";
    const char *val = unifyfs_path.c_str();
    unifyfs_handle fshdl;
    init_time.resumeTime();
    int rc = unifyfs_initialize(val, options, options_ct, &fshdl);
    init_time.pauseTime();
    REQUIRE(rc == UNIFYFS_SUCCESS);
    MPI_Barrier(MPI_COMM_WORLD);
    int rank, comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    //printf("My rank %d %d\n", rank, comm_size);
    fs::path unifyfs_filename = unifyfs_path / args.filename;
    unifyfs_gfid gfid;
    if (rank % args.ranks_per_node == 0) {
      int create_flags = 0;
      open_time.resumeTime();
      rc = unifyfs_create(fshdl, create_flags, unifyfs_filename.c_str(), &gfid);
      open_time.pauseTime();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank % args.ranks_per_node != 0) {
      int access_flags = O_WRONLY;
      open_time.resumeTime();
      rc = unifyfs_open(fshdl, access_flags, unifyfs_filename.c_str(), &gfid);
      open_time.pauseTime();
    }
    REQUIRE(rc == UNIFYFS_SUCCESS);
    REQUIRE(gfid != UNIFYFS_INVALID_GFID);

    /* Write data to file */
    int max_buff = 1000;
    int processed = 0;
    int j = 0;
    while( processed < args.iteration) {
      auto num_req_to_buf =
          args.iteration - processed >= max_buff ? max_buff : args.iteration - processed;
      auto write_data =
          std::vector<char>(args.request_size * num_req_to_buf, 'w');
      int write_req_ct = num_req_to_buf + 1;
      unifyfs_io_request write_req[num_req_to_buf + 2];

      for (int i = 0; i < num_req_to_buf; ++i) {
        write_req[i].op = UNIFYFS_IOREQ_OP_WRITE;
        write_req[i].gfid = gfid;
        write_req[i].nbytes = args.request_size;
        off_t base_offset = (off_t)rank * args.request_size * args.iteration;
        off_t relative_offset = j * args.request_size;
        write_req[i].offset = base_offset + relative_offset;
        write_req[i].user_buf = write_data.data() + (i * args.request_size);
        j++;
      }
      write_req[num_req_to_buf].op = UNIFYFS_IOREQ_OP_SYNC_META;
      write_req[num_req_to_buf].gfid = gfid;
      write_req[num_req_to_buf + 1].op = UNIFYFS_IOREQ_OP_SYNC_DATA;
      write_req[num_req_to_buf + 1].gfid = gfid;
      write_time.resumeTime();
      async_write_time.resumeTime();
      rc = unifyfs_dispatch_io(fshdl, write_req_ct, write_req);
      async_write_time.pauseTime();
      write_time.pauseTime();
      if (rc == UNIFYFS_SUCCESS) {
        int waitall = 1;
        write_time.resumeTime();
        rc = unifyfs_wait_io(fshdl, write_req_ct, write_req, waitall);
        write_time.pauseTime();
        if (rc == UNIFYFS_SUCCESS) {
          for (size_t i = 0; i < num_req_to_buf; i++) {
            REQUIRE(write_req[i].result.error == 0);
            REQUIRE(write_req[i].result.count == args.request_size);
          }
        }
      }
      processed += num_req_to_buf;
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
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
      REQUIRE(fs::file_size(pfs_filename) ==
              args.request_size * args.iteration * (comm_size));
      fprintf(stderr, "Size of file written %ld\n", fs::file_size(pfs_filename));
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }

  double total_init = 0.0, total_finalize = 0.0, total_open = 0.0,
         total_close = 0.0, total_awrite = 0.0, total_write = 0.0,
         total_flush = 0.0;
  auto init_a = init_time.getElapsedTime();
  auto finalize_a = finalize_time.getElapsedTime();
  auto open_a = open_time.getElapsedTime();
  auto close_a = close_time.getElapsedTime();
  auto awrite_a = async_write_time.getElapsedTime();
  auto write_a = write_time.getElapsedTime();
  auto flush_a = flush_time.getElapsedTime();
  MPI_Reduce(&init_a, &total_init, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&finalize_a, &total_finalize, 1, MPI_DOUBLE, MPI_SUM, 0,
             MPI_COMM_WORLD);
  MPI_Reduce(&open_a, &total_open, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&close_a, &total_close, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&awrite_a, &total_awrite, 1, MPI_DOUBLE, MPI_SUM, 0,
             MPI_COMM_WORLD);
  MPI_Reduce(&write_a, &total_write, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&flush_a, &total_flush, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if (rank == 0) {
    printf("Timing\t\t,%ld\t,%ld\t,%f\t,%f\t,%f\t,%f\t,%f\t,%f\t,%f,\t%s\n",
           args.iteration, args.request_size, total_init / comm_size,
           total_finalize / comm_size, total_open / comm_size,
           total_close / comm_size, total_awrite / comm_size,
           total_write / comm_size, total_flush / comm_size, usecase);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  fs::remove_all(pfs);
  fs::remove_all(bb);
  fs::remove_all(shm);
}
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(sig);
}
void deleteDirectoryContents(const fs::path& dir_path) {
  if (fs::exists(dir_path)){
    for (const auto& entry : fs::directory_iterator(dir_path))
      fs::remove_all(entry.path());
  }
}
TEST_CASE("Read-Only", "[type=read-only][optimization=buffered_read]") {
  int rank, comm_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

  args.filename = args.filename + "_" + std::to_string(comm_size);
  const char *PFS_VAR = std::getenv("pfs");
  const char *BB_VAR = std::getenv("BBPATH");
  const char *SHM_VAR = "/dev/shm";
  REQUIRE(PFS_VAR != nullptr);
  REQUIRE(BB_VAR != nullptr);
  REQUIRE(SHM_VAR != nullptr);
  fs::path pfs = fs::path(PFS_VAR) / "unifyfs" / "data";
  fs::path bb = fs::path(BB_VAR) / "unifyfs" / "data";
  fs::path shm = fs::path(SHM_VAR) / "unifyfs" / "data";
  if (rank == 0) {
    fprintf(stderr, "PFS_PATH %s in rank %d\n", pfs.c_str(), rank);
    deleteDirectoryContents(pfs);
    fs::create_directories(pfs);
  }
  if (rank % args.ranks_per_node == 0) {
    fprintf(stderr, "PFS_PATH %s in rank %d\n", pfs.string().c_str(), rank);
    fprintf(stderr, "BB_VAR %s in rank %d\n", bb.string().c_str(), rank);
    deleteDirectoryContents(bb);
    deleteDirectoryContents(shm);
    fs::create_directories(bb);
    fs::create_directories(shm);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  fs::path pfs_filename = pfs / args.filename;


  MPI_File fh_orig;
  int status_orig =
      MPI_File_open(MPI_COMM_WORLD, pfs_filename.c_str(),
                    MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &fh_orig);
  REQUIRE(status_orig == MPI_SUCCESS);
  auto write_data = std::vector<char>(args.request_size * args.iteration, 'w');
  MPI_Status stat_orig;
  off_t base_offset = (off_t)rank * args.request_size * args.iteration;
  auto ret_orig = MPI_File_write_at_all(
      fh_orig, base_offset, write_data.data(),
      args.request_size * args.iteration, MPI_CHAR, &stat_orig);
  int written_bytes;
  MPI_Get_count(&stat_orig, MPI_CHAR, &written_bytes);
  REQUIRE(written_bytes == args.request_size * args.iteration);
  status_orig = MPI_File_close(&fh_orig);
  REQUIRE(status_orig == MPI_SUCCESS);
  if (rank == 0) {
    REQUIRE(fs::file_size(pfs_filename) ==
            args.request_size * args.iteration * (comm_size));
    fprintf(stderr, "Size of file to be read %ld\n", fs::file_size(pfs_filename));
  }
  MPI_Barrier(MPI_COMM_WORLD);
  Timer init_time, finalize_time, open_time, close_time, read_time,
      prefetch_time;
  char usecase[256];
  SECTION("storage.pfs") {
    strcpy(usecase, "pfs");

    open_time.resumeTime();
    MPI_File fh_orig;
    int status_orig = MPI_File_open(MPI_COMM_WORLD, pfs_filename.c_str(),
                                    MPI_MODE_RDWR, MPI_INFO_NULL, &fh_orig);
    open_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
    for (int i = 0; i < args.iteration; ++i) {
      auto read_data = std::vector<char>(args.request_size, 'r');
      read_time.resumeTime();
      MPI_Status stat_orig;
      off_t base_offset = (off_t)rank * args.request_size * args.iteration;
      off_t relative_offset = i * args.request_size;
      auto ret_orig = MPI_File_read_at(
          fh_orig, base_offset + relative_offset, read_data.data(),
          args.request_size, MPI_CHAR, &stat_orig);
      int read_bytes;
      MPI_Get_count(&stat_orig, MPI_CHAR, &read_bytes);
      read_time.pauseTime();
      REQUIRE(read_bytes == args.request_size);
    }
    close_time.resumeTime();
    status_orig = MPI_File_close(&fh_orig);
    close_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
  }
  SECTION("storage.unifyfs.buffer") {
    strcpy(usecase, "unifyfs.buffer");
    size_t io_size = 256 * 1024 * 1024 * 1024LL / args.ranks_per_node;

    char logio_chunk_size[256];
    strcpy(logio_chunk_size, std::to_string(args.request_size).c_str());
    char logio_shmem_size[256];
    strcpy(logio_shmem_size,
           std::to_string(64 * 1024 * 1024 * 1024LL / args.ranks_per_node)
               .c_str());
    char logio_spill_size[256];
    strcpy(logio_spill_size, std::to_string(io_size).c_str());
    char logio_spill_dir[256];
    strcpy(logio_spill_dir, bb.string().c_str());
    unifyfs_cfg_option options_b[5];
    options_b[0] = {.opt_name = "logio.spill_dir", .opt_value = logio_spill_dir};
    options_b[1] = {.opt_name = "logio.chunk_size",
                    .opt_value = logio_chunk_size};
    options_b[2] = {.opt_name = "logio.spill_size",
                    .opt_value = logio_spill_size};
    options_b[3] = {.opt_name = "logio.shmem_size",
                  .opt_value = "0"};
    options_b[4] = {.opt_name = "client.local_extents",
                    .opt_value = "on"};

    fs::path unifyfs_path = "/unifyfs1";
    //    fprintf(stderr, "setting options by rank %d\n", rank);
    char *val = strdup(unifyfs_path.c_str());
    unifyfs_handle fshdl;
    init_time.resumeTime();
    int rc = unifyfs_initialize(val, options_b,5 , &fshdl);
    init_time.pauseTime();
    REQUIRE(rc == UNIFYFS_SUCCESS);
    // fprintf(stderr, "unifyfs initialized by rank %d\n", rank);
    unifyfs_gfid gfid;
    fs::path unifyfs_filename = unifyfs_path / args.filename;
    char unifyfs_filename_charp[256];
    strcpy(unifyfs_filename_charp, unifyfs_filename.c_str());
    unifyfs_stage ctx;
    ctx.checksum = 0;
    ctx.data_dist = UNIFYFS_STAGE_DATA_BALANCED;
    ctx.mode = UNIFYFS_STAGE_MODE_PARALLEL;
    ctx.mountpoint = unifyfs_filename_charp;
    ctx.rank = rank;
    ctx.total_ranks = comm_size;
    ctx.fshdl = fshdl;
    prefetch_time.resumeTime();
    rc = unifyfs_stage_transfer(&ctx, 1, pfs_filename.c_str(),
                                unifyfs_filename.c_str());
    prefetch_time.pauseTime();
    REQUIRE(rc == UNIFYFS_SUCCESS);

    // fprintf(stderr, "prefetch done by rank %d\n", rank);
    int access_flags = O_RDONLY;
    open_time.resumeTime();
    rc = unifyfs_open(fshdl, access_flags, unifyfs_filename.c_str(), &gfid);
    open_time.pauseTime();

    REQUIRE(rc == UNIFYFS_SUCCESS);
    REQUIRE(gfid != UNIFYFS_INVALID_GFID);
    int prefetch_ct = 1;
    unifyfs_io_request prefetch_sync[2];
    prefetch_sync[0].op = UNIFYFS_IOREQ_OP_SYNC_META;
    prefetch_sync[0].gfid = gfid;
    prefetch_sync[1].op = UNIFYFS_IOREQ_OP_SYNC_DATA;
    prefetch_sync[1].gfid = gfid;
    prefetch_time.resumeTime();
    rc = unifyfs_dispatch_io(fshdl, prefetch_ct, prefetch_sync);
    prefetch_time.pauseTime();
    if (rc == UNIFYFS_SUCCESS) {
      int waitall = 1;
      prefetch_time.resumeTime();
      rc = unifyfs_wait_io(fshdl, prefetch_ct, prefetch_sync, waitall);
      prefetch_time.pauseTime();
      if (rc == UNIFYFS_SUCCESS) {
        for (size_t i = 0; i < prefetch_ct; i++) {
          if (prefetch_sync[i].result.error != 0)
            fprintf(stderr,
                    "UNIFYFS ERROR: "
                    "OP_READ req failed - %s",
                    strerror(prefetch_sync[i].result.error));
          REQUIRE(prefetch_sync[i].result.error == 0);
        }
      }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    for (off_t i = 0; i < args.iteration; ++i) {
      auto read_data = std::vector<char>(args.request_size, 'r');
      unifyfs_io_request read_req;
      read_req.op = UNIFYFS_IOREQ_OP_READ;
      read_req.gfid = gfid;
      read_req.nbytes = args.request_size;
      off_t base_offset = (off_t)rank * args.request_size * args.iteration;
      off_t relative_offset = i * args.request_size;
      read_req.offset = base_offset + relative_offset;
      read_req.user_buf = read_data.data();
      read_time.resumeTime();
      rc = unifyfs_dispatch_io(fshdl, 1, &read_req);
      read_time.pauseTime();
      if (rc == UNIFYFS_SUCCESS) {
        int waitall = 1;
        read_time.resumeTime();
        rc = unifyfs_wait_io(fshdl, 1, &read_req, waitall);
        read_time.pauseTime();
        if (rc == UNIFYFS_SUCCESS) {
          if (read_req.result.error != 0)
            fprintf(stderr,
                    "UNIFYFS ERROR: "
                    "OP_READ req failed - %s\n",
                    strerror(read_req.result.error));
          REQUIRE(read_req.result.error == 0);
          if (read_req.result.count != args.request_size)
            fprintf(stderr,
                    "UNIFYFS ERROR: "
                    "OP_READ req failed - rank %d, iter %d, and read only %d of %d with offset %lu\n",
                    rank, i, read_req.result.count, args.request_size, read_req.offset);
          REQUIRE(read_req.result.count == args.request_size);
        }
      }
    }
    finalize_time.resumeTime();
    rc = unifyfs_finalize(fshdl);
    finalize_time.pauseTime();
    REQUIRE(rc == UNIFYFS_SUCCESS);
  }

  double total_init = 0.0, total_finalize = 0.0, total_open = 0.0,
         total_close = 0.0, total_read = 0.0, total_prefetch = 0.0;
  auto init_a = init_time.getElapsedTime();
  auto finalize_a = finalize_time.getElapsedTime();
  auto open_a = open_time.getElapsedTime();
  auto close_a = close_time.getElapsedTime();
  auto read_a = read_time.getElapsedTime();
  auto prefetch_a = prefetch_time.getElapsedTime();
  MPI_Reduce(&init_a, &total_init, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&finalize_a, &total_finalize, 1, MPI_DOUBLE, MPI_SUM, 0,
             MPI_COMM_WORLD);
  MPI_Reduce(&open_a, &total_open, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&close_a, &total_close, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&read_a, &total_read, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&prefetch_a, &total_prefetch, 1, MPI_DOUBLE, MPI_SUM, 0,
             MPI_COMM_WORLD);
  if (rank == 0) {
    printf("Timing\t\t,%ld\t,%ld\t,%f\t,%f\t,%f\t,%f\t,%f\t,%f,\t%s\n",
           args.iteration, args.request_size, total_init / comm_size,
           total_finalize / comm_size, total_open / comm_size,
           total_close / comm_size, total_read / comm_size,
           total_prefetch / comm_size, usecase);
  }
  MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE("Producer-Consumer", "[type=pc][optimization=buffered_io]") {
  int rank, comm_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

  args.filename = args.filename + "_" + std::to_string(comm_size);
  REQUIRE(comm_size > 1);
  REQUIRE(comm_size % 2 == 0);
  const char *PFS_VAR = std::getenv("pfs");
  const char *BB_VAR = std::getenv("BBPATH");
  const char *SHM_VAR = "/dev/shm";
  REQUIRE(PFS_VAR != nullptr);
  REQUIRE(BB_VAR != nullptr);
  REQUIRE(SHM_VAR != nullptr);
  fs::path pfs = fs::path(PFS_VAR) / "unifyfs" / "data";
  fs::path bb = fs::path(BB_VAR) / "unifyfs" / "data";
  fs::path shm = fs::path(SHM_VAR) / "unifyfs" / "data";
  if (rank == 0) {
    fs::remove_all(pfs);
    fs::create_directories(pfs);
  }
  if (rank % args.ranks_per_node == 0) {
    fs::remove_all(bb);
    fs::remove_all(shm);
    fs::create_directories(bb);
    fs::create_directories(shm);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  fs::path pfs_filename = pfs / args.filename;
  Timer init_time, finalize_time, open_time, close_time, write_time, read_time;
  char usecase[256];
  int num_producers = comm_size / 2;
  bool is_producer = rank / num_producers == 0;
  MPI_Comm producer_comm, consumer_comm;
  MPI_Comm_split(MPI_COMM_WORLD, is_producer, rank, &producer_comm);
  MPI_Comm_split(MPI_COMM_WORLD, !is_producer, rank, &consumer_comm);
  SECTION("storage.pfs") {
    strcpy(usecase, "pfs");
    open_time.resumeTime();
    MPI_File fh_orig;
    int status_orig = -1;
    if (is_producer) {
      status_orig = MPI_File_open(producer_comm, pfs_filename.c_str(),
                                  MPI_MODE_WRONLY | MPI_MODE_CREATE,
                                  MPI_INFO_NULL, &fh_orig);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (!is_producer) {
      status_orig = MPI_File_open(consumer_comm, pfs_filename.c_str(),
                                  MPI_MODE_RDONLY, MPI_INFO_NULL, &fh_orig);
    }
    open_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
    if (is_producer) {
      for (int i = 0; i < args.iteration; ++i) {
        int producer_rank;
        MPI_Comm_rank(producer_comm, &producer_rank);
        auto write_data = std::vector<char>(args.request_size, 'w');
        write_time.resumeTime();
        MPI_Status stat_orig;

        off_t base_offset = (off_t)producer_rank * args.request_size * args.iteration;
        off_t relative_offset = i * args.request_size;
        auto ret_orig = MPI_File_write_at_all(
            fh_orig, base_offset + relative_offset,
            write_data.data(), args.request_size, MPI_CHAR, &stat_orig);
        int written_bytes;
        MPI_Get_count(&stat_orig, MPI_CHAR, &written_bytes);
        write_time.pauseTime();
        REQUIRE(written_bytes == args.request_size);
      }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (!is_producer) {
      int consumer_rank;
      MPI_Comm_rank(consumer_comm, &consumer_rank);
      for (int i = 0; i < args.iteration; ++i) {
        auto read_data = std::vector<char>(args.request_size, 'r');
        read_time.resumeTime();
        MPI_Status stat_orig;
        off_t base_offset = (off_t)consumer_rank * args.request_size * args.iteration;
        off_t relative_offset = i * args.request_size;
        auto ret_orig = MPI_File_read_at_all(
            fh_orig, base_offset + relative_offset,
            read_data.data(), args.request_size, MPI_CHAR, &stat_orig);
        int read_bytes;
        MPI_Get_count(&stat_orig, MPI_CHAR, &read_bytes);
        read_time.pauseTime();
        REQUIRE(read_bytes == args.request_size);
        for (auto &val : read_data) {
          REQUIRE(val == 'w');
        }
      }
    }
    close_time.resumeTime();
    status_orig = MPI_File_close(&fh_orig);
    close_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
  }
  SECTION("storage.unifyfs.buffer") {
    strcpy(usecase, "unifyfs.buffer");
    unifyfs_handle fshdl;
    int rc;
    fs::path unifyfs_path = "/unifyfs1";
    /* Initialize unifyfs */

    const int options_ct = 6;
    unifyfs_cfg_option options[options_ct];
    size_t io_size = args.request_size * args.iteration * comm_size;

    char logio_chunk_size[256];
    strcpy(logio_chunk_size, std::to_string(args.request_size).c_str());
    char logio_shmem_size[256];
    strcpy(logio_shmem_size,
           std::to_string(64 * 1024 * 1024 * 1024LL / args.ranks_per_node)
               .c_str());
    char logio_spill_size[256];
    strcpy(logio_spill_size, std::to_string(io_size).c_str());
    char logio_spill_dir[256];
    strcpy(logio_spill_dir, bb.string().c_str());
    options[0] = {.opt_name = "unifyfs.consistency", .opt_value = "LAMINATED"};
    options[1] = {.opt_name = "client.fsync_persist", .opt_value = "off"};
    options[2] = {.opt_name = "logio.chunk_size",
                  .opt_value = logio_chunk_size};
    options[3] = {.opt_name = "logio.shmem_size",
                  .opt_value = "0"};
    options[4] = {.opt_name = "logio.spill_dir", .opt_value = logio_spill_dir};
    options[5] = {.opt_name = "logio.spill_size",
                  .opt_value = logio_spill_size};
    const char *val = unifyfs_path.c_str();

    init_time.resumeTime();
    rc = unifyfs_initialize(val, options, options_ct, &fshdl);
    init_time.pauseTime();

    fs::path unifyfs_filename = unifyfs_path / args.filename;

    if (is_producer) {
      int producer_rank;
      MPI_Comm_rank(producer_comm, &producer_rank);

      unifyfs_gfid gfid;
      /* Create or Open file for production */

      if (producer_rank == 0) {
        int create_flags = 0;
        open_time.resumeTime();
        rc = unifyfs_create(fshdl, create_flags, unifyfs_filename.c_str(),
                            &gfid);
        open_time.pauseTime();
      }
      MPI_Barrier(producer_comm);
      if (producer_rank != 0) {
        int access_flags = O_WRONLY;
        open_time.resumeTime();
        rc = unifyfs_open(fshdl, access_flags, unifyfs_filename.c_str(), &gfid);
        open_time.pauseTime();
      }
      REQUIRE(rc == UNIFYFS_SUCCESS);
      REQUIRE(gfid != UNIFYFS_INVALID_GFID);

      /* Write data to file */
      int max_buff = 1000;
      int processed = 0;
      int j = 0;
      while( processed < args.iteration) {
        auto num_req_to_buf =
            args.iteration - processed >= max_buff ? max_buff : args.iteration - processed;
        auto write_data =
            std::vector<char>(args.request_size * num_req_to_buf, 'w');
        int write_req_ct = num_req_to_buf +1;
        unifyfs_io_request write_req[num_req_to_buf + 2];

        for (int i = 0; i < num_req_to_buf; ++i) {
          write_req[i].op = UNIFYFS_IOREQ_OP_WRITE;
          write_req[i].gfid = gfid;
          write_req[i].nbytes = args.request_size;
          off_t base_offset = (off_t)producer_rank * args.request_size * args.iteration;
          off_t relative_offset = j * args.request_size;
          write_req[i].offset = base_offset + relative_offset;
          write_req[i].user_buf = write_data.data() + (i * args.request_size);
          j++;
        }
        write_req[num_req_to_buf].op = UNIFYFS_IOREQ_OP_SYNC_META;
        write_req[num_req_to_buf].gfid = gfid;
        write_req[num_req_to_buf + 1].op = UNIFYFS_IOREQ_OP_SYNC_DATA;
        write_req[num_req_to_buf + 1].gfid = gfid;
        write_time.resumeTime();
        rc = unifyfs_dispatch_io(fshdl, write_req_ct, write_req);
        write_time.pauseTime();
        if (rc == UNIFYFS_SUCCESS) {
          int waitall = 1;
          write_time.resumeTime();
          rc = unifyfs_wait_io(fshdl, write_req_ct, write_req, waitall);
          write_time.pauseTime();
          if (rc == UNIFYFS_SUCCESS) {
            for (size_t i = 0; i < num_req_to_buf; i++) {
              REQUIRE(write_req[i].result.error == 0);
              REQUIRE(write_req[i].result.count == args.request_size);
            }
          }
        }
        processed += num_req_to_buf;
      }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (!is_producer) {
      int consumer_rank;
      MPI_Comm_rank(consumer_comm, &consumer_rank);
      unifyfs_gfid gfid;
      /* Open file for consumption */

      int access_flags = O_RDONLY;
      open_time.resumeTime();
      rc = unifyfs_open(fshdl, access_flags, unifyfs_filename.c_str(), &gfid);
      open_time.pauseTime();
      REQUIRE(rc == UNIFYFS_SUCCESS);
      REQUIRE(gfid != UNIFYFS_INVALID_GFID);

      int prefetch_ct = 1;
      unifyfs_io_request read_sync[2];
      read_sync[0].op = UNIFYFS_IOREQ_OP_SYNC_META;
      read_sync[0].gfid = gfid;
      read_sync[1].op = UNIFYFS_IOREQ_OP_SYNC_DATA;
      read_sync[1].gfid = gfid;
      read_time.resumeTime();
      rc = unifyfs_dispatch_io(fshdl, prefetch_ct, read_sync);
      read_time.pauseTime();
      if (rc == UNIFYFS_SUCCESS) {
        int waitall = 1;
        read_time.resumeTime();
        rc = unifyfs_wait_io(fshdl, prefetch_ct, read_sync, waitall);
        read_time.pauseTime();
        if (rc == UNIFYFS_SUCCESS) {
          for (size_t i = 0; i < prefetch_ct; i++) {
            if (read_sync[i].result.error != 0)
              fprintf(stderr,
                      "UNIFYFS ERROR: "
                      "OP_READ req failed - %s",
                      strerror(read_sync[i].result.error));
            REQUIRE(read_sync[i].result.error == 0);
          }
        }
      }

      /* Read data from file */
      for (int i = 0; i < args.iteration; ++i) {
        auto read_data = std::vector<char>(args.request_size, 'r');
        unifyfs_io_request read_req;
        read_req.op = UNIFYFS_IOREQ_OP_READ;
        read_req.gfid = gfid;
        read_req.nbytes = args.request_size;
        off_t base_offset = (off_t)consumer_rank * args.request_size * args.iteration;
        off_t relative_offset = i * args.request_size;
        read_req.offset = base_offset + relative_offset;
        read_req.user_buf = read_data.data();
        read_time.resumeTime();
        rc = unifyfs_dispatch_io(fshdl, 1, &read_req);
        read_time.pauseTime();
        if (rc == UNIFYFS_SUCCESS) {
          int waitall = 1;
          read_time.resumeTime();
          rc = unifyfs_wait_io(fshdl, 1, &read_req, waitall);
          read_time.pauseTime();
          if (rc == UNIFYFS_SUCCESS) {
            if (read_req.result.error != 0)
              fprintf(stderr,
                      "UNIFYFS ERROR: "
                      "OP_READ req failed - %s\n",
                      strerror(read_req.result.error));
            REQUIRE(read_req.result.error == 0);
            if (read_req.result.count != args.request_size)
              fprintf(stderr,
                      "UNIFYFS ERROR: "
                      "OP_READ req failed - rank %d consumer_rank %d, iter %d, and read only %d of %d with offset %lu\n",
                      rank, consumer_rank, i, read_req.result.count, args.request_size, read_req.offset);
            REQUIRE(read_req.result.count == args.request_size);
          }
        }
      }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    // REQUIRE(args.iteration == successful_reads);
    finalize_time.resumeTime();
    rc = unifyfs_finalize(fshdl);
    finalize_time.pauseTime();
    REQUIRE(rc == UNIFYFS_SUCCESS);
  }
  double total_init = 0.0, total_finalize = 0.0, total_open = 0.0,
         total_close = 0.0, total_read = 0.0, total_write = 0.0;
  auto init_a = init_time.getElapsedTime();
  auto finalize_a = finalize_time.getElapsedTime();
  auto open_a = open_time.getElapsedTime();
  auto close_a = close_time.getElapsedTime();
  auto read_a = read_time.getElapsedTime();
  auto write_a = write_time.getElapsedTime();
  MPI_Reduce(&init_a, &total_init, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&finalize_a, &total_finalize, 1, MPI_DOUBLE, MPI_SUM, 0,
             MPI_COMM_WORLD);
  MPI_Reduce(&open_a, &total_open, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&close_a, &total_close, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&read_a, &total_read, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(&write_a, &total_write, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if (rank == 0) {
    printf("Timing\t\t,%ld\t,%ld\t,%f\t,%f\t,%f\t,%f\t,%f\t,%f\t,%s\n",
           args.iteration, args.request_size, total_init / comm_size,
           total_finalize / comm_size, total_open / comm_size,
           total_close / comm_size, total_write / comm_size,
           total_read / comm_size, usecase);
  }
  MPI_Barrier(MPI_COMM_WORLD);
}