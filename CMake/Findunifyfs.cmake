
set(UNIFYFS_FOUND TRUE)

if(NOT TARGET unifyfs::unifyfs)
    # Include directories
    find_path(UNIFYFS_INCLUDE_DIRS unifyfs.h PATH_SUFFIXES include/)
    if (NOT IS_DIRECTORY "${UNIFYFS_INCLUDE_DIRS}")
        set(UNIFYFS_FOUND FALSE)
    endif ()
    message("-- UNIFYFS_INCLUDE_DIRS: " ${UNIFYFS_INCLUDE_DIRS})
    get_filename_component(UNIFYFS_ROOT_DIR ${UNIFYFS_INCLUDE_DIRS}/.. ABSOLUTE)
    message("-- UNIFYFS_ROOT_DIR: " ${UNIFYFS_ROOT_DIR})
    find_path(UNIFYFS_LIBRARY_PATH libunifyfs.so PATH_SUFFIXES lib/)
    message("-- UNIFYFS_LIBRARY_PATH: " ${UNIFYFS_LIBRARY_PATH})
    set(UNIFYFS_LIBRARIES -L${UNIFYFS_LIBRARY_PATH} -lunifyfs -lmercury -lmercury_util -lmargo -labt)
    set(UNIFYFSD_EXEC ${UNIFYFS_ROOT_DIR}/bin/unifyfsd)
    set(UNIFYFS_EXEC ${UNIFYFS_ROOT_DIR}/bin/unifyfs)
    set(UNIFYFS_API_LIBRARIES -L${UNIFYFS_LIBRARY_PATH} -lunifyfs_api -lmercury -lmercury_util -lmargo -labt)

    set(UNIFYFS_DEFINITIONS "")
    add_library(unifyfs INTERFACE)
    add_library(unifyfs_api INTERFACE)
    add_library(unifyfs::unifyfs ALIAS unifyfs)
    add_library(unifyfs::unifyfs_api ALIAS unifyfs_api)
    target_include_directories(unifyfs INTERFACE ${UNIFYFS_INCLUDE_DIRS})
    target_include_directories(unifyfs_api INTERFACE ${UNIFYFS_INCLUDE_DIRS})
    target_link_libraries(unifyfs INTERFACE ${UNIFYFS_LIBRARIES})
    target_link_libraries(unifyfs_api INTERFACE ${UNIFYFS_API_LIBRARIES})
    target_compile_options(unifyfs INTERFACE ${UNIFYFS_DEFINITIONS})

    include(FindPackageHandleStandardArgs)
    # handle the QUIETLY and REQUIRED arguments and set ortools to TRUE
    # if all listed variables are TRUE
    find_package_handle_standard_args(unifyfs
            REQUIRED_VARS UNIFYFS_FOUND UNIFYFS_LIBRARIES UNIFYFS_EXEC UNIFYFSD_EXEC UNIFYFS_API_LIBRARIES UNIFYFS_INCLUDE_DIRS)
endif()