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
SLEEP_TIME=10

error_ct=0
if [[ ! -f "${TEST_EXEC}" ]]; then
  echo "TEST_EXEC ${TEST_EXEC} does not exists." >&2
  error_ct=$((error_ct + 1))
fi
if [[ ! -f "${UNIFYFS_EXEC}" ]]; then
  echo "UNIFYFS_EXEC ${UNIFYFS_EXEC} does not exists." >&2
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

export UNIFYFS_LOG_VERBOSITY=3

#echo "1" > $UNIFYFS_HOSTFILE
#echo $(hostname) >> $UNIFYFS_HOSTFILE

#export UNIFYFS_SERVER_HOSTFILE=$UNIFYFS_HOSTFILE
#echo "jsrun -r 1 -a 1 ${UNIFYFS_EXEC} --sharedfs-dir=${PFS} --log-dir $UNIFYFS_LOG_DIR --log-verbosity 5 -C &"
#jsrun -r 1 -a 1 ${UNIFYFS_EXEC} --sharedfs-dir=${PFS} --log-dir $UNIFYFS_LOG_DIR --log-verbosity 5 -C &
echo "Killing existing unifyfs daemons"
echo "${UNIFYFS_EXEC} terminate"
${UNIFYFS_EXEC} terminate
echo "jsrun -r 1 -a 1 ps -aef | grep unifyfs | grep -v run_single.sh | awk {'print $2'} | xargs kill -9 > /dev/null 2>&1 > /dev/null 2>&1"
#ps -aef | grep /usr/workspace/iopp/software/tailorfs/dependency/.spack-env/view/bin/unifyfs
jsrun -r 1 -a 1 `ps -aef | grep unifyfs | grep -v run_single.sh | awk {'print $2'} | xargs kill -9 > /dev/null 2>&1` > /dev/null 2>&1
echo "Cleaning up directories"
echo "jsrun -r 1 -a 1 rm -rf /dev/shm/* $BBPATH/unifyfs/* ${PFS}/* /tmp/na_sm* /tmp/kvstore /tmp/unifyfsd.margo-shm"
jsrun -r 1 -a 1 rm -rf /dev/shm/* $BBPATH/unifyfs/* ~/unifyfs/logs/* ${PFS}/* /tmp/kvstore /tmp/unifyfsd.margo-shm

mkdir -p $BBPATH/unifyfs/data
echo "UNIFYFS_LOG_DIR=$UNIFYFS_LOG_DIR UNIFYFS_SERVER_CORES=8 ${UNIFYFS_EXEC} start --share-dir=${PFS} -d"
UNIFYFS_LOG_DIR=$UNIFYFS_LOG_DIR UNIFYFS_SERVER_CORES=8 ${UNIFYFS_EXEC} start --share-dir=${PFS} -d

echo "jsrun -r 1 -a ${MPI_PROCS} -c ${MPI_PROCS} -d packed ${TEST_EXEC} ${TEST_ARGS}"
jsrun -r 1 -a ${MPI_PROCS} -c ${MPI_PROCS}  -d packed ${TEST_EXEC} ${TEST_ARGS}
status=$?

echo "Killing UnifyFS daemon"
echo "${UNIFYFS_EXEC} terminate"
${UNIFYFS_EXEC} terminate
# shellcheck disable=SC2046
jsrun -r 1 -a 1 `ps -aef | grep unifyfs | grep -v run_single.sh | awk {'print $2'} | xargs kill -9 > /dev/null 2>&1` > /dev/null 2>&1
echo "Stopped unifyfs daemon. sleeping for ${SLEEP_TIME} seconds"
sleep ${SLEEP_TIME}

echo "Cleaning up directories"
echo "jsrun -r 1 -a 1 rm -rf /dev/shm/* $BBPATH/unifyfs/* /tmp/na_sm* /tmp/kvstore /tmp/unifyfsd.margo-shm"
jsrun -r 1 -a 1 rm -rf /dev/shm/* $BBPATH/unifyfs/* /tmp/kvstore /tmp/unifyfsd.margo-shm

if [ $status -gt 0 ]; then
  echo "Test failed with code $status!" >&2
  exit $status
fi
echo "Finishing test."
exit 0
