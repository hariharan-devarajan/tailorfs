#!/bin/bash
SCRIPT_NAME=$1
nodes=(2 4 8 16 32 64 128)
TAILORFS_DIR=/usr/workspace/iopp/software/tailorfs
for node in "${nodes[@]}"
do
echo	bsub -J ${SCRIPT_NAME}_${node} -nnodes ${node} -stage storage=64 -W 30 -env NNODES=$node -core_isolation 0 -G asccasc -q pbatch -cwd ${TAILORFS_DIR}/scripts -o ${TAILORFS_DIR}/scripts/out_wo_${node}.log -e ${TAILORFS_DIR}/scripts/err_wo_${node}.log ${TAILORFS_DIR}/scripts/$SCRIPT_NAME $node
	bsub -J ${SCRIPT_NAME}_${node} -nnodes ${node} -stage storage=64 -W 30 -env NNODES=$node -core_isolation 0 -G asccasc -q pbatch -cwd ${TAILORFS_DIR}/scripts -o ${TAILORFS_DIR}/scripts/out_wo_${node}.log -e ${TAILORFS_DIR}/scripts/err_wo_${node}.log ${TAILORFS_DIR}/scripts/$SCRIPT_NAME $node
done
