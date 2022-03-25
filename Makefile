# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ubuntu/Projects/plot-cv

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/Projects/plot-cv

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target install/strip
install/strip: preinstall
	@echo "Installing the project stripped..."
	/usr/bin/cmake -DCMAKE_INSTALL_DO_STRIP=1 -P cmake_install.cmake
.PHONY : install/strip

# Special rule for the target install/strip
install/strip/fast: preinstall/fast
	@echo "Installing the project stripped..."
	/usr/bin/cmake -DCMAKE_INSTALL_DO_STRIP=1 -P cmake_install.cmake
.PHONY : install/strip/fast

# Special rule for the target install/local
install/local: preinstall
	@echo "Installing only the local directory..."
	/usr/bin/cmake -DCMAKE_INSTALL_LOCAL_ONLY=1 -P cmake_install.cmake
.PHONY : install/local

# Special rule for the target install/local
install/local/fast: preinstall/fast
	@echo "Installing only the local directory..."
	/usr/bin/cmake -DCMAKE_INSTALL_LOCAL_ONLY=1 -P cmake_install.cmake
.PHONY : install/local/fast

# Special rule for the target install
install: preinstall
	@echo "Install the project..."
	/usr/bin/cmake -P cmake_install.cmake
.PHONY : install

# Special rule for the target install
install/fast: preinstall/fast
	@echo "Install the project..."
	/usr/bin/cmake -P cmake_install.cmake
.PHONY : install/fast

# Special rule for the target list_install_components
list_install_components:
	@echo "Available install components are: \"Unspecified\""
.PHONY : list_install_components

# Special rule for the target list_install_components
list_install_components/fast: list_install_components

.PHONY : list_install_components/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@echo "Running CMake to regenerate build system..."
	/usr/bin/cmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@echo "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	cd /home/ubuntu/Projects/plot-cv && $(CMAKE_COMMAND) -E cmake_progress_start /home/ubuntu/Projects/plot-cv/CMakeFiles /home/ubuntu/Projects/plot-cv/quickjs/CMakeFiles/progress.marks
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f CMakeFiles/Makefile2 quickjs/all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/ubuntu/Projects/plot-cv/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f CMakeFiles/Makefile2 quickjs/clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f CMakeFiles/Makefile2 quickjs/preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f CMakeFiles/Makefile2 quickjs/preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	cd /home/ubuntu/Projects/plot-cv && $(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

# Convenience name for target.
quickjs/CMakeFiles/.ctags.dir/rule:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f CMakeFiles/Makefile2 quickjs/CMakeFiles/.ctags.dir/rule
.PHONY : quickjs/CMakeFiles/.ctags.dir/rule

# Convenience name for target.
.ctags: quickjs/CMakeFiles/.ctags.dir/rule

.PHONY : .ctags

# fast build rule for target.
.ctags/fast:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/.ctags.dir/build.make quickjs/CMakeFiles/.ctags.dir/build
.PHONY : .ctags/fast

# Convenience name for target.
quickjs/CMakeFiles/qjsc.dir/rule:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f CMakeFiles/Makefile2 quickjs/CMakeFiles/qjsc.dir/rule
.PHONY : quickjs/CMakeFiles/qjsc.dir/rule

# Convenience name for target.
qjsc: quickjs/CMakeFiles/qjsc.dir/rule

.PHONY : qjsc

# fast build rule for target.
qjsc/fast:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/build
.PHONY : qjsc/fast

# Convenience name for target.
quickjs/CMakeFiles/quickjs.dir/rule:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f CMakeFiles/Makefile2 quickjs/CMakeFiles/quickjs.dir/rule
.PHONY : quickjs/CMakeFiles/quickjs.dir/rule

# Convenience name for target.
quickjs: quickjs/CMakeFiles/quickjs.dir/rule

.PHONY : quickjs

# fast build rule for target.
quickjs/fast:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/build
.PHONY : quickjs/fast

# Convenience name for target.
quickjs/CMakeFiles/qjs.dir/rule:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f CMakeFiles/Makefile2 quickjs/CMakeFiles/qjs.dir/rule
.PHONY : quickjs/CMakeFiles/qjs.dir/rule

# Convenience name for target.
qjs: quickjs/CMakeFiles/qjs.dir/rule

.PHONY : qjs

# fast build rule for target.
qjs/fast:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/build
.PHONY : qjs/fast

# Convenience name for target.
quickjs/CMakeFiles/quickjs-static.dir/rule:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f CMakeFiles/Makefile2 quickjs/CMakeFiles/quickjs-static.dir/rule
.PHONY : quickjs/CMakeFiles/quickjs-static.dir/rule

# Convenience name for target.
quickjs-static: quickjs/CMakeFiles/quickjs-static.dir/rule

.PHONY : quickjs-static

# fast build rule for target.
quickjs-static/fast:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/build
.PHONY : quickjs-static/fast

# Convenience name for target.
quickjs/CMakeFiles/hello.c.dir/rule:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f CMakeFiles/Makefile2 quickjs/CMakeFiles/hello.c.dir/rule
.PHONY : quickjs/CMakeFiles/hello.c.dir/rule

# Convenience name for target.
hello.c: quickjs/CMakeFiles/hello.c.dir/rule

.PHONY : hello.c

# fast build rule for target.
hello.c/fast:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/hello.c.dir/build.make quickjs/CMakeFiles/hello.c.dir/build
.PHONY : hello.c/fast

cutils.o: cutils.c.o

.PHONY : cutils.o

# target to build an object file
cutils.c.o:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/cutils.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/cutils.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/cutils.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/cutils.c.o
.PHONY : cutils.c.o

cutils.i: cutils.c.i

.PHONY : cutils.i

# target to preprocess a source file
cutils.c.i:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/cutils.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/cutils.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/cutils.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/cutils.c.i
.PHONY : cutils.c.i

cutils.s: cutils.c.s

.PHONY : cutils.s

# target to generate assembly for a file
cutils.c.s:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/cutils.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/cutils.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/cutils.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/cutils.c.s
.PHONY : cutils.c.s

libbf.o: libbf.c.o

.PHONY : libbf.o

# target to build an object file
libbf.c.o:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/libbf.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/libbf.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/libbf.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/libbf.c.o
.PHONY : libbf.c.o

libbf.i: libbf.c.i

.PHONY : libbf.i

# target to preprocess a source file
libbf.c.i:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/libbf.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/libbf.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/libbf.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/libbf.c.i
.PHONY : libbf.c.i

libbf.s: libbf.c.s

.PHONY : libbf.s

# target to generate assembly for a file
libbf.c.s:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/libbf.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/libbf.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/libbf.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/libbf.c.s
.PHONY : libbf.c.s

libregexp.o: libregexp.c.o

.PHONY : libregexp.o

# target to build an object file
libregexp.c.o:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/libregexp.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/libregexp.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/libregexp.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/libregexp.c.o
.PHONY : libregexp.c.o

libregexp.i: libregexp.c.i

.PHONY : libregexp.i

# target to preprocess a source file
libregexp.c.i:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/libregexp.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/libregexp.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/libregexp.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/libregexp.c.i
.PHONY : libregexp.c.i

libregexp.s: libregexp.c.s

.PHONY : libregexp.s

# target to generate assembly for a file
libregexp.c.s:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/libregexp.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/libregexp.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/libregexp.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/libregexp.c.s
.PHONY : libregexp.c.s

libunicode.o: libunicode.c.o

.PHONY : libunicode.o

# target to build an object file
libunicode.c.o:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/libunicode.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/libunicode.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/libunicode.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/libunicode.c.o
.PHONY : libunicode.c.o

libunicode.i: libunicode.c.i

.PHONY : libunicode.i

# target to preprocess a source file
libunicode.c.i:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/libunicode.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/libunicode.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/libunicode.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/libunicode.c.i
.PHONY : libunicode.c.i

libunicode.s: libunicode.c.s

.PHONY : libunicode.s

# target to generate assembly for a file
libunicode.c.s:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/libunicode.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/libunicode.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/libunicode.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/libunicode.c.s
.PHONY : libunicode.c.s

qjs.o: qjs.c.o

.PHONY : qjs.o

# target to build an object file
qjs.c.o:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/qjs.c.o
.PHONY : qjs.c.o

qjs.i: qjs.c.i

.PHONY : qjs.i

# target to preprocess a source file
qjs.c.i:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/qjs.c.i
.PHONY : qjs.c.i

qjs.s: qjs.c.s

.PHONY : qjs.s

# target to generate assembly for a file
qjs.c.s:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/qjs.c.s
.PHONY : qjs.c.s

qjsc.o: qjsc.c.o

.PHONY : qjsc.o

# target to build an object file
qjsc.c.o:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/qjsc.c.o
.PHONY : qjsc.c.o

qjsc.i: qjsc.c.i

.PHONY : qjsc.i

# target to preprocess a source file
qjsc.c.i:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/qjsc.c.i
.PHONY : qjsc.c.i

qjsc.s: qjsc.c.s

.PHONY : qjsc.s

# target to generate assembly for a file
qjsc.c.s:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/qjsc.c.s
.PHONY : qjsc.c.s

qjscalc.o: qjscalc.c.o

.PHONY : qjscalc.o

# target to build an object file
qjscalc.c.o:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/qjscalc.c.o
.PHONY : qjscalc.c.o

qjscalc.i: qjscalc.c.i

.PHONY : qjscalc.i

# target to preprocess a source file
qjscalc.c.i:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/qjscalc.c.i
.PHONY : qjscalc.c.i

qjscalc.s: qjscalc.c.s

.PHONY : qjscalc.s

# target to generate assembly for a file
qjscalc.c.s:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/qjscalc.c.s
.PHONY : qjscalc.c.s

quickjs-debugger-transport-unix.o: quickjs-debugger-transport-unix.c.o

.PHONY : quickjs-debugger-transport-unix.o

# target to build an object file
quickjs-debugger-transport-unix.c.o:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs-debugger-transport-unix.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs-debugger-transport-unix.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs-debugger-transport-unix.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs-debugger-transport-unix.c.o
.PHONY : quickjs-debugger-transport-unix.c.o

quickjs-debugger-transport-unix.i: quickjs-debugger-transport-unix.c.i

.PHONY : quickjs-debugger-transport-unix.i

# target to preprocess a source file
quickjs-debugger-transport-unix.c.i:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs-debugger-transport-unix.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs-debugger-transport-unix.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs-debugger-transport-unix.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs-debugger-transport-unix.c.i
.PHONY : quickjs-debugger-transport-unix.c.i

quickjs-debugger-transport-unix.s: quickjs-debugger-transport-unix.c.s

.PHONY : quickjs-debugger-transport-unix.s

# target to generate assembly for a file
quickjs-debugger-transport-unix.c.s:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs-debugger-transport-unix.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs-debugger-transport-unix.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs-debugger-transport-unix.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs-debugger-transport-unix.c.s
.PHONY : quickjs-debugger-transport-unix.c.s

quickjs-debugger.o: quickjs-debugger.c.o

.PHONY : quickjs-debugger.o

# target to build an object file
quickjs-debugger.c.o:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs-debugger.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs-debugger.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs-debugger.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs-debugger.c.o
.PHONY : quickjs-debugger.c.o

quickjs-debugger.i: quickjs-debugger.c.i

.PHONY : quickjs-debugger.i

# target to preprocess a source file
quickjs-debugger.c.i:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs-debugger.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs-debugger.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs-debugger.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs-debugger.c.i
.PHONY : quickjs-debugger.c.i

quickjs-debugger.s: quickjs-debugger.c.s

.PHONY : quickjs-debugger.s

# target to generate assembly for a file
quickjs-debugger.c.s:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs-debugger.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs-debugger.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs-debugger.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs-debugger.c.s
.PHONY : quickjs-debugger.c.s

quickjs-find-module.o: quickjs-find-module.c.o

.PHONY : quickjs-find-module.o

# target to build an object file
quickjs-find-module.c.o:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs-find-module.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs-find-module.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs-find-module.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs-find-module.c.o
.PHONY : quickjs-find-module.c.o

quickjs-find-module.i: quickjs-find-module.c.i

.PHONY : quickjs-find-module.i

# target to preprocess a source file
quickjs-find-module.c.i:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs-find-module.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs-find-module.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs-find-module.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs-find-module.c.i
.PHONY : quickjs-find-module.c.i

quickjs-find-module.s: quickjs-find-module.c.s

.PHONY : quickjs-find-module.s

# target to generate assembly for a file
quickjs-find-module.c.s:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs-find-module.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs-find-module.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs-find-module.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs-find-module.c.s
.PHONY : quickjs-find-module.c.s

quickjs-libc.o: quickjs-libc.c.o

.PHONY : quickjs-libc.o

# target to build an object file
quickjs-libc.c.o:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs-libc.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs-libc.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs-libc.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs-libc.c.o
.PHONY : quickjs-libc.c.o

quickjs-libc.i: quickjs-libc.c.i

.PHONY : quickjs-libc.i

# target to preprocess a source file
quickjs-libc.c.i:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs-libc.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs-libc.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs-libc.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs-libc.c.i
.PHONY : quickjs-libc.c.i

quickjs-libc.s: quickjs-libc.c.s

.PHONY : quickjs-libc.s

# target to generate assembly for a file
quickjs-libc.c.s:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs-libc.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs-libc.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs-libc.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs-libc.c.s
.PHONY : quickjs-libc.c.s

quickjs.o: quickjs.c.o

.PHONY : quickjs.o

# target to build an object file
quickjs.c.o:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs.c.o
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs.c.o
.PHONY : quickjs.c.o

quickjs.i: quickjs.c.i

.PHONY : quickjs.i

# target to preprocess a source file
quickjs.c.i:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs.c.i
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs.c.i
.PHONY : quickjs.c.i

quickjs.s: quickjs.c.s

.PHONY : quickjs.s

# target to generate assembly for a file
quickjs.c.s:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjsc.dir/build.make quickjs/CMakeFiles/qjsc.dir/quickjs.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs.dir/build.make quickjs/CMakeFiles/quickjs.dir/quickjs.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/quickjs.c.s
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/quickjs-static.dir/build.make quickjs/CMakeFiles/quickjs-static.dir/quickjs.c.s
.PHONY : quickjs.c.s

repl.o: repl.c.o

.PHONY : repl.o

# target to build an object file
repl.c.o:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/repl.c.o
.PHONY : repl.c.o

repl.i: repl.c.i

.PHONY : repl.i

# target to preprocess a source file
repl.c.i:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/repl.c.i
.PHONY : repl.c.i

repl.s: repl.c.s

.PHONY : repl.s

# target to generate assembly for a file
repl.c.s:
	cd /home/ubuntu/Projects/plot-cv && $(MAKE) -f quickjs/CMakeFiles/qjs.dir/build.make quickjs/CMakeFiles/qjs.dir/repl.c.s
.PHONY : repl.c.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... install/strip"
	@echo "... install/local"
	@echo "... install"
	@echo "... list_install_components"
	@echo "... rebuild_cache"
	@echo "... edit_cache"
	@echo "... .ctags"
	@echo "... qjsc"
	@echo "... quickjs"
	@echo "... qjs"
	@echo "... quickjs-static"
	@echo "... hello.c"
	@echo "... cutils.o"
	@echo "... cutils.i"
	@echo "... cutils.s"
	@echo "... libbf.o"
	@echo "... libbf.i"
	@echo "... libbf.s"
	@echo "... libregexp.o"
	@echo "... libregexp.i"
	@echo "... libregexp.s"
	@echo "... libunicode.o"
	@echo "... libunicode.i"
	@echo "... libunicode.s"
	@echo "... qjs.o"
	@echo "... qjs.i"
	@echo "... qjs.s"
	@echo "... qjsc.o"
	@echo "... qjsc.i"
	@echo "... qjsc.s"
	@echo "... qjscalc.o"
	@echo "... qjscalc.i"
	@echo "... qjscalc.s"
	@echo "... quickjs-debugger-transport-unix.o"
	@echo "... quickjs-debugger-transport-unix.i"
	@echo "... quickjs-debugger-transport-unix.s"
	@echo "... quickjs-debugger.o"
	@echo "... quickjs-debugger.i"
	@echo "... quickjs-debugger.s"
	@echo "... quickjs-find-module.o"
	@echo "... quickjs-find-module.i"
	@echo "... quickjs-find-module.s"
	@echo "... quickjs-libc.o"
	@echo "... quickjs-libc.i"
	@echo "... quickjs-libc.s"
	@echo "... quickjs.o"
	@echo "... quickjs.i"
	@echo "... quickjs.s"
	@echo "... repl.o"
	@echo "... repl.i"
	@echo "... repl.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	cd /home/ubuntu/Projects/plot-cv && $(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

