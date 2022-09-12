#!/bin/bash
nodes=(1 2 4 8 16 32 64 128)
CSV=paper_motivation.csv
echo "nodes,iter,rs,init,fini,open,close,write,read,flush,prefetch,interface,workload,storage,access_pattern,file_sharing,process_grouping" > $CSV
for node in ${!nodes[@]}; do
  cat paper_motivation.sh_${node}_1024.out | grep Timing | cut -c 33- | awk -v x="${num_nodes}" '{print x","$0""  }' >> $CSV
done
