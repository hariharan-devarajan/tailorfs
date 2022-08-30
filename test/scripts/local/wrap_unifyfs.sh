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
if [[ ! -d "${SCRIPT_DIR}" ]]; then
  echo "SCRIPT_DIR ${SCRIPT_DIR} does not exists." >&2
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
echo "Configuring test for local"
pushd $SCRIPT_DIR
./finish.sh
./cleanup.sh $pfs $BBPATH $UNIFYFS_LOG_DIR
./setup.sh $pfs $BBPATH $UNIFYFS_LOG_DIR $UNIFYFS_ROOT_DIR $SCRIPT_DIR
echo "Run Test"
if [ "${MPI_PROCS}" = "1" ]; then
  echo "LD_PRELOAD=${SOURCE_DIR}/build/lib/libtailorfs.so ${TEST_EXEC} ${TEST_ARGS}"
  LD_PRELOAD=${SOURCE_DIR}/build/lib/libtailorfs.so ${TEST_EXEC} ${TEST_ARGS}
  status=$?
else
  LD_PRELOAD=${SOURCE_DIR}/build/lib/libtailorfs.so mpirun -n ${MPI_PROCS} ${TEST_EXEC} ${TEST_ARGS}
  status=$?
fi

./finish.sh
./cleanup.sh $pfs $BBPATH $UNIFYFS_LOG_DIR
popd
if [ $status -gt 0 ]; then
    echo "Test failed with code $status!" >&2
    exit $status
fi
echo "Finishing test."
exit 0