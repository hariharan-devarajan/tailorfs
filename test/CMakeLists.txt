find_package(Catch2 REQUIRED)
set(TEST_LIBS Catch2::Catch2 -lstdc++fs ${MPI_CXX_LIBRARIES})
set(TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/catch_config.h ${CMAKE_CURRENT_SOURCE_DIR}/test_utils.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(unifyfs)