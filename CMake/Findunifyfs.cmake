
set(UNIFYFS_FOUND TRUE)

if(NOT TARGET unifyfs::unifyfs)
    # Include directories
    find_path(UNIFYFS_INCLUDE_DIRS unifyfs.h PATH_SUFFIXES include/)
    if(NOT IS_DIRECTORY "${UNIFYFS_INCLUDE_DIRS}")
        set(UNIFYFS_FOUND FALSE)
    endif()
    #message(${UNIFYFS_INCLUDE_DIRS})
    find_path(UNIFYFS_LIBRARY_PATH libunifyfs.so PATH_SUFFIXES lib/)
    #message(${UNIFYFS_LIBRARY_PATH})
    set(UNIFYFS_LIBRARIES -lunifyfs)
    set(UNIFYFS_DEFINITIONS "")
    add_library(unifyfs INTERFACE)
    add_library(unifyfs::unifyfs ALIAS unifyfs)
    target_include_directories(unifyfs INTERFACE ${UNIFYFS_INCLUDE_DIRS})
    target_link_libraries(unifyfs INTERFACE -L${UNIFYFS_LIBRARY_PATH} ${UNIFYFS_LIBRARIES})
    target_compile_options(unifyfs INTERFACE ${UNIFYFS_DEFINITIONS})

    include(FindPackageHandleStandardArgs)
    # handle the QUIETLY and REQUIRED arguments and set ortools to TRUE
    # if all listed variables are TRUE
    find_package_handle_standard_args(unifyfs
            REQUIRED_VARS UNIFYFS_FOUND UNIFYFS_LIBRARIES UNIFYFS_INCLUDE_DIRS)
endif()