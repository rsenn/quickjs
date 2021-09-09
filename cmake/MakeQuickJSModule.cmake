function(make_module FNAME)
  message(STATUS "Building QuickJS module: ${FNAME}")
  string(REGEX REPLACE "_" "-" NAME "${FNAME}")
  string(REGEX REPLACE "-" "_" VNAME "${FNAME}")
  string(TOUPPER "${FNAME}" UUNAME)
  string(REGEX REPLACE "-" "_" UNAME "${UUNAME}")

  set(TARGET_NAME qjs-${NAME})

  if(ARGN)
    set(SOURCES ${ARGN})
  else(ARGN)
    set(SOURCES quickjs-${NAME}.c ${${VNAME}_SOURCES})
  endif(ARGN)

  # dump(VNAME ${VNAME}_SOURCES)
  add_library(${TARGET_NAME} SHARED ${SOURCES})
  add_library(${TARGET_NAME}-static STATIC ${SOURCES})

  set_target_properties(
    ${TARGET_NAME}
    PROPERTIES
      PREFIX ""
      RPATH "${OPENCV_LIBRARY_DIRS}:${QUICKJS_PREFIX}/lib:${QUICKJS_PREFIX}/lib/quickjs"
      OUTPUT_NAME "${VNAME}"
      BUILD_RPATH
      "${CMAKE_CURRENT_BINARY_DIR}:${CMAKE_CURRENT_BINARY_DIR}:${CMAKE_CURRENT_BINARY_DIR}/quickjs:${CMAKE_CURRENT_BINARY_DIR}/quickjs"
      COMPILE_FLAGS "${MODULE_COMPILE_FLAGS}")
  set_target_properties(${TARGET_NAME}-static PROPERTIES OUTPUT_NAME "${VNAME}" COMPILE_FLAGS "")
  target_compile_definitions(${TARGET_NAME} PRIVATE JS_SHARED_LIBRARY=1 JS_${UNAME}_MODULE=1
                                                    CONFIG_PREFIX="${QUICKJS_INSTALL_PREFIX}")
  target_compile_definitions(${TARGET_NAME}-static PRIVATE JS_${UNAME}_MODULE=1
                                                           CONFIG_PREFIX="${QUICKJS_INSTALL_PREFIX}")
  install(TARGETS ${TARGET_NAME} DESTINATION lib/quickjs
          PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ
                      WORLD_EXECUTE)
  # install(TARGETS ${TARGET_NAME}-static DESTINATION lib/quickjs)

  config_module(${TARGET_NAME})
endfunction()
