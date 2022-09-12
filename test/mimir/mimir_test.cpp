#include <catch_config.h>
#include <mimir/mimir.h>
#include <mpi.h>
#include <test_utils.h>

#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>

namespace tailorfs::test {}
namespace tt = tailorfs::test;
namespace fs = std::experimental::filesystem;
const uint64_t KB =1024;
const uint64_t MB = 1024 * 1024;
using namespace mimir;
/**
 * Test data structures
 */
namespace tailorfs::test {
void trim_utf8(std::string& hairy) {
  std::vector<bool> results;
  std::string smooth;
  size_t len = hairy.size();
  results.reserve(len);
  smooth.reserve(len);
  const unsigned char *bytes = (const unsigned char *) hairy.c_str();

  auto read_utf8 = [](const unsigned char *bytes, size_t len, size_t *pos) -> unsigned {
    int code_unit1 = 0;
    int code_unit2, code_unit3, code_unit4;

    if (*pos >= len) goto ERROR1;
    code_unit1 = bytes[(*pos)++];

    if (code_unit1 < 0x80) return code_unit1;
    else if (code_unit1 < 0xC2) goto ERROR1; // continuation or overlong 2-byte sequence
    else if (code_unit1 < 0xE0) {
      if (*pos >= len) goto ERROR1;
      code_unit2 = bytes[(*pos)++]; //2-byte sequence
      if ((code_unit2 & 0xC0) != 0x80) goto ERROR2;
      return (code_unit1 << 6) + code_unit2 - 0x3080;
    }
    else if (code_unit1 < 0xF0) {
      if (*pos >= len) goto ERROR1;
      code_unit2 = bytes[(*pos)++]; // 3-byte sequence
      if ((code_unit2 & 0xC0) != 0x80) goto ERROR2;
      if (code_unit1 == 0xE0 && code_unit2 < 0xA0) goto ERROR2; // overlong
      if (*pos >= len) goto ERROR2;
      code_unit3 = bytes[(*pos)++];
      if ((code_unit3 & 0xC0) != 0x80) goto ERROR3;
      return (code_unit1 << 12) + (code_unit2 << 6) + code_unit3 - 0xE2080;
    }
    else if (code_unit1 < 0xF5) {
      if (*pos >= len) goto ERROR1;
      code_unit2 = bytes[(*pos)++]; // 4-byte sequence
      if ((code_unit2 & 0xC0) != 0x80) goto ERROR2;
      if (code_unit1 == 0xF0 && code_unit2 <  0x90) goto ERROR2; // overlong
      if (code_unit1 == 0xF4 && code_unit2 >= 0x90) goto ERROR2; // > U+10FFFF
      if (*pos >= len) goto ERROR2;
      code_unit3 = bytes[(*pos)++];
      if ((code_unit3 & 0xC0) != 0x80) goto ERROR3;
      if (*pos >= len) goto ERROR3;
      code_unit4 = bytes[(*pos)++];
      if ((code_unit4 & 0xC0) != 0x80) goto ERROR4;
      return (code_unit1 << 18) + (code_unit2 << 12) + (code_unit3 << 6) + code_unit4 - 0x3C82080;
    }
    else goto ERROR1; // > U+10FFFF

  ERROR4:
    (*pos)--;
  ERROR3:
    (*pos)--;
  ERROR2:
    (*pos)--;
  ERROR1:
    return code_unit1 + 0xDC00;
  };

  unsigned c;
  size_t pos = 0;
  size_t pos_before;
  size_t inc = 0;
  bool valid;

  for (;;) {
    pos_before = pos;
    c = read_utf8(bytes, len, &pos);
    inc = pos - pos_before;
    if (!inc) break; // End of string reached.

    valid = false;

    if ( (                 c <= 0x00007F)
        ||   (c >= 0x000080 && c <= 0x0007FF)
        ||   (c >= 0x000800 && c <= 0x000FFF)
        ||   (c >= 0x001000 && c <= 0x00CFFF)
        ||   (c >= 0x00D000 && c <= 0x00D7FF)
        ||   (c >= 0x00E000 && c <= 0x00FFFF)
        ||   (c >= 0x010000 && c <= 0x03FFFF)
        ||   (c >= 0x040000 && c <= 0x0FFFFF)
        ||   (c >= 0x100000 && c <= 0x10FFFF) ) valid = true;

    if (c >= 0xDC00 && c <= 0xDCFF) {
      valid = false;
    }

    do results.push_back(valid); while (--inc);
  }

  size_t sz = results.size();
  for (size_t i = 0; i < sz; ++i) {
    if (results[i]) smooth.append(1, hairy.at(i));
  }

  hairy.swap(smooth);
}
enum StorageType : int { SHM = 0, LOCAL_SSD = 1, PFS = 2 };
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
  std::string binary_directory = "./";
  std::string config_file = "mimir_config.json";
  std::string filename = "test";
  size_t request_size = 65536;
  size_t iteration = 64;
  int ranks_per_node = 1;
  int num_nodes = 1;
  bool debug = false;
  StorageType storage_type = StorageType::LOCAL_SSD;
  AccessPattern access_pattern = AccessPattern::SEQUENTIAL;
  FileSharing file_sharing = FileSharing::PER_PROCESS;
  ProcessGrouping process_grouping = ProcessGrouping::ALL_PROCESS;
  UseCase usecase = UseCase::WRITE_ONLY;
};
//struct Arguments {
//  /* test args */
//  bool debug = false;
//  int ranks_per_node = 1;
//  std::string binary_directory = "./";
//  /* I/O args */
//  int io_size_per_app_mb = 1024;
//  /* main args */
//  std::string config_file = "mimir_config.json";
//  std::string file_prefix = "test";
//  StorageType storage_type = StorageType::PFS;
//  /* Job info */
//  uint16_t num_apps = 1;
//  uint32_t num_process_per_app = 1;
//  uint32_t num_files_per_app = 1;
//  float fpp_percentage = 1.0;
//  float sequential_percentage = 1.0;
//  /* Needs to add up to 1 */
//  float wo_file_ptg = 1.0;
//  float ro_file_ptg = 0.0;
//  float raw_file_ptg = 0.0;
//  float update_file_ptg = 0.0;
//  float worm_file_ptg = 0.0;
//  /* Needs to add up to 1 */
//  float all_app_ptg = 1.0;
//  float split_app_ptg = 0.0;
//  float alt_app_ptg = 0.0;
//};
struct Info {
  fs::path pfs;
  fs::path bb;
  fs::path shm;
  int rank;
  int comm_size;
  char hostname[256];
  std::string original_filename;
};
}  // namespace tailorfs::test
tt::Arguments args;
tt::Info info;


DEFINE_CLARA_OPS(tailorfs::test::StorageType)
DEFINE_CLARA_OPS(tailorfs::test::AccessPattern)
DEFINE_CLARA_OPS(tailorfs::test::FileSharing)
DEFINE_CLARA_OPS(tailorfs::test::ProcessGrouping)
DEFINE_CLARA_OPS(tailorfs::test::UseCase)

/**
 * Overridden methods for catch
 */
int init(int *argc, char ***argv) {
  MPI_Init(argc, argv);
  int rank, comm_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  if (args.debug) {
    TEST_LOGGER->level = cpplogger::LoggerType::LOG_INFO;
  }
  if (args.debug && rank == 0) {
    TEST_LOGGER_INFO("Connect to processes");
    getchar();
  }
  MPI_Barrier(MPI_COMM_WORLD);
  auto test_logger = TEST_LOGGER;
  return 0;
}
int finalize() {
  MPI_Finalize();
  return 0;
}
cl::Parser define_options() {
  return cl::Opt(args.debug, "debug")["--debug"]("Enable debugging.")|
         cl::Opt(args.binary_directory,
                 "binary_directory")["--binary_directory"]("binary_directory.")  |
         cl::Opt(args.config_file,
                 "config_file")["--config_file"]("config_file.")  |
         cl::Opt(args.ranks_per_node,
                 "ranks_per_node")["--ranks_per_node"]("# ranks per node.") |
         cl::Opt(args.num_nodes,
                 "num_nodes")["--num_nodes"]("# num nodes.") |
         cl::Opt(args.filename, "filename")["-f"]["--filename"](
             "Filename to be use for I/O.") |
         cl::Opt(args.request_size, "request_size")["-r"]["--request_size"](
             "Transfer size used for performing I/O") |
         cl::Opt(args.iteration,
                 "iteration")["-i"]["--iteration"]("Number of Iterations") |
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
             "2-> SPLIT_PROCESS_ALTERNATE")|
         cl::Opt(args.usecase,
                 "usecase")["--usecase"](
             "usecase: 0-> WO, 1-> RO, 2-> RAW 3-> Update 4-> WORM");
}

/**
 * Helper functions
 */
int pretest() {
  /* Other pretest operations*/
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

  TEST_LOGGER_INFO_ARGS("rank %d comm_size %d hostname %s", info.rank,
                        info.comm_size, info.hostname);
  int comm_size = args.num_nodes * args.ranks_per_node;
  info.pfs = fs::path(PFS_VAR) / "tailorfs" / ("data_" + std::to_string(comm_size)) ;
  info.bb = fs::path(BB_VAR) / "tailorfs" / ("data_" + std::to_string(comm_size)) ;
  info.shm = fs::path(SHM_VAR) / "tailorfs" / ("data_" + std::to_string(comm_size)) ;
  TEST_LOGGER_INFO_ARGS("pfs %s bb %s shm %s", info.pfs.c_str(),
                        info.bb.c_str(), info.shm.c_str());
  /* Validate args */
  if (args.usecase == tailorfs::test::UseCase::WRITE_ONLY ||
       args.usecase == tailorfs::test::UseCase::READ_ONLY)
    REQUIRE(args.process_grouping == tailorfs::test::ProcessGrouping::ALL_PROCESS);
  if (args.process_grouping == tailorfs::test::ProcessGrouping::SPLIT_PROCESS_ALTERNATE ||
       args.process_grouping == tailorfs::test::ProcessGrouping::SPLIT_PROCESS_HALF)
    REQUIRE(args.ranks_per_node > 1);
  TEST_LOGGER_INFO("Validated args");
  INFO("rank " << info.rank);
  INFO("hostname " << info.hostname);

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
    INFO("BB " << info.bb.string().c_str());
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
std::vector<std::string> split_no_duplicates(std::string x, char delim = ' ') {
  std::vector<std::string> tokens;
  std::unordered_set<std::string> tokens_set;
  std::string token;
  std::stringstream ss(x);
  while (getline(ss, token, delim)){
    tailorfs::test::trim_utf8(token);
    auto iter  = tokens_set.find(token);
    if (iter == tokens_set.end()) {
      tokens.push_back(token);
      tokens_set.emplace(token);
    }
  }
  return tokens;
}
std::vector<std::string> split(std::string x, char delim = ' ') {
  std::vector<std::string> tokens;
  std::string token;
  std::stringstream ss(x);
  while (getline(ss, token, delim)){
    tailorfs::test::trim_utf8(token);
      tokens.push_back(token);
  }
  return tokens;
}
int create_config(mimir::Config &config) {
  std::string LSB_HOSTS;
  std::vector<std::string> node_names;
  if (std::getenv("LSB_HOSTS") == nullptr) {
    node_names = std::vector<std::string>();
    LSB_HOSTS = "localhost";
    tailorfs::test::trim_utf8(LSB_HOSTS);
    node_names.emplace_back(LSB_HOSTS);
  } else {
    LSB_HOSTS = std::getenv("LSB_HOSTS");
    node_names = split_no_duplicates(LSB_HOSTS);
  }
  config._job_config._job_id = 0;
  config._job_config._devices.clear();
  int num_devices = 3;  // shm, bb, and pfs
  config._job_config._devices.emplace_back(info.shm.c_str(), 64 * 1024, false);  // shm
  config._job_config._devices.emplace_back(info.bb.c_str(), 512 * 1024, false);  // bb
  config._job_config._devices.emplace_back(info.pfs.c_str(),
                                           2 * 1024 * 1024, true);  // pfs
  config._job_config._job_time_minutes = 30;
  config._job_config._num_cores_per_node = args.ranks_per_node;
  config._job_config._num_gpus_per_node = 4;
  config._job_config._num_nodes = node_names.size();
  config._job_config._node_names = node_names;
  config._job_config._rpc_port = 8888;
  config._job_config._rpc_threads = 4;
  config._job_config._priority = 100;
  config._current_process_index = -1;

  int metadata_ops = 2;
  int io_ops = args.iteration;
  if (args.access_pattern == tailorfs::test::AccessPattern::RANDOM) {
    metadata_ops += args.iteration;
  }

  int NUM_FILES = 1;
  int comm_size = args.num_nodes * args.ranks_per_node;
  int write_comm_size = comm_size;
  int read_comm_size = comm_size;
  if (args.process_grouping != tailorfs::test::ProcessGrouping::ALL_PROCESS) {
    write_comm_size /= 2;
    read_comm_size /= 2;
  }

  mimir::WorkflowAdvice workflow_advice;
  mimir::ApplicationAdvice app_advice;
  app_advice._is_mpi = true;
  tailorfs::test::trim_utf8(args.config_file);
  auto file_parts =  split(args.config_file, '/');
  auto values =  split(file_parts[file_parts.size() - 1], '_');
  char workload[256];
  char process[1024];
  if (values[9] == "wo") strcpy(workload, "Write-Only");
  else if (values[9] == "ro") strcpy(workload, "Read-Only");
  else if (values[9] == "raw") strcpy(workload, "Read-After-Write");
  else if (values[9] == "update") strcpy(workload, "Update");
  else if (values[9] == "worm") strcpy(workload, "WORM");
  int access_pattern, file_sharing, process_grouping;
  if (values[8] == "seq") access_pattern = 0;
  else if (values[8] == "random") access_pattern = 1;
  if (values[7] == "fpp") file_sharing = 0;
  else if (values[7] == "shared") file_sharing = 1;
  if (values[10] == "all.json") process_grouping = 0;
  else if (values[10] == "split.json") process_grouping = 1;
  else if (values[10] == "alt.json") process_grouping = 2;
  app_advice._num_cpu_cores_used = comm_size;
  if (app_advice._is_mpi) {
    sprintf(process, "%d %s/io_tests --request_size %d --iteration %d "
            "--ranks_per_node %d --access_pattern %d --file_sharing %d "
            "--process_grouping %d --reporter compact %s", app_advice._num_cpu_cores_used,
            args.binary_directory.c_str(),atoi(values[5].c_str()) * 1024, atoi(values[6].c_str()),
            atoi(values[3].c_str()), access_pattern, file_sharing, process_grouping, workload);
  }else {
    sprintf(process, "%s/io_tests --request_size %d --iteration %d "
            "--ranks_per_node %d --access_pattern %d --file_sharing %d "
            "--process_grouping %d --reporter compact %s",args.binary_directory.c_str(),
            atoi(values[5].c_str()) * 1024, atoi(values[6].c_str()), atoi(values[3].c_str()), access_pattern,
            file_sharing, process_grouping, workload);
  }
  auto process_full = std::string(process);
  tailorfs::test::trim_utf8(process_full);
  uint32_t app_index = 0;
  workflow_advice._app_mapping.emplace(process_full, app_index);
  /* Application info */
  app_advice._name = "app-" + std::to_string(app_index);
  tailorfs::test::trim_utf8(app_advice._name);
  app_advice._num_gpus_used = 0;
  /* app file dag*/
  app_advice._application_file_dag.applications.emplace(app_index);
  workflow_advice._application_file_dag.applications.emplace(app_index);

  /** File Advice */
  if (args.file_sharing == tailorfs::test::FileSharing::PER_PROCESS) {
    if (args.process_grouping == tailorfs::test::ProcessGrouping::ALL_PROCESS) {
      NUM_FILES = args.num_nodes * args.ranks_per_node;
    } else {
      NUM_FILES = args.num_nodes * args.ranks_per_node / 2;
    }
    for (int file_index = 0; file_index < NUM_FILES; ++file_index) {
      mimir::FileAdvice file_advice;
      if (args.num_nodes > 1 && args.process_grouping == tailorfs::test::ProcessGrouping::SPLIT_PROCESS_HALF) {
        file_advice._file_sharing = mimir::FileSharing::FILE_SHARED_INTER_NODE;
      } else if (args.num_nodes > 1 && args.process_grouping == tailorfs::test::ProcessGrouping::SPLIT_PROCESS_ALTERNATE) {
        file_advice._file_sharing = mimir::FileSharing::FILE_SHARED_NODE_LOCAL;
      } else {
        file_advice._file_sharing = mimir::FileSharing::FILE_PER_PROCESS;
      }
      file_advice._name = (info.pfs / (args.filename + ".dat" + "_" +
                                       std::to_string(file_index)))
                              .string();
      tailorfs::test::trim_utf8(file_advice._name);
      file_advice._format = Format::FORMAT_BINARY;
      file_advice._size_mb = args.iteration * args.request_size * 1.0  / MB;
      file_advice._io_amount_mb =
          args.iteration * args.request_size * 1.0 / MB;
      /* File access type*/
      if (args.usecase == tailorfs::test::UseCase::WRITE_ONLY ||
          args.usecase == tailorfs::test::UseCase::READ_AFTER_WRITE ||
          args.usecase == tailorfs::test::UseCase::UPDATE ||
          args.usecase == tailorfs::test::UseCase::WORM) {
        if (args.request_size < 4 * KB) {
          file_advice._write_distribution._0_4kb = 1.0;
        } else if (args.request_size >= 4 * KB && args.request_size < 64 * KB ) {
          file_advice._write_distribution._4_64kb = 1.0;
        } else if (args.request_size >= 64 * KB && args.request_size < 1 * MB ) {
          file_advice._write_distribution._64kb_1mb = 1.0;
        } else if (args.request_size >= 1 * MB && args.request_size < 16 * MB ) {
          file_advice._write_distribution._1mb_16mb = 1.0;
        } else if (args.request_size >= 16 * MB ){
          file_advice._write_distribution._16mb = 1.0;
        }
      }
      if (args.usecase == tailorfs::test::UseCase::READ_ONLY ||
          args.usecase == tailorfs::test::UseCase::READ_AFTER_WRITE ||
          args.usecase == tailorfs::test::UseCase::WORM) {
        if (args.request_size < 4 * KB) {
          file_advice._read_distribution._0_4kb = 1.0;
        } else if (args.request_size >= 4 * KB && args.request_size < 64 * KB ) {
          file_advice._read_distribution._4_64kb = 1.0;
        } else if (args.request_size >= 64 * KB && args.request_size < 1 * MB ) {
          file_advice._read_distribution._64kb_1mb = 1.0;
        } else if (args.request_size >= 1 * MB && args.request_size < 16 * MB ) {
          file_advice._read_distribution._1mb_16mb = 1.0;
        } else if (args.request_size >= 16 * MB ){
          file_advice._read_distribution._16mb = 1.0;
        }
      }
      file_advice._current_device = 2;
      file_advice._placement_device = 0;
      file_advice._per_io_data = io_ops * 1.0 / (metadata_ops + io_ops);
      file_advice._per_io_metadata = metadata_ops * 1.0 / (metadata_ops + io_ops);
      config._file_repo.push_back(file_advice);
    }
  }
  else {
    mimir::FileAdvice file_advice;
    file_advice._file_sharing = mimir::FileSharing::FILE_SHARED_COLLECTIVE;
    file_advice._name = (info.pfs / (args.filename + ".dat" + "_0"))
                            .string();
    tailorfs::test::trim_utf8(file_advice._name);
    file_advice._format = Format::FORMAT_BINARY;
    file_advice._size_mb = args.iteration * args.request_size * write_comm_size * 1.0  / MB;
    file_advice._io_amount_mb = file_advice._size_mb;
    /* File access type*/
    if (args.usecase == tailorfs::test::UseCase::WRITE_ONLY ||
        args.usecase == tailorfs::test::UseCase::READ_AFTER_WRITE ||
        args.usecase == tailorfs::test::UseCase::UPDATE ||
        args.usecase == tailorfs::test::UseCase::WORM) {
      if (args.request_size < 4 * KB) {
        file_advice._write_distribution._0_4kb = 1.0;
      } else if (args.request_size >= 4 * KB && args.request_size < 64 * KB ) {
        file_advice._write_distribution._4_64kb = 1.0;
      } else if (args.request_size >= 64 * KB && args.request_size < 1 * MB ) {
        file_advice._write_distribution._64kb_1mb = 1.0;
      } else if (args.request_size >= 1 * MB && args.request_size < 16 * MB ) {
        file_advice._write_distribution._1mb_16mb = 1.0;
      } else if (args.request_size >= 16 * MB ){
        file_advice._write_distribution._16mb = 1.0;
      }
    }
    if (args.usecase == tailorfs::test::UseCase::READ_ONLY ||
        args.usecase == tailorfs::test::UseCase::READ_AFTER_WRITE ||
        args.usecase == tailorfs::test::UseCase::WORM) {
      if (args.request_size < 4 * KB) {
        file_advice._read_distribution._0_4kb = 1.0;
      } else if (args.request_size >= 4 * KB && args.request_size < 64 * KB ) {
        file_advice._read_distribution._4_64kb = 1.0;
      } else if (args.request_size >= 64 * KB && args.request_size < 1 * MB ) {
        file_advice._read_distribution._64kb_1mb = 1.0;
      } else if (args.request_size >= 1 * MB && args.request_size < 16 * MB ) {
        file_advice._read_distribution._1mb_16mb = 1.0;
      } else if (args.request_size >= 16 * MB ){
        file_advice._read_distribution._16mb = 1.0;
      }
    }
    file_advice._current_device = 2;
    file_advice._placement_device = 0;
    file_advice._per_io_data = io_ops * 1.0 / (metadata_ops + io_ops);
    file_advice._per_io_metadata = metadata_ops * 1.0 / (metadata_ops + io_ops);
    config._file_repo.push_back(file_advice);
  }



  app_advice._io_size_mb = args.request_size * args.iteration * comm_size / MB;
  app_advice._per_io_data = io_ops * 1.0 / (metadata_ops + io_ops);
  app_advice._per_io_metadata = metadata_ops * 1.0 / (metadata_ops + io_ops);
  app_advice._runtime_minutes =  config._job_config._job_time_minutes ;
  if (args.request_size < 4 * KB) {
    app_advice._ts_distribution._0_4kb = 1.0;
  } else if (args.request_size >= 4 * KB && args.request_size < 64 * KB ) {
    app_advice._ts_distribution._4_64kb = 1.0;
  } else if (args.request_size >= 64 * KB && args.request_size < 1 * MB ) {
    app_advice._ts_distribution._64kb_1mb = 1.0;
  } else if (args.request_size >= 1 * MB && args.request_size < 16 * MB ) {
    app_advice._ts_distribution._1mb_16mb = 1.0;
  } else if (args.request_size >= 16 * MB ){
    app_advice._ts_distribution._16mb = 1.0;
  }
  /* Rank file map*/
  for (int rank_index = 0; rank_index < comm_size; ++rank_index) {
    app_advice._rank_file_dag.ranks.emplace(rank_index);
  }

  if (args.file_sharing == tailorfs::test::FileSharing::PER_PROCESS) {
    for (int rank_index = 0; rank_index < comm_size; ++rank_index) {
      auto file_index = rank_index % write_comm_size;
      app_advice._rank_file_dag.files.emplace(file_index);
      app_advice._rank_file_dag.edges.emplace_back(rank_index, file_index);
    }
    for (int file_index = 0; file_index < NUM_FILES; ++file_index) {
      app_advice._application_file_dag.files.emplace(file_index);
      app_advice._application_file_dag.edges.emplace_back(app_index,
                                                          file_index);
    }

  } else {
    auto file_index = 0;
    for (int rank_index = 0; rank_index < comm_size; ++rank_index) {
      app_advice._rank_file_dag.files.emplace(file_index);
      app_advice._rank_file_dag.edges.emplace_back(rank_index, file_index);

    }
    app_advice._application_file_dag.files.emplace(file_index);
    app_advice._application_file_dag.edges.emplace_back(app_index,
                                                        file_index);
  }
  workflow_advice._application_file_dag = app_advice._application_file_dag;

  for (int file_index = 0; file_index < NUM_FILES; ++file_index) {
    if (args.usecase == tailorfs::test::UseCase::WRITE_ONLY) {
      app_advice._file_workload.emplace(file_index, WorkloadType::WRITE_ONLY_WORKLOAD);
      workflow_advice._file_workload.emplace(file_index, WorkloadType::WRITE_ONLY_WORKLOAD);
    } else if (args.usecase == tailorfs::test::UseCase::READ_ONLY) {
      app_advice._file_workload.emplace(file_index, WorkloadType::READ_ONLY_WORKLOAD);
      workflow_advice._file_workload.emplace(file_index, WorkloadType::READ_ONLY_WORKLOAD);
    } else if (args.usecase == tailorfs::test::UseCase::READ_AFTER_WRITE) {
      app_advice._file_workload.emplace(file_index, WorkloadType::RAW_WORKLOAD);
      workflow_advice._file_workload.emplace(file_index, WorkloadType::RAW_WORKLOAD);
    }  else if (args.usecase == tailorfs::test::UseCase::UPDATE) {
      app_advice._file_workload.emplace(file_index, WorkloadType::UPDATE_WORKLOAD);
      workflow_advice._file_workload.emplace(file_index, WorkloadType::UPDATE_WORKLOAD);
    } else if (args.usecase == tailorfs::test::UseCase::WORM) {
      app_advice._file_workload.emplace(file_index, WorkloadType::WORM_WORKLOAD);
      workflow_advice._file_workload.emplace(file_index, WorkloadType::WORM_WORKLOAD);
    }

    if (args.access_pattern == tailorfs::test::AccessPattern::SEQUENTIAL) {
      app_advice._file_access_pattern.emplace(file_index,
                                              AccessPattern::SEQUENTIAL);
      workflow_advice._file_access_pattern.emplace(file_index,
                                                   AccessPattern::SEQUENTIAL);
    } else {
      app_advice._file_access_pattern.emplace(file_index,
                                              AccessPattern::RANDOM);
      workflow_advice._file_access_pattern.emplace(file_index,
                                                   AccessPattern::RANDOM);
    }

    if (args.file_sharing == tailorfs::test::FileSharing::PER_PROCESS) {
      app_advice._independent_files.push_back(file_index);
      workflow_advice._independent_files.push_back(file_index);
      auto app_interfaces = std::unordered_set<mimir::InterfaceType>();
      app_interfaces.emplace(mimir::InterfaceType::POSIX);
      app_advice._interfaces_used.insert_or_assign(file_index, app_interfaces);
    } else {
      auto app_interfaces = std::unordered_set<mimir::InterfaceType>();
      app_interfaces.emplace(mimir::InterfaceType::MPIIO);
      app_advice._interfaces_used.insert_or_assign(file_index, app_interfaces);
    }
  }
  config._app_repo.push_back(app_advice);



  for(const auto&app_advice:config._app_repo) {
    for(const auto&element:app_advice._interfaces_used) {
      auto iter = workflow_advice._interfaces_used.find(element.first);
      std::unordered_set<mimir::InterfaceType> interfaces;
      if(iter == workflow_advice._interfaces_used.end()) {
        interfaces = element.second;
      } else {
        interfaces = iter->second;
      }
      for (const auto&interface: element.second) {
        interfaces.emplace(interface);
      }
      workflow_advice._interfaces_used.insert_or_assign(element.first, interfaces);
    }
  }
  workflow_advice._num_cpu_cores_used = config._job_config._node_names.size() *
                                        config._job_config._num_cores_per_node;
  workflow_advice._num_gpus_used = config._job_config._node_names.size() *
                                   config._job_config._num_gpus_per_node;
  workflow_advice._num_apps = 1;
  workflow_advice._io_size_mb = app_advice._io_size_mb;
  workflow_advice._per_io_data = app_advice._per_io_data;
  workflow_advice._per_io_metadata = app_advice._per_io_metadata;
  workflow_advice._runtime_minutes = config._job_config._job_time_minutes;
  workflow_advice._ts_distribution = app_advice._ts_distribution;
  config._workflow = workflow_advice;
  return 0;
}

/**
 * Test cases
 */
TEST_CASE("GenerateConfig",
          CONVERT_VAL(ranks_per_node, args.ranks_per_node) +
              CONVERT_ENUM(storage_type, args.storage_type)) {
  if (info.rank == 0) {
    REQUIRE(pretest() == 0);
    fs::path config_file = args.config_file;
    if (fs::exists(config_file)) fs::remove(config_file);
    mimir::Config config;
    create_config(config);
    using json = nlohmann::json;
    json j = config;
    json conf = {{"ranks_per_node", args.ranks_per_node},
                 {"storage_type", args.storage_type},
                 {"request_size", args.request_size},
                 {"usecase", args.usecase},
                 {"iteration", args.iteration},
                 {"process_grouping", args.process_grouping},
                 {"num_nodes", args.num_nodes},
                 {"file_sharing", args.file_sharing},
                 {"config_file", args.config_file},
                 {"filename", args.filename},
                 {"access_pattern", args.access_pattern},
                 {"binary_directory", args.binary_directory}};
    j["conf"] = conf;
    std::cout << j.dump() << std::endl;
    std::ofstream out(config_file.c_str());
    out << j;
    out.close();

    /**
     * Verification
     */
    std::ifstream t(config_file.c_str());
    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    std::string buffer(size, ' ');
    t.seekg(0);
    t.read(&buffer[0], size);
    t.close();
    json read_json = json::parse(buffer);
    mimir::Config config_r;
    read_json.get_to(config_r);
    REQUIRE(config_r.is_same(config));
    REQUIRE(posttest() == 0);
  }
  MPI_Barrier(MPI_COMM_WORLD);
}

TEST_CASE("LoadConfig",
          CONVERT_VAL(ranks_per_node, args.ranks_per_node) +
              CONVERT_ENUM(storage_type, args.storage_type)) {
  REQUIRE(pretest() == 0);

  const char *MIMIR_CONFIG_PATH = std::getenv(mimir::MIMIR_CONFIG_PATH);
  REQUIRE(MIMIR_CONFIG_PATH != nullptr);
  REQUIRE(mimir_init_config(false) == 0);
  mimir::Config gen_config;
  create_config(gen_config);
  auto loaded_config = MIMIR_CONFIG();
  REQUIRE(loaded_config->is_same(gen_config));
  REQUIRE(posttest() == 0);
}
