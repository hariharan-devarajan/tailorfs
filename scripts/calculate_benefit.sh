#!/bin/bash
nodes=(1 2 4 8 16 32 64 128)
CSV=paper_benefit.csv
echo "nodes,iter,rs,init,fini,open,close,write,read,interface,workload,storage,access_pattern,file_sharing,process_grouping,solution" > $CSV 
for node in ${!nodes[@]}; do
  cat paper_benefit.sh_${node}_1024.out  | grep Timing | grep PRINT | head -n 9 | cut -c 35- | awk -v x="${node}" '{print x","$0",tailorfs"  }' | sed 's/[ ]\+//g' >> $CSV
  cat paper_benefit.sh_${node}_1024.out  | grep Timing | grep PRINT | sed '10,18!d'  | cut -c 35- | awk -v x="${node}" '{print x","$0",baseline"  }'  | sed 's/[ ]\+//g'  >> $CSV
done
