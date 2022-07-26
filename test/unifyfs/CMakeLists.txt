set(examples unifyfs_basic)

function(gcc_base example)
    add_executable(${example} ${example}.cpp ${TEST_SRC})
    target_link_libraries(${example} ${TEST_LIBS} ${UNIFYFS_API_LIBRARIES} -lcrypto)
endfunction()

find_program(BASH_PROGRAM bash)
function(unifyfs_daemon unifyfs_logio_spill_dir unifyfs_log_dir unifyfs_hostfile pfs test_name test_exec request_size iteration storage_type access_pattern file_sharing process_grouping string)
    #TEST_EXEC=$1 UNIFYFS_EXEC=$2 UNIFYFS_LOGIO_SPILL_DIR=$3 UNIFYFS_LOG_DIR=$4 PFS=$5
    message("-- test args --request_size ${request_size} --iterations ${iteration}")
    set(RANKS_PER_NODE 32)
    set(TEST_ARGS --request_size ${request_size} --iteration ${iteration} --rpn ${RANKS_PER_NODE} --storage_type ${storage_type} --access_pattern ${access_pattern} --file_sharing ${file_sharing} --process_grouping ${process_grouping} ${string})

    add_test(Test_unify_${test_name} ${BASH_PROGRAM}
            ${CMAKE_CURRENT_SOURCE_DIR}/run_single.sh
            ${CMAKE_CURRENT_BINARY_DIR}/${test_exec}
            ${UNIFYFS_EXEC}
            ${unifyfs_logio_spill_dir}
            ${unifyfs_log_dir}
            ${pfs}
            ${unifyfs_hostfile}
            ${MPIEXEC_EXECUTABLE}
            ${RANKS_PER_NODE}
            ${TEST_ARGS} --section "storage.unifyfs.buffer")
    set_property(TEST Test_unify_${test_name} APPEND PROPERTY ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/dependency/.spack-env/view/lib)
    set_property(TEST Test_unify_${test_name} APPEND PROPERTY ENVIRONMENT PFS_PATH=${PFS_PATH})
    set_property(TEST Test_unify_${test_name} APPEND PROPERTY ENVIRONMENT SHM_PATH=${SHM_PATH})

    add_test(Test_pfs_${test_name} ${BASH_PROGRAM}
            ${CMAKE_CURRENT_SOURCE_DIR}/run_single.sh
            ${CMAKE_CURRENT_BINARY_DIR}/${test_exec}
            ${UNIFYFS_EXEC}
            ${unifyfs_logio_spill_dir}
            ${unifyfs_log_dir}
            ${pfs}
            ${unifyfs_hostfile}
            ${MPIEXEC_EXECUTABLE}
            ${RANKS_PER_NODE}
            ${TEST_ARGS} ${string} --section "storage.pfs")
    set_property(TEST Test_pfs_${test_name} APPEND PROPERTY ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/dependency/.spack-env/view/lib)
    set_property(TEST Test_pfs_${test_name} APPEND PROPERTY ENVIRONMENT PFS_PATH=${PFS_PATH})
    set_property(TEST Test_pfs_${test_name} APPEND PROPERTY ENVIRONMENT SHM_PATH=${SHM_PATH})
endfunction()

set(UNIFYFS_LOGIO_SPILL_DIR $ENV{BBPATH}/unifyfs)
set(UNIFYFS_LOG_DIR $ENV{HOME}/unifyfs/logs)
set(UNIFYFS_HOSTFILE $ENV{HOME}/unifyfs/hostfile)
#file(MAKE_DIRECTORY $ENV{BBPATH}/unifyfs)
file(MAKE_DIRECTORY ${UNIFYFS_LOG_DIR})
file(MAKE_DIRECTORY /dev/shm/unifyfs)
file(MAKE_DIRECTORY $ENV{pfs}/unifyfs)
message("-- Created Directories BB_PATH $ENV{BBPATH}/unifyfs")
message("-- Created Directories SHM_PATH /dev/shm/unifyfs")
message("-- Created Directories PFS_PATH $ENV{pfs}/unifyfs")

set(rqs 4 16 64 128 512 1024)
set(scales 1024)
set(iters 128 1024)# 10000)
set(storage_types 1)
set(file_sharings 0 1)
set(process_groupings 0 1 2)
set(access_patterns 0 1)

foreach (example ${examples})
    gcc_base(${example})
    foreach (rq ${rqs})
        foreach (scale ${scales})
            foreach (iter ${iters})
                math(EXPR ts "${rq} * ${scale}")
                foreach (storage_type ${storage_types})
                    foreach (file_sharing ${file_sharings})
                        set(process_grouping 0)
                        set(access_pattern 0)
                        unifyfs_daemon(${UNIFYFS_LOGIO_SPILL_DIR} ${UNIFYFS_LOG_DIR} ${UNIFYFS_HOSTFILE} $ENV{pfs}/unifyfs example_write_${rq}_${iter}_${storage_type}_${access_pattern}_${file_sharing}_${process_grouping} ${example} ${ts} ${iter} ${storage_type} ${access_pattern} ${file_sharing} ${process_grouping} "[type=write-only]")
                    endforeach ()
                endforeach ()
                foreach (storage_type ${storage_types})
                    foreach (file_sharing ${file_sharings})
                        foreach (access_pattern ${access_patterns})
                            set(process_grouping 0)
                            unifyfs_daemon(${UNIFYFS_LOGIO_SPILL_DIR} ${UNIFYFS_LOG_DIR} ${UNIFYFS_HOSTFILE} $ENV{pfs}/unifyfs example_read_${rq}_${iter}_${storage_type}_${access_pattern}_${file_sharing}_${process_grouping} ${example} ${ts} ${iter} ${storage_type} ${access_pattern} ${file_sharing} ${process_grouping} "[type=read-only]")
                        endforeach ()
                    endforeach ()
                endforeach ()
                foreach (storage_type ${storage_types})
                    foreach (file_sharing ${file_sharings})
                        foreach (access_pattern ${access_patterns})
                            foreach (process_grouping ${process_groupings})
                                unifyfs_daemon(${UNIFYFS_LOGIO_SPILL_DIR} ${UNIFYFS_LOG_DIR} ${UNIFYFS_HOSTFILE} $ENV{pfs}/unifyfs example_raw_${rq}_${iter}_${storage_type}_${access_pattern}_${file_sharing}_${process_grouping} ${example} ${ts} ${iter} ${storage_type} ${access_pattern} ${file_sharing} ${process_grouping} "[type=raw]")
                                unifyfs_daemon(${UNIFYFS_LOGIO_SPILL_DIR} ${UNIFYFS_LOG_DIR} ${UNIFYFS_HOSTFILE} $ENV{pfs}/unifyfs example_update_${rq}_${iter}_${storage_type}_${access_pattern}_${file_sharing}_${process_grouping} ${example} ${ts} ${iter} ${storage_type} ${access_pattern} ${file_sharing} ${process_grouping} "[type=update]")
                                unifyfs_daemon(${UNIFYFS_LOGIO_SPILL_DIR} ${UNIFYFS_LOG_DIR} ${UNIFYFS_HOSTFILE} $ENV{pfs}/unifyfs example_worm_${rq}_${iter}_${storage_type}_${access_pattern}_${file_sharing}_${process_grouping} ${example} ${ts} ${iter} ${storage_type} ${access_pattern} ${file_sharing} ${process_grouping} "[type=worm]")
                            endforeach ()
                        endforeach ()
                    endforeach ()
                endforeach ()
            endforeach()
        endforeach()
    endforeach()
endforeach ()
