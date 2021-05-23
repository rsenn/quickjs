# the name of the target operating system
set(CMAKE_SYSTEM_NAME GNU)

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

set(CMAKE_C_COMPILER /opt/wasienv/bin/wasicc)
set(CMAKE_C_COMPILER_AR /opt/wasienv/bin/wasiar)
set(CMAKE_C_COMPILER_RANLIB /opt/wasienv/bin/wasiranlib)
set(CMAKE_CXX_COMPILER /opt/wasienv/bin/wasic++)
set(CMAKE_LINKER /opt/wasienv/bin/wasild)
set(CMAKE_NM /opt/wasienv/bin/wasinm)
set(CMAKE_AR /opt/wasienv/bin/wasiar)

# which compilers to use for C and C++
include(CMakeForceCompiler)
cmake_force_c_compiler(/opt/wasienv/bin/wasicc GNU)
cmake_force_cxx_compiler(/opt/wasienv/bin/wasic++ GNU)
set(CMAKE_RANLIB /opt/wasienv/bin/wasiranlib)
set(CMAKE_AR /opt/wasienv/bin/wasiar)

# here is the target environment located
set(CMAKE_FIND_ROOT_PATH /opt/wasienv /usr/wasm32-unknown-wasi/sysroot/usr /usr/wasm32-unknown-wasi/sysroot/usr /usr/wasm32-unknown-wasi)
set(CMAKE_MODULE_PATH /opt/wasienv/lib/cmake)
set(CMAKE_PREFIX_PATH /opt/wasienv)
set(CMAKE_SYSTEM_PREFIX_PATH /opt/wasienv)

# adjust the default behaviour of the FIND_XXX() commands: search headers and libraries in the target environment, search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_FIND_LIBRARY_PREFIXES "lib" "")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".wasm" ".lib" ".a")
set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "")

set(CMAKE_CROSSCOMPILING TRUE)

set(WASI 1)
