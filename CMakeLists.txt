cmake_minimum_required(VERSION 3.0)
project(tailorfs)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_SOURCE_DIR}/dependency/.spack-env/view ${CMAKE_BINARY_DIR}/lib/cmake)
#message(${CMAKE_PREFIX_PATH})
set(CMAKE_CXX_STANDARD 17)
set(TAILORFS_CMAKE_DIR ${CMAKE_SOURCE_DIR}/CMake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${TAILORFS_CMAKE_DIR})
set(DEPENDENCY_LIB -lstdc++fs)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic")
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(external/cpp-logger)
add_subdirectory(external/brahma)
add_dependencies(brahma cpp-logger)
add_subdirectory(external/mimir)

find_package(mimir REQUIRED)
if (${MIMIR_FOUND})
    message(STATUS "[MIMIR] found mimir at ${MIMIR_INCLUDE_DIRS}")
    include_directories(${MIMIR_INCLUDE_DIRS})
    set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${MIMIR_LIBRARIES})
else ()
    message(FATAL_ERROR "-- [MIMIR] is needed for ${PROJECT_NAME} build")
endif ()

find_package(cpp-logger REQUIRED)
if (${CPP_LOGGER_FOUND})
    include_directories(${CPP_LOGGER_INCLUDE_DIRS})
    set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${CPP_LOGGER_LIBRARIES})
    message(STATUS "[CPP_LOGGER] found cpp-logger at ${CPP_LOGGER_INCLUDE_DIRS}")
else ()
    message(FATAL_ERROR "-- [CPP_LOGGER] is needed for ${PROJECT_NAME} build")
endif ()
#message(STATUS "DEPENDENCY_LIB ${DEPENDENCY_LIB}")
find_package(brahma REQUIRED)
if (${BRAHMA_FOUND})
    message(STATUS "[BRAHMA] found brahma at ${BRAHMA_INCLUDE_DIRS}")
    include_directories(${BRAHMA_INCLUDE_DIRS})
    set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${BRAHMA_LIBRARIES})
else ()
    message(FATAL_ERROR "-- [BRAHMA] is needed for ${PROJECT_NAME} build")
endif ()

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


set(TAILORFS_SRC
        src/tailorfs/brahma/posix.cpp
        src/tailorfs/brahma/stdio.cpp
        src/tailorfs/core/fsview_manager.cpp
        src/tailorfs/core/fsview/stdio_fsview.cpp
        src/tailorfs/core/fsview/mpiio_fsview.cpp
        src/tailorfs/core/fsview/posix_fsview.cpp
        src/tailorfs/core/fsview/unifyfs_fsview.cpp)
set(TAILORFS_PRIVATE_INCLUDE src/tailorfs/brahma/posix.h src/tailorfs/brahma/stdio.h)
set(TAILORFS_PUBLIC_INCLUDE include/tailorfs/tailorfs.h include/tailorfs/macro.h)

add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME}
        PRIVATE
        ${TAILORFS_SRC}
        )
target_include_directories(${PROJECT_NAME}
        PRIVATE
        # where the library itself will look for its internal headers
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )
target_link_libraries(${PROJECT_NAME} ${DEPENDENCY_LIB})
add_dependencies(${PROJECT_NAME} brahma)
add_dependencies(${PROJECT_NAME} cpp-logger)

add_executable(tailorfs_exe main.cpp)
target_link_libraries(tailorfs_exe tailorfs ${DEPENDENCY_LIB})

option(TAILORFS_BUILD_TEST "Build the unit tests" ON)
if (TAILORFS_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

