//
// Created by haridev on 9/6/22.
//

#ifndef TAILORFS_MPI_H
#define TAILORFS_MPI_H


#include <brahma/mpi.h>

#include <mimir/api/mimir_interceptor.h>
#include <tailorfs/core/fsview_manager.h>
#include <tailorfs/macro.h>
using namespace tailorfs;
namespace brahma {
  class MPITailorFS : public MPI {
  private:
    static std::shared_ptr<MPITailorFS> instance;
  public:
    MPITailorFS() : MPI() {
      TAILORFS_LOGINFO("MPI class intercepted", "");
    }
    ~MPITailorFS() = default;
    static std::shared_ptr<MPITailorFS> get_instance() {
      if (instance == nullptr) {
        instance = std::make_shared<MPITailorFS>();
        MPI::set_instance(instance);
      }
      return instance;
    }

    int MPI_Init(int *argc, char ***argv) override;
    int MPI_Finalize() override;

  };
}


#endif //TAILORFS_MPI_H
