prefix=/usr
exec_prefix=/usr
libdir=${exec_prefix}/lib/i386-linux-gnu
includedir=${prefix}/include/i386-linux-gnu

Name: libquickjs
Description: QuickJS Javascript Engine
Version: 2020-11-08
Libs: -L${libdir} -lquickjs
Cflags: -I${includedir}
