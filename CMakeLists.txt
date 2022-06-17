cmake_minimum_required(VERSION 3.0)
project(tailorfs)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_SOURCE_DIR}/dependency/.spack-env/view)
#message(${CMAKE_PREFIX_PATH})
set(CMAKE_CXX_STANDARD 14)
set(TAILORFS_CMAKE_DIR ${CMAKE_SOURCE_DIR}/CMake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${TAILORFS_CMAKE_DIR})

find_package(unifyfs REQUIRED)
if (NOT ${UNIFYFS_FOUND})
    message(FATAL_ERROR "UnifyFS is needed for TailorFS build")
else()
    include_directories(${UNIFYFS_INCLUDE_DIRS})
endif()
add_executable(tailorfs main.cpp)
target_link_libraries(tailorfs unifyfs::unifyfs)


