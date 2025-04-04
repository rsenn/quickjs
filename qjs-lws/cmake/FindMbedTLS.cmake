macro(find_mbedtls)
  message(STATUS "Finding MbedTLS library...")

  if(MBEDTLS_ROOT_DIR)
    # Disable re-rooting paths in find_path/find_library. This assumes MBEDTLS_ROOT_DIR is an absolute path.
    set(_EXTRA_FIND_ARGS "NO_DEFAULT_PATH")
  endif()

  find_path(MBEDTLS_INCLUDE_DIR NAMES mbedtls/ssl.h PATH_SUFFIXES include HINTS ${MBEDTLS_ROOT_DIR}
                                                                                ${_EXTRA_FIND_ARGS})

  # based on https://github.com/ARMmbed/mbedtls/issues/298
  if(MBEDTLS_INCLUDE_DIR AND EXISTS "${MBEDTLS_INCLUDE_DIR}/mbedtls/version.h")
    file(STRINGS "${MBEDTLS_INCLUDE_DIR}/mbedtls/version.h" VERSION_STRING_LINE
         REGEX "^#define MBEDTLS_VERSION_STRING[ \\t\\n\\r]+\"[^\"]*\"$")
    file(STRINGS "${MBEDTLS_INCLUDE_DIR}/mbedtls/version.h" VERSION_MAJOR_LINE
         REGEX "^#define MBEDTLS_VERSION_MAJOR[ \\t\\n\\r]+[0-9]+$")
    file(STRINGS "${MBEDTLS_INCLUDE_DIR}/mbedtls/version.h" VERSION_MINOR_LINE
         REGEX "^#define MBEDTLS_VERSION_MINOR[ \\t\\n\\r]+[0-9]+$")
    file(STRINGS "${MBEDTLS_INCLUDE_DIR}/mbedtls/version.h" VERSION_PATCH_LINE
         REGEX "^#define MBEDTLS_VERSION_PATCH[ \\t\\n\\r]+[0-9]+$")

    string(REGEX REPLACE "^#define MBEDTLS_VERSION_STRING[ \\t\\n\\r]+\"([^\"]*)\"$" "\\1"
                         MBEDTLS_VERSION "${VERSION_STRING_LINE}")
    string(REGEX REPLACE "^#define MBEDTLS_VERSION_MAJOR[ \\t\\n\\r]+([0-9]+)$" "\\1"
                         MBEDTLS_VERSION_MAJOR "${VERSION_MAJOR_LINE}")
    string(REGEX REPLACE "^#define MBEDTLS_VERSION_MINOR[ \\t\\n\\r]+([0-9]+)$" "\\1"
                         MBEDTLS_VERSION_MINOR "${VERSION_MINOR_LINE}")
    string(REGEX REPLACE "^#define MBEDTLS_VERSION_PATCH[ \\t\\n\\r]+([0-9]+)$" "\\1"
                         MBEDTLS_VERSION_PATCH "${VERSION_PATCH_LINE}")
  endif()

  if(MBEDTLS_USE_STATIC_LIBS)
    set(_MBEDTLS_LIB_NAME libmbedtls.a)
    set(_MBEDTLS_CRYPTO_LIB_NAME libmbedcrypto.a)
    set(_MBEDTLS_X509_LIB_NAME libmbedx509.a)
  else()
    set(_MBEDTLS_LIB_NAME mbedtls)
    set(_MBEDTLS_CRYPTO_LIB_NAME mbedcrypto)
    set(_MBEDTLS_X509_LIB_NAME mbedx509)
  endif()

  find_library(MBEDTLS_LIBRARY NAMES ${_MBEDTLS_LIB_NAME} PATH_SUFFIXES lib
               HINTS ${MBEDTLS_ROOT_DIR} ${_EXTRA_FIND_ARGS})
  find_library(MBEDTLS_CRYPTO_LIBRARY NAMES ${_MBEDTLS_CRYPTO_LIB_NAME} PATH_SUFFIXES lib
               HINTS ${MBEDTLS_ROOT_DIR} ${_EXTRA_FIND_ARGS})
  find_library(MBEDTLS_X509_LIBRARY NAMES ${_MBEDTLS_X509_LIB_NAME} PATH_SUFFIXES lib
               HINTS ${MBEDTLS_ROOT_DIR} ${_EXTRA_FIND_ARGS})

  set(MBEDTLS_LIBRARIES ${MBEDTLS_LIBRARY} ${MBEDTLS_CRYPTO_LIBRARY} ${MBEDTLS_X509_LIBRARY})

  if(MBEDTLS_INCLUDE_DIR AND MBEDTLS_LIBRARY)
    set(MBEDTLS_FOUND TRUE)
  endif(MBEDTLS_INCLUDE_DIR AND MBEDTLS_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    mbedTLS FOUND_VAR MBEDTLS_FOUND
    REQUIRED_VARS MBEDTLS_INCLUDE_DIR MBEDTLS_LIBRARY MBEDTLS_CRYPTO_LIBRARY MBEDTLS_X509_LIBRARY
                  MBEDTLS_LIBRARIES MBEDTLS_VERSION VERSION_VAR MBEDTLS_VERSION)

  if(MBEDTLS_FOUND)
    if(NOT TARGET mbedcrypto)
      add_library(mbedcrypto UNKNOWN IMPORTED)
      set_target_properties(
        mbedcrypto
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MBEDTLS_INCLUDE_DIR}"
                   IMPORTED_LINK_INTERFACE_LANGUAGES "C" IMPORTED_LOCATION
                                                         "${MBEDTLS_CRYPTO_LIBRARY}")
    endif()

    if(NOT TARGET mbedx509)
      add_library(mbedx509 UNKNOWN IMPORTED)
      set_target_properties(
        mbedx509
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MBEDTLS_INCLUDE_DIR}"
                   INTERFACE_LINK_LIBRARIES mbedcrypto
                   IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                   IMPORTED_LOCATION "${MBEDTLS_X509_LIBRARY}")
    endif()

    if(NOT TARGET mbedtls)
      add_library(mbedtls UNKNOWN IMPORTED)
      set_target_properties(
        mbedtls
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MBEDTLS_INCLUDE_DIR}" INTERFACE_LINK_LIBRARIES
                                                                          mbedx509
                   IMPORTED_LINK_INTERFACE_LANGUAGES "C" IMPORTED_LOCATION "${MBEDTLS_LIBRARY}")
    endif()

    get_target_property(MBEDTLS_LIBRARY_LOCATION mbedtls IMPORTED_LOCATION)

    if(EXISTS "${MBEDTLS_ROOT_DIR}/lib")
      set(LIBRARY_DIR "${MBEDTLS_ROOT_DIR}/lib")
    endif(EXISTS "${MBEDTLS_ROOT_DIR}/lib")

    if(NOT LIBRARY_DIR)
      string(REGEX REPLACE "/[^/]*$" "" LIBRARY_DIR "${MBEDTLS_LIBRARY}")
    endif(NOT LIBRARY_DIR)

    if(EXISTS "${LIBRARY_DIR}")
      rpath_append(CMAKE_INSTALL_RPATH "${LIBRARY_PATH}")
    endif(EXISTS "${LIBRARY_DIR}")

    set(MBEDTLS_LIBRARY_DIR "${LIBRARY_DIR}" CACHE PATH "MbedTLS library directory")
    set(MBEDTLS_INCLUDE_DIR "${MBEDTLS_INCLUDE_DIR}" CACHE PATH "MbedTLS include directory")

  else(MBEDTLS_FOUND)
    unset(MBEDTLS_LIBRARIES CACHE)
    unset(MBEDTLS_LIBRARY CACHE)
    unset(MBEDTLS_CRYPTO_LIBRARY CACHE)
    unset(MBEDTLS_X509_LIBRARY CACHE)
  endif(MBEDTLS_FOUND)

endmacro(find_mbedtls)
