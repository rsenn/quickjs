macro(build_libressl)
  message("-- Building LIBRESSL from source")

  include(ExternalProject)

  if(NOT DEFINED LIBRESSL_DEBUG)
    message(FATAL_ERROR "Please set LIBRESSL_DEBUG to 'OFF' or 'ON' before including this file.")
  endif()

  if(NOT DEFINED LIBRESSL_SOURCE_DIR)
    message(FATAL_ERROR "Please set LIBRESSL_SOURCE_DIR before including this file.")
  endif()

  if(NOT DEFINED LIBRESSL_C_FLAGS)
    message(FATAL_ERROR "Please set LIBRESSL_C_FLAGS before including this file.")
  endif()

  if(NOT DEFINED LIBRESSL_TARGET_NAME)
    message(FATAL_ERROR "Please set LIBRESSL_TARGET_NAME before including this file.")
  endif()

  if(LIBRESSL_DEBUG)
    set(LIBRESSL_BUILD_TYPE "Debug")
  else()
    set(LIBRESSL_BUILD_TYPE "MinSizeRel")
  endif()

  if((NOT DEFINED LIBRESSL_PREINCLUDE_PREFIX) OR (NOT DEFINED LIBRESSL_PREINCLUDE_HEADER))
    message(STATUS "Building libreSSL without pre-included headers and global symbols prefixing.")
  else()
    set(LIBRESSL_PREINCLUDE_C_FLAGS
        " -DLIB_PREFIX_NAME=${LIBRESSL_PREINCLUDE_PREFIX} -include ${LIBRESSL_PREINCLUDE_HEADER}")
    string(APPEND LIBRESSL_C_FLAGS " ${LIBRESSL_PREINCLUDE_C_FLAGS}")
  endif()

  #string(APPEND LIBRESSL_C_FLAGS " ${CMAKE_C_FLAGS}")

  if("${ARM_CPU_ARCHITECTURE}" STREQUAL "ARMv8-M.BASE")
    string(APPEND LIBRESSL_C_FLAGS " -DMULADDC_CANNOT_USE_R7")
  endif()

  if(TARGET ${LIBRESSL_TARGET_NAME})
    message(
      FATAL_ERROR
        "A target with name ${LIBRESSL_TARGET_NAME} is already defined. Please set LIBRESSL_TARGET_NAME to a unique value."
    )
  endif()

  include(ExternalProject)

  set(LIBRESSL_VERSION 3.5.1)

  ExternalProject_Add(
    ${LIBRESSL_TARGET_NAME}
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libressl
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/libressl
    PREFIX libressl
    DOWNLOAD_NAME libressl-${LIBRESSL_VERSION}.tar.gz
    URL https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-${LIBRESSL_VERSION}.tar.gz
    CMAKE_ARGS "-DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}" "-DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}"
               "-DCMAKE_BUILD_TYPE=${LIBRESSL_BUILD_TYPE}"
    CMAKE_CACHE_ARGS
      "-DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}"
      "-DCMAKE_C_COMPILER_ID:STRING=${CMAKE_C_COMPILER_ID}"
      "-DCMAKE_C_FLAGS:STRING=${LIBRESSL_C_FLAGS}"
      "-DCMAKE_C_FLAGS_DEBUG:STRING=${CMAKE_C_FLAGS_DEBUG}"
      "-DCMAKE_C_FLAGS_MINSIZEREL:STRING=${CMAKE_C_FLAGS_MINSIZEREL}"
      "-DCMAKE_C_FLAGS_RELEASE:STRING=${CMAKE_C_FLAGS_RELEASE}"
      "-DCMAKE_C_OUTPUT_EXTENSION:STRING=.o"
      "-DCMAKE_C_COMPILER_WORKS:BOOL=TRUE"
      "-DCMAKE_AR:STRING=${CMAKE_AR}"
      "-DCMAKE_C_CREATE_STATIC_LIBRARY:INTERNAL=${CMAKE_C_CREATE_STATIC_LIBRARY}"
      "-DCMAKE_C_LINK_EXECUTABLE:STRING=${CMAKE_C_LINK_EXECUTABLE}"
      "-DCMAKE_STATIC_LIBRARY_PREFIX_C:STRING=${CMAKE_STATIC_LIBRARY_PREFIX_C}"
      "-DCMAKE_STATIC_LIBRARY_PREFIX_CXX:STRING=${CMAKE_STATIC_LIBRARY_PREFIX_CXX}"
      "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    #LOG_CONFIGURE ON
    LOG_BUILD ON)

  ExternalProject_Get_Property(${LIBRESSL_TARGET_NAME} SOURCE_DIR BINARY_DIR)

  if(ARGN)
    ExternalProject_Add_StepDependencies(${LIBRESSL_TARGET_NAME} build ${ARGN})
  endif(ARGN)

  ExternalProject_Add_Step(${LIBRESSL_TARGET_NAME} COMMAND ${CMAKE_COMMAND} --build . --target
                                                           clean)

  add_custom_target(
    ${LIBRESSL_TARGET_NAME}_clean COMMAND ${CMAKE_COMMAND} --build ${BINARY_DIR} --target clean
    WORKING_DIRECTORY "${BINARY_DIR}" COMMENT "Cleaning libressl" VERBATIM)
  #add_custom_target(${LIBRESSL_TARGET_NAME}_install COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libressl -- install WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libressl COMMENT "Installing libressl to ${CMAKE_INSTALL_PREFIX}" VERBATIM)
  #add_dependencies(${LIBRESSL_TARGET_NAME}_install ${LIBRESSL_TARGET_NAME})

  set(LIBRESSL_CRYPTO_LIBRARY libcrypto CACHE STRING "LibreSSL crypto library" FORCE)
  add_library(${LIBRESSL_CRYPTO_LIBRARY} STATIC IMPORTED)
  add_dependencies(${LIBRESSL_CRYPTO_LIBRARY} ${LIBRESSL_TARGET_NAME})
  set_property(TARGET ${LIBRESSL_CRYPTO_LIBRARY} PROPERTY IMPORTED_LOCATION
                                                          ${BINARY_DIR}/libcrypto.a)

  set(LIBRESSL_SSL_LIBRARY libssl CACHE STRING "LibreSSL ssl library" FORCE)
  add_library(${LIBRESSL_SSL_LIBRARY} STATIC IMPORTED)
  add_dependencies(${LIBRESSL_SSL_LIBRARY} ${LIBRESSL_TARGET_NAME})
  set_property(TARGET ${LIBRESSL_SSL_LIBRARY} PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/libssl.a)

  set(LIBRESSL_TLS_LIBRARY libtls CACHE STRING "LibreSSL tls library" FORCE)
  add_library(${LIBRESSL_TLS_LIBRARY} STATIC IMPORTED)
  add_dependencies(${LIBRESSL_TLS_LIBRARY} ${LIBRESSL_TARGET_NAME})
  set_property(TARGET ${LIBRESSL_TLS_LIBRARY} PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/libtls.a)

  set(LIBRESSL_LIBRARIES
      "${LIBRESSL_TLS_LIBRARY};${LIBRESSL_SSL_LIBRARY};${LIBRESSL_CRYPTO_LIBRARY}"
      CACHE STRING "LibreSSL libraries" FORCE)

  set(LIBRESSL_LIBRARY_DIR "${BINARY_DIR}" CACHE PATH "LibreSSL library directory" FORCE)
  set(LIBRESSL_INCLUDE_DIR "${SOURCE_DIR}/include" CACHE PATH "LibreSSL include directory" FORCE)

  message(STATUS "LibreSSL libraries: ${LIBRESSL_LIBRARIES}")
  message(STATUS "LibreSSL library directory: ${LIBRESSL_LIBRARY_DIR}")
  message(STATUS "LibreSSL include directory: ${LIBRESSL_INCLUDE_DIR}")

endmacro(build_libressl)
