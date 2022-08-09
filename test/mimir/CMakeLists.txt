set(examples mimir_test)

function(gcc_base example)
    add_executable(${example} ${example}.cpp ${TEST_SRC})
    target_link_libraries(${example} ${TEST_LIBS})
endfunction()

foreach (example ${examples})
    gcc_base(${example})
endforeach()