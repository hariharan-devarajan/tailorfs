//
// Created by haridev on 9/6/22.
//

#include "mpi.h"
#include <tailorfs/tailorfs.h>

std::shared_ptr<brahma::MPITailorFS> brahma::MPITailorFS::instance = nullptr;
int brahma::MPITailorFS::MPI_Init(int *argc, char ***argv){
  int status = MPI::MPI_Init(argc, argv);
  tfs_init(true);
  return status;
}
int brahma::MPITailorFS::MPI_Finalize(){
  int status = MPI::MPI_Finalize();
  tfs_finalize(true);
  return status;
}
