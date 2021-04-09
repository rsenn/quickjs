# Copyright (c) 2020 Kosaki Mezumona This cmake script is distributed under the MIT License. See
# LICENSE for more details.
cmake_minimum_required(VERSION 3.1)
project(quickjs)

set(DISABLE_WERROR TRUE)

if(WIN32 OR MINGW)
  set(LIBWS2_32 ws2_32)
  set(BUILD_SHARED_LIBS ON)
endif(WIN32 OR MINGW)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)

include(${CMAKE_CURRENT_SOURCE_DIR}/defs.cmake)
include(FindThreads)

include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/check-flags.cmake)

check_library_exists(pthread pthread_create /usr/lib HAVE_LIBPTHREAD)
if(HAVE_LIBPTHREAD)
  set(LIBPTHREAD pthread)
endif(HAVE_LIBPTHREAD)

check_library_exists(m atan2 /usr/lib HAVE_LIBM)
if(HAVE_LIBM)
  set(LIBM m)
endif(HAVE_LIBM)

check_library_exists(dl dlopen /usr/lib HAVE_LIBDL)
if(HAVE_LIBDL)
  set(LIBDL dl)
endif(HAVE_LIBDL)

# link_libraries(${LIBDL} ${LIBM} ${LIBPTHREAD})

option(QUICKJS_NODEBUG "Disable debugging" ON)

if(QUICKJS_NODEBUG)
  add_definitions(-DNDEBUG)
endif(QUICKJS_NODEBUG)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(DEBUG ON)
else(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(DEBUG OFF)
endif(CMAKE_BUILD_TYPE MATCHES "Debug")

string(REPLACE "-O3" "-O2" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "-O3" "-O0" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

set(DUMP OFF)

option(DUMP_GC "Enable dump GC" ${DUMP})
option(DUMP_MEM "Enable dump MEM" ${DUMP})
option(DUMP_OBJECTS "Enable dump OBJECTS" OFF)
option(DUMP_SHAPES "Enable dump SHAPES" OFF)
option(DUMP_CC_TABLE "Enable dump CC_TABLE" ${DUMP})
option(DUMP_GC_FREE "Enable dump GC_FREE" ${DUMP})
option(DUMP_PROMISE "Enable dump PROMISE" OFF)
option(DUMP_REOP "Enable dump REOP" OFF)
option(DUMP_TABLE_SIZE "Enable dump TABLE_SIZE" ${DUMP})
option(DUMP_MODULE_RESOLVE "Enable dump MODULE_RESOLVE" OFF)
option(DUMP_READ_OBJECT "Enable dump READ_OBJECT" OFF)
option(DUMP_BYTECODE "Enable dump BYTECODE" OFF)
option(DUMP_LEAKS "Enable dump LEAKS" ${DUMP})

foreach(DEF DUMP_GC DUMP_MEM DUMP_OBJECTS DUMP_SHAPES DUMP_CC_TABLE DUMP_GC_FREE DUMP_PROMISE
            DUMP_REOP DUMP_TABLE_SIZE DUMP_MODULE_RESOLVE DUMP_READ_OBJECT DUMP_BYTECODE DUMP_LEAKS)
  if(${${DEF}})
    add_definitions(-D${DEF}=1)
  endif(${${DEF}})
endforeach(DEF DUMP_GC DUMP_MEM DUMP_OBJECTS DUMP_SHAPES DUMP_CC_TABLE DUMP_GC_FREE DUMP_PROMISE
           DUMP_REOP DUMP_TABLE_SIZE DUMP_MODULE_RESOLVE DUMP_READ_OBJECT DUMP_BYTECODE DUMP_LEAKS)

if(BUILD_SHARED_LIBS)
  set(A_NAME ${PROJECT_NAME}-static)
  set(SO_NAME ${PROJECT_NAME})
  set(LIB_TARGETS ${A_NAME} ${SO_NAME})
  set(QUICKJS_LIBRARY ${SO_NAME})
else(BUILD_SHARED_LIBS)
  set(A_NAME ${PROJECT_NAME})
  set(SO_NAME ${PROJECT_NAME}-shared)
  set(LIB_TARGETS ${A_NAME})
  set(QUICKJS_LIBRARY ${A_NAME})
endif(BUILD_SHARED_LIBS)

set(get_quickjs_cmake_in ${CMAKE_CURRENT_LIST_DIR}/get_quickjs.cmake.in)
set(get_quickjs_cmake ${CMAKE_CURRENT_BINARY_DIR}/get_quickjs.cmake)

macro(check_include NAME)
  string(TOUPPER "${NAME}" TMP)
  string(REPLACE . _ VARNAME "${TMP}")
  set(CMAKE_REQUIRED_QUIET OFF)
  message(STATUS "Checking for ${NAME} ...")
  check_include_file("${NAME}" "HAVE_${VARNAME}")
  set(FOUND "not found")
  if(HAVE_${VARNAME})
    list(APPEND CMAKE_EXTRA_INCLUDE_FILES "${NAME}")
    set(FOUND "found")
  endif(HAVE_${VARNAME})
  message(STATUS "Checking for ${NAME} ... ${FOUND}")
endmacro(check_include NAME)

check_include(signal.h)
check_include(unistd.h)
check_include(stdlib.h)
check_include(stdatomic.h)

if(NOT HAVE_STDATOMIC_H)
  add_definitions(-DNO_STDATOMIC_H=1)
endif(NOT HAVE_STDATOMIC_H)

list(JOIN CMAKE_EXTRA_INCLUDE_FILES " " EXTRA_INCLUDE_FILES)

message(STATUS "Extra includes: ${EXTRA_INCLUDE_FILES}")

if(HAVE_SIGNAL_H)
  list(APPEND CMAKE_EXTRA_INCLUDE_FILES "signal.h")
endif(HAVE_SIGNAL_H)

set(CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE")

check_function_exists(environ HAVE_ENVIRON)
check_symbol_exists(environ unistd.h HAVE_ENVIRON_UNISTD)

if(HAVE_ENVIRON AND NOT HAVE_ENVIRON_UNISTD)

endif(HAVE_ENVIRON AND NOT HAVE_ENVIRON_UNISTD)

check_function_exists(malloc_usable_size HAVE_MALLOC_USABLE_SIZE)

if(NOT HAVE_MALLOC_USABLE_SIZE)
  check_symbol_exists(malloc_usable_size "" HAVE_MALLOC_USABLE_SIZE_DEFINITION)
endif(NOT HAVE_MALLOC_USABLE_SIZE)

if(HAVE_MALLOC_USABLE_SIZE OR HAVE_MALLOC_USABLE_SIZE_DEFINITION)
  # add_definitions(-DHAVE_MALLOC_USABLE_SIZE=1)
else(HAVE_MALLOC_USABLE_SIZE OR HAVE_MALLOC_USABLE_SIZE_DEFINITION)
  add_definitions(-DDONT_HAVE_MALLOC_USABLE_SIZE=1)
endif(HAVE_MALLOC_USABLE_SIZE OR HAVE_MALLOC_USABLE_SIZE_DEFINITION)

if(MSYS OR CYGWIN)
  add_definitions(-DDONT_HAVE_MALLOC_USABLE_SIZE=1)
endif()

check_flags("-Wchar-subscripts;-Wundef;-Wuninitialized;-Wwrite-strings" CMAKE_C_FLAGS)

check_flags(
  "-Wall;-Wno-implicit-int-float-conversion;-Wno-array-bounds;-Wno-missing-field-initializers;-Wno-sign-compare;-Wno-unused;-Wno-format-truncation"
  CMAKE_C_FLAGS_DEBUG)

if(NOT QUICKJS_NODEBUG)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    check_flag("-ggdb" HAVE_DEBUG_GDB_FLAG CMAKE_C_FLAGS_DEBUG)
  endif(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    check_flag("-O0" HAVE_OPT_NONE_FLAG CMAKE_C_FLAGS_DEBUG)
  endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

endif(NOT QUICKJS_NODEBUG)
message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")

check_flag("-flto" HAVE_LTO_FLAG)
if(HAVE_LTO_FLAG)
  option(ENABLE_LTO "Enable link-time optimization" ON)
endif(HAVE_LTO_FLAG)
if(ENABLE_LTO)
  append_vars("-flto" CMAKE_C_FLAGS CMAKE_EXE_LINKER_FLAGS)
endif(ENABLE_LTO)

check_type_size(sighandler_t SIZEOF_SIGHANDLER_T)
if(HAVE_SIZEOF_SIGHANDLER_T)
  message(STATUS "Size of sighandler_t... ${SIZEOF_SIGHANDLER_T}")
else(HAVE_SIZEOF_SIGHANDLER_T)
  check_type_size(__sighandler_t SIZEOF___SIGHANDLER_T)
  if(HAVE_SIZEOF___SIGHANDLER_T)
    add_definitions(-Dsighandler_t=__sighandler_t)
    message(STATUS "Size of __sighandler_t... ${SIZEOF___SIGHANDLER_T}")
  endif(HAVE_SIZEOF___SIGHANDLER_T)
endif(HAVE_SIZEOF_SIGHANDLER_T)

# if(CMAKE_BUILD_TYPE STREQUAL "Debug") option(USE_EFENCE "Enable electric fence" OFF)
# check_library_exists(efence malloc /usr/lib HAVE_EFENCE) endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
# if(USE_EFENCE AND HAVE_EFENCE) set(ELECTRICFENCE_LIBRARY efence) endif(USE_EFENCE AND HAVE_EFENCE)

string(REPLACE "-O3" "-O2" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
string(REPLACE "-O3" "-O2" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
string(REPLACE "-O3" "-Os" CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}")

check_flag("-rdynamic" HAVE_LINK_RDYNAMIC_FLAG LINK_EXPORT)
# message("LINK_EXPORT = ${LINK_EXPORT}")
#[[
check_flag("-Wno-sign-compare" HAVE_WARN_NO_SIGN_COMPARE_FLAG CMAKE_C_FLAGS)
check_flag("-Wno-missing-field-initializers" HAVE_WARN_NO_MISSING_FIELD_INITIALIZERS_FLAG CMAKE_C_FLAGS)
check_flag("-Wno-unused-parameter" HAVE_WARN_NO_UNUSED_PARAMETER_FLAG CMAKE_C_FLAGS)
check_flag("-Wno-array-bounds" HAVE_WARN_NO_ARRAY_BOUNDS_FLAG CMAKE_C_FLAGS)
check_flag("-Wno-format-truncation" HAVE_WARN_NO_FORMAT_TRUNCATION_FLAG CMAKE_C_FLAGS)
check_flag("-Wno-discarded-qualifiers" HAVE_WARN_NO_DISCARDED_QUALIFIERS_FLAG CMAKE_C_FLAGS)
]]
if(CMAKE_CROSSCOMPILING)
  set(QJS "qjsc")
  set(QJS_DEPS)
  set(QJSC "qjsc")
  set(QJSC_DEPS)
else(CMAKE_CROSSCOMPILING)
  set(QJS "${CMAKE_CURRENT_BINARY_DIR}/qjs")
  set(QJS_DEPS qjs)
  set(QJSC "${CMAKE_CURRENT_BINARY_DIR}/qjsc")
  set(QJSC_DEPS qjsc)
endif(CMAKE_CROSSCOMPILING)

add_custom_command(
  OUTPUT repl.c COMMAND ${QJSC} -c -o ./repl.c -m ${CMAKE_CURRENT_SOURCE_DIR}/repl.js
  DEPENDS ${QJSC_DEPS} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generate from repl.js using qjs compiler" SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/repl.js)

add_custom_command(
  OUTPUT qjscalc.c
  COMMAND ${QJSC} -fbignum -c -o ./qjscalc.c -m ${CMAKE_CURRENT_SOURCE_DIR}/qjscalc.js
  DEPENDS ${QJSC_DEPS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generate from qjscalc.js using qjs compiler" SOURCES
          ${CMAKE_CURRENT_SOURCE_DIR}/qjscalc.js)

add_custom_target(
  hello.c
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/qjsc -e -o ${CMAKE_CURRENT_BINARY_DIR}/hello.c -m
          ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello.js
  DEPENDS ${QJSC_DEPS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generate from hello.js using qjs compiler"
  SOURCES examples/hello.js)

configure_file(${get_quickjs_cmake_in} ${get_quickjs_cmake} @ONLY)
# add_custom_command( OUTPUT ${quickjs_sources} DEPENDS ${get_quickjs_cmake} COMMENT Get quickjs
# sources. COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/get_quickjs.cmake )

add_library(${A_NAME} STATIC ${quickjs_sources})
target_compile_definitions(${A_NAME} PRIVATE _GNU_SOURCE=1 CONFIG_VERSION="${quickjs_version}"
                                             CONFIG_PREFIX="${CMAKE_INSTALL_PREFIX}" CONFIG_BIGNUM)
target_include_directories(${A_NAME} PUBLIC ${quickjs_sources_root})
set_target_properties(${A_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
if(NOT BUILD_SHARED_LIBS)
  set_target_properties(${A_NAME} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif(NOT BUILD_SHARED_LIBS)
target_link_libraries(${A_NAME} PUBLIC ${LIBDL} ${LIBM} ${LIBPTHREAD} ${LIBWS2_32})
# message("target_link_libraries ${A_NAME} PRIVATE ${LIBDL} ${LIBM} ${LIBPTHREAD}")

if(BUILD_SHARED_LIBS)
  add_library(${SO_NAME} SHARED ${quickjs_sources})
  target_compile_definitions(
    ${SO_NAME}
    PRIVATE _GNU_SOURCE=1 CONFIG_VERSION="${quickjs_version}"
            CONFIG_SOVERSION="${quickjs_soversion}" CONFIG_PREFIX="${CMAKE_INSTALL_PREFIX}"
            CONFIG_BIGNUM)
  target_include_directories(${SO_NAME} PUBLIC ${quickjs_sources_root})
  set_target_properties(${SO_NAME} PROPERTIES SOVERSION "${quickjs_soversion}" OUTPUT_NAME
                                                                               ${PROJECT_NAME})
  target_link_libraries(${SO_NAME} PUBLIC ${LIBDL} ${LIBM} ${LIBPTHREAD} ${LIBWS2_32})

endif(BUILD_SHARED_LIBS)
install(TARGETS ${LIB_TARGETS} ARCHIVE DESTINATION lib LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(FILES ${quickjs_includes} DESTINATION include/quickjs)

add_executable(
  qjs
  qjs.c
  quickjs.c
  quickjs.h
  libregexp.c
  libunicode.c
  cutils.c
  quickjs-libc.c
  quickjs-libc.h
  quickjs-find-module.c
  libbf.c
  ${CMAKE_CURRENT_BINARY_DIR}/repl.c
  ${CMAKE_CURRENT_BINARY_DIR}/qjscalc.c)

# target_link_libraries(qjs ${QUICKJS_LIBRARY})
target_link_libraries(qjs ${LIBPTHREAD} ${LIBM} ${LIBDL} ${LIBWS2_32})
set_target_properties(qjs PROPERTIES LINK_OPTIONS "${LINK_EXPORT}")

target_compile_definitions(qjs PRIVATE CONFIG_VERSION="${quickjs_version}" _GNU_SOURCE=1
                                       CONFIG_PREFIX="${CMAKE_INSTALL_PREFIX}" CONFIG_BIGNUM=1)

add_executable(
  qjsc
  qjsc.c quickjs.c quickjs.h libregexp.c libunicode.c cutils.c quickjs-libc.c quickjs-libc.h
  quickjs-find-module.c libbf.c)
target_link_libraries(qjsc ${LIBM} ${LIBDL} ${LIBPTHREAD} ${LIBWS2_32})

target_compile_definitions(
  qjsc
  PRIVATE _GNU_SOURCE=1 CONFIG_VERSION="${quickjs_version}" CONFIG_SOVERSION="${quickjs_soversion}"
          CONFIG_PREFIX="${CMAKE_INSTALL_PREFIX}" CONFIG_CC="${CMAKE_C_COMPILER}" CONFIG_LTO=1
          CONFIG_BIGNUM=1)

install(TARGETS qjs qjsc DESTINATION bin)
install(
  CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink qjs ${CMAKE_INSTALL_PREFIX}/bin/qjscalc)"
)

add_custom_target(.ctags COMMAND ctags -R ${CMAKE_CURRENT_SOURCE_DIR})

set(QUICKJS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(MODULE_SUBDIRS "ffi;net;modules")

option(MODULE_GLFW "Enable GLFW module" OFF)
option(MODULE_NANOVG "Enable NanoVG module" OFF)

if(MODULE_GLFW)
  list(APPEND MODULE_SUBDIRS qjs-glfw)
endif(MODULE_GLFW)
if(MODULE_NANOVG)
  list(APPEND MODULE_SUBDIRS qjs-nanovg)
endif(MODULE_NANOVG)

foreach(MODULE_SUBDIR ${MODULE_SUBDIRS})
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_SUBDIR}/CMakeLists.txt)
    add_subdirectory(${MODULE_SUBDIR})
  endif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_SUBDIR}/CMakeLists.txt)
endforeach(MODULE_SUBDIR ${MODULE_SUBDIRS})

message("BUILDSYSTEM_TARGETS = ${BUILDSYSTEM_TARGETS}")
