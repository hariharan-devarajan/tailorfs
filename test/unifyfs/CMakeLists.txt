set(examples unifyfs_basic)
set(BB_PATH $ENV{BB_PATH})
set(PFS_PATH $ENV{PFS_PATH})
set(SHM_PATH $ENV{SHM_PATH})
if (NOT DEFINED ENV{BB_PATH})
    message(FATAL_ERROR "BB_PATH not defined")
endif ()
if (NOT DEFINED ENV{PFS_PATH})
    message(FATAL_ERROR "PFS_PATH not defined")
endif ()
if (NOT DEFINED ENV{SHM_PATH})
    message(FATAL_ERROR "SHM_PATH not defined")
endif ()

function(gcc_base example)
    add_executable(${example} ${example}.cpp ${TEST_SRC})
    target_link_libraries(${example} ${TEST_LIBS} ${UNIFYFS_API_LIBRARIES})
endfunction()

find_program(BASH_PROGRAM bash)
function(unifyfs_daemon unifyfs_logio_spill_dir unifyfs_log_dir unifyfs_hostfile pfs test_name test_exec test_args)
    #TEST_EXEC=$1 UNIFYFS_EXEC=$2 UNIFYFS_LOGIO_SPILL_DIR=$3 UNIFYFS_LOG_DIR=$4 PFS=$5
    add_test(Test_${test_name} ${BASH_PROGRAM}
            ${CMAKE_CURRENT_SOURCE_DIR}/unifyfs_run_single.sh
            ${CMAKE_CURRENT_BINARY_DIR}/${test_exec}
            ${UNIFYFS_EXEC}
            ${unifyfs_logio_spill_dir}
            ${unifyfs_log_dir}
            ${pfs}
            ${unifyfs_hostfile}
            ${test_args})
    set_property(TEST Test_${test_name} APPEND PROPERTY ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/dependency/.spack-env/view/lib)
    set_property(TEST Test_${test_name} APPEND PROPERTY ENVIRONMENT PFS_PATH=${PFS_PATH}/unifyfs/pfs)
    set_property(TEST Test_${test_name} APPEND PROPERTY ENVIRONMENT BB_PATH=${BB_PATH}/unifyfs/ssd)
    set_property(TEST Test_${test_name} APPEND PROPERTY ENVIRONMENT SHM_PATH=${SHM_PATH}/unifyfs/shm)
endfunction()

set(UNIFYFS_LOGIO_SPILL_DIR ${BB_PATH})
set(UNIFYFS_LOG_DIR $ENV{HOME}/unifyfs/logs)
set(UNIFYFS_HOSTFILE $ENV{HOME}/unifyfs/hostfile)
file(MAKE_DIRECTORY ${BB_PATH})
file(MAKE_DIRECTORY ${UNIFYFS_LOG_DIR})
file(MAKE_DIRECTORY ${SHM_PATH})
file(MAKE_DIRECTORY ${PFS_PATH})

foreach (example ${examples})
    gcc_base(${example})
    unifyfs_daemon(${UNIFYFS_LOGIO_SPILL_DIR} ${UNIFYFS_LOG_DIR} ${UNIFYFS_HOSTFILE} ${PFS_PATH} example ${example} "")
endforeach ()