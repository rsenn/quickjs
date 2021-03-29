if(WIN32 OR MINGW)
  set(TRANSPORT_PLATFORM win)
else(WIN32 OR MINGW)
  set(TRANSPORT_PLATFORM unix)
endif(WIN32 OR MINGW)

set(quickjs_version "2020-11-08")
set(quickjs_soversion 1)
set(quickjs_url https://bellard.org/quickjs/quickjs-${quickjs_version}.tar.xz)
set(quickjs_sha1 371eae0896cc9e9f50864cb34f37d9481d843ce1)
set(quickjs_extras_url
    https://bellard.org/quickjs/quickjs-extras-${quickjs_version}.tar.xz)
set(quickjs_extras_sha1 211e43a5638668c80c8d438a3065660ab3af96df)
set(quickjs_extract_dir ${CMAKE_CURRENT_BINARY_DIR}/sources)
if(NOT quickjs_sources_root)
  set(quickjs_sources_root ${CMAKE_CURRENT_SOURCE_DIR})
endif(NOT quickjs_sources_root)
set(quickjs_sources
    ${quickjs_sources_root}/cutils.c
    ${quickjs_sources_root}/libbf.c
    ${quickjs_sources_root}/libregexp.c
    ${quickjs_sources_root}/libunicode.c
    ${quickjs_sources_root}/quickjs.c
    ${quickjs_sources_root}/quickjs-libc.c
    ${quickjs_sources_root}/quickjs-debugger.c
    ${quickjs_sources_root}/quickjs-debugger-transport-${TRANSPORT_PLATFORM}.c
    ${quickjs_sources_root}/quickjs-find-module.c)
set(quickjs_includes
    cutils.h
    libbf.h
    libregexp-opcode.h
    libregexp.h
    libunicode-table.h
    libunicode.h
    list.h
    quickjs-atom.h
    quickjs-libc.h
    quickjs-opcode.h
    quickjs.h
    unicode_gen_def.h)
