#!/bin/bash
SCRIPT_NAME=$1
node=$2
RS_KB=$3
LOG_NAME=${SCRIPT_NAME}_${node}_${RS_KB}.out
TAILORFS_DIR=/usr/workspace/iopp/software/tailorfs
echo	bsub -J ${SCRIPT_NAME}_${node} -nnodes ${node} -stage storage=64 -W 30 -env NNODES=$node -core_isolation 0 -G asccasc -q pbatch -cwd ${TAILORFS_DIR}/scripts -o ${TAILORFS_DIR}/scripts/${LOG_NAME} -e ${TAILORFS_DIR}/scripts/${LOG_NAME} ${TAILORFS_DIR}/scripts/$SCRIPT_NAME $node $RS_KB
	bsub -J ${SCRIPT_NAME}_${node} -nnodes ${node} -stage storage=64 -W 30 -env NNODES=$node -core_isolation 0 -G asccasc -q pbatch -cwd ${TAILORFS_DIR}/scripts -o ${TAILORFS_DIR}/scripts/${LOG_NAME} -e ${TAILORFS_DIR}/scripts/${LOG_NAME} ${TAILORFS_DIR}/scripts/$SCRIPT_NAME $node $RS_KB
