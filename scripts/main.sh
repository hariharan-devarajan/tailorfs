#!/bin/bash
nodes=(2)
TAILORFS_DIR=/usr/workspace/iopp/software/tailorfs
for node in "${nodes[@]}"
do
	bsub -J tailorfs_${node} -nnodes ${node} -stage storage=64 -W 30 -env NNODES=$node -core_isolation 0 -G asccasc -q pbatch ${TAILORFS_DIR}/scripts/run_wo_test.sh -cwd ${TAILORFS_DIR}/scripts -o ${TAILORFS_DIR}/scripts/out_wo_${node}.log -e ${TAILORFS_DIR}/scripts/err_wo_${node}.log
done
