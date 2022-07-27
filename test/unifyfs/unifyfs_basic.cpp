#include <catch_config.h>
#include <fcntl.h>
#include <mpi.h>
#include <test_utils.h>
#include <unifyfs.h>
#include <unifyfs/unifyfs_api.h>

#include <experimental/filesystem>
#include <iomanip>
#include <tailorfs/unifyfs-stage-transfer.cpp>

const uint64_t GB = 1024L * 1024L * 1024L;
#define CONVERT_ENUM(name, value) \
  "[name=" + std::to_string(static_cast<int>(value)) + "]"

#define DEFINE_CLARA_OPS(TYPE)                                 \
  std::ostream &operator<<(std::ostream &out, const TYPE &c) { \
    out << static_cast<int>(c) << std::endl;                   \
    return out;                                                \
  }                                                            \
  TYPE &operator>>(const std::stringstream &out, TYPE &c) {    \
    c = static_cast<TYPE>(atoi(out.str().c_str()));            \
    return c;                                                  \
  }
#define AGGREGATE_TIME(name)                                      \
  double total_##name = 0.0;                                      \
  auto name##_a = name##_time.getElapsedTime();                   \
  MPI_Reduce(&name##_a, &total_##name, 1, MPI_DOUBLE, MPI_SUM, 0, \
             MPI_COMM_WORLD);
#define PRINT_MSG(format, ...) fprintf (stderr, format, __VA_ARGS__)
namespace tailorfs::test {}

namespace fs = std::experimental::filesystem;
namespace tt = tailorfs::test;

/**
 * Test data structures
 */
namespace tailorfs::test {
enum StorageType : int { SHM = 0, LOCAL_SSD = 1 };
enum UseCase : int {
  WRITE_ONLY = 0,
  READ_ONLY = 1,
  READ_AFTER_WRITE = 2,
  UPDATE = 3,
  WORM = 4
};
enum AccessPattern : int { SEQUENTIAL = 0, RANDOM = 1 };
enum FileSharing : int { PER_PROCESS = 0, SHARED_FILE = 1 };
enum ProcessGrouping : int {
  ALL_PROCESS = 0,
  SPLIT_PROCESS_HALF = 1,
  SPLIT_PROCESS_ALTERNATE = 2
};
struct Arguments {
  std::string filename = "test.dat";
  size_t request_size = 65536;
  size_t iteration = 64;
  int ranks_per_node = 1;
  bool debug = false;
  StorageType storage_type = StorageType::LOCAL_SSD;
  AccessPattern access_pattern = AccessPattern::SEQUENTIAL;
  FileSharing file_sharing = FileSharing::PER_PROCESS;
  ProcessGrouping process_grouping = ProcessGrouping::ALL_PROCESS;
};
struct Info {
  fs::path pfs;
  fs::path bb;
  fs::path shm;
  int rank;
  int comm_size;
  char hostname[256];
  fs::path unifyfs_path = "/unifyfs1";
  std::string original_filename;
};
}  // namespace tailorfs::test
tailorfs::test::Arguments args;
tailorfs::test::Info info;

namespace tailorfs::test {
int buildUnifyFSOptions(UseCase use_case, unifyfs_handle *fshdl, Timer *init_time) {
  int options_ct = 0;
  char unifyfs_consistency[32] = "LAMINATED";
  options_ct++;
  char client_fsync_persist[32] = "off";
  options_ct++;
  char logio_chunk_size[32];
  strcpy(logio_chunk_size, std::to_string(args.request_size).c_str());
  options_ct++;
  char logio_shmem_size[32];
  strcpy(logio_shmem_size, "0");
  options_ct++;
  char logio_spill_dir[256];
  fs::path splill_dir;
  switch (args.storage_type) {
    case StorageType::SHM: {
      splill_dir = std::string(info.shm);
      break;
    }
    case StorageType::LOCAL_SSD: {
      splill_dir = std::string(info.bb);
      break;
    }
  };
  splill_dir = splill_dir;
  strcpy(logio_spill_dir, splill_dir.c_str());
  options_ct++;
  char logio_spill_size[32];
  strcpy(logio_spill_size,
         std::to_string(args.request_size * args.iteration + 1024L * 1024L)
             .c_str());
  options_ct++;
  char client_local_extents[32];
  char client_node_local_extents[32];
  switch (use_case) {
    case UseCase::READ_ONLY:
    case UseCase::READ_AFTER_WRITE:
    case UseCase::UPDATE:
    case UseCase::WORM: {
      strcpy(client_local_extents, "on");
      options_ct++;
      strcpy(client_node_local_extents, "on");
      options_ct++;
      break;
    }
    case UseCase::WRITE_ONLY: {
      strcpy(client_local_extents, "off");
      options_ct++;
      strcpy(client_node_local_extents, "off");
      options_ct++;
      break;
    }
  };
  unifyfs_cfg_option *options = static_cast<unifyfs_cfg_option *>(
      calloc(options_ct, sizeof(unifyfs_cfg_option)));
  options[0] = {.opt_name = "unifyfs.consistency",
                .opt_value = unifyfs_consistency};
  options[1] = {.opt_name = "client.fsync_persist",
                .opt_value = client_fsync_persist};
  options[2] = {.opt_name = "logio.chunk_size", .opt_value = logio_chunk_size};
  options[3] = {.opt_name = "logio.shmem_size", .opt_value = logio_shmem_size};
  options[4] = {.opt_name = "logio.spill_dir", .opt_value = logio_spill_dir};
  options[5] = {.opt_name = "logio.spill_size", .opt_value = logio_spill_size};
  options[6] = {.opt_name = "client.local_extents",
                .opt_value = client_local_extents};
  options[7] = {.opt_name = "client.node_local_extents",
                .opt_value = client_node_local_extents};
  INFO("unifyfs path " << info.unifyfs_path);
  const char *val = info.unifyfs_path.c_str();
  init_time->resumeTime();
  int rc = unifyfs_initialize(val, options, options_ct, fshdl);
  init_time->pauseTime();
  REQUIRE(rc == UNIFYFS_SUCCESS);
  return 0;
}
}  // namespace tailorfs::test

DEFINE_CLARA_OPS(tailorfs::test::StorageType)
DEFINE_CLARA_OPS(tailorfs::test::AccessPattern)
DEFINE_CLARA_OPS(tailorfs::test::FileSharing)
DEFINE_CLARA_OPS(tailorfs::test::ProcessGrouping)

/**
 * Overridden methods for catch
 */
int init(int *argc, char ***argv) {
  MPI_Init(argc, argv);
  int rank, comm_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  if (args.debug && rank == 0) {
    fprintf(stderr, "Connect to processes\n");
    fflush(stderr);
    getchar();
  }
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
         cl::Opt(args.ranks_per_node, "rpn")["-n"]["--rpn"]("Ranks per node") |
         cl::Opt(args.storage_type, "storage_type")["-s"]["--storage_type"](
             "Where to store the data 0-> SHM, 1->NODE_LOCAL_SSD, 2-> "
             "SHARED_SSD, 3->PFS") |
         cl::Opt(args.access_pattern,
                 "access_pattern")["-a"]["--access_pattern"](
             "Select Access Pattern: 0-> sequential, 1-> random") |
         cl::Opt(args.file_sharing, "file_sharing")["-b"]["--file_sharing"](
             "How the file is accessed 0-> FPP, 1->SHARED") |
         cl::Opt(args.process_grouping,
                 "process_grouping")["-c"]["--process_grouping"](
             "How the process are grouped: 0-> ALL, 1-> SPLIT_PROCESS_HALF, "
             "2-> SPLIT_PROCESS_ALTERNATE") |
         cl::Opt(args.debug, "debug")["-d"]["--debug"]("Enable Debug");
}

/**
 * Helper functions
 */
void delete_directory_contents(const fs::path &dir_path) {
  if (fs::exists(dir_path)) {
    for (const auto &entry : fs::directory_iterator(dir_path))
      fs::remove_all(entry.path());
  }
}
int pretest() {
  info.original_filename = args.filename;
  const char *PFS_VAR = std::getenv("pfs");
  const char *BB_VAR = std::getenv("BBPATH");
  const char *SHM_VAR = "/dev/shm";
  REQUIRE(PFS_VAR != nullptr);
  REQUIRE(BB_VAR != nullptr);
  REQUIRE(SHM_VAR != nullptr);
  MPI_Comm_rank(MPI_COMM_WORLD, &info.rank);
  MPI_Comm_size(MPI_COMM_WORLD, &info.comm_size);
  gethostname(info.hostname, 256);
  info.pfs = fs::path(PFS_VAR) / "unifyfs" / "data";
  info.bb = fs::path(BB_VAR) / "unifyfs" / "data";
  info.shm = fs::path(SHM_VAR) / "unifyfs" / "data";
  return 0;
}
int posttest() {
  MPI_Barrier(MPI_COMM_WORLD);
  fs::remove_all(info.pfs);
  fs::remove_all(info.bb);
  fs::remove_all(info.shm);
  return 0;
}
int clean_directories() {
  if (info.rank == 0) {
    fs::remove_all(info.pfs);
    fs::create_directories(info.pfs);
  }
  if (info.rank % args.ranks_per_node == 0) {
    INFO("rank " << info.rank << " on node " << info.hostname << " with BB "
                 << info.bb.string().c_str());
    fs::remove_all(info.bb);
    fs::remove_all(info.shm);
    fs::create_directories(info.bb);
    fs::create_directories(info.shm);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  REQUIRE(fs::exists(info.bb));
  REQUIRE(fs::exists(info.shm));
  REQUIRE(fs::exists(info.pfs));
  MPI_Barrier(MPI_COMM_WORLD);
  return 0;
}

int create_file(fs::path filename, uint64_t blocksize) {
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_File fh_orig;
  int status_orig;

  off_t base_offset = 0;
  switch (args.file_sharing) {
    case tt::FileSharing::PER_PROCESS: {
      status_orig =
          MPI_File_open(MPI_COMM_SELF, filename.c_str(),
                        MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &fh_orig);
      break;
    }
    case tt::FileSharing::SHARED_FILE: {
      status_orig =
          MPI_File_open(MPI_COMM_WORLD, filename.c_str(),
                        MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &fh_orig);
      base_offset = (off_t)info.rank * blocksize;
      break;
    }
  }
  REQUIRE(status_orig == MPI_SUCCESS);
  auto write_data = std::vector<char>(blocksize, 'w');
  MPI_Status stat_orig;
  auto ret_orig = MPI_File_write_at_all(fh_orig, base_offset, write_data.data(),
                                        blocksize, MPI_CHAR, &stat_orig);
  int written_bytes;
  MPI_Get_count(&stat_orig, MPI_CHAR, &written_bytes);
  REQUIRE(written_bytes == blocksize);
  status_orig = MPI_File_close(&fh_orig);
  REQUIRE(status_orig == MPI_SUCCESS);
  if (info.rank == 0) {
    switch (args.file_sharing) {
      case tt::FileSharing::PER_PROCESS: {
        REQUIRE(fs::file_size(filename) == blocksize);
        break;
      }
      case tt::FileSharing::SHARED_FILE: {
        REQUIRE(fs::file_size(filename) == blocksize * info.comm_size);
        break;
      }
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  return 0;
}

/**
 * Test cases
 */
TEST_CASE("Write-Only",
          "[type=write-only]"
          "[optimization=buffered_write]" CONVERT_ENUM(access_pattern,
                                                       args.access_pattern)
              CONVERT_ENUM(storage_type, args.storage_type)
                  CONVERT_ENUM(file_sharing, args.file_sharing)
                      CONVERT_ENUM(process_grouping, args.process_grouping)) {
  REQUIRE(pretest() == 0);
  switch (args.file_sharing) {
    case tt::FileSharing::PER_PROCESS: {
      args.filename = args.filename + "_" + std::to_string(info.rank) + "_of_" +
                      std::to_string(info.comm_size);
      break;
    }
    case tt::FileSharing::SHARED_FILE: {
      args.filename = args.filename + "_" + std::to_string(info.comm_size);
      break;
    }
  }
  REQUIRE(args.process_grouping == tt::ProcessGrouping::ALL_PROCESS);
  REQUIRE(args.access_pattern == tt::AccessPattern::SEQUENTIAL);
  REQUIRE(clean_directories() == 0);

  Timer init_time, finalize_time, open_time, close_time, write_time,
      awrite_time, flush_time;
  char usecase[256];
  bool is_run =false;
  INFO("rank " << info.rank);
  SECTION("storage.pfs") {
    strcpy(usecase, "pfs");
    fs::path filename = info.pfs / args.filename;
    open_time.resumeTime();
    MPI_File fh_orig;
    int status_orig;

    if (args.file_sharing == tt::FileSharing::PER_PROCESS) {
      status_orig =
          MPI_File_open(MPI_COMM_SELF, filename.c_str(),
                        MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &fh_orig);
    } else if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
      status_orig =
          MPI_File_open(MPI_COMM_WORLD, filename.c_str(),
                        MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &fh_orig);
    }
    open_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
    for (int i = 0; i < args.iteration; ++i) {
      auto write_data = std::vector<char>(args.request_size, 'w');
      write_time.resumeTime();
      MPI_Status stat_orig;
      off_t base_offset = (off_t)info.rank * args.request_size * args.iteration;
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
    is_run =true;
  }
  SECTION("storage.unifyfs") {
    strcpy(usecase, "unifyfs");
    unifyfs_handle fshdl;
    tt::buildUnifyFSOptions(tailorfs::test::WRITE_ONLY, &fshdl, &init_time);
    MPI_Barrier(MPI_COMM_WORLD);
    int rank, comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    fs::path unifyfs_filename = info.unifyfs_path / args.filename;
    unifyfs_gfid gfid;
    int rc = UNIFYFS_SUCCESS;

    INFO("unifyfs_filename " << unifyfs_filename);
    if (args.file_sharing == tt::FileSharing::PER_PROCESS) {
      int create_flags = 0;
      open_time.resumeTime();
      rc = unifyfs_create(fshdl, create_flags, unifyfs_filename.c_str(), &gfid);
      open_time.pauseTime();
    } else if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
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
    }
    INFO("unifyfs rc " << strerror(rc));
    REQUIRE(rc == UNIFYFS_SUCCESS);
    REQUIRE(gfid != UNIFYFS_INVALID_GFID);
    if (rank == 0) INFO("Writing data");
    /* Write data to file */
    int max_buff = 1000;
    int processed = 0;
    int j = 0;
    while (processed < args.iteration) {
      auto num_req_to_buf = args.iteration - processed >= max_buff
                                ? max_buff
                                : args.iteration - processed;
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
      awrite_time.resumeTime();
      rc = unifyfs_dispatch_io(fshdl, write_req_ct, write_req);
      awrite_time.pauseTime();
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
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) INFO("Flushing data");
    fs::path pfs_filename = info.pfs / args.filename;
    if (args.file_sharing == tt::FileSharing::PER_PROCESS) {
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
    } else if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
      if (rank == 0) {
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
      }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    finalize_time.resumeTime();
    rc = unifyfs_finalize(fshdl);
    finalize_time.pauseTime();
    REQUIRE(rc == UNIFYFS_SUCCESS);
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
      INFO("Size of file written " << fs::file_size(pfs_filename));
      switch (args.file_sharing) {
        case tt::FileSharing::PER_PROCESS: {
          REQUIRE(fs::file_size(pfs_filename) ==
                  args.request_size * args.iteration);
          break;
        }
        case tt::FileSharing::SHARED_FILE: {
          REQUIRE(fs::file_size(pfs_filename) ==
                  args.request_size * args.iteration * (comm_size));
          break;
        }
      }

    }
    MPI_Barrier(MPI_COMM_WORLD);

    is_run =true;
  }
  if(is_run) {
    AGGREGATE_TIME(init);
    AGGREGATE_TIME(finalize);
    AGGREGATE_TIME(open);
    AGGREGATE_TIME(close);
    AGGREGATE_TIME(awrite);
    AGGREGATE_TIME(write);
    AGGREGATE_TIME(flush);
    if (info.rank == 0) {
      PRINT_MSG("%10s,%10d,%10d,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10s,%10s,%d,%d,%d,%d\n",
                "Timing", args.iteration, args.request_size, total_init / info.comm_size, total_finalize / info.comm_size ,
                total_open / info.comm_size, total_close / info.comm_size, total_awrite / info.comm_size, total_write / info.comm_size, total_flush / info.comm_size,
                usecase, "write-only", args.storage_type, args.access_pattern, args.file_sharing, args.process_grouping);
    }
  }
  REQUIRE(posttest() == 0);
}
TEST_CASE("Read-Only",
          "[type=read-only]"
          "[optimization=buffered_read]" CONVERT_ENUM(access_pattern,
                                                      args.access_pattern)
              CONVERT_ENUM(storage_type, args.storage_type)
                  CONVERT_ENUM(file_sharing, args.file_sharing)
                      CONVERT_ENUM(process_grouping, args.process_grouping)) {
  REQUIRE(pretest() == 0);
  std::string filename_str;
  INFO(args.filename);
  switch (args.file_sharing) {
    case tt::FileSharing::PER_PROCESS: {
      filename_str = args.filename + "_" + std::to_string(info.rank) + "_of_" +
                      std::to_string(info.comm_size);
      break;
    }
    case tt::FileSharing::SHARED_FILE: {
      filename_str = args.filename + "_" + std::to_string(info.comm_size);
      break;
    }
  }
  INFO(filename_str);
  REQUIRE(args.process_grouping == tt::ProcessGrouping::ALL_PROCESS);
  REQUIRE(clean_directories() == 0);
  fs::path pfs_filename = info.pfs / filename_str;
  create_file(pfs_filename, args.request_size * args.iteration);

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
      off_t random_i = i;
      if (args.access_pattern == tt::AccessPattern::RANDOM) {
        random_i = rand() % args.iteration;
      }
      auto read_data = std::vector<char>(args.request_size, 'r');
      read_time.resumeTime();
      MPI_Status stat_orig;
      off_t base_offset = 0;
      if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
        base_offset = (off_t)info.rank * args.request_size * args.iteration;
      }
      off_t relative_offset = random_i * args.request_size;
      auto ret_orig = MPI_File_read_at(fh_orig, base_offset + relative_offset,
                                       read_data.data(), args.request_size,
                                       MPI_CHAR, &stat_orig);
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
  SECTION("storage.unifyfs") {
    strcpy(usecase, "unifyfs");
    unifyfs_handle fshdl;
    tt::buildUnifyFSOptions(tailorfs::test::READ_ONLY, &fshdl, &init_time);
    unifyfs_gfid gfid;
    fs::path unifyfs_filename = info.unifyfs_path / filename_str;
    char unifyfs_filename_charp[256];
    strcpy(unifyfs_filename_charp, unifyfs_filename.c_str());

    unifyfs_stage ctx;
    ctx.checksum = 0;
    ctx.data_dist = UNIFYFS_STAGE_DATA_BALANCED;
    ctx.mode = UNIFYFS_STAGE_MODE_PARALLEL;
    ctx.mountpoint = unifyfs_filename_charp;

    switch (args.file_sharing) {
      case tt::FileSharing::PER_PROCESS: {
        ctx.total_ranks = 1;
        ctx.rank = 0;
        break;
      }
      case tt::FileSharing::SHARED_FILE: {
        ctx.total_ranks = info.comm_size;
        ctx.rank = info.rank;

        break;
      }
    }


    int file_index = 1;
    ctx.fshdl = fshdl;
    ctx.block_size = args.request_size * args.iteration;
    prefetch_time.resumeTime();
    int rc = unifyfs_stage_transfer(&ctx, file_index, pfs_filename.c_str(),
                                    unifyfs_filename.c_str());
    prefetch_time.pauseTime();
    REQUIRE(rc == UNIFYFS_SUCCESS);


    // INFO("prefetch done by rank " << rank);
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
            INFO("UNIFYFS ERROR: "
                 << "OP_READ req failed - "
                 << strerror(prefetch_sync[i].result.error));
          REQUIRE(prefetch_sync[i].result.error == 0);
        }
      }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    for (off_t i = 0; i < args.iteration; ++i) {
      off_t random_i = i;
      if (args.access_pattern == tt::AccessPattern::RANDOM) {
        random_i = rand() % args.iteration;
      }
      auto read_data = std::vector<char>(args.request_size, 'r');
      unifyfs_io_request read_req;
      read_req.op = UNIFYFS_IOREQ_OP_READ;
      read_req.gfid = gfid;
      read_req.nbytes = args.request_size;
      off_t base_offset = 0;
      if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
        base_offset = (off_t)info.rank * args.request_size * args.iteration;
      }
      off_t relative_offset = random_i * args.request_size;
      read_req.offset = base_offset + relative_offset;
      INFO("rank: " << info.rank << " reads " << read_req.offset << " for iter "
                    << random_i);
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
            INFO("UNIFYFS ERROR: "
                 << "OP_READ req failed - " << strerror(read_req.result.error));
          REQUIRE(read_req.result.error == 0);
          if (read_req.result.count != args.request_size)
            INFO("UNIFYFS ERROR: "
                 << "OP_READ req failed - rank " << info.rank << " iter "
                 << random_i << ", and read only " << read_req.result.count
                 << " of " << args.request_size << " with offset "
                 << read_req.offset);
          REQUIRE(read_req.result.count == args.request_size);
        }
      }
    }
    finalize_time.resumeTime();
    rc = unifyfs_finalize(fshdl);
    finalize_time.pauseTime();
    REQUIRE(rc == UNIFYFS_SUCCESS);
  }
  AGGREGATE_TIME(init);
  AGGREGATE_TIME(finalize);
  AGGREGATE_TIME(open);
  AGGREGATE_TIME(close);
  AGGREGATE_TIME(read);
  AGGREGATE_TIME(prefetch);
  if (info.rank == 0) {
    PRINT_MSG("%10s,%10d,%10d,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10s,%10s,%d,%d,%d,%d\n",
              "Timing", args.iteration, args.request_size, total_init / info.comm_size, total_finalize / info.comm_size ,
              total_open / info.comm_size, total_close / info.comm_size, total_read / info.comm_size, total_prefetch / info.comm_size,
              usecase, "read-only", args.storage_type, args.access_pattern, args.file_sharing, args.process_grouping);
  }
  REQUIRE(posttest() == 0);
}
TEST_CASE("Read-After-Write",
          "[type=raw]"
          "[optimization=node_local_io]" CONVERT_ENUM(access_pattern,
                                                      args.access_pattern)
              CONVERT_ENUM(storage_type, args.storage_type)
                  CONVERT_ENUM(file_sharing, args.file_sharing)
                      CONVERT_ENUM(process_grouping, args.process_grouping)) {
  std::string filename_str;
  REQUIRE(pretest() == 0);
  if (args.process_grouping != tt::ProcessGrouping::ALL_PROCESS) {
    REQUIRE(info.comm_size > 1);
    REQUIRE(info.comm_size % 2 == 0);
  }
  MPI_Comm writer_comm = MPI_COMM_WORLD;
  MPI_Comm reader_comm = MPI_COMM_WORLD;
  bool is_writer = false;
  if (args.process_grouping == tt::ProcessGrouping::ALL_PROCESS) {
    writer_comm = MPI_COMM_WORLD;
    reader_comm = MPI_COMM_WORLD;
  } else if (args.process_grouping == tt::ProcessGrouping::SPLIT_PROCESS_HALF) {
    is_writer = info.comm_size / 2 == 0;
    MPI_Comm_split(MPI_COMM_WORLD, is_writer, info.rank, &writer_comm);
    MPI_Comm_split(MPI_COMM_WORLD, !is_writer, info.rank, &reader_comm);
  } else if (args.process_grouping ==
             tt::ProcessGrouping::SPLIT_PROCESS_ALTERNATE) {
    is_writer = info.comm_size % 2 == 0;
    MPI_Comm_split(MPI_COMM_WORLD, is_writer, info.rank, &writer_comm);
    MPI_Comm_split(MPI_COMM_WORLD, !is_writer, info.rank, &reader_comm);
  }
  int write_rank, write_comm_size;
  MPI_Comm_rank(writer_comm, &write_rank);
  MPI_Comm_size(writer_comm, &write_comm_size);

  int read_rank, read_comm_size;
  MPI_Comm_rank(reader_comm, &read_rank);
  MPI_Comm_size(reader_comm, &read_comm_size);

  if (tt::FileSharing::SHARED_FILE == args.file_sharing) {
    filename_str = args.filename + "_" + std::to_string(info.comm_size);
  } else if (tt::FileSharing::SHARED_FILE == args.file_sharing) {
    if (is_writer) {
      filename_str = args.filename + "_" + std::to_string(write_rank) +
                      "_of_" + std::to_string(write_comm_size);
    } else {
      filename_str = args.filename + "_" + std::to_string(read_rank) + "_of_" +
                      std::to_string(read_comm_size);
    }
  } else {
    REQUIRE(1 == 0);
  }
  REQUIRE(clean_directories() == 0);
  REQUIRE(create_file(info.pfs / filename_str,
                      args.request_size * args.iteration) == 0);
  fs::path pfs_filename = info.pfs / filename_str;
  Timer init_time, finalize_time, open_time, close_time, write_time, read_time;
  char usecase[256];
  SECTION("storage.pfs") {
    strcpy(usecase, "pfs");
    open_time.resumeTime();
    MPI_File fh_orig;
    int status_orig = -1;
    if (is_writer) {
      status_orig = MPI_File_open(writer_comm, pfs_filename.c_str(),
                                  MPI_MODE_WRONLY | MPI_MODE_CREATE,
                                  MPI_INFO_NULL, &fh_orig);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (!is_writer) {
      status_orig = MPI_File_open(reader_comm, pfs_filename.c_str(),
                                  MPI_MODE_RDONLY, MPI_INFO_NULL, &fh_orig);
    }
    open_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
    if (is_writer) {
      for (int i = 0; i < args.iteration; ++i) {
        auto write_data = std::vector<char>(args.request_size, 'w');
        write_time.resumeTime();
        MPI_Status stat_orig;
        off_t base_offset =
            (off_t)write_rank * args.request_size * args.iteration;
        off_t relative_offset = i * args.request_size;
        auto ret_orig = MPI_File_write_at_all(
            fh_orig, base_offset + relative_offset, write_data.data(),
            args.request_size, MPI_CHAR, &stat_orig);
        int written_bytes;
        MPI_Get_count(&stat_orig, MPI_CHAR, &written_bytes);
        write_time.pauseTime();
        REQUIRE(written_bytes == args.request_size);
      }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (!is_writer) {
      for (int i = 0; i < args.iteration; ++i) {
        auto read_data = std::vector<char>(args.request_size, 'r');
        read_time.resumeTime();
        MPI_Status stat_orig;
        off_t base_offset = 0;
        if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
          base_offset = (off_t)info.rank * args.request_size * args.iteration;
        }
        off_t relative_offset = i * args.request_size;
        auto ret_orig = MPI_File_read_at_all(
            fh_orig, base_offset + relative_offset, read_data.data(),
            args.request_size, MPI_CHAR, &stat_orig);
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
  SECTION("storage.unifyfs") {
    strcpy(usecase, "unifyfs");
    int rc;
    /* Initialize unifyfs */
    unifyfs_handle fshdl;
    REQUIRE(tt::buildUnifyFSOptions(tailorfs::test::READ_AFTER_WRITE, &fshdl, &init_time) == 0);
    fs::path unifyfs_filename = info.unifyfs_path / filename_str;

    if (is_writer) {
      unifyfs_gfid gfid;
      /* Create or Open file for production */
      if (write_rank == 0) {
        int create_flags = 0;
        open_time.resumeTime();
        rc = unifyfs_create(fshdl, create_flags, unifyfs_filename.c_str(),
                            &gfid);
        open_time.pauseTime();
      }
      MPI_Barrier(writer_comm);
      if (write_rank != 0) {
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
      while (processed < args.iteration) {
        auto num_req_to_buf = args.iteration - processed >= max_buff
                                  ? max_buff
                                  : args.iteration - processed;
        auto write_data =
            std::vector<char>(args.request_size * num_req_to_buf, 'w');
        int write_req_ct = num_req_to_buf + 1;
        unifyfs_io_request write_req[num_req_to_buf + 2];

        for (int i = 0; i < num_req_to_buf; ++i) {
          write_req[i].op = UNIFYFS_IOREQ_OP_WRITE;
          write_req[i].gfid = gfid;
          write_req[i].nbytes = args.request_size;
          off_t base_offset =
              (off_t)write_rank * args.request_size * args.iteration;
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
    rc = unifyfs_laminate(fshdl, unifyfs_filename.c_str());
    MPI_Barrier(MPI_COMM_WORLD);
    if (!is_writer) {
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
              INFO("UNIFYFS ERROR: "
                   << "OP_READ req failed - "
                   << strerror(read_sync[i].result.error));
            REQUIRE(read_sync[i].result.error == 0);
          }
        }
      }

      /* Read data from file */
      for (int i = 0; i < args.iteration; ++i) {
        off_t random_i = i;
        if (args.access_pattern == tt::AccessPattern::RANDOM) {
          random_i = rand() % args.iteration;
        }
        auto read_data = std::vector<char>(args.request_size, 'r');
        unifyfs_io_request read_req;
        read_req.op = UNIFYFS_IOREQ_OP_READ;
        read_req.gfid = gfid;
        read_req.nbytes = args.request_size;
        off_t base_offset = 0;
        if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
          base_offset = (off_t)info.rank * args.request_size * args.iteration;
        }
        off_t relative_offset = random_i * args.request_size;
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
              INFO("UNIFYFS ERROR: "
                   << "OP_READ req failed - "
                   << strerror(read_req.result.error));
            REQUIRE(read_req.result.error == 0);
            if (read_req.result.count != args.request_size)
              INFO("UNIFYFS ERROR: "
                   << "OP_READ req failed - rank " << info.rank << " iter "
                   << random_i << ", and read only " << read_req.result.count
                   << " of " << args.request_size << " with offset "
                   << read_req.offset);
            REQUIRE(read_req.result.count == args.request_size);
          }
        }
      }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    finalize_time.resumeTime();
    rc = unifyfs_finalize(fshdl);
    finalize_time.pauseTime();
    REQUIRE(rc == UNIFYFS_SUCCESS);
  }
  AGGREGATE_TIME(init);
  AGGREGATE_TIME(finalize);
  AGGREGATE_TIME(open);
  AGGREGATE_TIME(close);
  AGGREGATE_TIME(read);
  AGGREGATE_TIME(write);
  if (info.rank == 0) {

    PRINT_MSG("%10s,%10d,%10d,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10s,%10s,%d,%d,%d,%d\n",
              "Timing", args.iteration, args.request_size, total_init / info.comm_size, total_finalize / info.comm_size ,
              total_open / info.comm_size, total_close / info.comm_size, total_write / info.comm_size, total_read / info.comm_size,
              usecase, "raw", args.storage_type, args.access_pattern, args.file_sharing, args.process_grouping);
  }
  MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE("Update",
          "[type=update]"
          "[optimization=buffered_write]" CONVERT_ENUM(access_pattern,
                                                       args.access_pattern)
              CONVERT_ENUM(storage_type, args.storage_type)
                  CONVERT_ENUM(file_sharing, args.file_sharing)
                      CONVERT_ENUM(process_grouping, args.process_grouping)) {
  std::string filename_str;
  REQUIRE(pretest() == 0);
  switch (args.file_sharing) {
    case tt::FileSharing::PER_PROCESS: {
      filename_str = args.filename + "_" + std::to_string(info.rank) + "_of_" +
                      std::to_string(info.comm_size);
      break;
    }
    case tt::FileSharing::SHARED_FILE: {
      filename_str = args.filename + "_" + std::to_string(info.comm_size);
      break;
    }
  }
  REQUIRE(args.process_grouping == tt::ProcessGrouping::ALL_PROCESS);
  REQUIRE(args.access_pattern == tt::AccessPattern::SEQUENTIAL);
  REQUIRE(clean_directories() == 0);
  REQUIRE(create_file(info.pfs / filename_str,
                      args.request_size * args.iteration) == 0);

  Timer init_time, finalize_time, open_time, close_time, write_time,
      awrite_time, flush_time;
  char usecase[256];
  SECTION("storage.pfs") {
    strcpy(usecase, "pfs");
    fs::path filename = info.pfs / filename_str;
    open_time.resumeTime();
    MPI_File fh_orig;
    int status_orig =
        MPI_File_open(MPI_COMM_WORLD, filename.c_str(),
                      MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &fh_orig);
    open_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
    for (int i = 0; i < args.iteration; ++i) {
      off_t random_i = i;
      if (args.access_pattern == tt::AccessPattern::RANDOM) {
        random_i = rand() % args.iteration;
      }
      auto write_data = std::vector<char>(args.request_size, 'w');
      write_time.resumeTime();
      MPI_Status stat_orig;
      off_t base_offset = 0;
      if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
        base_offset = (off_t)info.rank * args.request_size * args.iteration;
      }
      off_t relative_offset = random_i * args.request_size;
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
  SECTION("storage.unifyfs") {
    strcpy(usecase, "unifyfs");
    unifyfs_handle fshdl;
    tt::buildUnifyFSOptions(tailorfs::test::UPDATE, &fshdl, &init_time);
    MPI_Barrier(MPI_COMM_WORLD);
    int rank, comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    fs::path unifyfs_filename = info.unifyfs_path / filename_str;
    unifyfs_gfid gfid;
    int rc;
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
    if (rank == 0) INFO("Writing data");
    /* Write data to file */
    int max_buff = 1000;
    int processed = 0;
    int j = 0;
    while (processed < args.iteration) {
      auto num_req_to_buf = args.iteration - processed >= max_buff
                                ? max_buff
                                : args.iteration - processed;
      auto write_data =
          std::vector<char>(args.request_size * num_req_to_buf, 'w');
      int write_req_ct = num_req_to_buf + 1;
      unifyfs_io_request write_req[num_req_to_buf + 2];

      for (int i = 0; i < num_req_to_buf; ++i) {
        off_t random_i = i;
        if (args.access_pattern == tt::AccessPattern::RANDOM) {
          random_i = rand() % args.iteration;
        }
        write_req[i].op = UNIFYFS_IOREQ_OP_WRITE;
        write_req[i].gfid = gfid;
        write_req[i].nbytes = args.request_size;
        off_t base_offset = 0;
        if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
          base_offset = (off_t)info.rank * args.request_size * args.iteration;
        }
        off_t relative_offset = j * args.request_size;
        write_req[i].offset = base_offset + relative_offset;
        write_req[i].user_buf =
            write_data.data() + (random_i * args.request_size);
        j++;
      }
      write_req[num_req_to_buf].op = UNIFYFS_IOREQ_OP_SYNC_META;
      write_req[num_req_to_buf].gfid = gfid;
      write_req[num_req_to_buf + 1].op = UNIFYFS_IOREQ_OP_SYNC_DATA;
      write_req[num_req_to_buf + 1].gfid = gfid;
      write_time.resumeTime();
      awrite_time.resumeTime();
      rc = unifyfs_dispatch_io(fshdl, write_req_ct, write_req);
      awrite_time.pauseTime();
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
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) INFO("Flushing data");
    fs::path pfs_filename = info.pfs / filename_str;
    if (rank == 0) {
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
    }
    MPI_Barrier(MPI_COMM_WORLD);

    finalize_time.resumeTime();
    rc = unifyfs_finalize(fshdl);
    finalize_time.pauseTime();
    REQUIRE(rc == UNIFYFS_SUCCESS);
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
      REQUIRE(fs::file_size(pfs_filename) ==
              args.request_size * args.iteration * (comm_size));
      INFO("Size of file written " << fs::file_size(pfs_filename));
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
  AGGREGATE_TIME(init);
  AGGREGATE_TIME(finalize);
  AGGREGATE_TIME(open);
  AGGREGATE_TIME(close);
  AGGREGATE_TIME(awrite);
  AGGREGATE_TIME(write);
  AGGREGATE_TIME(flush);
  if (info.rank == 0) {
    PRINT_MSG("%10s,%10d,%10d,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10s,%10s,%d,%d,%d,%d\n",
              "Timing", args.iteration, args.request_size, total_init / info.comm_size, total_finalize / info.comm_size ,
              total_open / info.comm_size, total_close / info.comm_size, total_awrite / info.comm_size, total_write / info.comm_size, total_flush / info.comm_size,
              usecase, "update", args.storage_type, args.access_pattern, args.file_sharing, args.process_grouping);
  }
  REQUIRE(posttest() == 0);
}

TEST_CASE("WORM",
          "[type=worm]"
          "[optimization=node_local_io]" CONVERT_ENUM(access_pattern,
                                                      args.access_pattern)
              CONVERT_ENUM(storage_type, args.storage_type)
                  CONVERT_ENUM(file_sharing, args.file_sharing)
                      CONVERT_ENUM(process_grouping, args.process_grouping)) {
  std::string filename_str;
  REQUIRE(pretest() == 0);
  if (args.process_grouping != tt::ProcessGrouping::ALL_PROCESS) {
    REQUIRE(info.comm_size > 1);
    REQUIRE(info.comm_size % 2 == 0);
  }
  MPI_Comm writer_comm = MPI_COMM_WORLD;
  MPI_Comm reader_comm = MPI_COMM_WORLD;
  bool is_writer = false;
  if (args.process_grouping == tt::ProcessGrouping::ALL_PROCESS) {
    writer_comm = MPI_COMM_WORLD;
    reader_comm = MPI_COMM_WORLD;
  } else if (args.process_grouping == tt::ProcessGrouping::SPLIT_PROCESS_HALF) {
    is_writer = info.comm_size / 2 == 0;
    MPI_Comm_split(MPI_COMM_WORLD, is_writer, info.rank, &writer_comm);
    MPI_Comm_split(MPI_COMM_WORLD, !is_writer, info.rank, &reader_comm);
  } else if (args.process_grouping ==
             tt::ProcessGrouping::SPLIT_PROCESS_ALTERNATE) {
    is_writer = info.comm_size % 2 == 0;
    MPI_Comm_split(MPI_COMM_WORLD, is_writer, info.rank, &writer_comm);
    MPI_Comm_split(MPI_COMM_WORLD, !is_writer, info.rank, &reader_comm);
  }
  int write_rank, write_comm_size;
  MPI_Comm_rank(writer_comm, &write_rank);
  MPI_Comm_size(writer_comm, &write_comm_size);

  int read_rank, read_comm_size;
  MPI_Comm_rank(reader_comm, &read_rank);
  MPI_Comm_size(reader_comm, &read_comm_size);

  if (tt::FileSharing::SHARED_FILE == args.file_sharing) {
    filename_str = args.filename + "_" + std::to_string(info.comm_size);
  } else if (tt::FileSharing::SHARED_FILE == args.file_sharing) {
    if (is_writer) {
      filename_str = args.filename + "_" + std::to_string(write_rank) +
                      "_of_" + std::to_string(write_comm_size);
    } else {
      filename_str = args.filename + "_" + std::to_string(read_rank) + "_of_" +
                      std::to_string(read_comm_size);
    }
  } else {
    REQUIRE(1 == 0);
  }
  REQUIRE(clean_directories() == 0);
  REQUIRE(create_file(info.pfs / filename_str,
                      args.request_size * args.iteration) == 0);
  fs::path pfs_filename = info.pfs / filename_str;
  Timer init_time, finalize_time, open_time, close_time, write_time, read_time;
  char usecase[256];
  SECTION("storage.pfs") {
    strcpy(usecase, "pfs");
    open_time.resumeTime();
    MPI_File fh_orig;
    int status_orig = -1;
    if (is_writer) {
      status_orig = MPI_File_open(writer_comm, pfs_filename.c_str(),
                                  MPI_MODE_WRONLY | MPI_MODE_CREATE,
                                  MPI_INFO_NULL, &fh_orig);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (!is_writer) {
      status_orig = MPI_File_open(reader_comm, pfs_filename.c_str(),
                                  MPI_MODE_RDONLY, MPI_INFO_NULL, &fh_orig);
    }
    open_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
    if (is_writer) {
      auto write_data =
          std::vector<char>(args.request_size * args.iteration, 'w');
      write_time.resumeTime();
      MPI_Status stat_orig;
      off_t base_offset =
          (off_t)write_rank * args.request_size * args.iteration;
      off_t relative_offset = 0;
      auto ret_orig = MPI_File_write_at_all(
          fh_orig, base_offset + relative_offset, write_data.data(),
          args.request_size * args.iteration, MPI_CHAR, &stat_orig);
      int written_bytes;
      MPI_Get_count(&stat_orig, MPI_CHAR, &written_bytes);
      write_time.pauseTime();
      REQUIRE(written_bytes == args.request_size * args.iteration);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (!is_writer) {
      for (int j = 0; j < 100; ++j) {
        for (int i = 0; i < args.iteration; ++i) {
          auto read_data = std::vector<char>(args.request_size, 'r');
          read_time.resumeTime();
          MPI_Status stat_orig;
          off_t base_offset = 0;
          if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
            base_offset = (off_t)info.rank * args.request_size * args.iteration;
          }
          off_t relative_offset = i * args.request_size;
          auto ret_orig = MPI_File_read_at_all(
              fh_orig, base_offset + relative_offset, read_data.data(),
              args.request_size, MPI_CHAR, &stat_orig);
          int read_bytes;
          MPI_Get_count(&stat_orig, MPI_CHAR, &read_bytes);
          read_time.pauseTime();
          REQUIRE(read_bytes == args.request_size);
          for (auto &val : read_data) {
            REQUIRE(val == 'w');
          }
        }
      }
    }
    close_time.resumeTime();
    status_orig = MPI_File_close(&fh_orig);
    close_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
  }
  SECTION("storage.unifyfs") {
    strcpy(usecase, "unifyfs");
    int rc;
    /* Initialize unifyfs */
    unifyfs_handle fshdl;
    tt::buildUnifyFSOptions(tailorfs::test::WORM, &fshdl, &init_time);
    fs::path unifyfs_filename = info.unifyfs_path / filename_str;

    if (is_writer) {
      unifyfs_gfid gfid;
      /* Create or Open file for production */
      if (write_rank == 0) {
        int create_flags = 0;
        open_time.resumeTime();
        rc = unifyfs_create(fshdl, create_flags, unifyfs_filename.c_str(),
                            &gfid);
        open_time.pauseTime();
      }
      MPI_Barrier(writer_comm);
      if (write_rank != 0) {
        int access_flags = O_WRONLY;
        open_time.resumeTime();
        rc = unifyfs_open(fshdl, access_flags, unifyfs_filename.c_str(), &gfid);
        open_time.pauseTime();
      }
      REQUIRE(rc == UNIFYFS_SUCCESS);
      REQUIRE(gfid != UNIFYFS_INVALID_GFID);

      /* Write data to file */
      int max_buff = args.iteration + 1;
      int processed = 0;
      int j = 0;
      while (processed < args.iteration) {
        auto num_req_to_buf = args.iteration - processed >= max_buff
                                  ? max_buff
                                  : args.iteration - processed;
        auto write_data =
            std::vector<char>(args.request_size * num_req_to_buf, 'w');
        int write_req_ct = num_req_to_buf + 1;
        unifyfs_io_request write_req[num_req_to_buf + 2];

        for (int i = 0; i < num_req_to_buf; ++i) {
          write_req[i].op = UNIFYFS_IOREQ_OP_WRITE;
          write_req[i].gfid = gfid;
          write_req[i].nbytes = args.request_size;
          off_t base_offset = 0;
          if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
            base_offset = (off_t)info.rank * args.request_size * args.iteration;
          }
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
    rc = unifyfs_laminate(fshdl, unifyfs_filename.c_str());
    MPI_Barrier(MPI_COMM_WORLD);
    if (!is_writer) {
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
              INFO("UNIFYFS ERROR: "
                   << "OP_READ req failed - "
                   << strerror(read_sync[i].result.error));
            REQUIRE(read_sync[i].result.error == 0);
          }
        }
      }

      /* Read data from file */
      for (int j = 0; j < 100; ++j) {
        for (int i = 0; i < args.iteration; ++i) {
          off_t random_i = i;
          if (args.access_pattern == tt::AccessPattern::RANDOM) {
            random_i = rand() % args.iteration;
          }
          auto read_data = std::vector<char>(args.request_size, 'r');
          unifyfs_io_request read_req;
          read_req.op = UNIFYFS_IOREQ_OP_READ;
          read_req.gfid = gfid;
          read_req.nbytes = args.request_size;
          off_t base_offset =
              (off_t)read_rank * args.request_size * args.iteration;
          off_t relative_offset = random_i * args.request_size;
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
                INFO("UNIFYFS ERROR: "
                     << "OP_READ req failed - "
                     << strerror(read_req.result.error));
              REQUIRE(read_req.result.error == 0);
              if (read_req.result.count != args.request_size)
                INFO("UNIFYFS ERROR: "
                     << "OP_READ req failed - rank " << info.rank << " iter "
                     << random_i << ", and read only " << read_req.result.count
                     << " of " << args.request_size << " with offset "
                     << read_req.offset);
              REQUIRE(read_req.result.count == args.request_size);
            }
          }
        }
      }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    finalize_time.resumeTime();
    rc = unifyfs_finalize(fshdl);
    finalize_time.pauseTime();
    REQUIRE(rc == UNIFYFS_SUCCESS);
  }
  AGGREGATE_TIME(init);
  AGGREGATE_TIME(finalize);
  AGGREGATE_TIME(open);
  AGGREGATE_TIME(close);
  AGGREGATE_TIME(read);
  AGGREGATE_TIME(write);
  if (info.rank == 0) {
    PRINT_MSG("%10s,%10d,%10d,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10s,%10s,%d,%d,%d,%d\n",
              "Timing", args.iteration, args.request_size, total_init / info.comm_size, total_finalize / info.comm_size ,
              total_open / info.comm_size, total_close / info.comm_size, total_write / info.comm_size, total_read / info.comm_size,
              usecase, "update", args.storage_type, args.access_pattern, args.file_sharing, args.process_grouping);
  }
  MPI_Barrier(MPI_COMM_WORLD);
}