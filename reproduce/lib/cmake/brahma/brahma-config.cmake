set(BRAHMA_FOUND TRUE)

# Include directories
set(BRAHMA_INCLUDE_DIRS "/usr/workspace/iopp/software/tailorfs/reproduce/include")
if (NOT IS_DIRECTORY "${BRAHMA_INCLUDE_DIRS}")
    set(BRAHMA_FOUND FALSE)
endif ()
#message(STATUS "BRAHMA_INCLUDE_DIRS: " ${BRAHMA_INCLUDE_DIRS})
get_filename_component(BRAHMA_ROOT_DIR ${BRAHMA_INCLUDE_DIRS}/.. ABSOLUTE)
#message(STATUS "BRAHMA_ROOT_DIR: " ${BRAHMA_ROOT_DIR})
set(BRAHMA_LIBRARY_PATH "/usr/workspace/iopp/software/tailorfs/reproduce/lib")
#message(STATUS "BRAHMA_LIBRARY_PATH: " ${BRAHMA_LIBRARY_PATH})
set(BRAHMA_LIBRARIES "-L${BRAHMA_LIBRARY_PATH} -lbrahma")
set(BRAHMA_DEFINITIONS "")
if (NOT TARGET brahma::brahma)
    add_library(brahma::brahma ALIAS brahma)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(brahma
            REQUIRED_VARS BRAHMA_FOUND BRAHMA_INCLUDE_DIRS BRAHMA_LIBRARIES)
endif ()
