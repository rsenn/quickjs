if(UNIX AND NOT APPLE)
  include(GNUInstallDirs)
endif()

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR lib
      CACHE STRING
            "Specify the output directory for libraries (default is lib)")
endif()

function(GET_SYSTEM_NAME HOST_SYSTEM_VAR TARGET_SYSTEM_VAR)
  if(MSVC OR DEFINED ENV{VSCMD_ARG_TGT_ARCH})
    set(HOST_SYSTEM $ENV{VSCMD_ARG_HOST_ARCH})
    set(TARGET_SYSTEM $ENV{VSCMD_ARG_TGT_ARCH})
  else(MSVC OR DEFINED ENV{VSCMD_ARG_TGT_ARCH})
    execute_process(COMMAND cc -dumpmachine OUTPUT_VARIABLE HOST_SYSTEM
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(
      COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE TARGET_SYSTEM
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif(MSVC OR DEFINED ENV{VSCMD_ARG_TGT_ARCH})

  set("${HOST_SYSTEM_VAR}" "${HOST_SYSTEM}" PARENT_SCOPE)
  set("${TARGET_SYSTEM_VAR}" "${TARGET_SYSTEM}" PARENT_SCOPE)
endfunction(GET_SYSTEM_NAME HOST_SYSTEM_VAR TARGET_SYSTEM_VAR)

get_system_name(HOST_SYSTEM_NAME SYSTEM_NAME)

if(NOT CMAKE_ARCH_LIBDIR)
  if(NOT SYSTEM_NAME)
    if(MSVC OR DEFINED ENV{VSCMD_ARG_TGT_ARCH})
      set(HOST_SYSTEM_NAME $ENV{VSCMD_ARG_HOST_ARCH})
      set(SYSTEM_NAME $ENV{VSCMD_ARG_TGT_ARCH})
    else(MSVC OR DEFINED ENV{VSCMD_ARG_TGT_ARCH})
      execute_process(COMMAND cc -dumpmachine OUTPUT_VARIABLE HOST_SYSTEM_NAME
                      OUTPUT_STRIP_TRAILING_WHITESPACE)
      execute_process(
        COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE SYSTEM_NAME
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif(MSVC OR DEFINED ENV{VSCMD_ARG_TGT_ARCH})
  endif(NOT SYSTEM_NAME)

  if(CMAKE_CROSSCOMPILING)
    if(NOT "${HOST_SYSTEM_NAME}" STREQUAL "${SYSTEM_NAME}")
      string(REGEX REPLACE i686 i386 CMAKE_CROSS_ARCH "${SYSTEM_NAME}")
    endif()

    set(CMAKE_CROSS_ARCH "${CMAKE_CROSS_ARCH}" CACHE STRING
                                                     "Cross compiling target")
  endif()

  string(REGEX REPLACE "-unknown-" "-" SYSTEM_NAME "${SYSTEM_NAME}")
  string(REGEX REPLACE "android[0-9]*" android SYSTEM_NAME "${SYSTEM_NAME}")

  if(SYSTEM_NAME AND NOT "${SYSTEM_NAME}" STREQUAL "")
    set(CMAKE_INSTALL_LIBDIR lib/${SYSTEM_NAME})
    set(CMAKE_ARCH_LIBDIR lib/${SYSTEM_NAME}
        CACHE STRING "Architecture specific libraries")
  endif(SYSTEM_NAME AND NOT "${SYSTEM_NAME}" STREQUAL "")
endif(NOT CMAKE_ARCH_LIBDIR)

#message("${CMAKE_C_COMPILER}: ${CMAKE_C_COMPILER}")
message("Architecture-specific library directory: ${CMAKE_ARCH_LIBDIR}")

if(SYSTEM_NAME MATCHES "diet")
  set(DIET TRUE CACHE BOOL "dietlibc")
  set(LIBDL "" CACHE STRING "dl library" FORCE)
endif(SYSTEM_NAME MATCHES "diet")
if(DIET)
  set(BUILD_SHARED_LIBS OFF)
endif(DIET)
