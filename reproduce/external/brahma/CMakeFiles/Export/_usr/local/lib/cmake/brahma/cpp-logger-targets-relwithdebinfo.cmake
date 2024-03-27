#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cpp-logger" for configuration "RelWithDebInfo"
set_property(TARGET cpp-logger APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(cpp-logger PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "/usr/local/lib/libcpp-logger.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libcpp-logger.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS cpp-logger )
list(APPEND _IMPORT_CHECK_FILES_FOR_cpp-logger "/usr/local/lib/libcpp-logger.so" )

# Import target "brahma" for configuration "RelWithDebInfo"
set_property(TARGET brahma APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(brahma PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "/usr/local/lib/libbrahma.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libbrahma.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS brahma )
list(APPEND _IMPORT_CHECK_FILES_FOR_brahma "/usr/local/lib/libbrahma.so" )

# Import target "mimir" for configuration "RelWithDebInfo"
set_property(TARGET mimir APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(mimir PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "/usr/local/lib/libmimir.so"
  IMPORTED_SONAME_RELWITHDEBINFO "libmimir.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS mimir )
list(APPEND _IMPORT_CHECK_FILES_FOR_mimir "/usr/local/lib/libmimir.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
