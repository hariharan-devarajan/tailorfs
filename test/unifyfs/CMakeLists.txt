set(examples unifyfs_basic)

function(gcc_base example)
    add_executable(${example} ${example}.cpp ${TEST_SRC})
    target_link_libraries(${example} ${TEST_LIBS} ${UNIFYFS_API_LIBRARIES} -lcrypto)
endfunction()

find_program(BASH_PROGRAM bash)
function(unifyfs_daemon unifyfs_logio_spill_dir unifyfs_log_dir unifyfs_hostfile pfs test_name test_exec request_size iteration string)
    #TEST_EXEC=$1 UNIFYFS_EXEC=$2 UNIFYFS_LOGIO_SPILL_DIR=$3 UNIFYFS_LOG_DIR=$4 PFS=$5
    message("-- test args --request_size ${request_size} --iterations ${iteration}")
    set(RANKS_PER_NODE 2)
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
            --request_size ${request_size} --iteration ${iteration} --rpn ${RANKS_PER_NODE} ${string} --section "storage.unifyfs.buffer")
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
            --request_size ${request_size} --iteration ${iteration} --rpn ${RANKS_PER_NODE} ${string} --section "storage.pfs")
    set_property(TEST Test_pfs_${test_name} APPEND PROPERTY ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/dependency/.spack-env/view/lib)
    set_property(TEST Test_pfs_${test_name} APPEND PROPERTY ENVIRONMENT PFS_PATH=${PFS_PATH})
    set_property(TEST Test_pfs_${test_name} APPEND PROPERTY ENVIRONMENT SHM_PATH=${SHM_PATH})
endfunction()

set(UNIFYFS_LOGIO_SPILL_DIR $ENV{BBPATH}/unifyfs)
set(UNIFYFS_LOG_DIR $ENV{HOME}/unifyfs/logs)
set(UNIFYFS_HOSTFILE $ENV{HOME}/unifyfs/hostfile)
file(MAKE_DIRECTORY $ENV{BBPATH}/unifyfs)
file(MAKE_DIRECTORY ${UNIFYFS_LOG_DIR})
file(MAKE_DIRECTORY /dev/shm/unifyfs)
file(MAKE_DIRECTORY $ENV{pfs}/unifyfs)
message("-- Created Directories BB_PATH $ENV{BBPATH}/unifyfs")
message("-- Created Directories SHM_PATH /dev/shm/unifyfs")
message("-- Created Directories PFS_PATH $ENV{pfs}/unifyfs")

set(rqs 4 16 64 128 512 1024)
set(scales 1024)
set(iters 128 1024)# 10000)

foreach (example ${examples})
    gcc_base(${example})
    foreach(rq ${rqs})
        foreach(scale ${scales})
            foreach(iter ${iters})
                math(EXPR ts "${rq} * ${scale}")
                unifyfs_daemon(${UNIFYFS_LOGIO_SPILL_DIR} ${UNIFYFS_LOG_DIR} ${UNIFYFS_HOSTFILE} $ENV{pfs}/unifyfs example_write_${rq}_${iter} ${example} ${ts} ${iter}  "[type=write-only]")
                unifyfs_daemon(${UNIFYFS_LOGIO_SPILL_DIR} ${UNIFYFS_LOG_DIR} ${UNIFYFS_HOSTFILE} $ENV{pfs}/unifyfs example_read_${rq}_${iter} ${example} ${ts} ${iter} "[type=read-only]")
                unifyfs_daemon(${UNIFYFS_LOGIO_SPILL_DIR} ${UNIFYFS_LOG_DIR} ${UNIFYFS_HOSTFILE} $ENV{pfs}/unifyfs example_pc_${rq}_${iter} ${example} ${ts} ${iter} "[type=pc]")
            endforeach()
        endforeach()
    endforeach()
endforeach ()
