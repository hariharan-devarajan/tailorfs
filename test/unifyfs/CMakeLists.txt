set(examples unifyfs_basic)
set(BB_PATH $ENV{BBPATH}/unifyfs)
set(PFS_PATH $ENV{pfs}/unifyfs)
set(SHM_PATH $ENV{SHM_PATH})


if (NOT DEFINED ENV{BBPATH})
    message(FATAL_ERROR "BBPATH not defined")
endif ()
if (NOT DEFINED ENV{pfs})
    message(FATAL_ERROR "pfs not defined")
endif ()
if (NOT DEFINED ENV{SHM_PATH})
    message(FATAL_ERROR "SHM_PATH not defined")
endif ()

function(gcc_base example)
    add_executable(${example} ${example}.cpp ${TEST_SRC})
    target_link_libraries(${example} ${TEST_LIBS} ${UNIFYFS_API_LIBRARIES} -lcrypto)
endfunction()

find_program(BASH_PROGRAM bash)
function(unifyfs_daemon unifyfs_logio_spill_dir unifyfs_log_dir unifyfs_hostfile pfs test_name test_exec request_size iteration string)
    #TEST_EXEC=$1 UNIFYFS_EXEC=$2 UNIFYFS_LOGIO_SPILL_DIR=$3 UNIFYFS_LOG_DIR=$4 PFS=$5
    message("-- test args --request_size ${request_size} --iterations ${iteration}")
    set(RANKS_PER_NODE 1)
    add_test(Test_${test_name} ${BASH_PROGRAM}
            ${CMAKE_CURRENT_SOURCE_DIR}/unifyfs_run_single.sh
            ${CMAKE_CURRENT_BINARY_DIR}/${test_exec}
            ${UNIFYFS_EXEC}
            ${unifyfs_logio_spill_dir}
            ${unifyfs_log_dir}
            ${pfs}
            ${unifyfs_hostfile}
            ${MPIEXEC_EXECUTABLE}
            ${RANKS_PER_NODE}
            --request_size ${request_size} --iteration ${iteration} --rpn ${RANKS_PER_NODE} ${string})# --section "storage.unifyfs.buffer")
    set_property(TEST Test_${test_name} APPEND PROPERTY ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/dependency/.spack-env/view/lib)
    set_property(TEST Test_${test_name} APPEND PROPERTY ENVIRONMENT PFS_PATH=${PFS_PATH})
    set_property(TEST Test_${test_name} APPEND PROPERTY ENVIRONMENT SHM_PATH=${SHM_PATH})
endfunction()

set(UNIFYFS_LOGIO_SPILL_DIR ${BB_PATH})
set(UNIFYFS_LOG_DIR $ENV{HOME}/unifyfs/logs)
set(UNIFYFS_HOSTFILE $ENV{HOME}/unifyfs/hostfile)
file(MAKE_DIRECTORY ${BB_PATH})
file(MAKE_DIRECTORY ${UNIFYFS_LOG_DIR})
file(MAKE_DIRECTORY ${SHM_PATH})
file(MAKE_DIRECTORY ${PFS_PATH})
message("-- Created Directories BB_PATH ${BB_PATH}")
message("-- Created Directories SHM_PATH ${SHM_PATH}")
message("-- Created Directories PFS_PATH ${PFS_PATH}")

set(rqs 4 16 64 128 512 1024)
set(scales 1024)
set(iters 100 1000)# 10000)

foreach (example ${examples})
    gcc_base(${example})
    foreach(rq ${rqs})
        foreach(scale ${scales})
            foreach(iter ${iters})
                math(EXPR ts "${rq} * ${scale}")
                unifyfs_daemon(${UNIFYFS_LOGIO_SPILL_DIR} ${UNIFYFS_LOG_DIR} ${UNIFYFS_HOSTFILE} ${PFS_PATH} example_write_${rq}_${iter} ${example} ${ts} ${iter}  "[type=write-only]")
                unifyfs_daemon(${UNIFYFS_LOGIO_SPILL_DIR} ${UNIFYFS_LOG_DIR} ${UNIFYFS_HOSTFILE} ${PFS_PATH} example_read_${rq}_${iter} ${example} ${ts} ${iter} "[type=read-only]")
                unifyfs_daemon(${UNIFYFS_LOGIO_SPILL_DIR} ${UNIFYFS_LOG_DIR} ${UNIFYFS_HOSTFILE} ${PFS_PATH} example_pc_${rq}_${iter} ${example} ${ts} ${iter} "[type=pc]")
            endforeach()
        endforeach()
    endforeach()
endforeach ()
