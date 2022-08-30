#!/bin/bash
UNIFYFS_ROOT_DIR=$1
echo "[TailorFS] ${UNIFYFS_ROOT_DIR}/bin/unifyfs terminate"
${UNIFYFS_ROOT_DIR}/bin/unifyfs terminate

echo "[TailorFS] jsrun -r 1 -a 1 ps -aef | grep unifyfs | grep -v run_single.sh | awk {'print $2'} | xargs kill -9 > /dev/null 2>&1 > /dev/null 2>&1"
jsrun -r 1 -a 1 `ps -aef | grep unifyfs | grep -v run_single.sh | awk {'print $2'} | xargs kill -9 > /dev/null 2>&1` > /dev/null 2>&1