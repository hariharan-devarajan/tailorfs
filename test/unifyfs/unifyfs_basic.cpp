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
TEST_CASE("basic", "[type=basic]") {
  char ssd[256], pfs[256];
  sprintf(ssd, "%s/unifyfs/ssd", std::getenv("HOME"));
  sprintf(pfs, "%s/unifyfs/pfs", std::getenv("HOME"));
  fs::create_directories(ssd);
  fs::create_directories(pfs);
  int my_rank, comm_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  int n_configs = 2;
  unifyfs_cfg_option chk_size[2];
  auto chksize = 32768;
  chk_size[0] = {.opt_name = "logio.chunk_size",
                 .opt_value = std::to_string(chksize).c_str()};
  chk_size[1] = {.opt_name = "logio.spill_dir", .opt_value = ssd};
  const char *unifyfs_prefix = "/unifyfs1";
  unifyfs_handle fshdl;
  int rc = unifyfs_initialize(unifyfs_prefix, chk_size, n_configs, &fshdl);
  REQUIRE(rc == UNIFYFS_SUCCESS);
  const char *filename = "/unifyfs1/file.0";
  int create_flags = 0;
  unifyfs_gfid gfid;
  rc = unifyfs_create(fshdl, create_flags, filename, &gfid);
  REQUIRE(rc == UNIFYFS_SUCCESS);
  REQUIRE(gfid != UNIFYFS_INVALID_GFID);
  size_t n_chks = args.iteration;
  size_t chunk_size = args.request_size;
  auto data_buf = std::vector<char>(n_chks * chunk_size, 'w');
  size_t block_size = chunk_size * comm_size;
  size_t n_reqs = n_chks + 1;
  unifyfs_io_request my_reqs[n_reqs];
  for (size_t i = 0; i < n_chks; i++) {
    my_reqs[i].op = UNIFYFS_IOREQ_OP_WRITE;
    my_reqs[i].gfid = gfid;
    my_reqs[i].nbytes = chunk_size;
    my_reqs[i].offset = (off_t)((i * block_size) + (my_rank * chunk_size));
    my_reqs[i].user_buf = data_buf.data() + (i * chunk_size);
  }
  my_reqs[n_chks].op = UNIFYFS_IOREQ_OP_SYNC_META;
  my_reqs[n_chks].gfid = gfid;

  rc = unifyfs_dispatch_io(fshdl, n_reqs, my_reqs);
  if (rc == UNIFYFS_SUCCESS) {
    int waitall = 1;
    rc = unifyfs_wait_io(fshdl, n_reqs, my_reqs, waitall);
    if (rc == UNIFYFS_SUCCESS) {
      for (size_t i = 0; i < n_reqs; i++) {
        assert(my_reqs[i].result.error == 0);
      }
    }
  }

  auto read_buf = std::vector<char>(n_chks * chunk_size, 'w');
  block_size = n_chks * chunk_size * comm_size;
  n_reqs = 1;
  unifyfs_io_request my_read_reqs[n_reqs];
  for (size_t i = 0; i < n_reqs; i++) {
    my_read_reqs[i].op = UNIFYFS_IOREQ_OP_READ;
    my_read_reqs[i].gfid = gfid;
    my_read_reqs[i].nbytes = block_size;
    my_read_reqs[i].offset = (off_t)((i * block_size));
    my_read_reqs[i].user_buf = data_buf.data();
  }
  rc = unifyfs_dispatch_io(fshdl, n_reqs, my_read_reqs);
  if (rc == UNIFYFS_SUCCESS) {
    int waitall = 1;
    rc = unifyfs_wait_io(fshdl, n_reqs, my_read_reqs, waitall);
    if (rc == UNIFYFS_SUCCESS) {
      for (size_t i = 0; i < n_reqs; i++) {
        REQUIRE(my_read_reqs[i].result.error == 0);
        REQUIRE(my_read_reqs[i].result.count == block_size);
        for (int i = 0; i < my_read_reqs[i].result.count; ++i) {
          REQUIRE(((char *)my_read_reqs[i].user_buf)[i] == 'w');
        }
      }
    }
  }
  const char *destfs_prefix = pfs;
  size_t n_files = 1;
  const int PATHLEN_MAX = 256;
  unifyfs_transfer_request mv_reqs[n_files];
  char src_file[PATHLEN_MAX];
  char dst_file[PATHLEN_MAX];
  for (int i = 0; i < (int)n_files; i++) {
    snprintf(src_file, sizeof(src_file), "%s/file.%d", unifyfs_prefix, i);
    snprintf(dst_file, sizeof(src_file), "%s/file.%d", destfs_prefix, i);
    mv_reqs[i].src_path = strdup(src_file);
    mv_reqs[i].dst_path = strdup(dst_file);
    mv_reqs[i].mode = UNIFYFS_TRANSFER_MODE_MOVE;
    mv_reqs[i].use_parallel = 1;
  }

  rc = unifyfs_dispatch_transfer(fshdl, n_files, mv_reqs);
  REQUIRE(rc == UNIFYFS_SUCCESS);
  if (rc == UNIFYFS_SUCCESS) {
    int waitall = 1;
    rc = unifyfs_wait_transfer(fshdl, n_files, mv_reqs, waitall);
    if (rc == UNIFYFS_SUCCESS) {
      for (int i = 0; i < (int)n_files; i++) {
        REQUIRE(my_reqs[i].result.error == 0);
      }
    }
  }
  rc = unifyfs_finalize(fshdl);
  REQUIRE(rc == UNIFYFS_SUCCESS);
}

void buffer_io(Timer &init_time, Timer &finalize_time, Timer &open_time,
               Timer &close_time, Timer &write_time, Timer &async_write_time,
               Timer &flush_time, fs::path &bb, fs::path &pfs,
               unifyfs_cfg_option *options, int options_ct, char *write_buf) {
  const int n_configs = options_ct + 1;
  unifyfs_cfg_option chk_size[n_configs];
  chk_size[0] = {.opt_name = "logio.spill_dir", .opt_value = bb.c_str()};
  for (int i = 1; i < n_configs; ++i) {
    chk_size[i] = options[i - 1];
  }
  fs::path unifyfs_path = "/unifyfs1";
  unifyfs_handle fshdl;
  init_time.resumeTime();
  int rc =
      unifyfs_initialize(unifyfs_path.c_str(), chk_size, n_configs, &fshdl);
  init_time.pauseTime();
  REQUIRE(rc == UNIFYFS_SUCCESS);
  fs::path unifyfs_filename = unifyfs_path / args.filename;
  int create_flags = 0;
  unifyfs_gfid gfid;
  open_time.resumeTime();
  rc = unifyfs_create(fshdl, create_flags, unifyfs_filename.c_str(), &gfid);
  open_time.pauseTime();
  REQUIRE(rc == UNIFYFS_SUCCESS);
  REQUIRE(gfid != UNIFYFS_INVALID_GFID);
  unifyfs_io_request write_req[args.iteration];
  for (int i = 0; i < args.iteration; ++i) {
    write_req[i].op = UNIFYFS_IOREQ_OP_WRITE;
    write_req[i].gfid = gfid;
    write_req[i].nbytes = args.request_size;
    write_req[i].offset = i * args.request_size;
    write_req[i].user_buf = write_buf + (i * args.request_size);
  }
  write_time.resumeTime();
  async_write_time.resumeTime();
  rc = unifyfs_dispatch_io(fshdl, args.iteration, write_req);
  async_write_time.pauseTime();
  if (rc == UNIFYFS_SUCCESS) {
    int waitall = 1;
    rc = unifyfs_wait_io(fshdl, args.iteration, write_req, waitall);
    if (rc == UNIFYFS_SUCCESS) {
      for (size_t i = 0; i < args.iteration; i++) {
        REQUIRE(write_req[i].result.error == 0);
        REQUIRE(write_req[i].result.count == args.request_size);
      }
    }
  }
  write_time.pauseTime();
  fs::path pfs_filename = pfs / args.filename;
  unifyfs_transfer_request mv_req;
  mv_req.src_path = unifyfs_filename.c_str();
  mv_req.dst_path = pfs_filename.c_str();
  mv_req.mode = UNIFYFS_TRANSFER_MODE_MOVE;
  mv_req.use_parallel = 1;
  flush_time.resumeTime();
  rc = unifyfs_dispatch_transfer(fshdl, 1, &mv_req);
  REQUIRE(rc == UNIFYFS_SUCCESS);
  if (rc == UNIFYFS_SUCCESS) {
    int waitall = 1;
    rc = unifyfs_wait_transfer(fshdl, 1, &mv_req, waitall);
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
  const char *PFS_VAR = std::getenv("PFS_PATH");
  const char *BB_VAR = std::getenv("BB_PATH");
  const char *SHM_VAR = std::getenv("SHM_PATH");
  REQUIRE(PFS_VAR != nullptr);
  REQUIRE(BB_VAR != nullptr);
  REQUIRE(SHM_VAR != nullptr);
  fs::path pfs = fs::path(PFS_VAR) / "unifyfs" / "data";
  fs::path bb = fs::path(BB_VAR) / "unifyfs" / "data";
  fs::create_directories(pfs);
  fs::create_directories(bb);
  auto write_data = std::vector<char>(args.iteration * args.request_size, 'w');
  Timer init_time, finalize_time, open_time, close_time, write_time,
      async_write_time, flush_time;
  char usecase[256];
  SECTION("storage.pfs") {
    strcpy(usecase, "pfs");
    fs::path filename = pfs / args.filename;
    open_time.resumeTime();
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_SYNC, 0666);
    open_time.pauseTime();
    REQUIRE(fd != -1);
    for (int i = 0; i < args.iteration; ++i) {
      write_time.resumeTime();
      int written_bytes = write(fd, write_data.data() + (i * args.request_size),
                                args.request_size);
      write_time.pauseTime();
      REQUIRE(written_bytes == args.request_size);
    }
    close_time.resumeTime();
    int status = close(fd);
    close_time.pauseTime();
    REQUIRE(status == 0);
    printf(
        "n\t\t,rq\t\t,init\t\t,fin\t\t,open\t\t,close\t\t,write\t\t,"
        "awrite\t\t,flush\t\t,case\n");
  }
  SECTION("storage.unifyfs.nobuffer") {
    strcpy(usecase, "unifyfs.nobuffer");
    const int n_configs = 1;
    unifyfs_cfg_option chk_size[n_configs];
    chk_size[0] = {.opt_name = "logio.spill_dir", .opt_value = bb.c_str()};
    fs::path unifyfs_path = "/unifyfs1";
    unifyfs_handle fshdl;
    init_time.resumeTime();
    int rc =
        unifyfs_initialize(unifyfs_path.c_str(), chk_size, n_configs, &fshdl);
    init_time.pauseTime();
    REQUIRE(rc == UNIFYFS_SUCCESS);
    fs::path unifyfs_filename = unifyfs_path / args.filename;
    int create_flags = 0;
    unifyfs_gfid gfid;
    open_time.resumeTime();
    rc = unifyfs_create(fshdl, create_flags, unifyfs_filename.c_str(), &gfid);
    open_time.pauseTime();
    REQUIRE(rc == UNIFYFS_SUCCESS);
    REQUIRE(gfid != UNIFYFS_INVALID_GFID);
    for (int i = 0; i < args.iteration; ++i) {
      unifyfs_io_request write_req;
      write_req.op = UNIFYFS_IOREQ_OP_WRITE;
      write_req.gfid = gfid;
      write_req.nbytes = args.request_size;
      write_req.offset = i * args.request_size;
      write_req.user_buf = write_data.data() + (i * args.request_size);
      write_time.resumeTime();
      async_write_time.resumeTime();
      rc = unifyfs_dispatch_io(fshdl, 1, &write_req);
      async_write_time.pauseTime();
      if (rc == UNIFYFS_SUCCESS) {
        int waitall = 1;
        rc = unifyfs_wait_io(fshdl, 1, &write_req, waitall);
        if (rc == UNIFYFS_SUCCESS) {
          for (size_t i = 0; i < 1; i++) {
            REQUIRE(write_req.result.error == 0);
            REQUIRE(write_req.result.count == args.request_size);
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
    REQUIRE(rc == UNIFYFS_SUCCESS);
    if (rc == UNIFYFS_SUCCESS) {
      int waitall = 1;
      rc = unifyfs_wait_transfer(fshdl, 1, &mv_req, waitall);
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
  SECTION("storage.unifyfs.buffer") {
    strcpy(usecase, "unifyfs.buffer");
    buffer_io(init_time, finalize_time, open_time, close_time, write_time,
              async_write_time, flush_time, bb, pfs, nullptr, 0,
              write_data.data());
  }
  SECTION("storage.unifyfs.consistency=laminate") {
    strcpy(usecase, "unifyfs.consistency=laminate");
    unifyfs_cfg_option chk_size[1];
    chk_size[0] = {.opt_name = "unifyfs.consistency", .opt_value = "LAMINATED"};
    buffer_io(init_time, finalize_time, open_time, close_time, write_time,
              async_write_time, flush_time, bb, pfs, chk_size, 1,
              write_data.data());
  }
  SECTION("storage.unifyfs.consistency=posix") {
    strcpy(usecase, "unifyfs.consistency=posix");
    unifyfs_cfg_option chk_size[1];
    chk_size[0] = {.opt_name = "unifyfs.consistency", .opt_value = "POSIX"};
    buffer_io(init_time, finalize_time, open_time, close_time, write_time,
              async_write_time, flush_time, bb, pfs, chk_size, 1,
              write_data.data());
  }
  SECTION("storage.unifyfs.consistency=none") {
    strcpy(usecase, "unifyfs.consistency=none");
    unifyfs_cfg_option chk_size[1];
    chk_size[0] = {.opt_name = "unifyfs.consistency", .opt_value = "NONE"};
    buffer_io(init_time, finalize_time, open_time, close_time, write_time,
              async_write_time, flush_time, bb, pfs, chk_size, 1,
              write_data.data());
  }
  SECTION("storage.client.fsync_persist=on") {
    strcpy(usecase, "client.fsync_persist=on");
    unifyfs_cfg_option chk_size[1];
    chk_size[0] = {.opt_name = "client.fsync_persist", .opt_value = "on"};
    buffer_io(init_time, finalize_time, open_time, close_time, write_time,
              async_write_time, flush_time, bb, pfs, chk_size, 1,
              write_data.data());
  }
  SECTION("storage.client.fsync_persist=off") {
    strcpy(usecase, "client.fsync_persist=off");
    unifyfs_cfg_option chk_size[1];
    chk_size[0] = {.opt_name = "client.fsync_persist", .opt_value = "off"};
    buffer_io(init_time, finalize_time, open_time, close_time, write_time,
              async_write_time, flush_time, bb, pfs, chk_size, 1,
              write_data.data());
  }
  SECTION("storage.client.local_extents") {
    strcpy(usecase, "client.local_extents=on");
    unifyfs_cfg_option chk_size[1];
    chk_size[0] = {.opt_name = "client.local_extents", .opt_value = "on"};
    buffer_io(init_time, finalize_time, open_time, close_time, write_time,
              async_write_time, flush_time, bb, pfs, chk_size, 1,
              write_data.data());
  }
  SECTION("storage.client.max_files") {
    strcpy(usecase, "client.max_files=4");
    unifyfs_cfg_option chk_size[1];
    chk_size[0] = {.opt_name = "client.max_files", .opt_value = "4"};
    buffer_io(init_time, finalize_time, open_time, close_time, write_time,
              async_write_time, flush_time, bb, pfs, chk_size, 1,
              write_data.data());
  }
  SECTION("storage.client.write_index_size") {
    strcpy(usecase, "client.write_index_size=4,194,304");
    unifyfs_cfg_option chk_size[1];
    chk_size[0] = {.opt_name = "client.write_index_size",
                   .opt_value = "4194304"};
    buffer_io(init_time, finalize_time, open_time, close_time, write_time,
              async_write_time, flush_time, bb, pfs, chk_size, 1,
              write_data.data());
  }
  SECTION("storage.client.write_sync") {
    strcpy(usecase, "client.write_sync=on");
    unifyfs_cfg_option chk_size[1];
    chk_size[0] = {.opt_name = "client.write_sync", .opt_value = "on"};
    buffer_io(init_time, finalize_time, open_time, close_time, write_time,
              async_write_time, flush_time, bb, pfs, chk_size, 1,
              write_data.data());
  }
  SECTION("storage.logio.chunk_size") {
    strcpy(usecase,
           ("logio.chunk_size=" + std::to_string(args.request_size)).c_str());
    unifyfs_cfg_option chk_size[1];
    chk_size[0] = {.opt_name = "logio.chunk_size",
                   .opt_value = std::to_string(args.request_size).c_str()};
    buffer_io(init_time, finalize_time, open_time, close_time, write_time,
              async_write_time, flush_time, bb, pfs, chk_size, 1,
              write_data.data());
  }
  SECTION("storage.logio.shmem_size") {
    strcpy(usecase, ("logio.shmem_size=" +
                     std::to_string(args.request_size * args.iteration))
                        .c_str());
    unifyfs_cfg_option chk_size[1];
    chk_size[0] = {
        .opt_name = "logio.shmem_size",
        .opt_value =
            std::to_string(args.request_size * args.iteration).c_str()};
    buffer_io(init_time, finalize_time, open_time, close_time, write_time,
              async_write_time, flush_time, bb, pfs, chk_size, 1,
              write_data.data());
  }
  printf("%ld\t,%ld\t,%f\t,%f\t,%f\t,%f\t,%f\t,%f\t,%f,\t%s\n", args.iteration,
         args.request_size, init_time.getElapsedTime(),
         finalize_time.getElapsedTime(), open_time.getElapsedTime(),
         close_time.getElapsedTime(), write_time.getElapsedTime(),
         async_write_time.getElapsedTime(), flush_time.getElapsedTime(),
         usecase);
}
