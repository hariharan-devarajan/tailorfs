cmake_minimum_required(VERSION 3.12)
#------------------------------------------------------------------------------
# Version information
#------------------------------------------------------------------------------

set(TAILORFS_VERSION_MAJOR "0")
set(TAILORFS_VERSION_MINOR "1")
set(TAILORFS_VERSION_PATCH "0")
set(TAILORFS_PACKAGE ${PROJECT_NAME})
set(TAILORFS_PACKAGE_NAME ${PROJECT_NAME})
set(TAILORFS_PACKAGE_VERSION "${TAILORFS_VERSION_MAJOR}.${TAILORFS_VERSION_MINOR}.${TAILORFS_VERSION_PATCH}")
set(TAILORFS_PACKAGE_VERSION_MAJOR "${TAILORFS_VERSION_MAJOR}.${TAILORFS_VERSION_MINOR}")
set(TAILORFS_PACKAGE_VERSION_MINOR "${TAILORFS_VERSION_PATCH}")
set(TAILORFS_PACKAGE_STRING "${TAILORFS_PACKAGE_NAME} ${TAILORFS_PACKAGE_VERSION}")
set(TAILORFS_PACKAGE_TARNAME "${TAILORFS_PACKAGE}")

project(tailorfs LANGUAGES C CXX)

# Convenience defines
string(TOUPPER "${PROJECT_NAME}" UPPER_PROJECT_NAME)
string(TOLOWER "${PROJECT_NAME}" LOWER_PROJECT_NAME)

#------------------------------------------------------------------------------
# Internal Includes for header and libraries
#------------------------------------------------------------------------------
# Get installation directories -- these get used in various places;
# best to just make them available
option (TAILORFS_LIBDIR_AS_LIB OFF)
if (NOT TAILORFS_LIBDIR_AS_LIB)
    include(GNUInstallDirs)
endif ()
#
if (CMAKE_INSTALL_LIBDIR)
   message(STATUS "CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR}")
   set(TAILORFS_LIBDIR ${CMAKE_INSTALL_LIBDIR})
   set(TAILORFS_INSTALL_INCLUDE_DIR
       ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
   set(TAILORFS_INSTALL_DOCDIR
       ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DOCDIR})
   set(TAILORFS_INSTALL_SYSCONFDIR
       ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/modulefiles)
else ()
   set(TAILORFS_LIBDIR "lib")
   set(TAILORFS_INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include")
   set(TAILORFS_INSTALL_DOCDIR "${CMAKE_INSTALL_PREFIX}/doc")
   set(TAILORFS_INSTALL_SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc/modulefiles")
   message(STATUS "TAILORFS_LIBDIR set to ${TAILORFS_LIBDIR}")
endif ()

#------------------------------------------------------------------------------
# Internal Paths for cmake libraries and Setup install and output Directories
#------------------------------------------------------------------------------
# This sets where to look for dependent libraries
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_BINARY_DIR} ${CMAKE_INSTALL_PREFIX})
# This sets where to look for dependent library's cmake files
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/${TAILORFS_LIBDIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/share/cmake)

#------------------------------------------------------------------------------
if (NOT TAILORFS_EXTERNALLY_CONFIGURED)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "Single Directory for all Executables.")
    set(CMAKE_INCLUDE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/include CACHE PATH "Store the headers.")
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${TAILORFS_LIBDIR} CACHE PATH "Single Directory for all Libraries")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${TAILORFS_LIBDIR} CACHE PATH "Single Directory for all static libraries.")
endif ()

include_directories(${CMAKE_BINARY_DIR}/include)
link_directories(${CMAKE_BINARY_DIR}/${TAILORFS_LIBDIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#-----------------------------------------------------------------------------
# Targets built within this project are exported at Install time for use
# by other projects.
#-----------------------------------------------------------------------------
if (NOT TAILORFS_EXPORTED_TARGETS)
    set(TAILORFS_EXPORTED_TARGETS "tailorfs-targets")
endif ()
#------------------------------------------------------------------------------
# Build options
#------------------------------------------------------------------------------
# option(ENABLE_TAILORFS_DEBUG "Include debugging prints and logging" OFF) # This is not needed as we have CMAKE_BUILD_TYPE
# This is verbose, maybe an alternate might help simplify
#option(BUILD_URPC "Build TAILORFS's URPC code" OFF)
#option(ENABLE_PERFFLOW "Build with PerfFlow Aspect support" OFF)
#option(ENABLE_UCX_DTL "Build TAILORFS's UCX data transport layer" OFF)

# This option is off by default because non-developers should not use
# this option under normal circumstances.
option(TAILORFS_WARNINGS_AS_ERRORS "Build with warnings promoted to errors." OFF)
mark_as_advanced(TAILORFS_WARNINGS_AS_ERRORS)

option (TAILORFS_USE_CLANG_LIBCXX OFF)
if (TAILORFS_USE_CLANG_LIBCXX)
    set (GCC_TOOLCHAIN "")
endif (TAILORFS_USE_CLANG_LIBCXX)

# Testing
option (TAILORFS_ENABLE_TESTING "Enable tests for TAILORFS." OFF)

#------------------------------------------------------------------------------
# Compiler setup
#------------------------------------------------------------------------------
include(ExternalProject)
# It requires include(GNUInstallDirs) before if that is needed
include(SetupCompiler)

#------------------------------------------------------------------------------
# Build Type based configurations
#------------------------------------------------------------------------------

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release") # Setting default as Release
endif()

if (NOT DEFINED BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ON)
endif ()
#------------------------------------------------------------------------------
# Disallow in-source build
#------------------------------------------------------------------------------
if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR
            "${PROJECT_NAME} requires an out of source Build. "
            "Please create a separate binary directory and run CMake there.")
endif ()
#------------------------------------------------------------------------------
# Set a default build type if none was specified
#------------------------------------------------------------------------------
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
            "MinSizeRel" "RelWithDebInfo")
endif ()
#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

# Required Dependencies
# =============================================================================
set(DEPENDENCY_LIB "")
find_package(mimir REQUIRED)
if (${MIMIR_FOUND})
    message(STATUS "[MIMIR] found mimir at ${MIMIR_INCLUDE_DIRS}")
    include_directories(${MIMIR_INCLUDE_DIRS})
    set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${MIMIR_LIBRARIES})
else ()
    message(FATAL_ERROR "-- [MIMIR] is needed for ${PROJECT_NAME} build")
endif ()

find_package(cpp-logger REQUIRED)
if (${cpp-logger_FOUND})
    include_directories(${CPP_LOGGER_INCLUDE_DIRS})
    set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${CPP_LOGGER_LIBRARIES})
    message(STATUS "[CPP_LOGGER] found cpp-logger at ${CPP_LOGGER_INCLUDE_DIRS}")
else ()
    message(FATAL_ERROR "-- [CPP_LOGGER] is needed for ${PROJECT_NAME} build")
endif ()
#message(STATUS "DEPENDENCY_LIB ${DEPENDENCY_LIB}")
find_package(brahma REQUIRED)
if (${brahma_FOUND})
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
    message(STATUS "[UnifyFS] found headers at ${UNIFYFS_INCLUDE_DIRS}")
    include_directories(${UNIFYFS_INCLUDE_DIRS})
    set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${UNIFYFS_API_LIBRARIES})
endif ()
find_package(MPI COMPONENTS CXX REQUIRED)
if (MPI_FOUND)
    message(STATUS "[MPI] found mpi.h at ${MPI_CXX_INCLUDE_DIRS}")
    include_directories(${MPI_CXX_INCLUDE_DIRS})
    set(DEPENDENCY_LIB ${DEPENDENCY_LIB} ${MPI_CXX_LIBRARIES})
endif ()
# Optional Dependencies
# =============================================================================


function(tailorfs_install_headers public_headers current_dir)
    message("-- [${PROJECT_NAME}] " "installing headers ${public_headers}")
    foreach (header ${public_headers})
        file(RELATIVE_PATH header_file_path "${PROJECT_SOURCE_DIR}/src" "${header}")
        message("-- [${PROJECT_NAME}] " "installing header ${header_file_path}")
        get_filename_component(header_directory_path "${header_file_path}" DIRECTORY)
        install(
                FILES ${header}
                DESTINATION "include/${header_directory_path}"
        )
        file(COPY ${header}
                DESTINATION "${CMAKE_INCLUDE_OUTPUT_DIRECTORY}/${header_directory_path}")
    endforeach ()
endfunction()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)  # public header
set(TAILORFS_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tailorfs/brahma/posix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tailorfs/brahma/stdio.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tailorfs/brahma/mpiio.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tailorfs/brahma/mpi.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tailorfs/core/fsview_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tailorfs/core/process_state.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tailorfs/core/fsview/stdio_fsview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tailorfs/core/fsview/mpiio_fsview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tailorfs/core/fsview/posix_fsview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ src/tailorfs/core/fsview/unifyfs_fsview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ src/tailorfs/util/unifyfs-stage-transfer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tailorfs/tailorfs.cpp)
set(TAILORFS_PRIVATE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/src/tailorfs/brahma/posix.h ${CMAKE_CURRENT_SOURCE_DIR}/src/tailorfs/brahma/stdio.h)
set(TAILORFS_PUBLIC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/tailorfs/tailorfs.h ${CMAKE_CURRENT_SOURCE_DIR}/include/tailorfs/macro.h)

add_library(${PROJECT_NAME} SHARED ${TAILORFS_SRC} ${TAILORFS_PRIVATE_INCLUDE} ${TAILORFS_PUBLIC_INCLUDE})
target_link_libraries(${PROJECT_NAME} ${DEPENDENCY_LIB})
tailorfs_install_headers(${TAILORFS_PUBLIC_INCLUDE})
install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${TAILORFS_EXPORTED_TARGETS}
        LIBRARY DESTINATION ${TAILORFS_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${TAILORFS_INSTALL_LIB_DIR}
        RUNTIME DESTINATION ${TAILORFS_INSTALL_BIN_DIR}
)
#cmake_policy(SET CMP0079 NEW) # In case that we need more control over the target building order

if(TAILORFS_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

#-----------------------------------------------------------------------------
# Configure the config.cmake file for the build directory
#-----------------------------------------------------------------------------
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure_files/${PROJECT_NAME}-config.cmake.build.in
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/${PROJECT_NAME}-config.cmake @ONLY
)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure_files/${PROJECT_NAME}-config.cmake.install.in
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/install/${PROJECT_NAME}-config.cmake @ONLY
)
install(
        FILES
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/install/${PROJECT_NAME}-config.cmake
        DESTINATION
        ${TAILORFS_LIBDIR}/cmake/${PROJECT_NAME}
)
#-----------------------------------------------------------------------------
# Configure the ${PROJECT_NAME}-config-version .cmake file for the install directory
#-----------------------------------------------------------------------------
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/configure_files/${PROJECT_NAME}-config-version.cmake.in
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/${PROJECT_NAME}-config-version.cmake @ONLY
)

install(
        FILES
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/${PROJECT_NAME}-config-version.cmake
        DESTINATION
        ${TAILORFS_LIBDIR}/cmake/${PROJECT_NAME}
)

export(EXPORT ${TAILORFS_EXPORTED_TARGETS}
        FILE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cmake/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake")

# Write the configure file
configure_file("${CMAKE_SOURCE_DIR}/cmake/configure_files/tailorfs_config.hpp.in"
               "${CMAKE_INCLUDE_OUTPUT_DIRECTORY}/tailorfs/tailorfs_config.hpp" @ONLY)


################################################################
# Install TAILORFS
################################################################

include(CMakePackageConfigHelpers)

# Write the version file. This is independent of build/install tree.
write_basic_package_version_file(
  TAILORFSConfigVersion.cmake
  VERSION "${TAILORFS_PACKAGE_VERSION}"
  COMPATIBILITY SameMajorVersion)

install(FILES
        "${CMAKE_INCLUDE_OUTPUT_DIRECTORY}/tailorfs/tailorfs_config.hpp"
        DESTINATION
        ${TAILORFS_INSTALL_INCLUDE_DIR}/tailorfs)

install(EXPORT
        ${TAILORFS_EXPORTED_TARGETS}
        DESTINATION
        ${TAILORFS_LIBDIR}/cmake/${PROJECT_NAME}
        FILE
        ${TAILORFS_EXPORTED_TARGETS}.cmake
        )

# Install license and readme
install(FILES
        "${PROJECT_SOURCE_DIR}/LICENSE"
        "${PROJECT_SOURCE_DIR}/README.md"
        DESTINATION
        ${TAILORFS_INSTALL_DOCDIR})


###############################################################################
#                      Print out configuration summary
###############################################################################
include(tailorfs-utils)
if (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
  set(TAILORFS_GNU_LINUX TRUE)
endif ()

# NOTE: message() outputs to stderr by default. We now use a string to
# maintain this information and then have cmake echo it to stdout. The
# only side effects are that if you use the CMake GUI, you won't see
# this output anymore (they only report stderr) and that if you add
# something to the list, you must remember your newline!
set(_str "\n== TAILORFS Configuration Summary ==\n\n")
string(APPEND _str "  PROJECT_SOURCE_DIR:   ${PROJECT_SOURCE_DIR}\n"
  "  PROJECT_BINARY_DIR:   ${PROJECT_BINARY_DIR}\n\n"
  "  CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}\n"
  "  CMAKE_BUILD_TYPE:     ${CMAKE_BUILD_TYPE}\n\n")
if (CMAKE_BUILD_TYPE MATCHES None)
  string(APPEND _str
    "  CXX FLAGS:            ${CMAKE_CXX_FLAGS}\n")
elseif (CMAKE_BUILD_TYPE MATCHES Release)
  string(APPEND _str
    "  CXX FLAGS:            ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}\n")
  string(APPEND _str
    "  C FLAGS:              ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}\n")
elseif (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
  string(APPEND _str
    "  CXX FLAGS:            ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}\n")
  string(APPEND _str
    "  C FLAGS:              ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELWITHDEBINFO}\n")
elseif (CMAKE_BUILD_TYPE MATCHES Debug)
  string(APPEND _str
    "  CXX FLAGS:            ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}\n")
  string(APPEND _str
    "  C FLAGS:              ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}\n")
endif ()
string(APPEND _str "\n")

string(APPEND _str
  "  TAILORFS_PACKAGE_VERSION:        ${TAILORFS_PACKAGE_VERSION}\n")
string(APPEND _str
  "  TAILORFS_GIT_VERSION:            ${TAILORFS_GIT_VERSION}\n")
string(APPEND _str
  "  GLIBC_VERSION:               ${GLIBC_VERSION}\n")

#Print the true/false variables
append_str_tf(_str
  TAILORFS_GNU_LINUX
  TAILORFS_COMMUNICATION_ENABLE_UCX
  TAILORFS_RUNTIME_ENABLE_LIBUV
  TAILORFS_LIBDIR_AS_LIB
  TAILORFS_USE_CLANG_LIBCXX
  TAILORFS_WARNINGS_AS_ERRORS
  TAILORFS_ENABLE_TESTING
)
string(APPEND _str
  "\n== End TAILORFS Configuration Summary ==\n")

# Output to stdout
execute_process(COMMAND ${CMAKE_COMMAND} -E echo "${_str}")
set(_str)


###############################################################################
# Write a basic modulefile
###############################################################################

set(TAILORFS_MODULEFILE_NAME "tailorfs-${TAILORFS_PACKAGE_VERSION}.lua")
#  CACHE STRING
#  "The name of the TAILORFS modulefile to install. Must end in .lua.")

if (NOT (TAILORFS_MODULEFILE_NAME MATCHES ".+\.lua"))
  message(WARNING
    "TAILORFS_MODULEFILE_NAME must have extension \".lua\". Appending.")
  set(TAILORFS_MODULEFILE_NAME "${TAILORFS_MODULEFILE_NAME}.lua"
    CACHE STRING "" FORCE)
endif ()

configure_file(
  "${CMAKE_SOURCE_DIR}/cmake/configure_files/tailorfs_module.lua.in"
  "${CMAKE_BINARY_DIR}/tailorfs_module.lua.install"
  @ONLY)

install(FILES "${CMAKE_BINARY_DIR}/tailorfs_module.lua.install"
  RENAME "${TAILORFS_MODULEFILE_NAME}"
  DESTINATION
  "${TAILORFS_INSTALL_SYSCONFDIR}")

