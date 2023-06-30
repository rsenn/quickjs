if(WIN32 OR MINGW)
  set(TRANSPORT_PLATFORM win)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
else(WIN32 OR MINGW)
  set(TRANSPORT_PLATFORM unix)
endif(WIN32 OR MINGW)

if(NOT QUICKJS_SOURCES_ROOT)
  set(QUICKJS_SOURCES_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
endif(NOT QUICKJS_SOURCES_ROOT)

file(READ "${QUICKJS_SOURCES_ROOT}/VERSION" version)
string(STRIP "${version}" QUICKJS_VERSION)

if(NOT QUICKJS_VERSION)
  set(QUICKJS_VERSION "2021-03-27")
endif(NOT QUICKJS_VERSION)

if(NOT QUICKJS_PREFIX)
  set(QUICKJS_PREFIX "${CMAKE_INSTALL_PREFIX}")
endif(NOT QUICKJS_PREFIX)
set(QUICKJS_SOVERSION 1.0)
set(QUICKJS_URL https://bellard.org/quickjs/quickjs-${QUICKJS_VERSION}.tar.xz)
set(QUICKJS_SHA1 371eae0896cc9e9f50864cb34f37d9481d843ce1)
set(QUICKJS_EXTRAS_URL https://bellard.org/quickjs/quickjs-extras-${QUICKJS_VERSION}.tar.xz)
set(QUICKJS_EXTRAS_SHA1 211e43a5638668c80c8d438a3065660ab3af96df)
set(QUICKJS_EXTRACT_DIR ${CMAKE_CURRENT_BINARY_DIR}/sources)
if(NOT QUICKJS_SOURCES_ROOT)
  set(QUICKJS_SOURCES_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
endif(NOT QUICKJS_SOURCES_ROOT)
set(QUICKJS_INCLUDES cutils.h libbf.h libregexp-opcode.h libregexp.h libunicode-table.h libunicode.h list.h quickjs-atom.h quickjs-libc.h quickjs-opcode.h quickjs-debugger.h
                     quickjs.h unicode_gen_def.h)

set(QUICKJS_SOURCES
    ${QUICKJS_SOURCES_ROOT}/cutils.c ${QUICKJS_SOURCES_ROOT}/libbf.c ${QUICKJS_SOURCES_ROOT}/libregexp.c ${QUICKJS_SOURCES_ROOT}/libunicode.c ${QUICKJS_SOURCES_ROOT}/quickjs.c
    ${QUICKJS_SOURCES_ROOT}/quickjs-libc.c ${QUICKJS_SOURCES_ROOT}/quickjs-find-module.c ${QUICKJS_SOURCES_ROOT}/quickjs-debugger-transport-${TRANSPORT_PLATFORM}.c
    ${QUICKJS_SOURCES_ROOT}/win32-poll.c ${QUICKJS_INCLUDES})

#message("CONFIG_DEBUGGER = ${CONFIG_DEBUGGER}")
if(CONFIG_DEBUGGER)
  set(QUICKJS_SOURCES_DEBUGGER ${QUICKJS_SOURCES_ROOT}/quickjs-debugger.c ${QUICKJS_SOURCES_ROOT}/quickjs-debugger-transport-${TRANSPORT_PLATFORM}.c)
  #set(QUICKJS_SOURCES ${QUICKJS_SOURCES} ${QUICKJS_SOURCES_ROOT}/quickjs-debugger.c ${QUICKJS_SOURCES_ROOT}/quickjs-debugger-transport-${TRANSPORT_PLATFORM}.c)
  message(STATUS "Enabling quickjs-debugger")
endif(CONFIG_DEBUGGER)

if(WIN32)
  set(QUICKJS_SOURCES ${QUICKJS_SOURCES} quickjs.def)
endif(WIN32)

string(REPLACE ";" "\n" sources "${QUICKJS_SOURCES}")

#message("QUICKJS_SOURCES = ${QUICKJS_SOURCES}")
# dump(QUICKJS_SOURCES)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/UseMultiArch.cmake)

string(REGEX REPLACE "-pc-" "-" QUICKJS_HOST_ARCH "${SYSTEM_NAME}")

if(NOT "${HOST_SYSTEM_NAME}" STREQUAL "${QUICKJS_HOST_ARCH}")
  string(REGEX REPLACE i686 i386 QUICKJS_CROSS_ARCH "${QUICKJS_HOST_ARCH}")
  # set(QUICKJS_CROSS_ARCH ${SYSTEM_NAME})
 else(NOT "${HOST_SYSTEM_NAME}" STREQUAL "${QUICKJS_HOST_ARCH}")
  set(QUICKJS_CROSS_ARCH "")
endif(NOT "${HOST_SYSTEM_NAME}" STREQUAL "${QUICKJS_HOST_ARCH}")

# message("HOST_SYSTEM_NAME = ${HOST_SYSTEM_NAME}") message("SYSTEM_NAME = ${SYSTEM_NAME}") message("QUICKJS_CROSS_ARCH = ${QUICKJS_CROSS_ARCH}")

set(QUICKJS_BINARY_DIR ${QUICKJS_PREFIX}/bin)
set(QUICKJS_INCLUDE_DIR ${QUICKJS_PREFIX}/include)
if(QUICKJS_CROSS_ARCH)
  set(QUICKJS_LIBRARY_DIR ${QUICKJS_PREFIX}/lib/${QUICKJS_CROSS_ARCH})
  #set(QUICKJS_BINARY_DIR ${QUICKJS_PREFIX}/bin/${QUICKJS_CROSS_ARCH})
  #set(QUICKJS_INCLUDE_DIR ${QUICKJS_PREFIX}/include/${QUICKJS_CROSS_ARCH})
else(QUICKJS_CROSS_ARCH)
  set(QUICKJS_LIBRARY_DIR ${QUICKJS_PREFIX}/lib)
endif(QUICKJS_CROSS_ARCH)

# message("libdir = ${QUICKJS_LIBRARY_DIR}") message("bindir = ${QUICKJS_BINARY_DIR}") message("includedir = ${QUICKJS_INCLUDE_DIR}")

#option(USE_WORKER "Enable worker support" ON)

set(CONFIG_VERSION "${QUICKJS_VERSION}" CACHE STRING "QuickJS version")

dump(CMAKE_SHARED_LIBRARY_SUFFIX)
set(CONFIG_SHEXT "${CMAKE_SHARED_LIBRARY_SUFFIX}" CACHE STRING "Shared module extension")

file(
  WRITE "${CMAKE_CURRENT_BINARY_DIR}/quickjs.pc"
  "prefix=${QUICKJS_PREFIX}\nexec_prefix=\${prefix}\nlibdir=\${exec_prefix}/lib\nincludedir=\${prefix}/include\n\nName: quickjs\nDescription: QuickJS\nVersion: ${QUICKJS_VERSION}\nLibs: -L\${libdir} -lquickjs\nCflags: -I\${includedir}\n"
)
