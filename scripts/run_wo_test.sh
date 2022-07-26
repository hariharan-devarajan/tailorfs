#!/bin/bash
pushd /usr/workspace/iopp/software/tailorfs/build
echo "Number of nodes allocated: $NNODES"
source /usr/workspace/iopp/install_scripts/bin/iopp-init
source /usr/workspace/iopp/install_scripts/bin/spack-init
export pfs=$pfs/$NNODES
mkdir -p $pfs
echo "pfs location: $pfs"
echo "bb location: $BBPATH"
ctest  ctest -V -R Test_[a-z]+_example_write_
