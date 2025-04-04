macro(build_mbedtls)
  message("-- Building MBEDTLS from source")

  include(ExternalProject)

  if(NOT DEFINED MBEDTLS_DEBUG)
    message(FATAL_ERROR "Please set MBEDTLS_DEBUG to 'OFF' or 'ON' before including this file.")
  endif()

  if(NOT DEFINED MBEDTLS_SOURCE_DIR)
    message(FATAL_ERROR "Please set MBEDTLS_SOURCE_DIR before including this file.")
  endif()

  if(NOT DEFINED MBEDTLS_C_FLAGS)
    message(FATAL_ERROR "Please set MBEDTLS_C_FLAGS before including this file.")
  endif()

  if(NOT DEFINED MBEDTLS_TARGET_NAME)
    message(FATAL_ERROR "Please set MBEDTLS_TARGET_NAME before including this file.")
  endif()

  if(MBEDTLS_DEBUG)
    set(MBEDTLS_BUILD_TYPE "Debug")
  else()
    set(MBEDTLS_BUILD_TYPE "MinSizeRel")
  endif()

  if((NOT DEFINED MBEDTLS_PREINCLUDE_PREFIX) OR (NOT DEFINED MBEDTLS_PREINCLUDE_HEADER))
    message(STATUS "Building mbedTLS without pre-included headers and global symbols prefixing.")
  else()
    set(MBEDTLS_PREINCLUDE_C_FLAGS
        " -DLIB_PREFIX_NAME=${MBEDTLS_PREINCLUDE_PREFIX} -include ${MBEDTLS_PREINCLUDE_HEADER}")
    string(APPEND MBEDTLS_C_FLAGS " ${MBEDTLS_PREINCLUDE_C_FLAGS}")
  endif()

  #string(APPEND MBEDTLS_C_FLAGS " ${CMAKE_C_FLAGS}")

  if("${ARM_CPU_ARCHITECTURE}" STREQUAL "ARMv8-M.BASE")
    string(APPEND MBEDTLS_C_FLAGS " -DMULADDC_CANNOT_USE_R7")
  endif()

  if(TARGET ${MBEDTLS_TARGET_NAME})
    message(
      FATAL_ERROR
        "A target with name ${MBEDTLS_TARGET_NAME} is already defined. Please set MBEDTLS_TARGET_NAME to a unique value."
    )
  endif()

  include(ExternalProject)

  set(MBEDTLS_VERSION 2.27.0)

  ExternalProject_Add(
    ${MBEDTLS_TARGET_NAME}
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/mbedtls
    PREFIX mbedtls
    DOWNLOAD_NAME mbedtls-${MBEDTLS_VERSION}.tar.gz
    URL https://github.com/ARMmbed/mbedtls/archive/refs/tags/v${MBEDTLS_VERSION}.tar.gz
    CMAKE_ARGS -DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF "-DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}"
               "-DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}" "-DCMAKE_BUILD_TYPE=${MBEDTLS_BUILD_TYPE}"
    CMAKE_CACHE_ARGS
      "-DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}"
      "-DCMAKE_C_COMPILER_ID:STRING=${CMAKE_C_COMPILER_ID}"
      "-DCMAKE_C_FLAGS:STRING=${MBEDTLS_C_FLAGS}"
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

  ExternalProject_Get_Property(${MBEDTLS_TARGET_NAME} SOURCE_DIR BINARY_DIR)

  if(ARGN)
    ExternalProject_Add_StepDependencies(${MBEDTLS_TARGET_NAME} build ${ARGN})
  endif(ARGN)

  ExternalProject_Add_Step(${MBEDTLS_TARGET_NAME} COMMAND ${CMAKE_COMMAND} --build . --target clean)

  add_custom_target(
    ${MBEDTLS_TARGET_NAME}_clean COMMAND ${CMAKE_COMMAND} --build ${BINARY_DIR} --target clean
    WORKING_DIRECTORY "${BINARY_DIR}" COMMENT "Cleaning mbedtls" VERBATIM)
  #add_custom_target(${MBEDTLS_TARGET_NAME}_install COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/mbedtls -- install WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/mbedtls COMMENT "Installing mbedtls to ${CMAKE_INSTALL_PREFIX}" VERBATIM)
  #add_dependencies(${MBEDTLS_TARGET_NAME}_install ${MBEDTLS_TARGET_NAME})

  set(MBEDTLS_LIBRARY mbedtls CACHE STRING "MbedTLS library" FORCE)
  add_library(${MBEDTLS_LIBRARY} STATIC IMPORTED)
  add_dependencies(${MBEDTLS_LIBRARY} ${MBEDTLS_TARGET_NAME})
  set_property(TARGET ${MBEDTLS_LIBRARY} PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/libmbedtls.a)

  set(MBEDTLS_X509_LIBRARY mbedx509 CACHE STRING "MbedTLS x509 library" FORCE)
  add_library(${MBEDTLS_X509_LIBRARY} STATIC IMPORTED)
  add_dependencies(${MBEDTLS_X509_LIBRARY} ${MBEDTLS_TARGET_NAME})
  set_property(TARGET ${MBEDTLS_X509_LIBRARY} PROPERTY IMPORTED_LOCATION
                                                       ${BINARY_DIR}/libmbedx509.a)

  set(MBEDTLS_CRYPTO_LIBRARY mbedcrypto CACHE STRING "MbedTLS crypto library" FORCE)
  add_library(${MBEDTLS_CRYPTO_LIBRARY} STATIC IMPORTED)
  add_dependencies(${MBEDTLS_CRYPTO_LIBRARY} ${MBEDTLS_TARGET_NAME})
  set_property(TARGET ${MBEDTLS_CRYPTO_LIBRARY} PROPERTY IMPORTED_LOCATION
                                                         ${BINARY_DIR}/libmbedcrypto.a)

  set(MBEDTLS_LIBRARIES "${MBEDTLS_LIBRARY};${MBEDTLS_CRYPTO_LIBRARY};${MBEDTLS_X509_LIBRARY}"
      CACHE STRING "MbedTLS libraries" FORCE)

  set(MBEDTLS_LIBRARY_DIR "${BINARY_DIR}" CACHE PATH "MbedTLS library directory" FORCE)
  set(MBEDTLS_INCLUDE_DIR "${SOURCE_DIR}/include" CACHE PATH "MbedTLS include directory" FORCE)

  message(STATUS "MbedTLS libraries: ${MBEDTLS_LIBRARIES}")
  message(STATUS "MbedTLS library directory: ${MBEDTLS_LIBRARY_DIR}")
  message(STATUS "MbedTLS include directory: ${MBEDTLS_INCLUDE_DIR}")

endmacro(build_mbedtls)
