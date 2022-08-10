#include <catch_config.h>
#include <mimir/mimir.h>
#include <mpi.h>
#include <test_utils.h>

#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>

namespace tailorfs::mimir::test {}
namespace tt = tailorfs::mimir::test;
namespace fs = std::experimental::filesystem;

using namespace mimir;

/**
 * Test data structures
 */
namespace tailorfs::mimir::test {

enum StorageType : int { SHM = 0, LOCAL_SSD = 1, PFS = 2 };
struct Arguments {
  /* test args */
  bool debug = false;
  int ranks_per_node = 1;
  /* I/O args */
  int io_size_per_app_mb = 1024;
  /* main args */
  std::string config_file = "mimir_config.json";
  std::string file_prefix = "test";
  StorageType storage_type = StorageType::PFS;
  /* Job info */
  uint16_t num_apps = 1;
  uint32_t num_process_per_app = 1;
  uint32_t num_files_per_app = 1;
  float fpp_percentage = 1.0;
  float sequential_percentage = 1.0;
  /* Needs to add up to 1 */
  float wo_file_ptg = 1.0;
  float ro_file_ptg = 0.0;
  float raw_file_ptg = 0.0;
  float update_file_ptg = 0.0;
  float worm_file_ptg = 0.0;
  /* Needs to add up to 1 */
  float all_app_ptg = 1.0;
  float split_app_ptg = 0.0;
  float alt_app_ptg = 0.0;
};
struct Info {
  fs::path pfs;
  fs::path bb;
  fs::path shm;
  int rank;
  int comm_size;
  char hostname[256];
  std::string original_filename;
};
}  // namespace tailorfs::mimir::test
tt::Arguments args;
tt::Info info;

DEFINE_CLARA_OPS(tt::StorageType)

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
  return cl::Opt(args.debug, "debug")["--debug"]("Enable debugging.") |
         cl::Opt(args.ranks_per_node,
                 "ranks_per_node")["--ranks_per_node"]("# ranks per node.") |
         cl::Opt(args.io_size_per_app_mb,
                 "io_size_per_app_mb")["--io_size_per_app_mb"](
             "io size per app MB.") |
         cl::Opt(args.config_file, "config_file")["--config_file"](
             "Name of the config file (JSON).") |
         cl::Opt(args.file_prefix,
                 "file_prefix")["--file_prefix"]("Prefix used by filenames") |
         cl::Opt(args.storage_type, "storage_type")["--storage_type"](
             "Where to store the data 0-> SHM, 1->NODE_LOCAL_SSD, 2->PFS") |
         cl::Opt(args.num_apps,
                 "num_apps")["--num_apps"]("# of apps in the workflow") |
         cl::Opt(args.num_process_per_app,
                 "num_process_per_app")["--num_process_per_app"](
             "# of processes per app") |
         cl::Opt(
             args.num_files_per_app,
             "num_files_per_app")["--num_files_per_app"]("# of files per app") |
         cl::Opt(args.fpp_percentage, "fpp_percentage")["--fpp_percentage"](
             "% of total files accessed FPP") |
         cl::Opt(args.sequential_percentage,
                 "sequential_percentage")["--sequential_percentage"](
             "% of total files accessed sequentially.") |
         cl::Opt(args.wo_file_ptg, "wo_file_ptg")["--wo_file_ptg"](
             "% of total files with wo access pattern") |
         cl::Opt(args.ro_file_ptg, "ro_file_ptg")["--ro_file_ptg"](
             "% of total files with ro access pattern") |
         cl::Opt(args.raw_file_ptg, "raw_file_ptg")["--raw_file_ptg"](
             "% of total files with raw access pattern") |
         cl::Opt(args.update_file_ptg, "update_file_ptg")["--update_file_ptg"](
             "% of total files with update access pattern") |
         cl::Opt(args.worm_file_ptg, "worm_file_ptg")["--worm_file_ptg"](
             "% of total files with worm access pattern") |
         cl::Opt(args.all_app_ptg, "all_app_ptg")["--all_app_ptg"](
             "% of apps where all processes do the same pattern") |
         cl::Opt(args.split_app_ptg, "split_app_ptg")["--split_app_ptg"](
             "% of apps where first half does one op and other half does "
             "another ") |
         cl::Opt(args.alt_app_ptg, "alt_app_ptg")["--alt_app_ptg"](
             "% of apps where alternate processes do different ops.");
}

/**
 * Helper functions
 */
int pretest() {
  /* Other pretest operations*/
  info.original_filename = args.file_prefix;
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
  info.pfs = fs::path(PFS_VAR) / "tailorfs" / "data";
  info.bb = fs::path(BB_VAR) / "tailorfs" / "data";
  info.shm = fs::path(SHM_VAR) / "tailorfs" / "data";
  TEST_LOGGER_INFO_ARGS("pfs %s bb %s shm %s", info.pfs.c_str(),
                        info.bb.c_str(), info.shm.c_str());
  /* Validate args */
  REQUIRE(args.wo_file_ptg + args.ro_file_ptg + args.raw_file_ptg +
              args.worm_file_ptg + args.update_file_ptg ==
          1.0);
  REQUIRE(args.all_app_ptg + args.split_app_ptg + args.alt_app_ptg == 1.0);
  if (args.wo_file_ptg == 1.0 || args.ro_file_ptg == 1.0) {
    REQUIRE(args.all_app_ptg == 1.0);
  }
  if (args.split_app_ptg > 0.0 || args.alt_app_ptg > 0.0) {
    REQUIRE(args.ranks_per_node > 1);
  }
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
std::vector<std::string> split(std::string x, char delim = ' ') {
  x += delim;  // includes a delimiter at the end so last word is also read
  auto set_splitted = std::unordered_set<std::string>();
  std::string temp = "";
  int count = 0;
  for (int i = 0; i < x.length(); i++) {
    if (x[i] == delim) {
      if (count > 0) set_splitted.emplace(temp);
      temp = "";
      i++;
      count++;
    }
    temp += x[i];
  }
  auto splitted =
      std::vector<std::string>(set_splitted.begin(), set_splitted.end());
  return splitted;
}
int create_config(mimir::Config &config) {
  auto LSB_HOSTS = std::getenv("LSB_HOSTS");
  if (LSB_HOSTS == nullptr) {
    LSB_HOSTS = "localhost";
  }
  auto node_names = split(LSB_HOSTS);
  config._job_config._job_id = 0;
  config._job_config._devices.clear();
  int num_devices = 3;  // shm, bb, and pfs
  config._job_config._devices.emplace_back(info.shm.c_str(), 64 * 1024);  // shm
  config._job_config._devices.emplace_back(info.bb.c_str(), 512 * 1024);  // bb
  config._job_config._devices.emplace_back(info.pfs.c_str(),
                                           2 * 1024 * 1024);  // pfs
  config._job_config._job_time_minutes = 30;
  config._job_config._num_cores_per_node = args.ranks_per_node;
  config._job_config._num_gpus_per_node = 4;
  config._job_config._num_nodes = node_names.size();
  config._job_config._node_names = node_names;
  config._job_config._rpc_port = 8888;
  config._job_config._rpc_threads = 4;
  config._job_config._priority = 100;

  const int NUM_FILES = args.num_apps * args.num_files_per_app;
  for (int file_index = 0; file_index < NUM_FILES; ++file_index) {
    mimir::FileAdvice file_advice;
    file_advice._name = (info.pfs / (args.file_prefix + "_" +
                                     std::to_string(file_index) + ".dat"))
                            .string();
    file_advice._format = Format::FORMAT_BINARY;
    file_advice._size_mb = args.io_size_per_app_mb / args.num_files_per_app;
    file_advice._io_amount_mb =
        args.io_size_per_app_mb / args.num_files_per_app;
    /* File access type*/
    int wo_file = floor(NUM_FILES * args.wo_file_ptg);
    int ro_file = floor(NUM_FILES * args.ro_file_ptg);
    int raw_file = floor(NUM_FILES * args.raw_file_ptg);
    int update_file = floor(NUM_FILES * args.update_file_ptg);
    int worm_file = NUM_FILES - wo_file - ro_file - raw_file - update_file;
    if (file_index < wo_file) {
      file_advice._write_distribution._0_4kb = 1.0;
    } else if (file_index < ro_file) {
      file_advice._read_distribution._0_4kb = 1.0;
      file_advice._prefetch = true;
    } else if (file_index < raw_file) {
      file_advice._read_distribution._0_4kb = 1.0;
      file_advice._write_distribution._0_4kb = 1.0;
    } else if (file_index < update_file) {
      file_advice._write_distribution._0_4kb = 1.0;
    } else if (file_index < worm_file) {
      file_advice._write_distribution._0_4kb = 1.0;
      file_advice._read_distribution._0_4kb = 1.0;
    }
    file_advice._current_device = 2;
    file_advice._placement_device = 0;
    file_advice._per_io_data = 0.75;
    file_advice._per_io_metadata = 0.25;
    config._file_repo.push_back(file_advice);
  }
  mimir::WorkflowAdvice workflow_advice;
  for (uint32_t app_index = 0; app_index < args.num_apps; ++app_index) {
    mimir::ApplicationAdvice app_advice;
    /* Application info */
    app_advice._name = "app-" + std::to_string(app_index);
    /* app file dag*/
    app_advice._application_file_dag.applications.push_back(app_index);
    workflow_advice._application_file_dag.applications.push_back(app_index);

    app_advice._num_cpu_cores_used =
        floor(config._job_config._node_names.size() *
              config._job_config._num_cores_per_node * 1.0 / args.num_apps);
    app_advice._num_gpus_used =
        floor(config._job_config._node_names.size() *
              config._job_config._num_gpus_per_node * 1.0 / args.num_apps);
    app_advice._io_size_mb = args.io_size_per_app_mb;
    app_advice._per_io_data = 0.75;
    app_advice._per_io_metadata = 0.25;
    app_advice._runtime_minutes =
        config._job_config._job_time_minutes / args.num_apps;
    app_advice._ts_distribution._0_4kb = 1.0;
    /* Access Pattern */
    int num_files_seq = floor(1.0 * NUM_FILES * args.sequential_percentage);
    int num_files_random = NUM_FILES - num_files_seq;
    int num_files_strided = 0;
    /* File access type*/
    int num_files_independent = floor(NUM_FILES * args.fpp_percentage);
    int num_file_shared = NUM_FILES - num_files_independent;
    /* Rank file map*/
    for (int rank_index = 0; rank_index < args.num_process_per_app;
         ++rank_index) {
      app_advice._rank_file_dag.ranks.push_back(rank_index);
    }
    int rank_in_app = 0;
    for (uint32_t file_index = app_index * args.num_files_per_app;
         file_index < (app_index + 1) * args.num_files_per_app; ++file_index) {
      /* Rank file map*/
      app_advice._rank_file_dag.files.push_back(file_index);
      /* App file dag */
      app_advice._application_file_dag.files.push_back(file_index);
      app_advice._application_file_dag.edges.emplace_back(app_index,
                                                          file_index);

      workflow_advice._application_file_dag.files.push_back(file_index);
      workflow_advice._application_file_dag.edges.emplace_back(app_index,
                                                               file_index);

      /* File access pattern */
      if (file_index < num_files_seq) {
        app_advice._file_access_pattern.emplace(file_index,
                                                AccessPattern::SEQUENTIAL);
        workflow_advice._file_access_pattern.emplace(file_index,
                                                     AccessPattern::SEQUENTIAL);
      } else if (file_index < num_files_seq + num_files_random) {
        app_advice._file_access_pattern.emplace(file_index,
                                                AccessPattern::RANDOM);
        workflow_advice._file_access_pattern.emplace(file_index,
                                                     AccessPattern::RANDOM);
      } else {
        app_advice._file_access_pattern.emplace(file_index,
                                                AccessPattern::STRIDED);
        workflow_advice._file_access_pattern.emplace(file_index,
                                                     AccessPattern::STRIDED);
      }
      /* File access type*/
      bool has_independent = false, has_shared = false;
      if (file_index < num_files_independent) {
        app_advice._independent_files.push_back(file_index);
        workflow_advice._independent_files.push_back(file_index);
        /* Rank file map*/
        app_advice._rank_file_dag.edges.push_back(
            Edge<RankIndex, FileIndex>(rank_in_app, file_index));
        has_independent = true;
      } else {
        auto shared_file_apps = std::vector<ApplicationIndex>();
        for (int shared_app_index = floor(file_index / args.num_files_per_app);
             shared_app_index < args.num_apps; ++shared_app_index) {
          shared_file_apps.push_back(shared_app_index);
        }
        workflow_advice._shared_files.emplace(file_index, shared_file_apps);

        /* Rank file map*/
        for (uint32_t rank_index = 0; rank_index < args.num_process_per_app;
             ++rank_index) {
          app_advice._rank_file_dag.edges.emplace_back(rank_index, file_index);
        }
        has_shared = true;
      }
      if (has_independent)
        app_advice._interfaces_used.push_back(InterfaceType::POSIX);
      if (has_shared)
        app_advice._interfaces_used.push_back(InterfaceType::MPIIO);
      rank_in_app++;
      rank_in_app = rank_in_app % args.num_process_per_app;
    }
    config._app_repo.push_back(app_advice);
  }

  workflow_advice._num_cpu_cores_used = config._job_config._node_names.size() *
                                        config._job_config._num_cores_per_node;
  workflow_advice._num_gpus_used = config._job_config._node_names.size() *
                                   config._job_config._num_gpus_per_node;
  workflow_advice._num_apps = args.num_apps;
  workflow_advice._io_size_mb = args.io_size_per_app_mb * args.num_apps;
  workflow_advice._per_io_data = 0.75;
  workflow_advice._per_io_metadata = 0.25;
  workflow_advice._runtime_minutes = config._job_config._job_time_minutes;
  workflow_advice._ts_distribution._0_4kb = 1.0;
}

/**
 * Test cases
 */
TEST_CASE("GenerateConfig",
          CONVERT_VAL(ranks_per_node, args.ranks_per_node) +
              CONVERT_ENUM(storage_type, args.storage_type) +
              CONVERT_VAL(num_apps, args.num_apps) +
              CONVERT_VAL(num_process_per_app, args.num_process_per_app) +
              CONVERT_VAL(num_files_per_app, args.num_files_per_app) +
              CONVERT_VAL(fpp_percentage, args.fpp_percentage) +
              CONVERT_VAL(sequential_percentage, args.sequential_percentage) +
              CONVERT_VAL(wo_file_ptg, args.wo_file_ptg) +
              CONVERT_VAL(ro_file_ptg, args.ro_file_ptg) +
              CONVERT_VAL(raw_file_ptg, args.raw_file_ptg) +
              CONVERT_VAL(update_file_ptg, args.update_file_ptg) +
              CONVERT_VAL(worm_file_ptg, args.worm_file_ptg) +
              CONVERT_VAL(all_app_ptg, args.all_app_ptg) +
              CONVERT_VAL(split_app_ptg, args.split_app_ptg) +
              CONVERT_VAL(alt_app_ptg, args.alt_app_ptg)) {
  REQUIRE(pretest() == 0);

  fs::path config_file = args.config_file;
  if (fs::exists(config_file)) fs::remove(config_file);
  mimir::Config config;
  create_config(config);
  using json = nlohmann::json;
  json j = config;
  json conf = {{"ranks_per_node", args.ranks_per_node},
               {"storage_type", args.storage_type},
               {"num_apps", args.num_apps},
               {"num_process_per_app", args.num_process_per_app},
               {"fpp_percentage", args.fpp_percentage},
               {"sequential_percentage", args.sequential_percentage},
               {"wo_file_ptg", args.wo_file_ptg},
               {"ro_file_ptg", args.ro_file_ptg},
               {"raw_file_ptg", args.raw_file_ptg},
               {"update_file_ptg", args.update_file_ptg},
               {"worm_file_ptg", args.worm_file_ptg},
               {"all_app_ptg", args.all_app_ptg},
               {"split_app_ptg", args.split_app_ptg},
               {"alt_app_ptg", args.alt_app_ptg}};
  j["conf"] = conf;
  std::ofstream out(config_file.c_str());
  out << j;
  out.close();
  std::cout << j.dump() << std::endl;

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
  REQUIRE(config_r == config);
  REQUIRE(posttest() == 0);
}

TEST_CASE("LoadConfig",
          CONVERT_VAL(ranks_per_node, args.ranks_per_node) +
              CONVERT_ENUM(storage_type, args.storage_type) +
              CONVERT_VAL(num_apps, args.num_apps) +
              CONVERT_VAL(num_process_per_app, args.num_process_per_app) +
              CONVERT_VAL(num_files_per_app, args.num_files_per_app) +
              CONVERT_VAL(fpp_percentage, args.fpp_percentage) +
              CONVERT_VAL(sequential_percentage, args.sequential_percentage) +
              CONVERT_VAL(wo_file_ptg, args.wo_file_ptg) +
              CONVERT_VAL(ro_file_ptg, args.ro_file_ptg) +
              CONVERT_VAL(raw_file_ptg, args.raw_file_ptg) +
              CONVERT_VAL(update_file_ptg, args.update_file_ptg) +
              CONVERT_VAL(worm_file_ptg, args.worm_file_ptg) +
              CONVERT_VAL(all_app_ptg, args.all_app_ptg) +
              CONVERT_VAL(split_app_ptg, args.split_app_ptg) +
              CONVERT_VAL(alt_app_ptg, args.alt_app_ptg)) {
  REQUIRE(pretest() == 0);

  const char *MIMIR_CONFIG_PATH = std::getenv(mimir::MIMIR_CONFIG_PATH);
  REQUIRE(MIMIR_CONFIG_PATH != nullptr);
  REQUIRE(mimir_init_config() == 0);
  mimir::Config gen_config;
  create_config(gen_config);
  auto loaded_config = MIMIR_CONFIG();
  REQUIRE(loaded_config->is_same(gen_config));
  REQUIRE(posttest() == 0);
}
