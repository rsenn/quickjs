include(CheckLibraryExists)

check_library_exists(pthread pthread_create /usr/lib HAVE_LIBPTHREAD)
if(HAVE_LIBPTHREAD)
  set(LIBPTHREAD pthread)
endif(HAVE_LIBPTHREAD)

check_library_exists(m atan2 /usr/lib HAVE_LIBM)
if(HAVE_LIBM)
  set(LIBM m)
endif(HAVE_LIBM)

check_library_exists(dl dlopen /usr/lib HAVE_LIBDL)
if(HAVE_LIBDL)
  set(LIBDL dl)
endif(HAVE_LIBDL)
