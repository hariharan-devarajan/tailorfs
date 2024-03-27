#!/bin/bash
### LSF syntax
#BSUB -cwd /usr/workspace/iopp/software/tailorfs/scripts
#BSUB -nnodes 1128      #number of nodes
#BSUB -W 02:00             #walltime in minutes
#BSUB -G asccasc           #account
#BSUB -J paper_benefit   #name of job
#BSUB -q pbatch            #queue to use
##BSUB -stage storage=64            #add BB

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

echo "[TAILORFS PRINT] Timing,ops,init,fin,mpiio_init,mpiio_fini,posix_init,posix_fin,stdio_init,stdio_fin"  
ctest -R test_internal_lassen_anatomy_${NUM_NODES}_32_1000$ -VV
ctest -R test_internal_lassen_anatomy_${NUM_NODES}_32_10000$ -VV
ctest -R test_internal_lassen_anatomy_${NUM_NODES}_32_100000$ -VV
ctest -R test_internal_lassen_anatomy_${NUM_NODES}_32_1000000$ -VV
