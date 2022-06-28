#ifndef UNIFYFS_STAGE_H
#define UNIFYFS_STAGE_H

#include <unifyfs/unifyfs_api.h>

#ifndef UNIFYFS_STAGE_MD5_BLOCKSIZE
#define UNIFYFS_STAGE_MD5_BLOCKSIZE (1048576)
#endif

#ifndef UNIFYFS_STAGE_TRANSFER_BLOCKSIZE
#define UNIFYFS_STAGE_TRANSFER_BLOCKSIZE (16 * 1048576)
#endif

extern int verbose;

enum {
  UNIFYFS_STAGE_MODE_SERIAL = 0,   /* sequential file transfers */
  UNIFYFS_STAGE_MODE_PARALLEL = 1, /* concurrent file transfers */
  UNIFYFS_STAGE_DATA_BALANCED = 2, /* balanced data placement */
  UNIFYFS_STAGE_DATA_SKEWED = 3    /* skewed data placement */
};

struct _unifyfs_stage {
  int checksum;  /* perform checksum? 0:no, 1:yes */
  int data_dist; /* data distribution? UNIFYFS_STAGE_DATA_xxxx */
  int mode;      /* transfer mode? UNIFYFS_STAGE_MODE_xxxx */

  int rank;        /* my rank */
  int total_ranks; /* mpi world size */

  char* mountpoint;    /* unifyfs mountpoint */
  char* manifest_file; /* manifest file containing the transfer list */

  unifyfs_handle fshdl; /* UnifyFS API client handle */
};
typedef struct _unifyfs_stage unifyfs_stage;

/**
 * @brief transfer source file to destination according to stage context
 *
 * @param ctx               stage context
 * @param file_index        file index within manifest
 * @param src_file_path     source file path
 * @param dst_file_path     destination file path
 *
 * @return 0 on success, errno otherwise
 */
int unifyfs_stage_transfer(unifyfs_stage* ctx, int file_index,
                           const char* src_file_path,
                           const char* dst_file_path);

#endif /* UNIFYFS_STAGE_H */
