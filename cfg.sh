cfg() {
  : ${build:=`gcc -dumpmachine`}
  : ${VERBOSE:=OFF}

  if [ -n "$TOOLCHAIN" ]; then
    toolchain=`basename "$TOOLCHAIN" .cmake`
    compilers=$(grep -i compiler $TOOLCHAIN |sed 's,.*compiler\s*(\([^ ]*\)[^)]*).*,\1,p' -n)
    sysroot=$(sed -n '/SYSROOT/ { \|^\s*/| {   s,^\s*,,; p; q } }' $TOOLCHAIN; sed -n '\|^\s*/| {   s,^\s*,,; p; q }' $TOOLCHAIN)
    if [ -n "$sysroot" -a -d "$sysroot" ]; then
      SYSROOT="$sysroot"
      : ${prefix:="$sysroot"}
      echo "SYSROOT $SYSROOT" 1>&2
    fi
    for c in $compilers; do 
      h=$("$c" -dumpmachine )
      if [ -z "$host" ]; then
        [ -n "$h" ] && host=$h
        [ -n "$h" ] && CC="$c"
     fi
    done
    if [ -e "$TOOLCHAIN" ]; then
      cmakebuild=$(basename "$TOOLCHAIN" .cmake)
      cmakebuild=${cmakebuild%.toolchain}
      cmakebuild=${cmakebuild#toolchain-}
    fi
  fi

 (if [ -z "$host" ]; then
    host=$build
    case "$host" in
      *musl*) host="$host" prefix=/opt/musl ;;
      *diet*) host="$host" prefix=/opt/diet ;;
      x86_64-w64-mingw32) host="$host" prefix=/mingw64 ;;
      i686-w64-mingw32) host="$host" prefix=/mingw32 ;;
      x86_64-pc-*) host="$host" prefix=/usr ;;
      i686-pc-*) host="$host" prefix=/usr ;;
    esac
  fi
  : ${prefix:=/usr/local}
  : ${libdir:=$prefix/lib}
  [ -d "$libdir/$host" ] && libdir=$libdir/$host

  case $(uname -o) in
   # MSys|MSYS|Msys) SYSTEM="MSYS" ;;
    *) SYSTEM="Unix" ;;
  esac

  case "$STATIC:$TYPE" in
    YES:*|yes:*|y:*|1:*|ON:*|on:* | *:*[Ss]tatic*) set -- "$@" \
      -DENABLE_PIC=OFF ;;
  esac
  if [ -z "$generator" ]; then
#    if type ninja 2>/dev/null; then
#      builddir=$builddir-ninja
#      generator="Ninja"
#    else
      generator="Unix Makefiles"
#    fi
  fi
  case "$generator" in
    *" - "*) break ;;
    *)
  if type codelite 2>/dev/null; then
    generator="Sublime Text 2 - $generator"
  fi
  ;;
  esac

  if [ -n "$host" ]; then
    case "$host" in
      x86_64*) machine=x64 ;;
      i[3-6]86*) machine=x86 ;;
      aarch64*) machine=arm64 ;;
      arm-*) machine=arm ;;
      *) machine=${host%%-*} ;;
    esac
    case "$host" in
      *-linux-*) os=linux ;;
      *-mingw32*) os=mingw32 ;;
      *-msys*) os=msys ;;
      *) os=${host#*-}; os=${os%%-*} ;;
    esac
  fi
    case "$SHARED" in
      TRUE|ON|YES|1) link=shared ;;
      *) link=static SHARED=OFF ;;
    esac
    : ${TYPE:=RelWithDebInfo}
    case "$TYPE" in
      *Debug*) type=debug suffix=debug symbols=debug ;;
      *Deb*) type=relwithdebinfo suffix=relwithdeb symbols=debug ;;
      *MinSize*) type=minsizerel suffix=minsize symbols= ;;
      *) type=release suffix=release symbols= ;;
    esac
    case "$suffix" in
      relwithdeb*) unset suffix ;;
    esac
    if [ -z "$CC" ]; then
      for compiler in gcc clang cc; do
        if type $compiler >/dev/null 2>/dev/null; then
          CC="$compiler"
          break
        fi
      done
    fi
    : ${builddir:=build/$os-$machine-$link${suffix:+-$suffix}}
 (mkdir -p $builddir
  : ${relsrcdir=`realpath --relative-to "$builddir" .`}
  set -x
  cd $builddir
  ${CMAKE:-cmake} -Wno-dev \
    -G "$generator" \
    ${SHARED:+-DBUILD_SHARED_LIBS=$SHARED} \
    ${VERBOSE:+-DCMAKE_VERBOSE_MAKEFILE=$VERBOSE} \
    -DCMAKE_BUILD_TYPE="${TYPE}" \
    ${CC:+-DCMAKE_C_COMPILER="$CC"} \
    ${CXX:+-DCMAKE_CXX_COMPILER="$CXX"} \
    ${PKG_CONFIG:+-DPKG_CONFIG_EXECUTABLE="$PKG_CONFIG"} \
    ${TOOLCHAIN:+-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN"} \
    ${CC:+-DCMAKE_C_COMPILER="$CC"} \
    ${CXX:+-DCMAKE_CXX_COMPILER="$CXX"} \
    ${MAKE:+-DCMAKE_MAKE_PROGRAM="$MAKE"} \
    ${prefix:+-DCMAKE_INSTALL_PREFIX="$prefix"} \
    "$@" \
    $relsrcdir 2>&1 ) |tee "${builddir##*/}.log")
}

cfg-android ()
{
  (: ${builddir=build/android}
    cfg \
  -DCMAKE_INSTALL_PREFIX=/opt/arm-linux-androideabi/sysroot/usr \
  -DCMAKE_VERBOSE_MAKEFILE=TRUE \
  -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN:-/opt/android-cmake/android.cmake} \
  -DANDROID_NATIVE_API_LEVEL=21 \
  -DPKG_CONFIG_EXECUTABLE=arm-linux-androideabi-pkg-config \
  -DCMAKE_PREFIX_PATH=/opt/arm-linux-androideabi/sysroot/usr \
  -DCMAKE_MAKE_PROGRAM=/usr/bin/make \
   -DCMAKE_MODULE_PATH="/opt/OpenCV-3.4.1-android-sdk/sdk/native/jni/abi-armeabi-v7a" \
   -DOpenCV_DIR="/opt/OpenCV-3.4.1-android-sdk/sdk/native/jni/abi-armeabi-v7a" \
   "$@"
    )
}

cfg-diet() {
 (: ${build=$(${CC:-gcc} -dumpmachine)}
  : ${host=${build/-gnu/-diet}}
  : ${prefix=/opt/diet}
  : ${libdir=/opt/diet/lib-${host%%-*}}
  : ${bindir=/opt/diet/bin-${host%%-*}}

  : ${CC="diet-gcc"}
  export CC

  builddir=build/${host%-*}-diet \
  PKG_CONFIG="PKG_CONFIG_PATH=$libdir/pkgconfig pkg-config" \
  cfg \
    -DCMAKE_INSTALL_PREFIX="$prefix" \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
      ${launcher:+-DCMAKE_C_COMPILER_LAUNCHER="$launcher"} \
    "$@")
}

cfg-diet64() {
 (build=$(gcc -dumpmachine)
  host=${build%%-*}-linux-diet
  host=x86_64-${host#*-}

  builddir=build/$host \
  CC="diet-gcc" \
  cfg-diet \
  "$@")
}

cfg-diet32() {
 (build=$(gcc -dumpmachine)
  host=${build%%-*}-linux-diet
  host=i686-${host#*-}

  builddir=build/$host \
  CFLAGS="-m32" \
  launcher="/opt/diet/bin-i386/diet" \
  cfg-diet \
  "$@")
}

cfg-mingw() {
 (build=$(gcc -dumpmachine)
  : ${host=${build%%-*}-w64-mingw32}
  : ${prefix=/usr/$host/sys-root/mingw}

  test -s /usr/x86_64-w64-mingw32/sys-root/toolchain-mingw64.cmake &&
  TOOLCHAIN=/usr/x86_64-w64-mingw32/sys-root/toolchain-mingw64.cmake

  builddir=build/$host \
  bindir=$prefix/bin \
  libdir=$prefix/lib \
  cfg \
    "$@")
}
cfg-emscripten() {
 (build=$(${CC:-emcc} -dumpmachine)
  host=${build/-gnu/-emscriptenlibc}
  : ${builddir=build/${host%-*}-emscripten}
  : ${prefix=/opt/emsdk/emscripten/incoming/system}
  : ${libdir=/opt/emsdk/emscripten/incoming/system/lib}
  : ${bindir=/opt/emsdk/emscripten/incoming/system/bin}

  CC="emcc" \
  PKG_CONFIG="PKG_CONFIG_PATH=$libdir/pkgconfig pkg-config" \
  cfg \
    -DCMAKE_INSTALL_PREFIX="$prefix" \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    "$@")
}

cfg-tcc() {
 (build=$(cc -dumpmachine)
  host=${build/-gnu/-tcc}
  builddir=build/$host
  prefix=/usr
  includedir=/usr/lib/$build/tcc/include
  libdir=/usr/lib/$build/tcc/
  bindir=/usr/bin

  CC=${TCC:-tcc} \
  cfg \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    "$@")
}

cfg-musl() {
 (: ${build=$(${CC:-gcc} -dumpmachine)}
  : ${host=${build/-gnu/-musl}}

 : ${prefix:=/opt/musl}
 : ${includedir=/usr/include/$host}
 : ${libdir=/usr/lib/$host}
 : ${bindir=/usr/bin/$host}

 export prefix

  builddir=build/$host \
  CC=musl-gcc \
  PKG_CONFIG=musl-pkg-config \
  cfg \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    "$@")
}


cfg-musl64() {
 (build=$(gcc -dumpmachine)
  host=${build%%-*}-linux-musl
  host=x86_64-${host#*-}

  builddir=build/$host \
  CFLAGS="-m64" \
  cfg-musl \
  -DCMAKE_C_COMPILER="musl-gcc" \
  "$@")
}

cfg-musl32() {
 (build=$(gcc -dumpmachine)
  host=$(echo "$build" | sed "s|x86_64|i686| ; s|-gnu|-musl|")

  builddir=build/$host \
  CFLAGS="-m32" \
  cfg-musl \
  -DCMAKE_C_COMPILER="musl-gcc" \
  "$@")
}

cfg-msys() {
 (build=$(gcc -dumpmachine)
  : ${host=${build%%-*}-pc-msys}
  : ${prefix=/usr/$host/sys-root/msys}

  builddir=build/$host \
  bindir=$prefix/bin \
  libdir=$prefix/lib \
  CC="$host-gcc" \
  cfg \
    -DCMAKE_CROSSCOMPILING=TRUE \
    "$@")
}

cfg-msys32() {
 (build=$(gcc -dumpmachine)
  host=${build%%-*}-pc-msys
  host=i686-${host#*-}
  cfg-msys "$@")
}

cfg-termux()
{
  (builddir=build/termux
    cfg \
  -DCMAKE_INSTALL_PREFIX=/data/data/com.termux/files/usr \
  -DCMAKE_VERBOSE_MAKEFILE=TRUE \
  -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN:-/opt/android-cmake/android.cmake} \
  -DANDROID_NATIVE_API_LEVEL=21 \
  -DPKG_CONFIG_EXECUTABLE=arm-linux-androideabi-pkg-config \
  -DCMAKE_PREFIX_PATH=/data/data/com.termux/files/usr \
  -DCMAKE_MAKE_PROGRAM=/usr/bin/make \
   -DCMAKE_MODULE_PATH="/data/data/com.termux/files/usr/lib/cmake" \
   "$@"
    )
}
cfg-wasm() {
  export VERBOSE
 (EMCC=$(which emcc)
  EMSCRIPTEN=$(dirname "$EMCC");
  EMSCRIPTEN=${EMSCRIPTEN%%/bin*};
  test -f /opt/cmake-toolchains/generic/Emscripten-wasm.cmake && TOOLCHAIN=/opt/cmake-toolchains/generic/Emscripten-wasm.cmake
  test '!' -f "$TOOLCHAIN" && TOOLCHAIN=$(find "$EMSCRIPTEN" -iname emscripten.cmake);
  test -f "$TOOLCHAIN" || unset TOOLCHAIN;
  : ${prefix:="$EMSCRIPTEN"}
  builddir=build/emscripten-wasm \
  CC="$EMCC" \
  cfg \
    -DEMSCRIPTEN_PREFIX="$EMSCRIPTEN" \
    ${TOOLCHAIN:+-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN"} \
    -DCMAKE_EXE_LINKER_FLAGS="-s WASM=1" \
    -DCMAKE_EXECUTABLE_SUFFIX=".html" \
    -DCMAKE_EXECUTABLE_SUFFIX_INIT=".html" \
    -DUSE_{ZLIB,BZIP,LZMA,SSL}=OFF \
  "$@")
}

cfg-msys32() {
 (build=$(gcc -dumpmachine)
  host=${build%%-*}-pc-msys
  host=i686-${host#*-}
  cfg-msys "$@")
}

cfg-msys() {
 (build=$(gcc -dumpmachine)
  : ${host=${build%%-*}-pc-msys}
  : ${prefix=/usr/$host/sys-root/msys}

  builddir=build/$host \
  bindir=$prefix/bin \
  libdir=$prefix/lib \
  CC="$host-gcc" \
  cfg \
    -DCMAKE_CROSSCOMPILING=TRUE \
    "$@")
}

cfg-tcc() {
 (build=$(cc -dumpmachine)
  host=${build/-gnu/-tcc}
  builddir=build/$host
  prefix=/usr
  includedir=/usr/lib/$build/tcc/include
  libdir=/usr/lib/$build/tcc/
  bindir=/usr/bin

  CC=${TCC:-tcc} \
  cfg \
    -DCMAKE_VERBOSE_MAKEFILE=ON \
    "$@")
}
