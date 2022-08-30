#!/bin/bash
pfs=$1
BBPATH=$2
UNIFYFS_LOG_DIR=$3
UNIFYFS_ROOT_DIR=$4
SCRIPT_DIR=$5
mkdir -p /var/tmp/kvstore
echo "${UNIFYFS_ROOT_DIR}/bin/unifyfsd start --sharedfs-dir=${pfs}/unifyfsd --log-dir $UNIFYFS_LOG_DIR --runstate-dir ${BBPATH}/unifyfsd &"
${UNIFYFS_ROOT_DIR}/bin/unifyfsd start --sharedfs-dir=${pfs}/unifyfsd --log-dir $UNIFYFS_LOG_DIR --runstate-dir ${BBPATH}/unifyfsd &
UNIFYFS_EXEC_PID=$!
echo $UNIFYFS_EXEC_PID > $SCRIPT_DIR/unifyfs.pid
sleep 5
echo "[TailorFS] started Unifyfs with ${UNIFYFS_EXEC_PID}"