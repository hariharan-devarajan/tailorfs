cmake_minimum_required(VERSION 3.0)
project(tailorfs)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_SOURCE_DIR}/dependency/.spack-env/view)
#message(${CMAKE_PREFIX_PATH})
set(CMAKE_CXX_STANDARD 17)
set(TAILORFS_CMAKE_DIR ${CMAKE_SOURCE_DIR}/CMake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${TAILORFS_CMAKE_DIR})
set(DEPENDENCY_LIB)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic")
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(external/brahma)
set(DEPENDENCY_LIB ${DEPENDENCY_LIB} brahma)
include_directories(external/brahma/include)

add_subdirectory(external/cpp-logger)
set(DEPENDENCY_LIB ${DEPENDENCY_LIB} cpp-logger)
include_directories(external/cpp-logger/include)

add_subdirectory(external/mimir)
set(DEPENDENCY_LIB ${DEPENDENCY_LIB} mimir)
include_directories(external/mimir/include)


find_package(unifyfs REQUIRED)
if (NOT ${UNIFYFS_FOUND})
    message(FATAL_ERROR "UnifyFS is needed for TailorFS build")
else ()
    include_directories(${UNIFYFS_INCLUDE_DIRS})
    set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${UNIFYFS_API_LIBRARIES})
endif ()
find_package(MPI COMPONENTS CXX REQUIRED)
if (MPI_FOUND)
    message(STATUS "found mpi.h at ${MPI_CXX_INCLUDE_DIRS}")
    include_directories(${MPI_CXX_INCLUDE_DIRS})
    set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${MPI_CXX_LIBRARIES})
endif ()


set(TAILORFS_SRC src/tailorfs/brahma/posix.cpp src/tailorfs/brahma/stdio.cpp)
set(TAILORFS_PRIVATE_INCLUDE src/tailorfs/brahma/posix.h src/tailorfs/brahma/stdio.h)
set(TAILORFS_PUBLIC_INCLUDE include/tailorfs/tailorfs.h include/tailorfs/macro.h)

add_library(tailorfs SHARED ${TAILORFS_PUBLIC_INCLUDE} ${TAILORFS_PRIVATE_INCLUDE} ${TAILORFS_SRC})
target_link_libraries(tailorfs ${DEPENDENCY_LIB})
add_dependencies(tailorfs brahma)

add_executable(tailorfs_exe main.cpp)
target_link_libraries(tailorfs_exe tailorfs ${DEPENDENCY_LIB})

option(TAILORFS_BUILD_TEST "Build the unit tests" ON)
if (TAILORFS_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

