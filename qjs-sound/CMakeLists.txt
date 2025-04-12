project(qjs-sound C CXX)

cmake_minimum_required(VERSION 3.19)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/LabSound/include")
#add_subdirectory(stk)
#add_subdirectory(sndobj)

#[[set(_LAB_CXX_FLAGS "${_LAB_CXX_FLAGS} -fPIC -w")
set(_NQR_CXX_FLAGS "${_NQR_CXX_FLAGS} -fPIC -w")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -w")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -w")]]

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/functions.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/check-flags.cmake)

check_flag("-w" F_NOWARN MODULE_COMPILE_FLAGS)
check_flag("-fPIC" F_PIC MODULE_COMPILE_FLAGS)
check_flag("-fvisibility=hidden" VISIBILITY_HIDDEN MODULE_COMPILE_FLAGS)

#string(REGEX REPLACE "[\n\r;]+" " " MODULE_COMPILE_FLAGS "${MODULE_COMPILE_FLAGS}")
message("MODULE_COMPILE_FLAGS: ${MODULE_COMPILE_FLAGS}")

include(ExternalProject)

ExternalProject_Add(
  LabSound
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/LabSound"
  BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/LabSound"
  PREFIX LabSound
  CMAKE_ARGS "-DLABSOUND_NO_BUILD_EXAMPLES:BOOL=ON"
             "-DBUILD_SHARED_LIBS:BOOL=OFF"
             #"-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
             "-DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo"
             "-DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF"
             "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON"
             "-DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}"
             "-DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}"
             "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS} ${MODULE_COMPILE_FLAGS}"
             "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} ${MODULE_COMPILE_FLAGS}"
             "-DNYQUIST_COMPILE_FLAGS:STRING=-fPIC"
  INSTALL_COMMAND "")

ExternalProject_Get_Property(LabSound BINARY_DIR)
set(LABSOUND_BINARY_DIR "${BINARY_DIR}")
message("Binary dir of LabSound = ${LABSOUND_BINARY_DIR}")

#add_subdirectory(libLabSound)

include(CheckCCompilerFlag)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindQuickJS.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/QuickJSModule.cmake)

set(labsound_LINK_DIRECTORIES ${LABSOUND_BINARY_DIR}/bin ${LABSOUND_BINARY_DIR}/third_party/libnyquist/lib)

if(CMAKE_BUILD_TYPE MATCHES "Rel")
  #set(DEBUG_SUFFIX _d)
else(CMAKE_BUILD_TYPE MATCHES "Rel")
  set(DEBUG_SUFFIX "")
endif(CMAKE_BUILD_TYPE MATCHES "Rel")

add_library(libLabSound STATIC IMPORTED GLOBAL)
set_target_properties(
  libLabSound
  PROPERTIES IMPORTED_LOCATION "${LABSOUND_BINARY_DIR}/bin/libLabSound${DEBUG_SUFFIX}.a"
             IMPORTED_LOCATION_DEBUG "${LABSOUND_BINARY_DIR}/bin/libLabSound${DEBUG_SUFFIX}.a" IMPORTED_CONFIGURATIONS
                                                                                               "RELEASE;DEBUG")
add_library(libLabSoundRtAudio STATIC IMPORTED GLOBAL)
set_target_properties(
  libLabSoundRtAudio
  PROPERTIES IMPORTED_LOCATION "${LABSOUND_BINARY_DIR}/bin/libLabSoundRtAudio${DEBUG_SUFFIX}.a"
             IMPORTED_LOCATION_DEBUG "${LABSOUND_BINARY_DIR}/bin/libLabSoundRtAudio${DEBUG_SUFFIX}.a"
             IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")

link_directories(${LABSOUND_BINARY_DIR}/bin ${LABSOUND_BINARY_DIR}/third_party/libnyquist/lib)
set(labsound_LIBRARIES
    #  ${LABSOUND_BINARY_DIR}/bin/libLabSound${DEBUG_SUFFIX}.a
    #A ${LABSOUND_BINARY_DIR}/bin/libLabSoundRtAudio${DEBUG_SUFFIX}.a
    libLabSound libLabSoundRtAudio ${LABSOUND_BINARY_DIR}/third_party/libnyquist/lib/liblibnyquist.a
    #LabSound_d libnyquist_d
    pulse-simple samplerate)

link_directories(${labsound_LINK_DIRECTORIES})

find_quickjs()
configure_quickjs()
add_definitions(-fvisibility=hidden)

if(LINUX OR ANDROID)
  add_definitions(-D__LINUX_PULSE__)
endif()
file(GLOB COMMON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/stk/src/*.cpp)
list(FILTER COMMON_SOURCES EXCLUDE REGEX "Inet")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "Rt")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "Tcp")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "Udp")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "Messager")
list(FILTER COMMON_SOURCES EXCLUDE REGEX "Thread")
list(APPEND COMMON_SOURCES quickjs-stk.cpp)
dump(COMMON_SOURCES)
include_directories(stk/include stk/src/include/)
set(labsound_NAME sound)
make_module(labsound cpp)
#make_module(stk cpp)
