set(examples mimir_test)

function(gcc_base example)
    add_executable(${example} ${example}.cpp ${TEST_SRC})
    target_link_libraries(${example} ${TEST_LIBS} ${MIMIR_LIBRARIES})
    add_dependencies(${example} mimir)
endfunction()

foreach (example ${examples})
    gcc_base(${example})
endforeach ()

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
set(rqs_kb 4 1024)
set(iters 1024)
set(num_apps 1)
set(file_sharings "fpp" "shared")
set(access_patterns "seq" "random")
set(ops "wo" "ro" "raw" "update" "worm")
set(process_groupings "all" "split" "alt")
set(count 0)
function(create_test)
    #[[message(STATUS "machine: " ${machine} " ranks_per_node:" ${ranks_per_node}
            " num_app:" ${num_app} " rq_kb:" ${rq_kb}
            " iter:" ${iter} " io_size_per_app_mb:" ${io_size_per_app_mb}
            " file_sharing:" ${file_sharing} " access_pattern:" ${access_pattern}
            " num_process_per_app:" ${num_process_per_app} " num_files_per_app:" ${num_files_per_app}
            " fpp_percentage:" ${fpp_percentage} " sequential_percentage:" ${sequential_percentage}
            " wo_file_ptg:" ${wo_file_ptg} " ro_file_ptg:" ${ro_file_ptg}
            " raw_file_ptg:" ${raw_file_ptg} " update_file_ptg:" ${update_file_ptg}
            " worm_file_ptg:" ${worm_file_ptg} " all_app_ptg:" ${all_app_ptg}
            " split_app_ptg:" ${split_app_ptg} " alt_app_ptg:" ${alt_app_ptg})]]

    math(EXPR count "${count} + 1")
    set(count ${count} PARENT_SCOPE)
    #message(STATUS ${count})
    set(config_file_suffix ${NUM_NODES}_${ranks_per_node}_${num_app}_${rq_kb}_${iter}_${file_sharing}_${access_pattern}_${op}_${process_grouping})

    set(config_file ${CMAKE_CURRENT_SOURCE_DIR}/json_files/${machine}/mimir_config_${config_file_suffix}.json)
    set(TEST_ARGS --ranks_per_node ${ranks_per_node}
            --io_size_per_app_mb ${io_size_per_app_mb}
            --config_file ${config_file}
            --num_apps ${num_apps}
            --num_process_per_app ${num_process_per_app}
            --num_files_per_app ${num_files_per_app}
            --fpp_percentage ${fpp_percentage}
            --sequential_percentage ${sequential_percentage}
            --wo_file_ptg ${wo_file_ptg}
            --ro_file_ptg ${ro_file_ptg}
            --raw_file_ptg ${raw_file_ptg}
            --update_file_ptg ${update_file_ptg}
            --worm_file_ptg ${worm_file_ptg}
            --all_app_ptg ${all_app_ptg}
            --split_app_ptg ${split_app_ptg}
            --alt_app_ptg ${alt_app_ptg}
            --binary_directory ${CMAKE_BINARY_DIR}/bin
            --reporter compact
            )
    set(test_name test_generate_config_${machine}_${config_file_suffix})
    add_test(${test_name} jsrun -p 1 ${CMAKE_BINARY_DIR}/bin/mimir_test ${TEST_ARGS} GenerateConfig)
    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT pfs=$ENV{pfs})
    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT BBPATH=$ENV{BBPATH})
    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:${CMAKE_SOURCE_DIR}/dependency/.spack-env/view/lib)


    set(test_name test_load_config_${machine}_${config_file_suffix})
    add_test(${test_name} jsrun -p 1 ${CMAKE_CURRENT_BINARY_DIR}/mimir_test ${TEST_ARGS} LoadConfig)
    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT pfs=$ENV{pfs})
    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT PFS_PATH=$ENV{pfs})
    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT BBPATH=$ENV{BBPATH})
    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT MIMIR_CONFIG_PATH=${config_file})
    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:${CMAKE_SOURCE_DIR}/dependency/.spack-env/view/lib)
endfunction()
foreach (machine ${machines})
    if (${machine} STREQUAL "local")
        set(ranks_per_nodes ${ranks_per_nodes_local})
    elseif (${machine} STREQUAL "lassen")
        set(ranks_per_nodes ${ranks_per_nodes_lassen})
    endif ()
    foreach (ranks_per_node ${ranks_per_nodes})
        foreach (num_app ${num_apps})
            foreach (rq_kb ${rqs_kb})
                foreach (iter ${iters})
                    math(EXPR io_size_per_app_mb "${rq_kb} * ${iter} * ${ranks_per_node} * ${NUM_NODES}  / 1024 / ${num_app}")
                    foreach (file_sharing ${file_sharings})
                        foreach (access_pattern ${access_patterns})
                            math(EXPR num_process_per_app "${ranks_per_node} * ${NUM_NODES} / ${num_app}")
                            if (file_sharing STREQUAL "fpp")
                                set(num_files_per_app ${num_process_per_app})
                                set(fpp_percentage "1.0")
                            elseif (file_sharing STREQUAL "shared")
                                set(num_files_per_app 1)
                                set(fpp_percentage "0.0")
                            endif ()
                            if (access_pattern STREQUAL "seq")
                                set(sequential_percentage "1.0")
                            elseif (access_pattern STREQUAL "random")
                                set(sequential_percentage "0.0")
                            endif ()
                            foreach (op ${ops})
                                set(wo_file_ptg "0.0")
                                set(ro_file_ptg "0.0")
                                set(raw_file_ptg "0.0")
                                set(update_file_ptg "0.0")
                                set(worm_file_ptg "0.0")
                                set(all_app_ptg "0.0")
                                set(split_app_ptg "0.0")
                                set(alt_app_ptg "0.0")
                                if (op STREQUAL "wo")
                                    set(wo_file_ptg "1.0")
                                elseif (op STREQUAL "ro")
                                    set(ro_file_ptg "1.0")
                                elseif (op STREQUAL "raw")
                                    set(raw_file_ptg "1.0")
                                elseif (op STREQUAL "update")
                                    set(update_file_ptg "1.0")
                                elseif (op STREQUAL "worm")
                                    set(worm_file_ptg "1.0")
                                endif ()
                                if (op STREQUAL "wo" OR op STREQUAL "ro" OR op STREQUAL "update")
                                    set(process_grouping "all")
                                    set(all_app_ptg "1.0")
                                    set(split_app_ptg "0.0")
                                    set(alt_app_ptg "0.0")
                                    create_test()
                                elseif (op STREQUAL "raw" OR op STREQUAL "worm")
                                    foreach (process_grouping ${process_groupings})
                                        if (ranks_per_node GREATER 1 OR process_grouping STREQUAL "all")
                                            set(all_app_ptg "0.0")
                                            set(split_app_ptg "0.0")
                                            set(alt_app_ptg "0.0")
                                            if (process_grouping STREQUAL "all")
                                                set(all_app_ptg "1.0")
                                            elseif (process_grouping STREQUAL "split")
                                                set(split_app_ptg "1.0")
                                            elseif (process_grouping STREQUAL "alt")
                                                set(alt_app_ptg "1.0")
                                            endif ()
                                            create_test()
                                        endif ()
                                    endforeach ()

                                endif ()
                            endforeach ()
                        endforeach ()
                    endforeach ()
                endforeach ()
            endforeach ()
        endforeach ()
    endforeach ()
endforeach ()






