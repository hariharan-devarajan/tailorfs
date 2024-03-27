//
// Created by hariharan on 8/8/22.
//

#ifndef BRAHMA_MPIIO_H
#define BRAHMA_MPIIO_H

#include <mpi.h>
#include "interface.h"
#include "brahma/interceptor.h"

namespace brahma {
class MPIIO :public brahma::Interface{
 private:
  static std::shared_ptr<MPIIO> my_instance;

 public:
  MPIIO():Interface(){}
  static std::shared_ptr<MPIIO> get_instance() {
    if (my_instance == nullptr) {
      BRAHMA_LOGINFO("MPIIO class not intercepted but used", "");
      my_instance = std::make_shared<MPIIO>();
    }
    return my_instance;
  }

  static int set_instance(std::shared_ptr<MPIIO> instance_i) {
    if (instance_i != nullptr) {
      my_instance = instance_i;
      return 0;
    } else {
      BRAHMA_LOGERROR("instance_i is not set", "");
      throw std::runtime_error("instance_i is not set");
    }
  }

  virtual int MPI_File_close(MPI_File *fh);

  virtual int MPI_File_set_size(MPI_File fh, MPI_Offset size);

  virtual int MPI_File_iread_at(MPI_File fh, MPI_Offset offset, void *buf,
                                 int count, MPI_Datatype datatype,
                                 MPI_Request *request);

  virtual int MPI_File_iread(MPI_File fh, void *buf, int count,
                              MPI_Datatype datatype, MPI_Request *request);

  virtual int MPI_File_iread_shared(MPI_File fh, void *buf, int count,
                                     MPI_Datatype datatype,
                                     MPI_Request *request);

  virtual int MPI_File_iwrite_at(MPI_File fh, MPI_Offset offset,
                                  const void *buf, int count,
                                  MPI_Datatype datatype, MPI_Request *request);

  virtual int MPI_File_iwrite(MPI_File fh, const void *buf, int count,
                               MPI_Datatype datatype, MPI_Request *request);

  virtual int MPI_File_iwrite_shared(MPI_File fh, const void *buf, int count,
                                      MPI_Datatype datatype,
                                      MPI_Request *request);

  virtual int MPI_File_open(MPI_Comm comm, const char *filename, int amode,
                             MPI_Info info, MPI_File *fh);

  virtual int MPI_File_read_all_begin(MPI_File fh, void *buf, int count,
                                       MPI_Datatype datatype);

  virtual int MPI_File_read_all(MPI_File fh, void *buf, int count,
                                 MPI_Datatype datatype, MPI_Status *status);

  virtual int MPI_File_read_at_all(MPI_File fh, MPI_Offset offset, void *buf,
                                    int count, MPI_Datatype datatype,
                                    MPI_Status *status);

  virtual int MPI_File_read_at_all_begin(MPI_File fh, MPI_Offset offset,
                                          void *buf, int count,
                                          MPI_Datatype datatype);

  virtual int MPI_File_read_at(MPI_File fh, MPI_Offset offset, void *buf,
                                int count, MPI_Datatype datatype,
                                MPI_Status *status);

  virtual int MPI_File_read(MPI_File fh, void *buf, int count,
                             MPI_Datatype datatype, MPI_Status *status);

  virtual int MPI_File_read_ordered_begin(MPI_File fh, void *buf, int count,
                                           MPI_Datatype datatype);

  virtual int MPI_File_read_ordered(MPI_File fh, void *buf, int count,
                                     MPI_Datatype datatype, MPI_Status *status);

  virtual int MPI_File_read_shared(MPI_File fh, void *buf, int count,
                                    MPI_Datatype datatype, MPI_Status *status);

  virtual int MPI_File_sync(MPI_File fh);

  virtual int MPI_File_write_all_begin(MPI_File fh, const void *buf, int count,
                                        MPI_Datatype datatype);

  virtual int MPI_File_write_all(MPI_File fh, const void *buf, int count,
                                  MPI_Datatype datatype, MPI_Status *status);

  virtual int MPI_File_write_at_all_begin(MPI_File fh, MPI_Offset offset,
                                           const void *buf, int count,
                                           MPI_Datatype datatype);

  virtual int MPI_File_write_at_all(MPI_File fh, MPI_Offset offset,
                                     const void *buf, int count,
                                     MPI_Datatype datatype, MPI_Status *status);

  virtual int MPI_File_write_at(MPI_File fh, MPI_Offset offset,
                                 const void *buf, int count,
                                 MPI_Datatype datatype, MPI_Status *status);

  virtual int MPI_File_write(MPI_File fh, const void *buf, int count,
                              MPI_Datatype datatype, MPI_Status *status);

  virtual int MPI_File_write_ordered_begin(MPI_File fh, const void *buf,
                                            int count, MPI_Datatype datatype);

  virtual int MPI_File_write_ordered(MPI_File fh, const void *buf, int count,
                                      MPI_Datatype datatype,
                                      MPI_Status *status);

  virtual int MPI_File_write_shared(MPI_File fh, const void *buf, int count,
                                     MPI_Datatype datatype, MPI_Status *status);
};

}  // namespace brahma

GOTCHA_MACRO_TYPEDEF(MPI_File_close, int, (MPI_File * fh), (fh),
                     brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_set_size, int, (MPI_File fh, MPI_Offset size),
                     (fh, size), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_iread_at, int,
                     (MPI_File fh, MPI_Offset offset, void *buf, int count,
                      MPI_Datatype datatype, MPI_Request *request),
                     (fh, offset, buf, count, datatype, request),
                     brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_iread, int,
                     (MPI_File fh, void *buf, int count, MPI_Datatype datatype,
                      MPI_Request *request),
                     (fh, buf, count, datatype, request), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_iread_shared, int,
                     (MPI_File fh, void *buf, int count, MPI_Datatype datatype,
                      MPI_Request *request),
                     (fh, buf, count, datatype, request), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_iwrite_at, int,
                     (MPI_File fh, MPI_Offset offset, const void *buf,
                      int count, MPI_Datatype datatype, MPI_Request *request),
                     (fh, offset, buf, count, datatype, request),
                     brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_iwrite, int,
                     (MPI_File fh, const void *buf, int count,
                      MPI_Datatype datatype, MPI_Request *request),
                     (fh, buf, count, datatype, request), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_iwrite_shared, int,
                     (MPI_File fh, const void *buf, int count,
                      MPI_Datatype datatype, MPI_Request *request),
                     (fh, buf, count, datatype, request), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_open, int,
                     (MPI_Comm comm, const char *filename, int amode,
                      MPI_Info info, MPI_File *fh),
                     (comm, filename, amode, info, fh), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_read_all_begin, int,
                     (MPI_File fh, void *buf, int count, MPI_Datatype datatype),
                     (fh, buf, count, datatype), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_read_all, int,
                     (MPI_File fh, void *buf, int count, MPI_Datatype datatype,
                      MPI_Status *status),
                     (fh, buf, count, datatype, status), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_read_at_all, int,
                     (MPI_File fh, MPI_Offset offset, void *buf, int count,
                      MPI_Datatype datatype, MPI_Status *status),
                     (fh, offset, buf, count, datatype, status), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_read_at_all_begin, int,
                     (MPI_File fh, MPI_Offset offset, void *buf, int count,
                      MPI_Datatype datatype),
                     (fh, offset, buf, count, datatype), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_read_at, int,
                     (MPI_File fh, MPI_Offset offset, void *buf, int count,
                      MPI_Datatype datatype, MPI_Status *status),
                     (fh, offset, buf, count, datatype, status), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_read, int,
                     (MPI_File fh, void *buf, int count, MPI_Datatype datatype,
                      MPI_Status *status),
                     (fh, buf, count, datatype, status), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_read_ordered_begin, int,
                     (MPI_File fh, void *buf, int count, MPI_Datatype datatype),
                     (fh, buf, count, datatype), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_read_ordered, int,
                     (MPI_File fh, void *buf, int count, MPI_Datatype datatype,
                      MPI_Status *status),
                     (fh, buf, count, datatype, status), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_read_shared, int,
                     (MPI_File fh, void *buf, int count, MPI_Datatype datatype,
                      MPI_Status *status),
                     (fh, buf, count, datatype, status), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_sync, int, (MPI_File fh), (fh), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_write_all_begin, int,
                     (MPI_File fh, const void *buf, int count,
                      MPI_Datatype datatype),
                     (fh, buf, count, datatype), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_write_all, int,
                     (MPI_File fh, const void *buf, int count,
                      MPI_Datatype datatype, MPI_Status *status),
                     (fh, buf, count, datatype, status), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_write_at_all_begin, int,
                     (MPI_File fh, MPI_Offset offset, const void *buf,
                      int count, MPI_Datatype datatype),
                     (fh, offset, buf, count, datatype), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_write_at_all, int,
                     (MPI_File fh, MPI_Offset offset, const void *buf,
                      int count, MPI_Datatype datatype, MPI_Status *status),
                     (fh, offset, buf, count, datatype, status), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_write_at, int,
                     (MPI_File fh, MPI_Offset offset, const void *buf,
                      int count, MPI_Datatype datatype, MPI_Status *status),
                     (fh, offset, buf, count, datatype, status), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_write, int,
                     (MPI_File fh, const void *buf, int count,
                      MPI_Datatype datatype, MPI_Status *status),
                     (fh, buf, count, datatype, status), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_write_ordered_begin, int,
                     (MPI_File fh, const void *buf, int count,
                      MPI_Datatype datatype),
                     (fh, buf, count, datatype), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_write_ordered, int,
                     (MPI_File fh, const void *buf, int count,
                      MPI_Datatype datatype, MPI_Status *status),
                     (fh, buf, count, datatype, status), brahma::MPIIO);

GOTCHA_MACRO_TYPEDEF(MPI_File_write_shared, int,
                     (MPI_File fh, const void *buf, int count,
                      MPI_Datatype datatype, MPI_Status *status),
                     (fh, buf, count, datatype, status), brahma::MPIIO);
#endif  // BRAHMA_MPIIO_H
