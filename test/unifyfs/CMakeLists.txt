set(examples unifyfs_basic)

function(gcc_base example)
    add_executable(${example} ${example}.cpp ${TEST_SRC})
    target_link_libraries(${example} ${TEST_LIBS} ${UNIFYFS_API_LIBRARIES})
endfunction()

find_program(BASH_PROGRAM bash)
function(unifyfs_daemon unifyfs_logio_spill_dir unifyfs_log_dir pfs test_name test_exec test_args)
    #TEST_EXEC=$1 UNIFYFS_EXEC=$2 UNIFYFS_LOGIO_SPILL_DIR=$3 UNIFYFS_LOG_DIR=$4 PFS=$5
    add_test(Test_${test_name} ${BASH_PROGRAM}
            ${CMAKE_CURRENT_SOURCE_DIR}/unifyfs_run_single.sh
            ${CMAKE_CURRENT_BINARY_DIR}/${test_exec}
            ${UNIFYFS_EXEC}
            ${unifyfs_logio_spill_dir}
            ${unifyfs_log_dir}
            ${pfs}
            ${test_args})
endfunction()

set(UNIFYFS_LOGIO_SPILL_DIR $ENV{HOME}/unifyfs/ssd)
set(UNIFYFS_LOG_DIR $ENV{HOME}/unifyfs/logs)
set(PFS $ENV{HOME}/unifyfs/pfs)
file(MAKE_DIRECTORY ${UNIFYFS_LOGIO_SPILL_DIR})
file(MAKE_DIRECTORY ${UNIFYFS_LOG_DIR})
file(MAKE_DIRECTORY ${PFS})

foreach (example ${examples})
    gcc_base(${example})
    unifyfs_daemon(${UNIFYFS_LOGIO_SPILL_DIR} ${UNIFYFS_LOG_DIR} ${PFS} example ${example} "")
endforeach ()