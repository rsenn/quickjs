project(qjs-sound C CXX)

#add_subdirectory(stk)
#add_subdirectory(sndobj)

set(_LAB_CXX_FLAGS "${_LAB_CXX_FLAGS} -fPIC -w")
set(_NQR_CXX_FLAGS "${_NQR_CXX_FLAGS} -fPIC -w")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -w")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -w")

add_subdirectory(LabSound)


include(CheckCCompilerFlag)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/functions.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/check-flags.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindQuickJS.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/QuickJSModule.cmake)

link_directories(${CMAKE_CURRENT_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}/LabSound/third_party/libnyquist/lib)

set(labsound_LINK_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}/LabSound/third_party/libnyquist/lib)
set(labsound_LIBRARIES 
  ${CMAKE_CURRENT_BINARY_DIR}/bin/libLabSound_d.a ${CMAKE_CURRENT_BINARY_DIR}/bin/libLabSoundRtAudio_d.a  ${CMAKE_CURRENT_BINARY_DIR}/LabSound/third_party/libnyquist/lib/liblibnyquist_d.a
  #LabSound_d libnyquist_d
   pulse-simple samplerate)

check_flag("-fvisibility=hidden" VISIBILITY_HIDDEN MODULE_COMPILE_FLAGS)

find_quickjs()

make_module(labsound cpp)

