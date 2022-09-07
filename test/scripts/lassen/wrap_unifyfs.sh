#!/bin/bash
MACHINE=$1
MPI_PROCS=$2
SOURCE_DIR=$3
TEST_EXEC=$4
UNIFYFS_LOG_DIR=$5
UNIFYFS_ROOT_DIR=$6
SCRIPT_DIR=$7
TEST_ARGS="${@:8}"
error_ct=0
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
pushd $SCRIPT_DIR
echo "[TailorFS] Configuring test for lassen"
#./finish.sh $UNIFYFS_ROOT_DIR
#./cleanup.sh
#./setup.sh $pfs $UNIFYFS_LOG_DIR $UNIFYFS_ROOT_DIR

echo "[TailorFS] Run Test"
jsrun --env LD_PRELOAD=${SOURCE_DIR}/build/lib/libtailorfs.so -r 1 -a ${MPI_PROCS} -c ${MPI_PROCS}  -d packed ${TEST_EXEC} ${TEST_ARGS}
status=$?

#./finish.sh $UNIFYFS_ROOT_DIR
#./cleanup.sh
popd
if [ $status -gt 0 ]; then
    echo "Test failed with code $status!" >&2
    exit $status
fi
echo "Finishing test."
exit 0