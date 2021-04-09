if(WIN32 OR MINGW)
  set(TRANSPORT_PLATFORM win)
else(WIN32 OR MINGW)
  set(TRANSPORT_PLATFORM unix)
endif(WIN32 OR MINGW)


if(NOT quickjs_sources_root)
  set(quickjs_sources_root "${CMAKE_CURRENT_SOURCE_DIR}")
endif(NOT quickjs_sources_root)

message(STATUS "${quickjs_sources_root}/VERSION")
file(READ "${quickjs_sources_root}/VERSION" version)
string(STRIP "${version}" quickjs_version)

if(NOT quickjs_version)
  set(quickjs_version "2021-03-27")
endif(NOT quickjs_version)

set(quickjs_soversion 1)
set(quickjs_url https://bellard.org/quickjs/quickjs-${quickjs_version}.tar.xz)
set(quickjs_sha1 371eae0896cc9e9f50864cb34f37d9481d843ce1)
set(quickjs_extras_url
    https://bellard.org/quickjs/quickjs-extras-${quickjs_version}.tar.xz)
set(quickjs_extras_sha1 211e43a5638668c80c8d438a3065660ab3af96df)
set(quickjs_extract_dir ${CMAKE_CURRENT_BINARY_DIR}/sources)
if(NOT quickjs_sources_root)
  set(quickjs_sources_root ${CMAKE_CURRENT_SOURCE_DIR})
endif(NOT quickjs_sources_root)
set(quickjs_sources
    ${quickjs_sources_root}/cutils.c
    ${quickjs_sources_root}/libbf.c
    ${quickjs_sources_root}/libregexp.c
    ${quickjs_sources_root}/libunicode.c
    ${quickjs_sources_root}/quickjs.c
    ${quickjs_sources_root}/quickjs-libc.c
    ${quickjs_sources_root}/quickjs-find-module.c)
set(quickjs_includes
    cutils.h
    libbf.h
    libregexp-opcode.h
    libregexp.h
    libunicode-table.h
    libunicode.h
    list.h
    quickjs-atom.h
    quickjs-libc.h
    quickjs-opcode.h
    quickjs.h
    unicode_gen_def.h)


execute_process(COMMAND cc -dumpmachine OUTPUT_VARIABLE HOST_SYSTEM_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE SYSTEM_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)

string(REGEX REPLACE "-pc-" "-" quickjs_host_arch "${SYSTEM_NAME}")

if(NOT "${HOST_SYSTEM_NAME}" STREQUAL "${quickjs_host_arch}")
  string(REGEX REPLACE i686 i386 quickjs_cross_arch "${quickjs_host_arch}")
  # set(quickjs_cross_arch ${SYSTEM_NAME}) endif(CMAKE_CROSSCOMPILING)
else(NOT "${HOST_SYSTEM_NAME}" STREQUAL "${SYSTEM_NAME}")
  set(quickjs_cross_arch "")
endif(NOT "${HOST_SYSTEM_NAME}" STREQUAL "${SYSTEM_NAME}")

message("HOST_SYSTEM_NAME = ${HOST_SYSTEM_NAME}")
message("SYSTEM_NAME = ${SYSTEM_NAME}")
message("quickjs_cross_arch = ${quickjs_cross_arch}")

if(quickjs_cross_arch)
  set(quickjs_libdir lib/${quickjs_cross_arch})
  set(quickjs_bindir bin/${quickjs_cross_arch})
  set(quickjs_includedir include/${quickjs_cross_arch})
else(quickjs_cross_arch)
  set(quickjs_libdir lib)
  set(quickjs_bindir bin)
  set(quickjs_includedir include)
endif(quickjs_cross_arch)

message("libdir = ${quickjs_libdir}")
message("bindir = ${quickjs_bindir}")
message("includedir = ${quickjs_includedir}")

