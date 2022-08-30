#include <catch_config.h>
#include <fcntl.h>
#include <mpi.h>
#include <test_utils.h>

#include <experimental/filesystem>
#include <iomanip>

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
  bool tailorfs = false;
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
  char* debug_str = getenv("IOTEST_DEBUG");
  args.debug = false;
  if(debug_str != nullptr) {
    if(strcmp(debug_str, "1") == 0) {
      args.debug = true;
    }
  }
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
         cl::Opt(args.ranks_per_node,
                 "rpn")["-n"]["--ranks_per_node"]("Ranks per node") |
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
         cl::Opt(args.tailorfs, "tailorfs")["--tailorfs"]("tailorfs")|
         cl::Opt(args.debug, "debug")["-d"]["--debug"]("Enable Debug");
}

/**
 * Helper functions
 */
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
  info.pfs = fs::path(PFS_VAR) / "tailorfs" / "data";
  info.bb = fs::path(BB_VAR) / "tailorfs" / "data";
  info.shm = fs::path(SHM_VAR) / "tailorfs" / "data";
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
  }
  fs::create_directories(info.pfs);
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
  INFO("pfs " << info.pfs);
  fs::create_directories(info.pfs);
  INFO("filename " << filename);
  off_t base_offset = 0;
  switch (args.file_sharing) {
    case tt::FileSharing::PER_PROCESS: {
      status_orig = MPI_File_open(MPI_COMM_SELF, filename.c_str(),
                                  MPI_MODE_RDWR | MPI_MODE_CREATE,
                                  MPI_INFO_NULL, &fh_orig);
      char msg[MPI_MAX_ERROR_STRING];
      int resultlen;
      MPI_Error_string(status_orig, msg, &resultlen);
      INFO("status_orig " << msg);
      REQUIRE(status_orig == MPI_SUCCESS);
      break;
    }
    case tt::FileSharing::SHARED_FILE: {
      status_orig = MPI_File_open(MPI_COMM_WORLD, filename.c_str(),
                                  MPI_MODE_RDWR | MPI_MODE_CREATE,
                                  MPI_INFO_NULL, &fh_orig);
      base_offset = (off_t)info.rank * blocksize;
      REQUIRE(status_orig == MPI_SUCCESS);
      break;
    }
  }

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
          std::string() +
              std::string("[type=write-only][optimization=buffered_write]") +
              CONVERT_ENUM(access_pattern, args.access_pattern) +
              CONVERT_ENUM(storage_type, args.storage_type) +
              CONVERT_ENUM(file_sharing, args.file_sharing) +
              CONVERT_ENUM(process_grouping, args.process_grouping)) {
  REQUIRE(pretest() == 0);
  switch (args.file_sharing) {
    case tt::FileSharing::PER_PROCESS: {
      args.filename = args.filename + "_" + std::to_string(info.rank);
      break;
    }
    case tt::FileSharing::SHARED_FILE: {
      args.filename = args.filename + "_0";
      break;
    }
  }
  REQUIRE(args.process_grouping == tt::ProcessGrouping::ALL_PROCESS);
  REQUIRE(args.access_pattern == tt::AccessPattern::SEQUENTIAL);
  REQUIRE(clean_directories() == 0);

  Timer init_time, finalize_time, open_time, close_time, write_time;
  bool is_run = false;
  INFO("rank " << info.rank);
  fs::path filename = info.pfs / args.filename;
  INFO("filename " << filename);

  char interface[256];
  if (args.file_sharing == tt::FileSharing::PER_PROCESS) {
    strcpy(interface, "POSIX");
    open_time.resumeTime();
    int fd = open(filename.c_str(), O_CREAT | O_WRONLY, 0666);
    open_time.pauseTime();
    REQUIRE(fd != -1);
    for (int i = 0; i < args.iteration; ++i) {
      auto write_data = std::vector<char>(args.request_size, 'w');
      write_time.resumeTime();
      ssize_t written_bytes = write(fd, write_data.data(), args.request_size);
      write_time.pauseTime();

      REQUIRE(written_bytes == args.request_size);
    }
    close_time.resumeTime();
    int status = close(fd);
    close_time.pauseTime();
    assert(status == 0);
    is_run = true;
  } else if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
    strcpy(interface, "MPIIO");
    MPI_File fh_orig;
    int status_orig;
    open_time.resumeTime();
    status_orig =
        MPI_File_open(MPI_COMM_WORLD, filename.c_str(),
                      MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &fh_orig);
    open_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
    for (int i = 0; i < args.iteration; ++i) {
      auto write_data = std::vector<char>(args.request_size, 'w');

      MPI_Status stat_orig;
      off_t base_offset = (off_t)info.rank * args.request_size * args.iteration;
      off_t relative_offset = i * args.request_size;
      write_time.resumeTime();
      auto ret_orig = MPI_File_write_at_all(
          fh_orig, base_offset + relative_offset, write_data.data(),
          args.request_size, MPI_CHAR, &stat_orig);
      write_time.pauseTime();
    }
    close_time.resumeTime();
    status_orig = MPI_File_close(&fh_orig);
    close_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
    is_run = true;
  }
  if (is_run) {
    AGGREGATE_TIME(init);
    AGGREGATE_TIME(finalize);
    AGGREGATE_TIME(open);
    AGGREGATE_TIME(close);
    AGGREGATE_TIME(write);
    if (info.rank == 0) {
      PRINT_MSG(
          "%10s,%10d,%10d,"
          "%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,"
          "%10s,%10s,"
          "%d,%d,%d,%d\n",
          "Timing", args.iteration, args.request_size,
          total_init / info.comm_size, total_finalize / info.comm_size,
          total_open / info.comm_size, total_close / info.comm_size,
          total_write / info.comm_size, 0.0, interface, "wo", args.storage_type,
          args.access_pattern, args.file_sharing, args.process_grouping);
    }
  }
  REQUIRE(posttest() == 0);
}
TEST_CASE("Read-Only",
          std::string("[type=read-only]") +
              std::string("[optimization=buffered_read]") +
              CONVERT_ENUM(access_pattern, args.access_pattern) +
              CONVERT_ENUM(storage_type, args.storage_type) +
              CONVERT_ENUM(file_sharing, args.file_sharing) +
              CONVERT_ENUM(process_grouping, args.process_grouping)) {
  REQUIRE(pretest() == 0);
  std::string filename_str;
  INFO(args.filename);
  switch (args.file_sharing) {
    case tt::FileSharing::PER_PROCESS: {
      filename_str = args.filename + "_" + std::to_string(info.rank);
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

  Timer init_time, finalize_time, open_time, close_time, read_time;
  char interface[256];
  bool is_run = false;
  if (args.file_sharing == tt::FileSharing::PER_PROCESS) {
    strcpy(interface, "POSIX");
    open_time.resumeTime();
    int fd = open(pfs_filename.c_str(), O_RDONLY);
    open_time.pauseTime();
    REQUIRE(fd != -1);
    for (int i = 0; i < args.iteration; ++i) {
      auto read_data = std::vector<char>(args.request_size, 'r');
      MPI_Status stat_orig;
      off_t random_i = i;
      if (args.access_pattern == tt::AccessPattern::RANDOM) {
        random_i = rand() % args.iteration;
      }
      off_t offset = random_i * args.request_size;
      if (args.access_pattern == tt::AccessPattern::RANDOM) {
        off_t status = lseek(fd, offset, SEEK_SET);
        REQUIRE(status == offset);
      }
      read_time.resumeTime();
      ssize_t read_bytes = read(fd, read_data.data(), args.request_size);
      read_time.pauseTime();
      REQUIRE(read_bytes == args.request_size);
    }
    close_time.resumeTime();
    int status = close(fd);
    close_time.pauseTime();
    assert(status == 0);
    is_run = true;
  } else if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
    strcpy(interface, "MPIIO");
    MPI_File fh_orig;
    int status_orig;
    open_time.resumeTime();
    status_orig = MPI_File_open(MPI_COMM_WORLD, pfs_filename.c_str(),
                                MPI_MODE_RDONLY, MPI_INFO_NULL, &fh_orig);
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
      off_t base_offset = (off_t)info.rank * args.request_size * args.iteration;
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
    is_run = true;
  }
  AGGREGATE_TIME(init);
  AGGREGATE_TIME(finalize);
  AGGREGATE_TIME(open);
  AGGREGATE_TIME(close);
  AGGREGATE_TIME(read);
  if (is_run && info.rank == 0) {
    PRINT_MSG(
        "%10s,%10d,%10d,"
        "%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,"
        "%10s,%10s,%d,%d,%d,%d\n",
        "Timing", args.iteration, args.request_size,
        total_init / info.comm_size, total_finalize / info.comm_size,
        total_open / info.comm_size, total_close / info.comm_size,
        total_read / info.comm_size, interface, "ro", args.storage_type,
        args.access_pattern, args.file_sharing, args.process_grouping);
  }
  REQUIRE(posttest() == 0);
}
TEST_CASE("Read-After-Write",
          std::string("[type=raw]") +
              std::string("[optimization=node_local_io]") +
              CONVERT_ENUM(access_pattern, args.access_pattern) +
              CONVERT_ENUM(storage_type, args.storage_type) +
              CONVERT_ENUM(file_sharing, args.file_sharing) +
              CONVERT_ENUM(process_grouping, args.process_grouping)) {
  std::string filename_str;
  REQUIRE(pretest() == 0);

  INFO("rank " << info.rank);
  if (args.process_grouping != tt::ProcessGrouping::ALL_PROCESS) {
    REQUIRE(info.comm_size > 1);
    REQUIRE(info.comm_size % 2 == 0);
  }
  MPI_Comm writer_comm = MPI_COMM_WORLD;
  MPI_Comm reader_comm = MPI_COMM_WORLD;
  bool is_writer = false;
  bool is_reader = false;
  if (args.process_grouping == tt::ProcessGrouping::ALL_PROCESS) {
    writer_comm = MPI_COMM_WORLD;
    reader_comm = MPI_COMM_WORLD;
    is_writer = true;
    is_reader = true;

  } else if (args.process_grouping == tt::ProcessGrouping::SPLIT_PROCESS_HALF) {
    is_writer = info.rank / (info.comm_size / 2) == 0;
    is_reader = !is_writer;
    MPI_Comm_split(MPI_COMM_WORLD, is_writer, info.rank, &writer_comm);
    MPI_Comm_split(MPI_COMM_WORLD, is_reader, info.rank, &reader_comm);
  } else if (args.process_grouping ==
             tt::ProcessGrouping::SPLIT_PROCESS_ALTERNATE) {
    is_writer = info.rank % 2 == 0;
    is_reader = !is_writer;
    MPI_Comm_split(MPI_COMM_WORLD, is_writer, info.rank, &writer_comm);
    MPI_Comm_split(MPI_COMM_WORLD, is_reader, info.rank, &reader_comm);
  }
  int write_rank, write_comm_size;
  MPI_Comm_rank(writer_comm, &write_rank);
  MPI_Comm_size(writer_comm, &write_comm_size);

  int read_rank, read_comm_size;
  MPI_Comm_rank(reader_comm, &read_rank);
  MPI_Comm_size(reader_comm, &read_comm_size);

  if (tt::FileSharing::SHARED_FILE == args.file_sharing) {
    if (is_writer) {
      filename_str = args.filename + "_" + std::to_string(write_comm_size);
    } else {
      filename_str = args.filename + "_" + std::to_string(read_comm_size);
    }
  } else if (tt::FileSharing::PER_PROCESS == args.file_sharing) {
    if (is_writer) {
      filename_str = args.filename + "_" + std::to_string(write_rank);
    } else {
      filename_str = args.filename + "_" + std::to_string(read_rank);
    }
  } else {
    REQUIRE(1 == 0);
  }
  REQUIRE(clean_directories() == 0);
  INFO("filename " << filename_str);
  fs::path pfs_filename = info.pfs / filename_str;
  Timer init_time, finalize_time, open_time, close_time, write_time, read_time;
  char interface[256];
  bool is_run = false;
  if (tt::FileSharing::PER_PROCESS == args.file_sharing) {
    strcpy(interface, "POSIX");
    if (is_writer) {
      open_time.resumeTime();
      int fd = open(pfs_filename.c_str(), O_CREAT | O_WRONLY, 0666);
      open_time.pauseTime();
      REQUIRE(fd != -1);
      for (int i = 0; i < args.iteration; ++i) {
        auto write_data = std::vector<char>(args.request_size, 'w');
        write_time.resumeTime();
        ssize_t written_bytes = write(fd, write_data.data(), args.request_size);
        write_time.pauseTime();
        REQUIRE(written_bytes == args.request_size);
      }
      close_time.resumeTime();
      int status = close(fd);
      close_time.pauseTime();
      assert(status == 0);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (is_reader) {
      open_time.resumeTime();
      int fd = open(pfs_filename.c_str(), O_RDONLY);
      open_time.pauseTime();
      REQUIRE(fd != -1);
      for (int i = 0; i < args.iteration; ++i) {
        off_t random_i = i;
        if (args.access_pattern == tt::AccessPattern::RANDOM) {
          random_i = rand() % args.iteration;
        }
        auto read_data = std::vector<char>(args.request_size, 'r');
        read_time.resumeTime();
        MPI_Status stat_orig;
        off_t offset = random_i * args.request_size;
        if (args.access_pattern == tt::AccessPattern::RANDOM) {
          off_t status = lseek(fd, offset, SEEK_SET);
          REQUIRE(status == offset);
        }
        read_time.resumeTime();
        ssize_t read_bytes = read(fd, read_data.data(), args.request_size);
        read_time.pauseTime();
        REQUIRE(read_bytes == args.request_size);
      }
      close_time.resumeTime();
      int status = close(fd);
      close_time.pauseTime();
      assert(status == 0);
    }
    is_run = true;
  } else if (tt::FileSharing::SHARED_FILE == args.file_sharing) {
    strcpy(interface, "MPIIO");
    MPI_File fh_orig;
    int status_orig = -1;
    if (is_writer) {
      open_time.resumeTime();
      status_orig = MPI_File_open(writer_comm, pfs_filename.c_str(),
                                  MPI_MODE_WRONLY | MPI_MODE_CREATE,
                                  MPI_INFO_NULL, &fh_orig);
      open_time.pauseTime();
      char msg[MPI_MAX_ERROR_STRING];
      int resultlen;
      MPI_Error_string(status_orig, msg, &resultlen);
      INFO("status_orig " << msg);
      REQUIRE(status_orig == MPI_SUCCESS);
      for (int i = 0; i < args.iteration; ++i) {
        auto write_data = std::vector<char>(args.request_size, 'w');
        write_time.resumeTime();
        MPI_Status stat_orig;
        off_t base_offset =
            (off_t)write_rank * args.request_size * args.iteration;
        off_t relative_offset = i * args.request_size;
        INFO("offset " << base_offset + relative_offset << " i" << i);
        auto ret_orig = MPI_File_write_at_all(
            fh_orig, base_offset + relative_offset, write_data.data(),
            args.request_size, MPI_CHAR, &stat_orig);
        int written_bytes = 0;
        MPI_Get_count(&stat_orig, MPI_CHAR, &written_bytes);
        write_time.pauseTime();
        REQUIRE(written_bytes == args.request_size);
      }
      close_time.resumeTime();
      status_orig = MPI_File_close(&fh_orig);
      close_time.pauseTime();
      REQUIRE(status_orig == MPI_SUCCESS);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (is_reader) {
      open_time.resumeTime();
      status_orig = MPI_File_open(reader_comm, pfs_filename.c_str(),
                                  MPI_MODE_RDONLY, MPI_INFO_NULL, &fh_orig);
      open_time.pauseTime();
      char msg[MPI_MAX_ERROR_STRING];
      int resultlen;
      MPI_Error_string(status_orig, msg, &resultlen);
      INFO("status_orig " << msg);
      REQUIRE(status_orig == MPI_SUCCESS);
      for (int i = 0; i < args.iteration; ++i) {
        auto read_data = std::vector<char>(args.request_size, 'r');
        read_time.resumeTime();
        MPI_Status stat_orig;
        off_t base_offset = 0;
        if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
          base_offset = (off_t)read_rank * args.request_size * args.iteration;
        }
        off_t relative_offset = i * args.request_size;
        INFO("offset " << base_offset + relative_offset << " i " << i);
        auto ret_orig = MPI_File_read_at_all(
            fh_orig, base_offset + relative_offset, read_data.data(),
            args.request_size, MPI_CHAR, &stat_orig);
        int read_bytes;
        MPI_Error_string(ret_orig, msg, &resultlen);
        INFO("ret_orig " << msg);
        MPI_Get_count(&stat_orig, MPI_CHAR, &read_bytes);
        read_time.pauseTime();
        REQUIRE(read_bytes == args.request_size);
        for (auto &val : read_data) {
          REQUIRE(val == 'w');
        }
      }
      close_time.resumeTime();
      status_orig = MPI_File_close(&fh_orig);
      close_time.pauseTime();
      REQUIRE(status_orig == MPI_SUCCESS);
    }
    is_run = true;
  }
  AGGREGATE_TIME(init);
  AGGREGATE_TIME(finalize);
  AGGREGATE_TIME(open);
  AGGREGATE_TIME(close);
  AGGREGATE_TIME(read);
  AGGREGATE_TIME(write);
  if (is_run && info.rank == 0) {
    PRINT_MSG(
        "%10s,%10d,%10d,"
        "%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,"
        "%10s,%10s,%d,%d,%d,%d\n",
        "Timing", args.iteration, args.request_size,
        total_init / info.comm_size, total_finalize / info.comm_size,
        total_open / info.comm_size, total_close / info.comm_size,
        total_write / write_comm_size, total_read / read_comm_size, interface,
        "raw", args.storage_type, args.access_pattern, args.file_sharing,
        args.process_grouping);
  }
  MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE("Update", std::string("[type=update]") +
                        std::string("[optimization=buffered_write]") +
                        CONVERT_ENUM(access_pattern, args.access_pattern) +
                        CONVERT_ENUM(storage_type, args.storage_type) +
                        CONVERT_ENUM(file_sharing, args.file_sharing) +
                        CONVERT_ENUM(process_grouping, args.process_grouping)) {
  REQUIRE(pretest() == 0);
  switch (args.file_sharing) {
    case tt::FileSharing::PER_PROCESS: {
      args.filename = args.filename + "_" + std::to_string(info.rank);
      create_file(info.pfs / args.filename, args.request_size * args.iteration);
      break;
    }
    case tt::FileSharing::SHARED_FILE: {
      args.filename = args.filename + "_" + std::to_string(info.comm_size);
      create_file(info.pfs / args.filename,
                  args.request_size * args.iteration * info.comm_size);
      break;
    }
  }
  REQUIRE(args.process_grouping == tt::ProcessGrouping::ALL_PROCESS);
  REQUIRE(clean_directories() == 0);

  Timer init_time, finalize_time, open_time, close_time, write_time;
  char interface[256];
  bool is_run = false;
  INFO("rank " << info.rank);

  fs::path filename = info.pfs / args.filename;
  if (args.file_sharing == tt::FileSharing::PER_PROCESS) {
    strcpy(interface, "POSIX");
    open_time.resumeTime();
    int fd = open(filename.c_str(), O_CREAT | O_WRONLY, 0666);
    open_time.pauseTime();
    REQUIRE(fd != -1);
    for (int i = 0; i < args.iteration; ++i) {
      auto write_data = std::vector<char>(args.request_size, 'w');
      off_t random_i = i;
      if (args.access_pattern == tt::AccessPattern::RANDOM) {
        random_i = rand() % args.iteration;
      }
      off_t offset = random_i * args.request_size;
      if (args.access_pattern == tt::AccessPattern::RANDOM) {
        off_t status = lseek(fd, offset, SEEK_SET);
        REQUIRE(status == offset);
      }
      write_time.resumeTime();
      ssize_t written_bytes = write(fd, write_data.data(), args.request_size);
      write_time.pauseTime();
      REQUIRE(written_bytes == args.request_size);
    }
    close_time.resumeTime();
    int status = close(fd);
    close_time.pauseTime();
    assert(status == 0);
    is_run = true;
  } else if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
    strcpy(interface, "MPIIO");
    MPI_File fh_orig;
    int status_orig;
    open_time.resumeTime();
    status_orig =
        MPI_File_open(MPI_COMM_WORLD, filename.c_str(),
                      MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &fh_orig);
    open_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
    for (int i = 0; i < args.iteration; ++i) {
      auto write_data = std::vector<char>(args.request_size, 'w');
      MPI_Status stat_orig;
      off_t base_offset = (off_t)info.rank * args.request_size * args.iteration;
      off_t random_i = i;
      if (args.access_pattern == tt::AccessPattern::RANDOM) {
        random_i = rand() % args.iteration;
      }
      off_t relative_offset = random_i * args.request_size;
      write_time.resumeTime();
      auto ret_orig = MPI_File_write_at_all(
          fh_orig, base_offset + relative_offset, write_data.data(),
          args.request_size, MPI_CHAR, &stat_orig);
      write_time.pauseTime();
      int written_bytes;
      MPI_Get_count(&stat_orig, MPI_CHAR, &written_bytes);
      REQUIRE(written_bytes == args.request_size);
    }
    close_time.resumeTime();
    status_orig = MPI_File_close(&fh_orig);
    close_time.pauseTime();
    REQUIRE(status_orig == MPI_SUCCESS);
    is_run = true;
  }
  if (is_run) {
    AGGREGATE_TIME(init);
    AGGREGATE_TIME(finalize);
    AGGREGATE_TIME(open);
    AGGREGATE_TIME(close);
    AGGREGATE_TIME(write);
    if (info.rank == 0) {
      PRINT_MSG(
          "%10s,%10d,%10d,"
          "%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,"
          "%10s,%10s,%d,%d,%d,%d\n",
          "Timing", args.iteration, args.request_size,
          total_init / info.comm_size, total_finalize / info.comm_size,
          total_open / info.comm_size, total_close / info.comm_size,
          total_write / info.comm_size, 0, interface, "up", args.storage_type,
          args.access_pattern, args.file_sharing, args.process_grouping);
    }
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

  INFO("rank " << info.rank);
  if (args.process_grouping != tt::ProcessGrouping::ALL_PROCESS) {
    REQUIRE(info.comm_size > 1);
    REQUIRE(info.comm_size % 2 == 0);
  }
  MPI_Comm writer_comm = MPI_COMM_WORLD;
  MPI_Comm reader_comm = MPI_COMM_WORLD;
  bool is_writer = false;
  bool is_reader = false;
  if (args.process_grouping == tt::ProcessGrouping::ALL_PROCESS) {
    writer_comm = MPI_COMM_WORLD;
    reader_comm = MPI_COMM_WORLD;
    is_writer = true;
    is_reader = true;

  } else if (args.process_grouping == tt::ProcessGrouping::SPLIT_PROCESS_HALF) {
    is_writer = info.rank / (info.comm_size / 2) == 0;
    is_reader = !is_writer;
    MPI_Comm_split(MPI_COMM_WORLD, is_writer, info.rank, &writer_comm);
    MPI_Comm_split(MPI_COMM_WORLD, is_reader, info.rank, &reader_comm);
  } else if (args.process_grouping ==
             tt::ProcessGrouping::SPLIT_PROCESS_ALTERNATE) {
    is_writer = info.rank % 2 == 0;
    is_reader = !is_writer;
    MPI_Comm_split(MPI_COMM_WORLD, is_writer, info.rank, &writer_comm);
    MPI_Comm_split(MPI_COMM_WORLD, is_reader, info.rank, &reader_comm);
  }
  int write_rank, write_comm_size;
  MPI_Comm_rank(writer_comm, &write_rank);
  MPI_Comm_size(writer_comm, &write_comm_size);

  int read_rank, read_comm_size;
  MPI_Comm_rank(reader_comm, &read_rank);
  MPI_Comm_size(reader_comm, &read_comm_size);

  if (tt::FileSharing::SHARED_FILE == args.file_sharing) {
    if (is_writer) {
      filename_str = args.filename + "_" + std::to_string(write_comm_size);
    } else {
      filename_str = args.filename + "_" + std::to_string(read_comm_size);
    }
  } else if (tt::FileSharing::PER_PROCESS == args.file_sharing) {
    if (is_writer) {
      filename_str = args.filename + "_" + std::to_string(write_rank);
    } else {
      filename_str = args.filename + "_" + std::to_string(read_rank);
    }
  } else {
    REQUIRE(1 == 0);
  }
  REQUIRE(clean_directories() == 0);
  INFO("filename " << filename_str);
  fs::path pfs_filename = info.pfs / filename_str;
  Timer init_time, finalize_time, open_time, close_time, write_time, read_time;
  char interface[256];
  bool is_run = false;
  if (tt::FileSharing::PER_PROCESS == args.file_sharing) {
    strcpy(interface, "POSIX");
    if (is_writer) {
      open_time.resumeTime();
      int fd = open(pfs_filename.c_str(), O_CREAT | O_WRONLY, 0666);
      open_time.pauseTime();
      REQUIRE(fd != -1);
      auto write_data =
          std::vector<char>(args.request_size * args.iteration, 'w');
      write_time.resumeTime();
      ssize_t written_bytes =
          write(fd, write_data.data(), args.request_size * args.iteration);
      write_time.pauseTime();
      REQUIRE(written_bytes == args.request_size * args.iteration);
      close_time.resumeTime();
      int status = close(fd);
      close_time.pauseTime();
      assert(status == 0);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (is_reader) {
      open_time.resumeTime();
      int fd = open(pfs_filename.c_str(), O_RDONLY);
      open_time.pauseTime();
      REQUIRE(fd != -1);
      for (int i = 0; i < args.iteration; ++i) {
        off_t random_i = i;
        if (args.access_pattern == tt::AccessPattern::RANDOM) {
          random_i = rand() % args.iteration;
        }
        auto read_data = std::vector<char>(args.request_size, 'r');
        read_time.resumeTime();
        MPI_Status stat_orig;
        off_t offset = random_i * args.request_size;
        if (args.access_pattern == tt::AccessPattern::RANDOM) {
          off_t status = lseek(fd, offset, SEEK_SET);
          REQUIRE(status == offset);
        }
        read_time.resumeTime();
        ssize_t read_bytes = read(fd, read_data.data(), args.request_size);
        read_time.pauseTime();
        REQUIRE(read_bytes == args.request_size);
      }
      close_time.resumeTime();
      int status = close(fd);
      close_time.pauseTime();
      assert(status == 0);
    }
    is_run = true;
  } else if (tt::FileSharing::SHARED_FILE == args.file_sharing) {
    strcpy(interface, "MPIIO");
    MPI_File fh_orig;
    int status_orig = -1;
    if (is_writer) {
      open_time.resumeTime();
      status_orig = MPI_File_open(writer_comm, pfs_filename.c_str(),
                                  MPI_MODE_WRONLY | MPI_MODE_CREATE,
                                  MPI_INFO_NULL, &fh_orig);
      open_time.pauseTime();
      char msg[MPI_MAX_ERROR_STRING];
      int resultlen;
      MPI_Error_string(status_orig, msg, &resultlen);
      INFO("status_orig " << msg);
      REQUIRE(status_orig == MPI_SUCCESS);
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
      int written_bytes = 0;
      MPI_Get_count(&stat_orig, MPI_CHAR, &written_bytes);
      write_time.pauseTime();
      REQUIRE(written_bytes == args.request_size * args.iteration);
      close_time.resumeTime();
      status_orig = MPI_File_close(&fh_orig);
      close_time.pauseTime();
      REQUIRE(status_orig == MPI_SUCCESS);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (is_reader) {
      open_time.resumeTime();
      status_orig = MPI_File_open(reader_comm, pfs_filename.c_str(),
                                  MPI_MODE_RDONLY, MPI_INFO_NULL, &fh_orig);
      open_time.pauseTime();
      char msg[MPI_MAX_ERROR_STRING];
      int resultlen;
      MPI_Error_string(status_orig, msg, &resultlen);
      INFO("status_orig " << msg);
      REQUIRE(status_orig == MPI_SUCCESS);
      for (int i = 0; i < args.iteration; ++i) {
        auto read_data = std::vector<char>(args.request_size, 'r');
        read_time.resumeTime();
        MPI_Status stat_orig;
        off_t base_offset = 0;
        if (args.file_sharing == tt::FileSharing::SHARED_FILE) {
          base_offset = (off_t)read_rank * args.request_size * args.iteration;
        }
        off_t relative_offset = i * args.request_size;
        INFO("offset " << base_offset + relative_offset << " i " << i);
        auto ret_orig = MPI_File_read_at_all(
            fh_orig, base_offset + relative_offset, read_data.data(),
            args.request_size, MPI_CHAR, &stat_orig);
        int read_bytes;
        MPI_Error_string(ret_orig, msg, &resultlen);
        INFO("ret_orig " << msg);
        MPI_Get_count(&stat_orig, MPI_CHAR, &read_bytes);
        read_time.pauseTime();
        REQUIRE(read_bytes == args.request_size);
        for (auto &val : read_data) {
          REQUIRE(val == 'w');
        }
      }
      close_time.resumeTime();
      status_orig = MPI_File_close(&fh_orig);
      close_time.pauseTime();
      REQUIRE(status_orig == MPI_SUCCESS);
    }
    is_run = true;
  }
  AGGREGATE_TIME(init);
  AGGREGATE_TIME(finalize);
  AGGREGATE_TIME(open);
  AGGREGATE_TIME(close);
  AGGREGATE_TIME(read);
  AGGREGATE_TIME(write);
  if (info.rank == 0) {
    PRINT_MSG(
        "%10s,%10d,%10d,"
        "%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,%10.6f,"
        "%10s,%10s,%d,%d,%d,%d\n",
        "Timing", args.iteration, args.request_size,
        total_init / info.comm_size, total_finalize / info.comm_size,
        total_open / info.comm_size, total_close / info.comm_size,
        total_write / write_comm_size, total_read / read_comm_size, interface,
        "worm", args.storage_type, args.access_pattern, args.file_sharing,
        args.process_grouping);
  }
  MPI_Barrier(MPI_COMM_WORLD);
}