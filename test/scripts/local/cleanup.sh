#!/bin/bash
pfs=$1
BBPATH=$2
UNIFYFS_LOG_DIR=$3

rm -rf ${pfs}/unifyfsd ${BBPATH}/unifyfsd $UNIFYFS_LOG_DIR/*
mkdir -p ${pfs}/unifyfsd ${BBPATH}/unifyfsd
echo "[TailorFS] Local Cleanup finished"