
```
export LD_LIBRARY_PATH=/usr/workspace/iopp/software/tailorfs/dependency/.spack-env/view/lib
export PFS_PATH=/p/gpfs1/haridev/unifyfs
export SHM_PATH=/dev/shm

export UNIFYFS_LOG_VERBOSITY=3

UNIFYFS_LOG_DIR=/g/g92/haridev/unifyfs/logs UNIFYFS_SERVER_CORES=2 /usr/workspace/iopp/software/tailorfs/dependency/.spack-env/view/bin/unifyfs start --share-dir=/p/gpfs1/haridev/unifyfs &

jsrun -r 1 -a 20 -c 20 -d packed /usr/workspace/iopp/software/tailorfs/build/test/unifyfs/unifyfs_basic --request_size 4096 --iteration 100 --rpn 20 "[type=read-only]"
```