project(qjs-sound C CXX)

cmake_minimum_required(VERSION 3.19)

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
#message("MODULE_COMPILE_FLAGS: ${MODULE_COMPILE_FLAGS}")
 
include(ExternalProject)

ExternalProject_Add(
  LabSound
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/LabSound"
  BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/LabSound"
  PREFIX LabSound
  CMAKE_ARGS
    "-DBUILD_SHARED_LIBS:BOOL=OFF"
    "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
    "-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON"
    "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON"
    "-DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}"
    "-DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}"
    "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS} ${MODULE_COMPILE_FLAGS}"
    "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} ${MODULE_COMPILE_FLAGS}"
    "-D_LAB_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} ${MODULE_COMPILE_FLAGS}"
    #"-D_NQR_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} ${MODULE_COMPILE_FLAGS}"
    INSTALL_COMMAND "" )


ExternalProject_Get_property(LabSound BINARY_DIR)
set(LABSOUND_BINARY_DIR "${BINARY_DIR}")
message("Binary dir of LabSound = ${LABSOUND_BINARY_DIR}")

#add_subdirectory(LabSound)

include(CheckCCompilerFlag)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindQuickJS.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/QuickJSModule.cmake)

set(labsound_LINK_DIRECTORIES
    ${LABSOUND_BINARY_DIR}/bin
    ${LABSOUND_BINARY_DIR}/third_party/libnyquist/lib)

set(labsound_LIBRARIES
    ${LABSOUND_BINARY_DIR}/bin/libLabSound_d.a
    ${LABSOUND_BINARY_DIR}/bin/libLabSoundRtAudio_d.a
    ${LABSOUND_BINARY_DIR}/third_party/libnyquist/lib/liblibnyquist.a
    #LabSound_d libnyquist_d
    pulse-simple
    samplerate)

link_directories(${labsound_LINK_DIRECTORIES})


find_quickjs()
configure_quickjs()

make_module(labsound cpp)
