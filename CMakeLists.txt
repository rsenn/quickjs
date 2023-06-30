# Copyright (c) 2020 Kosaki Mezumona This cmake script is distributed under the MIT License. See LICENSE for more details.
cmake_minimum_required(VERSION 3.1)
project(quickjs)

message("Building project [1;31m${PROJECT_NAME}[0m")

set(DISABLE_WERROR TRUE)

if(CACHE{CMAKE_BUILD_TYPE})
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release MinSizeRel RelWithDebInfo)
endif(CACHE{CMAKE_BUILD_TYPE})

option(CONFIG_BIGNUM "Enable bignum support" ON)
option(CONFIG_DLFCN "Enable dlfcn module loader" ON)
option(CONFIG_ATOMICS "Enable atomics" ON)

if(WASI OR EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".wasm")
  option(CONFIG_WORKER "Enable Worker" OFF)
  option(CONFIG_DEBUGGER "Enable debugger protocol" OFF)
else(WASI OR EMSCRIPTEN)
  option(CONFIG_WORKER "Enable Worker" ON)
  option(CONFIG_DEBUGGER "Enable debugger protocol" ON)
endif(WASI OR EMSCRIPTEN)

if(CONFIG_WORKER)
  set(USE_WORKER "${CONFIG_WORKER}")
endif(CONFIG_WORKER)

option(CONFIG_ALL_UNICODE "Enable all-Unicode support" OFF)
option(CONFIG_CHECK_JSVALUE "JSValue consistency mode" OFF)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(FindThreads)

include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckTypeSize)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/UseMultiArch.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/check-libraries.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/functions.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/try-compiler.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFlags.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/check-shared.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/defs.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindQuickJS.cmake)

check_c_standard_flag(c99)

if(C_STANDARD_FLAG)
  set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${C_STANDARD_FLAG})
  #list(APPEND CMAKE_C_FLAGS ${C_STANDARD_FLAG})
  string(REPLACE ";" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_STANDARD_FLAG}")
endif(C_STANDARD_FLAG)

configure_quickjs()
configure_quickjs_module_path()

message(STATUS "!!! C module directory: ${QUICKJS_C_MODULE_DIR}")

if(NOT QUICKJS_BUILD_ROOT)
  set(QUICKJS_BUILD_ROOT "${CMAKE_CURRENT_BINARY_DIR}")
endif(NOT QUICKJS_BUILD_ROOT)

message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "System name: ${SYSTEM_NAME}")

#[[if(NOT DEFINED BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS "${SUPPORT_SHARED_LIBS}" CACHE BOOL "Build quickjs as shared library")
endif(NOT DEFINED BUILD_SHARED_LIBS)]]

option(BUILD_SHARED_LIBS "Build shared libraries" ${SUPPORT_SHARED_LIBS})

if("${SYSTEM_NAME}" MATCHES ".*wasi")
  #message(STATUS "\x1b[1;31mwasienv WASM build\x1b[0m")
  set(BUILD_SHARED_LIBS FALSE)
  set(ENABLE_SHARED FALSE)
elseif(WIN32 OR MINGW)
  set(LIBWS2_32 ws2_32)
  set(BUILD_SHARED_LIBS TRUE)
endif()

# link_libraries( ${LIBM} ${LIBPTHREAD})

if(CONFIG_DEBUGGER)
  message(STATUS "Enabling quickjs-debugger")

  set(CONFIG_DEBUGGER 1)
  #add_definitions(-DCONFIG_DEBUGGER)
  list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_DEFAULT_SOURCE=1 -D_BSD_SOURCE=1)
  check_include_file("netdb.h" HAVE_NETDB_H)
  if(HAVE_NETDB_H)
    list(APPEND CMAKE_REQUIRED_INCLUDES netdb.h)
  endif(HAVE_NETDB_H)

  #check_function_exists(gethostbyname_r HAVE_GETHOSTBYNAME_R)
  check_function_exists(gethostbyname HAVE_GETHOSTBYNAME)
  if(NOT HAVE_GETHOSTBYNAME)
    check_include_file("arpa/inet.h" HAVE_ARPA_INET_H)
    if(HAVE_ARPA_INET_H)
      list(APPEND CMAKE_REQUIRED_INCLUDES arpa/inet.h)
    endif(HAVE_ARPA_INET_H)

    check_function_exists(inet_pton HAVE_INET_PTON)
    if(NOT HAVE_INET_PTON)
      check_function_exists(inet_aton HAVE_INET_ATON)
    endif(NOT HAVE_INET_PTON)
  endif(NOT HAVE_GETHOSTBYNAME)
endif(CONFIG_DEBUGGER)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(DEBUG ON)
else(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(DEBUG OFF)
  add_definitions(-DNDEBUG)
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

set(DUMP_REDIR "" CACHE STRING "printf-like macro for all DUMP_* options")

if("${DUMP_REDIR}" STREQUAL "printf")
  set(DUMP_REDIR "")
endif("${DUMP_REDIR}" STREQUAL "printf")

if("${DUMP_REDIR}" STREQUAL "")
  set(DUMP_MACRO OFF CACHE INTERNAL "Enable dump macro")
  #unset(DUMP_REDIR)
  set(DUMP_DEFINE "")
else("${DUMP_REDIR}" STREQUAL "")
  set(DUMP_MACRO ON CACHE INTERNAL "Enable dump macro")
  if(NOT "${DUMP_REDIR}" MATCHES ".*[()].*")
    set(DUMP_REDIR "${DUMP_REDIR}(__VA_ARGS__)")
  endif(NOT "${DUMP_REDIR}" MATCHES ".*[()].*")

  set(DUMP_DEFINE
      "\n#ifdef BUILD_QUICKJS\n/* printf - Redirect output */\n#define printf(...) ${DUMP_REDIR}\n\n#define errprintf(...) fprintf(stderr, __VA_ARGS__)\n#endif /* BUILD_QUICKJS */\n"
  )
endif("${DUMP_REDIR}" STREQUAL "")

foreach(DEF DUMP_GC DUMP_MEM DUMP_OBJECTS DUMP_SHAPES DUMP_CC_TABLE DUMP_GC_FREE DUMP_PROMISE DUMP_REOP DUMP_TABLE_SIZE DUMP_MODULE_RESOLVE DUMP_READ_OBJECT DUMP_BYTECODE
            DUMP_LEAKS)
  if(${${DEF}})
    add_definitions(-D${DEF}=1)
  endif(${${DEF}})
endforeach(DEF DUMP_GC DUMP_MEM DUMP_OBJECTS DUMP_SHAPES DUMP_CC_TABLE DUMP_GC_FREE DUMP_PROMISE DUMP_REOP DUMP_TABLE_SIZE DUMP_MODULE_RESOLVE DUMP_READ_OBJECT DUMP_BYTECODE
           DUMP_LEAKS)

if(BUILD_SHARED_LIBS AND SUPPORT_SHARED_LIBS)
  set(A_NAME ${PROJECT_NAME}-static)
  set(SO_NAME ${PROJECT_NAME})
  set(LIB_TARGETS ${A_NAME} ${SO_NAME})
  if(NOT QUICKJS_LIBRARY)
    set(QUICKJS_LIBRARY "${SO_NAME}" CACHE PATH "QuickJS library")
  endif(NOT QUICKJS_LIBRARY)
else(BUILD_SHARED_LIBS AND SUPPORT_SHARED_LIBS)
  set(A_NAME ${PROJECT_NAME})
  set(SO_NAME ${PROJECT_NAME}-shared)
  set(LIB_TARGETS ${A_NAME})
  if(NOT QUICKJS_LIBRARY)
    set(QUICKJS_LIBRARY "${A_NAME}" CACHE PATH "QuickJS library")
  endif(NOT QUICKJS_LIBRARY)

endif(BUILD_SHARED_LIBS AND SUPPORT_SHARED_LIBS)

set(QUICKJS_LIBRARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "QuickJS library directory")
set(QUICKJS_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "QuickJS include directory")

#dump(QUICKJS_LIBRARY QUICKJS_LIBRARY_DIR QUICKJS_INCLUDE_DIR)

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

macro(check_function NAME)
  string(TOUPPER "${NAME}" TMP)
  string(REPLACE . _ VARNAME "${TMP}")
  set(CMAKE_REQUIRED_QUIET OFF)
  message(STATUS "Checking for ${NAME})() ...")
  check_function_exists("${NAME}" "HAVE_${VARNAME}")
  set(FOUND "not found")
  message(STATUS "Checking for ${NAME}() ... ${FOUND}")
endmacro(check_function NAME)

check_include(signal.h)
check_include(unistd.h)
check_include(stdlib.h)
check_include(stdatomic.h)
check_include(fenv.h)
check_include(math.h)

if(HAVE_FENV_H)
  list(APPEND CMAKE_REQUIRED_INCLUDES "fenv.h")
endif(HAVE_FENV_H)
if(HAVE_MATH_H)
  list(APPEND CMAKE_REQUIRED_INCLUDES "math.h")
endif(HAVE_MATH_H)

check_symbol_exists(__dietlibc__ "" HAVE_DIETLIBC)

if(HAVE_DIETLIBC)
  check_symbol_exists(_REDIR_TIME64 "" HAVE__REDIR_TIME64)
  if(NOT HAVE__REDIR_TIME64)
    add_definitions("-D_REDIR_TIME64=0")
  endif(NOT HAVE__REDIR_TIME64)

endif(HAVE_DIETLIBC)

check_symbol_exists(NAN math.h HAVE_NAN)
if(NOT HAVE_NAN)
  add_definitions("-DNAN=__builtin_nan(\"\")")
endif(NOT HAVE_NAN)

check_symbol_exists(INFINITY math.h HAVE_INFINITY)
if(NOT HAVE_INFINITY)
  add_definitions("-DINFINITY=\"__builtin_inf()\"")
endif(NOT HAVE_INFINITY)

# check_function_exists(trunc HAVE_TRUNC) if(NOT HAVE_TRUNC) add_definitions("-D\"trunc(x)=__builtin_trunc(x)\"") endif(NOT HAVE_TRUNC)

set(HAVE_VARIABLES)

macro(check_builtin NAME VAR)
  set(RESULT "${${VAR}}")
  if(RESULT STREQUAL "")
    set(SOURCE "int main() { void *ptr;  ptr = &${NAME}; return ptr != 0; }")
    set(C_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/try-${NAME}.c")
    file(WRITE "${C_SOURCE}" "${SOURCE}")
    try_compile(RESULT "${CMAKE_CURRENT_BINARY_DIR}" "${C_SOURCE}" OUTPUT_VARIABLE OUTPUT LINK_OPTIONS -c LINK_LIBRARIES "${ARGN}")

    if(NOT RESULT)
      set(RESULT FALSE)
    endif(NOT RESULT)
  endif(RESULT STREQUAL "")
  if(RESULT)
    set(MSG found)
  else(RESULT)
    set(MSG missing)
  endif(RESULT)
  set("${VAR}" "${RESULT}" PARENT_SCOPE)

  message(STATUS "Checking builtin ${NAME} -- ${MSG}")
  set(CMAKE_REQUIRED_QUIET FALSE)
endmacro(check_builtin NAME VAR)

function(check_func NAME)
  string(TOUPPER "${NAME}" UNAME)
  # set(BUILTIN ${ARGN})
  if(NOT BUILTIN)
    set(BUILTIN "__builtin_${NAME}")
  endif(NOT BUILTIN)
  if(ARGN)
    set(LIB "${ARGN}")
  endif(ARGN)
  if(LIB)
    check_library_exists("${LIB}" "${NAME}" "" RESULT)
  else(LIB)
    check_function_exists(${NAME} RESULT)
  endif(LIB)
  set("HAVE_${UNAME}" "${RESULT}" PARENT_SCOPE)

  if(NOT RESULT)
    check_builtin("${BUILTIN}" B_RESULT)
    set("HAVE_${BUILTIN}" "${B_RESULT}" PARENT_SCOPE)
    if(B_RESULT)
      # add_definitions("-D${NAME}(x...)=${BUILTIN}(x)")
    endif(B_RESULT)
  endif(NOT RESULT)
  if(RESULT)
    list(APPEND HAVE_VARIABLES HAVE_${UNAME})
    set(HAVE_VARIABLES "${HAVE_VARIABLES}" PARENT_SCOPE)
  endif(RESULT)
endfunction(check_func NAME)

set(MATH_FUNCS acos acosh asin asinh atan atan2 atanh cbrt ceil cos cosh exp expm1 fabs fesetround floor fmax fmin fmod hypot isfinite log log10 log1p log2 lrint pow round sin
               sinh sqrt tan tanh trunc)
foreach(MATH_FUNC ${MATH_FUNCS})
  check_func(${MATH_FUNC} ${LIBM})
endforeach(MATH_FUNC ${MATH_FUNCS})

set(SEPARATOR " | ")

#dump(HAVE_VARIABLES)

check_symbol_exists(FE_DOWNWARD fenv.h HAVE_FE_DOWNWARD)
if(NOT HAVE_FE_DOWNWARD)
  add_definitions(-DFE_DOWNWARD=0x400)
endif(NOT HAVE_FE_DOWNWARD)

check_symbol_exists(FE_UPWARD fenv.h HAVE_FE_UPWARD)
if(NOT HAVE_FE_UPWARD)
  add_definitions(-DFE_UPWARD=0x800)
endif(NOT HAVE_FE_UPWARD)

if(NOT HAVE_STDATOMIC_H)
  add_definitions(-DNO_STDATOMIC_H=1)
endif(NOT HAVE_STDATOMIC_H)

# list(JOIN CMAKE_EXTRA_INCLUDE_FILES " " EXTRA_INCLUDE_FILES)
string(REPLACE ";" " " EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES}")

message(STATUS "Extra includes: ${EXTRA_INCLUDE_FILES}")

if(HAVE_SIGNAL_H)
  list(APPEND CMAKE_EXTRA_INCLUDE_FILES "signal.h")
endif(HAVE_SIGNAL_H)

set(CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE")

check_function_exists(environ HAVE_ENVIRON)
check_symbol_exists(environ unistd.h HAVE_ENVIRON_UNISTD)

#[[check_function(pclose)
check_function(popen)
check_function(tmpfile)
check_function(utimes)
check_function(fork)
check_function(setuid)
check_function(setgid)
check_function(execve)
check_function(waitpid)
check_function(pipe)]]

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

check_flags("-Wall;-Wno-array-bounds;-Wno-missing-field-initializers;-Wno-sign-compare;-Wno-unused;-Wno-format-truncation" CMAKE_C_FLAGS_DEBUG)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
  check_flag("-ggdb" HAVE_DEBUG_GDB_FLAG CMAKE_C_FLAGS_DEBUG)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  check_flag("-O0" HAVE_OPT_NONE_FLAG CMAKE_C_FLAGS_DEBUG)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")

check_flag("-flto" HAVE_LTO_FLAG)
if(HAVE_LTO_FLAG)
  set(CONFIG_LTO TRUE CACHE BOOL "Define if your compiler supports link-time optimization")
  option(ENABLE_LTO "Enable link-time optimization" OFF)
endif(HAVE_LTO_FLAG)

if(CONFIG_LTO)
  add_definitions(-DCONFIG_LTO)
endif(CONFIG_LTO)

if(ENABLE_LTO)
  message(STATUS "Enabling link-time optimization")
  #append_vars("-flto" CMAKE_C_FLAGS CMAKE_EXE_LINKER_FLAGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
endif(ENABLE_LTO)

if(WASI OR WASM)
  set(SIZEOF_SIGHANDLER_T 4)
  set(HAVE_SIZEOF_SIGHANDLER_T TRUE)
endif(WASI OR WASM)

if(NOT HAVE_SIZEOF_SIGHANDLER_T)
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
endif(NOT HAVE_SIZEOF_SIGHANDLER_T)

check_function_def(poll)
check_include_def(sys/poll.h)
check_include_def(dlfcn.h)
check_include_def(stdatomic.h)
check_include_def(alloca.h)
check_include_def(malloc.h)
check_library_exists(pthread pthread_create "${CMAKE_LIBRARY_PATH}" PTHREADS)

if(WIN32 OR MINGW)

  check_include_def(winsock2.h)

  if(HAVE_WINSOCK2_H)
    include(CheckTypeSize)

    set(CMAKE_EXTRA_INCLUDE_FILES winsock2.h)

    check_type_size(SOCKET SOCKET_SIZE)
    check_type_size("WSAPOLLFD" WSAPOLLFD_SIZE)

    unset(CMAKE_EXTRA_INCLUDE_FILES)

  endif(HAVE_WINSOCK2_H)

  check_include_def(process.h)

endif(WIN32 OR MINGW)

if(PTHREADS AND HAVE_STDATOMIC_H)
  set(USE_WORKER ON)
endif(PTHREADS AND HAVE_STDATOMIC_H)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  option(USE_EFENCE "Enable electric fence" OFF)
  if(USE_EFENCE)
    check_library_exists(efence malloc /usr/lib HAVE_EFENCE)
  endif(USE_EFENCE)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")

if(HAVE_EFENCE)
  set(ELECTRICFENCE_LIBRARY efence)
else(HAVE_EFENCE)
  set(ELECTRICFENCE_LIBRARY)
endif(HAVE_EFENCE)

string(REPLACE "-O3" "-O2" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
string(REPLACE "-O3" "-O2" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
string(REPLACE "-O3" "-Os" CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}")

if(NOT WIN32)
  if(NOT MINGW)
    if(UNIX)
      check_flag("-rdynamic" HAVE_LINK_RDYNAMIC_FLAG LINK_EXPORT)
    endif(UNIX)
  endif(NOT MINGW)
endif(NOT WIN32)
# message("LINK_EXPORT = ${LINK_EXPORT}")
#[[
check_flag("-Wno-sign-compare" HAVE_WARN_NO_SIGN_COMPARE_FLAG CMAKE_C_FLAGS)
check_flag("-Wno-missing-field-initializers" HAVE_WARN_NO_MISSING_FIELD_INITIALIZERS_FLAG CMAKE_C_FLAGS)
check_flag("-Wno-unused-parameter" HAVE_WARN_NO_UNUSED_PARAMETER_FLAG CMAKE_C_FLAGS)
check_flag("-Wno-array-bounds" HAVE_WARN_NO_ARRAY_BOUNDS_FLAG CMAKE_C_FLAGS)
check_flag("-Wno-format-truncation" HAVE_WARN_NO_FORMAT_TRUNCATION_FLAG CMAKE_C_FLAGS)
check_flag("-Wno-discarded-qualifiers" HAVE_WARN_NO_DISCARDED_QUALIFIERS_FLAG CMAKE_C_FLAGS)
]]
option(BUILD_QJSC "Build quickjs compiler" ON)

if(${SYSTEM_NAME} MATCHES wasi)
  set(BUILD_QJSC OFF)
endif(${SYSTEM_NAME} MATCHES wasi)

if(LIBDL)
  message(STATUS "LIBDL = ${LIBDL}")
  link_libraries(${LIBDL})
endif(LIBDL)

if(CMAKE_CROSSCOMPILING OR NOT BUILD_QJSC)
  set(QJS "qjsc")
  set(QJS_DEPS)
  set(QJSC "qjsc" CACHE STRING "QuickJS compiler")
  set(QJSC_DEPS)
else(CMAKE_CROSSCOMPILING OR NOT BUILD_QJSC)
  set(QJS "${CMAKE_CURRENT_BINARY_DIR}/qjs")
  set(QJS_DEPS qjs)
  set(QJSC "${CMAKE_CURRENT_BINARY_DIR}/qjsc" CACHE STRING "QuickJS compiler")
  set(QJSC_DEPS qjsc)
endif(CMAKE_CROSSCOMPILING OR NOT BUILD_QJSC)

add_custom_command(OUTPUT repl.c COMMAND "${QJSC}" -c -o ./repl.c -m ${CMAKE_CURRENT_SOURCE_DIR}/repl.js DEPENDS ${QJSC_DEPS} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMENT "Generate from repl.js using qjs compiler" SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/repl.js)

add_custom_command(OUTPUT qjscalc.c COMMAND "${QJSC}" -fbignum -c -o ./qjscalc.c -m ${CMAKE_CURRENT_SOURCE_DIR}/qjscalc.js DEPENDS ${QJSC_DEPS}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generate from qjscalc.js using qjs compiler" SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/qjscalc.js)

add_custom_target(hello.c COMMAND "${QJSC}" -e -o ${CMAKE_CURRENT_BINARY_DIR}/hello.c -m ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello.js DEPENDS ${QJSC_DEPS}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generate from hello.js using qjs compiler" SOURCES examples/hello.js)

execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE HOST_SYSTEM_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file(${get_quickjs_cmake_in} ${get_quickjs_cmake} @ONLY)

configure_file(quickjs-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/quickjs-config.h)

# add_custom_command( OUTPUT ${QUICKJS_SOURCES} DEPENDS ${get_quickjs_cmake} COMMENT Get quickjs sources. COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/get_quickjs.cmake )
add_library(${A_NAME} STATIC ${QUICKJS_SOURCES} ${QUICKJS_SOURCES_DEBUGGER})
target_compile_definitions(
  ${A_NAME}
  PRIVATE _GNU_SOURCE=1 BUILD_QUICKJS=1 HAVE_QUICKJS_CONFIG_H=1 CONFIG_VERSION="${QUICKJS_VERSION}" QUICKJS_PREFIX="${QUICKJS_INSTALL_PREFIX}"
          #QUICKJS_MODULE_PATH="${QUICKJS_MODULE_PATH}"
          HOST_SYSTEM_NAME="${HOST_SYSTEM_NAME}" CONFIG_BIGNUM)
target_include_directories(${A_NAME} PUBLIC ${QUICKJS_SOURCES_ROOT})
set_target_properties(${A_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
if(NOT BUILD_SHARED_LIBS)
  set_target_properties(${A_NAME} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif(NOT BUILD_SHARED_LIBS)
target_link_libraries(${A_NAME} PUBLIC ${LIBM} ${LIBPTHREAD} ${LIBWS2_32})
# message("target_link_libraries ${A_NAME} PRIVATE ${LIBM} ${LIBPTHREAD}")

if(BUILD_SHARED_LIBS AND SUPPORT_SHARED_LIBS)
  message("Building shared library: YES")
  add_library(${SO_NAME} SHARED ${QUICKJS_SOURCES} ${QUICKJS_INCLUDES} ${QUICKJS_SOURCES_DEBUGGER})
  target_compile_definitions(${SO_NAME} PRIVATE _GNU_SOURCE=1 BUILD_QUICKJS=1 HAVE_QUICKJS_CONFIG_H=1 CONFIG_VERSION="${QUICKJS_VERSION}"
                                                QUICKJS_PREFIX="${QUICKJS_INSTALL_PREFIX}" SYSTEM_NAME="${SYSTEM_NAME}" CONFIG_BIGNUM)
  target_include_directories(${SO_NAME} PUBLIC ${QUICKJS_SOURCES_ROOT})
  if(MSYS)
    set(SOPREFIX "msys-")
  else()
    set(SOPREFIX "lib")
  endif()
  set_target_properties(${SO_NAME} PROPERTIES #SOVERSION "${QUICKJS_SOVERSION}"
                                              PREFIX "${SOPREFIX}" OUTPUT_NAME ${PROJECT_NAME})
  get_target_property(SOSUFFIX ${SO_NAME} SUFFIX)

  if("${CMAKE_SHARED_LIBRARY_SUFFIX}" MATCHES ".so")
    set(SOSUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}.${QUICKJS_SOVERSION}")
  else("${CMAKE_SHARED_LIBRARY_SUFFIX}" MATCHES ".so")
    set(SOSUFFIX "-${QUICKJS_SOVERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif("${CMAKE_SHARED_LIBRARY_SUFFIX}" MATCHES ".so")
  #set_target_properties(${SO_NAME} PROPERTIES SUFFIX "${SOSUFFIX}")

  target_link_libraries(${SO_NAME} PUBLIC ${LIBM} ${LIBPTHREAD} ${LIBWS2_32})

else(BUILD_SHARED_LIBS AND SUPPORT_SHARED_LIBS)
  message("Building shared library: NO")
endif(BUILD_SHARED_LIBS AND SUPPORT_SHARED_LIBS)

if(LIB_TARGETS)
  install(TARGETS ${LIB_TARGETS} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif(LIB_TARGETS)

if(NOT CMAKE_INSTALL_INCLUDEDIR)
  set(CMAKE_INSTALL_INCLUDEDIR include)
endif(NOT CMAKE_INSTALL_INCLUDEDIR)

install(FILES ${QUICKJS_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR}/quickjs-config.h DESTINATION ${QUICKJS_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/quickjs)

file(GLOB CMAKE_SCRIPTS cmake/*QuickJS*)
install(FILES ${CMAKE_SCRIPTS} DESTINATION ${QUICKJS_PREFIX}/lib/cmake)

add_executable(qjs qjs.c #${QUICKJS_SOURCES} ${QUICKJS_SOURCES_DEBUGGER}
                   ${CMAKE_CURRENT_BINARY_DIR}/repl.c ${CMAKE_CURRENT_BINARY_DIR}/qjscalc.c)

target_link_libraries(qjs ${QUICKJS_LIBRARY} ${LIBPTHREAD} ${LIBM} ${LIBWS2_32})
set_target_properties(qjs PROPERTIES LINK_OPTIONS "${LINK_EXPORT}")

target_compile_definitions(qjs PRIVATE _GNU_SOURCE=1 BUILD_QUICKJS=1 HAVE_QUICKJS_CONFIG_H=1 CONFIG_VERSION="${QUICKJS_VERSION}" QUICKJS_PREFIX="${QUICKJS_INSTALL_PREFIX}"
                                       CONFIG_BIGNUM=1)

if(WASI)
  set(BUILD_QJSC FALSE CACHE BOOL "Build qjsc")
endif(WASI)
if(BUILD_QJSC)
  add_executable(qjsc qjsc.c ${QUICKJS_SOURCES} ${QUICKJS_SOURCES_DEBUGGER})
  target_link_libraries(qjsc ${LIBM} ${LIBPTHREAD} ${LIBWS2_32})

  target_compile_definitions(qjsc PRIVATE CONFIG_VERSION="${QUICKJS_VERSION}" QUICKJS_PREFIX="${QUICKJS_INSTALL_PREFIX}" CONFIG_CC="${CMAKE_C_COMPILER}" _GNU_SOURCE=1
                                          BUILD_QUICKJS=1 HAVE_QUICKJS_CONFIG_H=1 CONFIG_BIGNUM=1)
  install(TARGETS qjsc DESTINATION ${QUICKJS_PREFIX}/bin)
endif(BUILD_QJSC)

install(TARGETS qjs DESTINATION ${QUICKJS_BINARY_DIR})
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink qjs ${CMAKE_INSTALL_PREFIX}/bin/qjscalc)")

add_custom_target(.ctags COMMAND ctags -R ${CMAKE_CURRENT_SOURCE_DIR})

set(QUICKJS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(MODULE_SUBDIRS "")

option(MODULE_GLFW "Enable GLFW module" ON)
option(MODULE_NANOVG "Enable NanoVG module" ON)
option(MODULE_IMGUI "Enable ImGui module" ON)
option(MODULE_FFI "Enable FFI module" ON)
option(MODULE_NET "Enable NET module" ON)
option(MODULE_HTTP "Enable HTTP module" ON)
option(MODULE_MODULES "Enable MODULES module" ON)

if(MODULE_GLFW)
  list(APPEND MODULE_SUBDIRS qjs-glfw)
endif(MODULE_GLFW)
if(MODULE_NANOVG)
  list(APPEND MODULE_SUBDIRS qjs-nanovg)
endif(MODULE_NANOVG)
if(MODULE_IMGUI)
  list(APPEND MODULE_SUBDIRS qjs-imgui)
endif(MODULE_IMGUI)
if(MODULE_FFI)
  list(APPEND MODULE_SUBDIRS qjs-ffi)
endif(MODULE_FFI)
if(MODULE_NET)
  list(APPEND MODULE_SUBDIRS qjs-net)
endif(MODULE_NET)
if(MODULE_HTTP)
  list(APPEND MODULE_SUBDIRS qjs-http)
endif(MODULE_HTTP)
if(MODULE_MODULES)
  list(APPEND MODULE_SUBDIRS qjs-modules)
endif(MODULE_MODULES)
message(STATUS "Module subdirectories: ${MODULE_SUBDIRS}")

set(QUICKJS_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH "QuickJS install directory")
set(QUICKJS_SOURCES_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "QuickJS source directory")

foreach(MODULE_SUBDIR ${MODULE_SUBDIRS})
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_SUBDIR}/CMakeLists.txt)
    add_subdirectory(${MODULE_SUBDIR})
  endif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_SUBDIR}/CMakeLists.txt)
endforeach(MODULE_SUBDIR ${MODULE_SUBDIRS})

file(
  WRITE "${CMAKE_CURRENT_BINARY_DIR}/quickjs.pc"
  "prefix=${QUICKJS_PREFIX}\nexec_prefix=\${prefix}\nlibdir=\${exec_prefix}/lib\nincludedir=\${prefix}/include\n\nName: quickjs\nDescription: QuickJS\nVersion: ${QUICKJS_VERSION}\nLibs: -L\${libdir} -lquickjs\nCflags: -I\${includedir}\n"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/quickjs.pc" DESTINATION "${QUICKJS_PREFIX}/lib/pkgconfig")
