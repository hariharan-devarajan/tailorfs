function(gcc_base example)
    add_executable(${example} ${example}.cpp ${TEST_SRC})
    target_link_libraries(${example} ${TEST_LIBS} ${UNIFYFS_API_LIBRARIES} -lcrypto)
endfunction()
gcc_base(unifyfs_basic)

find_program(BASH_PROGRAM bash)
function(unifyfs_daemon unifyfs_logio_spill_dir unifyfs_log_dir unifyfs_hostfile pfs test_name test_exec request_size iteration storage_type access_pattern file_sharing process_grouping string)
    #TEST_EXEC=$1 UNIFYFS_EXEC=$2 UNIFYFS_LOGIO_SPILL_DIR=$3 UNIFYFS_LOG_DIR=$4 PFS=$5
    #message("-- test args --request_size ${request_size} --iterations ${iteration}")
    set(RANKS_PER_NODE 32)
    set(TEST_ARGS --request_size ${request_size} --iteration ${iteration} --rpn ${RANKS_PER_NODE} --storage_type ${storage_type} --access_pattern ${access_pattern} --file_sharing ${file_sharing} --process_grouping ${process_grouping} --reporter compact ${string})

    add_test(Test_unifyd_${test_name} ${BASH_PROGRAM}
            ${CMAKE_CURRENT_SOURCE_DIR}/run_single.sh
            ${CMAKE_CURRENT_BINARY_DIR}/${test_exec}
            ${UNIFYFS_EXEC}
            ${unifyfs_logio_spill_dir}
            ${unifyfs_log_dir}
            ${pfs}
            ${unifyfs_hostfile}
            ${MPIEXEC_EXECUTABLE}
            ${RANKS_PER_NODE}
            ${TEST_ARGS} --section "storage.unifyfs" --default_unifyfs 1)
    set_property(TEST Test_unifyd_${test_name} APPEND PROPERTY ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/dependency/.spack-env/view/lib)
    set_property(TEST Test_unifyd_${test_name} APPEND PROPERTY ENVIRONMENT PFS_PATH=${PFS_PATH})
    set_property(TEST Test_unifyd_${test_name} APPEND PROPERTY ENVIRONMENT SHM_PATH=${SHM_PATH})

    add_test(Test_unifyo_${test_name} ${BASH_PROGRAM}
            ${CMAKE_CURRENT_SOURCE_DIR}/run_single.sh
            ${CMAKE_CURRENT_BINARY_DIR}/${test_exec}
            ${UNIFYFS_EXEC}
            ${unifyfs_logio_spill_dir}
            ${unifyfs_log_dir}
            ${pfs}
            ${unifyfs_hostfile}
            ${MPIEXEC_EXECUTABLE}
            ${RANKS_PER_NODE}
            ${TEST_ARGS} --section "storage.unifyfs" --default_unifyfs 0)
    set_property(TEST Test_unifyo_${test_name} APPEND PROPERTY ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/dependency/.spack-env/view/lib)
    set_property(TEST Test_unifyo_${test_name} APPEND PROPERTY ENVIRONMENT PFS_PATH=${PFS_PATH})
    set_property(TEST Test_unifyo_${test_name} APPEND PROPERTY ENVIRONMENT SHM_PATH=${SHM_PATH})

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

if (NOT DEFINED ENV{LSB_MAX_NUM_PROCESSORS})
    message(STATUS "Environment variable LSB_MAX_NUM_PROCESSORS not defined. Default to 41.")
    set(LSB_MAX_NUM_PROCESSORS 41)
else ()
    set(LSB_MAX_NUM_PROCESSORS $ENV{LSB_MAX_NUM_PROCESSORS})
endif ()


set(MAX_CPUS ${LSB_MAX_NUM_PROCESSORS})
math(EXPR NUM_NODES "(${MAX_CPUS} - 1 ) / 40")
message(STATUS "Number of nodes: ${NUM_NODES}")
set(ranks_per_nodes_local 1 2)
set(ranks_per_nodes_lassen 1 2 32)
set(machines "local" "lassen")
set(rqs_kb 4 16 64 128 512 1024)
set(iters 128 1024)
set(storages "shm" "bb" "pfs")
set(file_sharings "fpp" "shared")
set(access_patterns "seq" "random")
set(ops "wo" "ro" "raw" "update" "worm")
set(process_groupings "all" "split" "alt")
set(interfaces "posix" "stdio" "mpiio" "unify-d" "unify-o")

function(create_test)
    #    message(STATUS "machine: " ${machine} " ranks_per_node:" ${ranks_per_node}
    #            " rq_kb:" ${rq_kb} " iter:" ${iter} " storage:" ${storage}
    #            " file_sharing:" ${file_sharing} " access_pattern:" ${access_pattern}
    #            " op:" ${op} " process_grouping:" ${process_grouping} " interface:" ${interface}
    #            " unifyc_index:" ${unifyc_index})
    set(test_suffix ${machine}_${NUM_NODES}_${ranks_per_node}_${rq_kb}_${iter}_${interface}_${storage}_${file_sharing}_${access_pattern}_${op}_${process_grouping})
    #message(STATUS ${test_suffix})
    if (${machine} STREQUAL "local" AND ${ranks_per_node} STREQUAL "1")
        set(mpi_exec)
    elseif (${machine} STREQUAL "local")
        set(mpi_exec mpirun -n ${ranks_per_node})
    elseif (${machine} STREQUAL "lassen")
        set(mpi_exec jsrun -r 1 -a ${ranks_per_node} -c ${ranks_per_node} -d packed)
    endif ()
    set(test_name benchmark_${test_suffix})
    set(TEST_ARGS
            --request_size ${rq_bytes}
            --iteration ${iter}
            --ranks_per_node ${ranks_per_node}
            --storage_type ${storage_index}
            --access_pattern ${access_pattern_index}
            --file_sharing ${file_sharing_index}
            --process_grouping ${process_grouping_index}
            --interface ${interface_index}
            --default_unifyfs ${unifyc_index}
            --reporter compact
            ${test_filter_str})
    if (interface STREQUAL "unify-o" OR interface STREQUAL "unify-d")
        add_test(${test_name} ${BASH_PROGRAM}
                ${CMAKE_CURRENT_SOURCE_DIR}/run_single.sh
                ${machine}
                ${ranks_per_node}
                ${CMAKE_SOURCE_DIR}
                ${CMAKE_BINARY_DIR}/bin/unifyfs_basic
                $ENV{HOME}/unifyfs/logs
                ${UNIFYFS_ROOT_DIR}
                ${TEST_ARGS})
        set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:${CMAKE_SOURCE_DIR}/dependency/.spack-env/view/lib)
        set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT PFS_PATH=$ENV{pfs})
        set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT BBPATH=$ENV{BBPATH})
    else ()
        add_test(${test_name} ${mpi_exec} ${CMAKE_BINARY_DIR}/bin/unifyfs_basic ${TEST_ARGS})
        set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT pfs=$ENV{pfs})
        set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT BBPATH=$ENV{BBPATH})
        set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:${CMAKE_SOURCE_DIR}/dependency/.spack-env/view/lib)
    endif ()

endfunction()

foreach (machine ${machines})
    if (${machine} STREQUAL "local")
        set(ranks_per_nodes ${ranks_per_nodes_local})
    elseif (${machine} STREQUAL "lassen")
        set(ranks_per_nodes ${ranks_per_nodes_lassen})
    endif ()
    foreach (ranks_per_node ${ranks_per_nodes})
        foreach (rq_kb ${rqs_kb})
            math(EXPR rq_bytes "${rq_kb} * 1024")
            foreach (iter ${iters})
                foreach (storage ${storages})
                    if (storage STREQUAL "shm")
                        set(storage_index 0)
                    elseif (storage STREQUAL "bb")
                        set(storage_index 1)
                    elseif (storage STREQUAL "pfs")
                        set(storage_index 2)
                    endif ()
                    foreach (file_sharing ${file_sharings})
                        if (file_sharing STREQUAL "fpp")
                            set(file_sharing_index 0)
                        elseif (file_sharing STREQUAL "shared")
                            set(file_sharing_index 1)
                        endif ()
                        foreach (access_pattern ${access_patterns})
                            if (access_pattern STREQUAL "seq")
                                set(access_pattern_index 0)
                            elseif (access_pattern STREQUAL "random")
                                set(access_pattern_index 1)
                            endif ()
                            foreach (op ${ops})
                                if (op STREQUAL "wo")
                                    set(test_filter_str "Write-Only")
                                elseif (op STREQUAL "ro")
                                    set(test_filter_str "Read-Only")
                                elseif (op STREQUAL "raw")
                                    set(test_filter_str "Read-After-Write")
                                elseif (op STREQUAL "update")
                                    set(test_filter_str "Update")
                                elseif (op STREQUAL "worm")
                                    set(test_filter_str "WORM")
                                endif ()
                                foreach (interface ${interfaces})
                                    if (interface STREQUAL "posix")
                                        set(interface_index 0)
                                    elseif (interface STREQUAL "stdio")
                                        set(interface_index 1)
                                    elseif (interface STREQUAL "mpiio")
                                        set(interface_index 2)
                                    elseif (interface STREQUAL "unify-o" OR interface STREQUAL "unify-d")
                                        set(interface_index 3)
                                    endif ()
                                    set(unifyc_index 0)
                                    if (interface STREQUAL "posix" OR interface STREQUAL "stdio")
                                        if (file_sharing STREQUAL "fpp")
                                            if (access_pattern STREQUAL "seq" AND op STREQUAL "wo")
                                                set(process_grouping "all")
                                                set(process_grouping_index 0)
                                                create_test()
                                            elseif (op STREQUAL "update" OR op STREQUAL "ro")
                                                set(process_grouping "all")
                                                set(process_grouping_index 0)
                                                create_test()
                                            elseif (op STREQUAL "raw" OR op STREQUAL "worm")
                                                foreach (process_grouping ${process_groupings})
                                                    if (process_grouping STREQUAL "all")
                                                        set(process_grouping_index 0)
                                                    elseif (process_grouping STREQUAL "split")
                                                        set(process_grouping_index 1)
                                                    elseif (process_grouping STREQUAL "alt")
                                                        set(process_grouping_index 1)
                                                    endif ()
                                                    if (ranks_per_node GREATER 1 OR process_grouping STREQUAL "all")
                                                        create_test()
                                                    endif ()
                                                endforeach ()
                                            endif ()
                                        endif ()
                                    elseif (interface STREQUAL "mpiio")
                                        if (access_pattern STREQUAL "seq" AND op STREQUAL "wo")
                                            set(process_grouping "all")
                                            set(process_grouping_index 0)
                                            create_test()
                                        elseif (op STREQUAL "update" OR op STREQUAL "ro")
                                            set(process_grouping "all")
                                            set(process_grouping_index 0)
                                            create_test()
                                        elseif (op STREQUAL "raw" OR op STREQUAL "worm")
                                            foreach (process_grouping ${process_groupings})
                                                if (process_grouping STREQUAL "all")
                                                    set(process_grouping_index 0)
                                                elseif (process_grouping STREQUAL "split")
                                                    set(process_grouping_index 1)
                                                elseif (process_grouping STREQUAL "alt")
                                                    set(process_grouping_index 1)
                                                endif ()
                                                if (ranks_per_node GREATER 1 OR process_grouping STREQUAL "all")
                                                    create_test()
                                                endif ()
                                            endforeach ()
                                        endif ()
                                    else ()
                                        if (interface STREQUAL "unify-o")
                                            set(unifyc_index 0)
                                        elseif (interface STREQUAL "unify-d")
                                            set(unifyc_index 1)
                                        endif ()
                                        if (access_pattern STREQUAL "seq" AND op STREQUAL "wo")
                                            set(process_grouping "all")
                                            set(process_grouping_index 0)
                                            create_test()
                                        elseif (op STREQUAL "update" OR op STREQUAL "ro")
                                            set(process_grouping "all")
                                            set(process_grouping_index 0)
                                            create_test()
                                        elseif (op STREQUAL "raw" OR op STREQUAL "worm")
                                            foreach (process_grouping ${process_groupings})
                                                if (process_grouping STREQUAL "all")
                                                    set(process_grouping_index 0)
                                                elseif (process_grouping STREQUAL "split")
                                                    set(process_grouping_index 1)
                                                elseif (process_grouping STREQUAL "alt")
                                                    set(process_grouping_index 1)
                                                endif ()
                                                if (ranks_per_node GREATER 1 OR process_grouping STREQUAL "all")
                                                    create_test()
                                                endif ()
                                            endforeach ()
                                        endif ()
                                    endif ()
                                endforeach ()
                            endforeach ()
                        endforeach ()
                    endforeach ()
                endforeach ()
            endforeach ()
        endforeach ()
    endforeach ()
endforeach ()
