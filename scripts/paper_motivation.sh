#!/bin/bash
### LSF syntax
source /usr/workspace/iopp/install_scripts/bin/iopp-init

NUM_NODES=$1
RS_KB=$2
TAILORFS_DIR=/usr/workspace/iopp/software/tailorfs
export BBPATH=/dev/shm/bb/
mkdir -p $BBPATH

pushd $TAILORFS_DIR
spack env activate -p ./dependency
export CC=/usr/tce/packages/gcc/gcc-8.3.1/bin/gcc
export CXX=/usr/tce/packages/gcc/gcc-8.3.1/bin/g++
mkdir build_${NUM_NODES}
pushd build_${NUM_NODES}
rm -rf *
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/tce/packages/gcc/gcc-8.3.1/bin/gcc -DCMAKE_CXX_COMPILER=/usr/tce/packages/gcc/gcc-8.3.1/bin/g++ -G "CodeBlocks - Unix Makefiles" ${TAILORFS_DIR}
cmake --build ${TAILORFS_DIR}/build_${NUM_NODES} --target all -- -j

#ctest -V -R "benchmark_lassen_${NUM_NODES}_32_${RS_KB}_1024_fpp_(posix|stdio|mpiio)"
ctest -V -R "benchmark_lassen_${NUM_NODES}_32_${RS_KB}_1024_shared"
