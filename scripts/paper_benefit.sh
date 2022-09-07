#!/bin/bash
### LSF syntax
#BSUB -cwd /usr/workspace/iopp/software/tailorfs/scripts
#BSUB -nnodes 1128      #number of nodes
#BSUB -W 02:00             #walltime in minutes
#BSUB -G asccasc           #account
#BSUB -J paper_benefit   #name of job
#BSUB -q pbatch            #queue to use
#BSUB -stage storage=64            #add BB

NUM_NODES=$1
TAILORFS_DIR=/usr/workspace/iopp/software/tailorfs
pushd $TAILORFS_DIR
mkdir build_${NUM_NODES}
pushd build_${NUM_NODES}
cmake ../
make -j

ctest -R generate
echo "Timing, DirectIO"
export TAILORFS_DIRECT=1
ctest -V -R test_baseline_mb_lassen_${NUM_NODES}_32_1_4_1024_fpp

echo "Timing, Baseline"
export TAILORFS_DIRECT=0
ctest -V -R test_baseline_mb_lassen_${NUM_NODES}_32_1_4_1024_fpp

echo "Timing, TailorFS"
ctest -V -R test_baseline_mb_lassen_${NUM_NODES}_32_1_4_1024_fpp

sleep 10
