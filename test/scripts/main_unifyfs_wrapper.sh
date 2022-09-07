#!/bin/bash
MACHINE=$1
MPI_PROCS=$2
SOURCE_DIR=$3
TEST_EXEC=$4
UNIFYFS_LOG_DIR=$5
UNIFYFS_ROOT_DIR=$6
TEST_ARGS="${@:7}"
error_ct=0
mkdir -p $UNIFYFS_LOG_DIR

if [[ ! -f "${TEST_EXEC}" ]]; then
  echo "TEST_EXEC ${TEST_EXEC} does not exists." >&2
  error_ct=$((error_ct + 1))
fi
if [[ ! -d "${UNIFYFS_ROOT_DIR}" ]]; then
  echo "UNIFYFS_ROOT_DIR ${UNIFYFS_ROOT_DIR} does not exists." >&2
  error_ct=$((error_ct + 1))
fi
if [[ ! -d "${SOURCE_DIR}" ]]; then
  echo "SOURCE_DIR ${SOURCE_DIR} does not exists." >&2
  error_ct=$((error_ct + 1))
fi
if [[ ! -d "${UNIFYFS_LOG_DIR}" ]]; then
  echo "UNIFYFS_LOG_DIR ${UNIFYFS_LOG_DIR} does not exists." >&2
  error_ct=$((error_ct + 1))
fi
if [ $error_ct -gt 0 ]; then
  echo "Arguments are wrong !!!" >&2
  exit $error_ct
fi

SCRIPT_DIR=${SOURCE_DIR}/test/scripts
if [ "${MACHINE}" = "local" ]; then
  $SCRIPT_DIR/local/wrap_unifyfs.sh $MACHINE $MPI_PROCS $SOURCE_DIR $TEST_EXEC $UNIFYFS_LOG_DIR $UNIFYFS_ROOT_DIR $SCRIPT_DIR/local $TEST_ARGS
  status=$?
  if [ $status -gt 0 ]; then
      echo "[TailorFS] Test failed with code $status!" >&2
      exit $status
  fi
  echo "[TailorFS] Finishing test."
  exit 0
elif [ "${MACHINE}" = "lassen" ]; then
  $SCRIPT_DIR/lassen/wrap_unifyfs.sh $MACHINE $MPI_PROCS $SOURCE_DIR $TEST_EXEC $UNIFYFS_LOG_DIR $UNIFYFS_ROOT_DIR $SCRIPT_DIR/lassen $TEST_ARGS
  status=$?
  if [ $status -gt 0 ]; then
      echo "[TailorFS] Test failed with code $status!" >&2
      exit $status
  fi
  echo "[TailorFS] Finishing test."
  exit 0
else
  echo "${MACHINE} not supported"
  exit 1
fi