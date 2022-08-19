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
    echo "Killing existing unifyfs daemons"
    echo "${UNIFYFS_ROOT_DIR}/bin/unifyfs terminate"
    ${UNIFYFS_ROOT_DIR}/bin/unifyfs terminate
    echo "jsrun -r 1 -a 1 ps -aef | grep unifyfs | grep -v run_single.sh | awk {'print $2'} | xargs kill -9 > /dev/null 2>&1 > /dev/null 2>&1"
    ps -aef | grep /usr/workspace/iopp/software/tailorfs/dependency/.spack-env/view/bin/unifyfs
    jsrun -r 1 -a 1 `ps -aef | grep unifyfs | grep -v run_single.sh | awk {'print $2'} | xargs kill -9 > /dev/null 2>&1` > /dev/null 2>&1
    echo "Cleaning up directories"
    echo "jsrun -r 1 -a 1 rm -rf /dev/shm/* /tmp/na_sm* /tmp/unifyfsd.margo-shm"
    jsrun -r 1 -a 1 rm -rf /dev/shm/* ~/unifyfs/logs/* /tmp/unifyfsd.margo-shm

    echo "Run UnifyFS daemon"
    UNIFYFS_LOG_DIR=$UNIFYFS_LOG_DIR UNIFYFS_SERVER_CORES=8 ${UNIFYFS_ROOT_DIR}/bin/unifyfs start --share-dir=${pfs}/unifyfs/share-dir -d

    echo "Run Test"
    jsrun -r 1 -a ${MPI_PROCS} -c ${MPI_PROCS}  -d packed ${TEST_EXEC} ${TEST_ARGS}
    status=$?

    echo "Killing UnifyFS daemon"
    echo "${UNIFYFS_ROOT_DIR}/bin/unifyfs terminate"
    ${UNIFYFS_ROOT_DIR}/bin/unifyfs terminate
    # shellcheck disable=SC2046
    jsrun -r 1 -a 1 `ps -aef | grep unifyfs | grep -v run_single.sh | awk {'print $2'} | xargs kill -9 > /dev/null 2>&1` > /dev/null 2>&1
    echo "Stopped unifyfs daemon. sleeping for ${SLEEP_TIME} seconds"
    #sleep ${SLEEP_TIME}

    echo "Cleaning up directories"
    echo "jsrun -r 1 -a 1 rm -rf /dev/shm/* $BBPATH/unifyfs/* /tmp/na_sm* /tmp/kvstore /tmp/unifyfsd.margo-shm"
    jsrun -r 1 -a 1 rm -rf /dev/shm/* $BBPATH/unifyfs/* /tmp/kvstore /tmp/unifyfsd.margo-shm

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

