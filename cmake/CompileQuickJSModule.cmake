function(compile_module SOURCE)
  basename(BASE "${SOURCE}" .js)
  message(STATUS "Compile QuickJS module '${BASE}.c' from '${SOURCE}'")

  if(ARGN)
    set(OUTPUT_FILE ${ARGN})
  else(ARGN)
    set(OUTPUT_FILE ${BASE}.c)
  endif(ARGN)
  add_custom_command(
    OUTPUT "${OUTPUT_FILE}"
    COMMAND qjsc -v -c -o "${OUTPUT_FILE}" -m "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}"
    DEPENDS ${QJSC_DEPS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Generate ${OUTPUT_FILE} from ${SOURCE} using qjs compiler" SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}
    DEPENDS qjs-inspect qjs-misc)

endfunction(compile_module SOURCE)
