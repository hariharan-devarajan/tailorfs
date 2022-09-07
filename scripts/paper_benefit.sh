#!/bin/bash
### LSF syntax
#BSUB -cwd /usr/workspace/iopp/software/tailorfs/scripts
#BSUB -nnodes 1128      #number of nodes
#BSUB -W 02:00             #walltime in minutes
#BSUB -G asccasc           #account
#BSUB -J paper_benefit   #name of job
#BSUB -q pbatch            #queue to use
#BSUB -stage storage=64            #add BB

source /usr/workspace/iopp/install_scripts/bin/iopp-init

NUM_NODES=$1
TAILORFS_DIR=/usr/workspace/iopp/software/tailorfs
pushd $TAILORFS_DIR
spack env activate -p ./dependency
export CC=/usr/tce/packages/gcc/gcc-8.3.1/bin/gcc
export CXX=/usr/tce/packages/gcc/gcc-8.3.1/bin/g++
mkdir build_${NUM_NODES}
pushd build_${NUM_NODES}
rm -rf *
cmake ../
cmake --build /usr/WS2/iopp/software/tailorfs/build_${NUM_NODES} --target all -- -j

ctest -R test_generate_config_lassen_${NUM_NODES}_32_1_4_1024_fpp
echo "Timing, DirectIO"
export TAILORFS_DIRECT=1
ctest -V -R test_baseline_mb_lassen_${NUM_NODES}_32_1_4_1024_fpp

echo "Timing, Baseline"
export TAILORFS_DIRECT=0
ctest -V -R test_baseline_mb_lassen_${NUM_NODES}_32_1_4_1024_fpp

echo "Timing, TailorFS"
ctest -V -R test_baseline_mb_lassen_${NUM_NODES}_32_1_4_1024_fpp

sleep 10
