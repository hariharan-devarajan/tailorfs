#!/bin/bash

echo "[TailorFS] jsrun -r 1 -a 1 rm -rf /dev/shm/* /tmp/na_sm* /tmp/unifyfsd.margo-shm"
jsrun -r 1 -a 1 rm -rf /dev/shm/* ~/unifyfs/logs/* /tmp/unifyfsd.margo-shm
echo "[TailorFS] Local Cleanup finished"