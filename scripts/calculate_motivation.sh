#!/bin/bash
nodes=(1 2 4 8 16 32 64 128)
CSV=paper_motivation.csv
echo "nodes,iter,rs,init,fini,open,close,write,read,flush,prefetch,interface,workload,storage,access_pattern,file_sharing,process_grouping" > $CSV
for req in 4 64 1024; do
for node in ${nodes[@]}; do
  cat paper_motivation.sh_${node}_${req}.out | grep Timing | cut -c 33- | awk -v x="${node}" '{print x","$0""  }' >> $CSV
done
done
