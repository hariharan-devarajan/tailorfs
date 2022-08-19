#!/bin/bash

#!/bin/bash
MACHINE=$1
MPI_PROCS=$2
SOURCE_DIR=$3
TEST_EXEC=$4
UNIFYFS_LOG_DIR=$5
UNIFYFS_ROOT_DIR=$6
TEST_ARGS="${@:7}"
SLEEP_TIME=5

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


export UNIFYFS_LOG_VERBOSITY=3
if [ "${MACHINE}" = "local" ]; then
    echo "Configuring test for local"
    echo "Run UnifyFS daemon"
    rm -rf ${pfs}/unifyfsd ${BBPATH}/unifyfsd $UNIFYFS_LOG_DIR/*
    mkdir -p ${pfs}/unifyfsd ${BBPATH}/unifyfsd
    echo "${UNIFYFS_ROOT_DIR}/bin/unifyfsd start --sharedfs-dir=${pfs}/unifyfsd --log-dir $UNIFYFS_LOG_DIR --runstate-dir ${BBPATH}/unifyfsd &"
    ${UNIFYFS_ROOT_DIR}/bin/unifyfsd start --sharedfs-dir=${PFS_PATH}/unifyfsd --log-dir $UNIFYFS_LOG_DIR --runstate-dir ${BBPATH}/unifyfsd &
    UNIFYFS_EXEC_PID=$!
    echo "process spawned ${UNIFYFS_EXEC_PID}"
    sleep ${SLEEP_TIME}
    ps -aef | grep unifyfsd
    echo "Run Test"
    if [ "${MPI_PROCS}" = "1" ]; then
      echo "${TEST_EXEC} ${TEST_ARGS}"
      ${TEST_EXEC} ${TEST_ARGS}
      status=$?
    else
      mpirun -n ${MPI_PROCS} ${TEST_EXEC} ${TEST_ARGS}
      status=$?
    fi
    echo "Stop UnifyFS daemon"
    ps -aef | grep unifyfsd | grep -v grep | awk '{print $2}' | xargs kill -9
    popd
    if [ $status -gt 0 ]; then
        echo "Test failed with code $status!" >&2
        exit $status
    fi
    echo "Finishing test."
    exit 0
elif [ "${MACHINE}" = "lassen" ]; then
    echo "Configuring test for ${MACHINE}"
    echo "Run UnifyFS daemon"
    UNIFYFS_LOG_DIR=$UNIFYFS_LOG_DIR UNIFYFS_SERVER_CORES=8 ${UNIFYFS_EXEC} start --share-dir=${pfs}/unifyfs/share-dir -d
    echo "Run Test"
    jsrun -r 1 -a ${MPI_PROCS} -c ${MPI_PROCS}  -d packed ${TEST_EXEC} ${TEST_ARGS}
    status=$?
    echo "Killing UnifyFS daemon"
    ${UNIFYFS_EXEC} terminate
    if [ $status -gt 0 ]; then
      echo "Test failed with code $status!" >&2
      exit $status
    fi
    echo "Finishing test."
    exit 0
else
  echo "${MACHINE} not supported"
  exit 1
fi

