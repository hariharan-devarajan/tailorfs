#!/bin/bash
SCRIPT_NAME=$1
node=$2
RS_KB=$3
LOG_NAME=${SCRIPT_NAME}_${node}_${RS_KB}_shared.out
TAILORFS_DIR=/usr/workspace/iopp/software/tailorfs
echo	bsub -J ${SCRIPT_NAME}_${node}_${RS_KB} -nnodes ${node} -W $((60*6)) -env NNODES=$node -core_isolation 0 -G asccasc -q pbatch -cwd ${TAILORFS_DIR}/scripts -o ${TAILORFS_DIR}/scripts/${LOG_NAME} -e ${TAILORFS_DIR}/scripts/${LOG_NAME} ${TAILORFS_DIR}/scripts/$SCRIPT_NAME $node $RS_KB
	bsub -J ${SCRIPT_NAME}_${node}_${RS_KB} -nnodes ${node} -W $((60*6)) -env NNODES=$node -core_isolation 0 -G asccasc -q pbatch -cwd ${TAILORFS_DIR}/scripts -o ${TAILORFS_DIR}/scripts/${LOG_NAME} -e ${TAILORFS_DIR}/scripts/${LOG_NAME} ${TAILORFS_DIR}/scripts/$SCRIPT_NAME $node $RS_KB
