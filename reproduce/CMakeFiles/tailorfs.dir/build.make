# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.21

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/tce/packages/cmake/cmake-3.21.1/bin/cmake

# The command to remove a file.
RM = /usr/tce/packages/cmake/cmake-3.21.1/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /usr/workspace/iopp/software/tailorfs

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /usr/workspace/iopp/software/tailorfs/reproduce

# Include any dependencies generated for this target.
include CMakeFiles/tailorfs.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/tailorfs.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/tailorfs.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/tailorfs.dir/flags.make

CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.o: CMakeFiles/tailorfs.dir/flags.make
CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.o: ../src/tailorfs/brahma/posix.cpp
CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.o: CMakeFiles/tailorfs.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.o"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.o -MF CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.o.d -o CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.o -c /usr/workspace/iopp/software/tailorfs/src/tailorfs/brahma/posix.cpp

CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.i"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/workspace/iopp/software/tailorfs/src/tailorfs/brahma/posix.cpp > CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.i

CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.s"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/workspace/iopp/software/tailorfs/src/tailorfs/brahma/posix.cpp -o CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.s

CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.o: CMakeFiles/tailorfs.dir/flags.make
CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.o: ../src/tailorfs/brahma/stdio.cpp
CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.o: CMakeFiles/tailorfs.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.o"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.o -MF CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.o.d -o CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.o -c /usr/workspace/iopp/software/tailorfs/src/tailorfs/brahma/stdio.cpp

CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.i"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/workspace/iopp/software/tailorfs/src/tailorfs/brahma/stdio.cpp > CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.i

CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.s"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/workspace/iopp/software/tailorfs/src/tailorfs/brahma/stdio.cpp -o CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.s

CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.o: CMakeFiles/tailorfs.dir/flags.make
CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.o: ../src/tailorfs/brahma/mpiio.cpp
CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.o: CMakeFiles/tailorfs.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.o"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.o -MF CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.o.d -o CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.o -c /usr/workspace/iopp/software/tailorfs/src/tailorfs/brahma/mpiio.cpp

CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.i"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/workspace/iopp/software/tailorfs/src/tailorfs/brahma/mpiio.cpp > CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.i

CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.s"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/workspace/iopp/software/tailorfs/src/tailorfs/brahma/mpiio.cpp -o CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.s

CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.o: CMakeFiles/tailorfs.dir/flags.make
CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.o: ../src/tailorfs/brahma/mpi.cpp
CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.o: CMakeFiles/tailorfs.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.o"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.o -MF CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.o.d -o CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.o -c /usr/workspace/iopp/software/tailorfs/src/tailorfs/brahma/mpi.cpp

CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.i"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/workspace/iopp/software/tailorfs/src/tailorfs/brahma/mpi.cpp > CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.i

CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.s"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/workspace/iopp/software/tailorfs/src/tailorfs/brahma/mpi.cpp -o CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.s

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.o: CMakeFiles/tailorfs.dir/flags.make
CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.o: ../src/tailorfs/core/fsview_manager.cpp
CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.o: CMakeFiles/tailorfs.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.o"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.o -MF CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.o.d -o CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.o -c /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview_manager.cpp

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.i"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview_manager.cpp > CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.i

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.s"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview_manager.cpp -o CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.s

CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.o: CMakeFiles/tailorfs.dir/flags.make
CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.o: ../src/tailorfs/core/process_state.cpp
CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.o: CMakeFiles/tailorfs.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.o"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.o -MF CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.o.d -o CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.o -c /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/process_state.cpp

CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.i"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/process_state.cpp > CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.i

CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.s"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/process_state.cpp -o CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.s

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.o: CMakeFiles/tailorfs.dir/flags.make
CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.o: ../src/tailorfs/core/fsview/stdio_fsview.cpp
CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.o: CMakeFiles/tailorfs.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.o"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.o -MF CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.o.d -o CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.o -c /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview/stdio_fsview.cpp

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.i"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview/stdio_fsview.cpp > CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.i

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.s"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview/stdio_fsview.cpp -o CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.s

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.o: CMakeFiles/tailorfs.dir/flags.make
CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.o: ../src/tailorfs/core/fsview/mpiio_fsview.cpp
CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.o: CMakeFiles/tailorfs.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.o"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.o -MF CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.o.d -o CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.o -c /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview/mpiio_fsview.cpp

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.i"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview/mpiio_fsview.cpp > CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.i

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.s"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview/mpiio_fsview.cpp -o CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.s

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.o: CMakeFiles/tailorfs.dir/flags.make
CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.o: ../src/tailorfs/core/fsview/posix_fsview.cpp
CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.o: CMakeFiles/tailorfs.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.o"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.o -MF CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.o.d -o CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.o -c /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview/posix_fsview.cpp

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.i"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview/posix_fsview.cpp > CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.i

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.s"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview/posix_fsview.cpp -o CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.s

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.o: CMakeFiles/tailorfs.dir/flags.make
CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.o: ../src/tailorfs/core/fsview/unifyfs_fsview.cpp
CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.o: CMakeFiles/tailorfs.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.o"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.o -MF CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.o.d -o CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.o -c /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview/unifyfs_fsview.cpp

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.i"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview/unifyfs_fsview.cpp > CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.i

CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.s"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/workspace/iopp/software/tailorfs/src/tailorfs/core/fsview/unifyfs_fsview.cpp -o CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.s

CMakeFiles/tailorfs.dir/main.cpp.o: CMakeFiles/tailorfs.dir/flags.make
CMakeFiles/tailorfs.dir/main.cpp.o: ../main.cpp
CMakeFiles/tailorfs.dir/main.cpp.o: CMakeFiles/tailorfs.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object CMakeFiles/tailorfs.dir/main.cpp.o"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tailorfs.dir/main.cpp.o -MF CMakeFiles/tailorfs.dir/main.cpp.o.d -o CMakeFiles/tailorfs.dir/main.cpp.o -c /usr/workspace/iopp/software/tailorfs/main.cpp

CMakeFiles/tailorfs.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tailorfs.dir/main.cpp.i"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/workspace/iopp/software/tailorfs/main.cpp > CMakeFiles/tailorfs.dir/main.cpp.i

CMakeFiles/tailorfs.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tailorfs.dir/main.cpp.s"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/workspace/iopp/software/tailorfs/main.cpp -o CMakeFiles/tailorfs.dir/main.cpp.s

CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.o: CMakeFiles/tailorfs.dir/flags.make
CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.o: ../src/tailorfs/util/unifyfs-stage-transfer.cpp
CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.o: CMakeFiles/tailorfs.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building CXX object CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.o"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.o -MF CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.o.d -o CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.o -c /usr/workspace/iopp/software/tailorfs/src/tailorfs/util/unifyfs-stage-transfer.cpp

CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.i"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/workspace/iopp/software/tailorfs/src/tailorfs/util/unifyfs-stage-transfer.cpp > CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.i

CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.s"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/workspace/iopp/software/tailorfs/src/tailorfs/util/unifyfs-stage-transfer.cpp -o CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.s

CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.o: CMakeFiles/tailorfs.dir/flags.make
CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.o: ../src/tailorfs/tailorfs.cpp
CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.o: CMakeFiles/tailorfs.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building CXX object CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.o"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.o -MF CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.o.d -o CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.o -c /usr/workspace/iopp/software/tailorfs/src/tailorfs/tailorfs.cpp

CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.i"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/workspace/iopp/software/tailorfs/src/tailorfs/tailorfs.cpp > CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.i

CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.s"
	/usr/tce/packages/gcc/gcc-8.3.1/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/workspace/iopp/software/tailorfs/src/tailorfs/tailorfs.cpp -o CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.s

# Object files for target tailorfs
tailorfs_OBJECTS = \
"CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.o" \
"CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.o" \
"CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.o" \
"CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.o" \
"CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.o" \
"CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.o" \
"CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.o" \
"CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.o" \
"CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.o" \
"CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.o" \
"CMakeFiles/tailorfs.dir/main.cpp.o" \
"CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.o" \
"CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.o"

# External object files for target tailorfs
tailorfs_EXTERNAL_OBJECTS =

lib/libtailorfs.so: CMakeFiles/tailorfs.dir/src/tailorfs/brahma/posix.cpp.o
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/src/tailorfs/brahma/stdio.cpp.o
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpiio.cpp.o
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/src/tailorfs/brahma/mpi.cpp.o
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview_manager.cpp.o
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/src/tailorfs/core/process_state.cpp.o
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/stdio_fsview.cpp.o
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/mpiio_fsview.cpp.o
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/posix_fsview.cpp.o
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/src/tailorfs/core/fsview/unifyfs_fsview.cpp.o
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/main.cpp.o
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/src/tailorfs/util/unifyfs-stage-transfer.cpp.o
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/src/tailorfs/tailorfs.cpp.o
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/build.make
lib/libtailorfs.so: /usr/tce/packages/spectrum-mpi/ibm/spectrum-mpi-2020.08.19/lib/libmpiprofilesupport.so
lib/libtailorfs.so: /usr/tce/packages/spectrum-mpi/ibm/spectrum-mpi-2020.08.19/lib/libmpi_ibm.so
lib/libtailorfs.so: CMakeFiles/tailorfs.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Linking CXX shared library lib/libtailorfs.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tailorfs.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/tailorfs.dir/build: lib/libtailorfs.so
.PHONY : CMakeFiles/tailorfs.dir/build

CMakeFiles/tailorfs.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/tailorfs.dir/cmake_clean.cmake
.PHONY : CMakeFiles/tailorfs.dir/clean

CMakeFiles/tailorfs.dir/depend:
	cd /usr/workspace/iopp/software/tailorfs/reproduce && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/workspace/iopp/software/tailorfs /usr/workspace/iopp/software/tailorfs /usr/workspace/iopp/software/tailorfs/reproduce /usr/workspace/iopp/software/tailorfs/reproduce /usr/workspace/iopp/software/tailorfs/reproduce/CMakeFiles/tailorfs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/tailorfs.dir/depend
