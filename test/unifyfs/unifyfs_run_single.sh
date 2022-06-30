#!/bin/bash

#!/bin/bash

TEST_EXEC=$1
UNIFYFS_EXEC=$2
UNIFYFS_LOGIO_SPILL_DIR=$3
UNIFYFS_LOG_DIR=$4
PFS=$5
UNIFYFS_HOSTFILE=$6
MPI_EXEC=$7
MPI_PROCS=$8
TEST_ARGS="${@:9}"
SLEEP_TIME=5

error_ct=0
if [[ ! -f "${TEST_EXEC}" ]]; then
  echo "TEST_EXEC ${TEST_EXEC} does not exists." >&2
  error_ct=$((error_ct + 1))
fi
if [[ ! -f "${UNIFYFS_EXEC}" ]]; then
  echo "UNIFYFS_EXEC ${UNIFYFS_EXEC} does not exists." >&2
  error_ct=$((error_ct + 1))
fi
if [[ ! -d "${UNIFYFS_LOGIO_SPILL_DIR}" ]]; then
  echo "UNIFYFS_LOGIO_SPILL_DIR ${UNIFYFS_LOGIO_SPILL_DIR} does not exists." >&2
  error_ct=$((error_ct + 1))
fi
if [[ ! -d "${UNIFYFS_LOG_DIR}" ]]; then
  echo "UNIFYFS_LOG_DIR ${UNIFYFS_LOG_DIR} does not exists." >&2
  error_ct=$((error_ct + 1))
fi
if [[ ! -d "${PFS}" ]]; then
  echo "PFS ${PFS} does not exists." >&2
  error_ct=$((error_ct + 1))
fi

if [ $error_ct -gt 0 ]; then
  echo "Arguments are wrong !!!" >&2
  exit $error_ct
fi

echo "1" > $UNIFYFS_HOSTFILE
echo $(hostname) >> $UNIFYFS_HOSTFILE

export UNIFYFS_SERVER_HOSTFILE=$UNIFYFS_HOSTFILE
#echo "jsrun -r 1 -a 1 ${UNIFYFS_EXEC} --sharedfs-dir=${PFS} --log-dir $UNIFYFS_LOG_DIR --log-verbosity 5 -C &"
#jsrun -r 1 -a 1 ${UNIFYFS_EXEC} --sharedfs-dir=${PFS} --log-dir $UNIFYFS_LOG_DIR --log-verbosity 5 -C &
mkdir -p $BBPATH/unifyfs/data
echo "UNIFYFS_LOG_DIR=$UNIFYFS_LOG_DIR UNIFYFS_LOGIO_SPILL_DIR=$BBPATH/unifyfs/data ${UNIFYFS_EXEC} start --share-dir=${PFS} --debug &"
UNIFYFS_LOG_DIR=$UNIFYFS_LOG_DIR UNIFYFS_LOGIO_SPILL_DIR=$BBPATH/unifyfs/data ${UNIFYFS_EXEC} start --share-dir=${PFS} --debug &

UNIFYFS_EXEC_PID=$!
echo "process spawned ${UNIFYFS_EXEC_PID}"

echo "Started unifyfs daemon. sleeping for ${SLEEP_TIME} seconds"
sleep ${SLEEP_TIME}



echo "jsrun -r 1 -a ${MPI_PROCS} -c ${MPI_PROCS} -d packed ${TEST_EXEC} ${TEST_ARGS}"
jsrun -r 1 -a ${MPI_PROCS} -c ${MPI_PROCS}  -d packed ${TEST_EXEC} ${TEST_ARGS}
status=$?
echo "Killing UnifyFS daemon with PID ${UNIFYFS_EXEC_PID}"
kill -9 ${UNIFYFS_EXEC_PID}

echo "Stopped unifyfs daemon. sleeping for ${SLEEP_TIME} seconds"
sleep ${SLEEP_TIME}

if [ $status -gt 0 ]; then
  echo "Test failed with code $status!" >&2
  exit $status
fi
echo "Finishing test."
exit 0