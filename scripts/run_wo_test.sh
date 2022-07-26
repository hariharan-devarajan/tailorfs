#!/bin/bash
pushd /usr/workspace/iopp/software/tailorfs

source /usr/workspace/iopp/install_scripts/bin/iopp-init
source /usr/workspace/iopp/install_scripts/bin/spack-init
pushd dependency
spack env activate -p .
popd
pushd build
echo "Number of nodes allocated: $NNODES"
source ~/.profile
export pfs=$pfs/$NNODES
mkdir -p $pfs
echo "pfs location: $pfs"
echo "bb location: $BBPATH"
ctest  ctest -V -R Test_[a-z]+_example_write_
