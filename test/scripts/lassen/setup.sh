#!/bin/bash
pfs=$1
UNIFYFS_LOG_DIR=$2
UNIFYFS_ROOT_DIR=$3
UNIFYFS_LOG_DIR=$UNIFYFS_LOG_DIR UNIFYFS_SERVER_CORES=8 ${UNIFYFS_ROOT_DIR}/bin/unifyfs start --share-dir=${pfs}/unifyfs/share-dir -d
echo "[TailorFS] started Unifyfs"