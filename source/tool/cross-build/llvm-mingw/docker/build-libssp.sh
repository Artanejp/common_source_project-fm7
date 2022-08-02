#!/bin/sh

set -e

if [ $# -lt 1 ]; then
    echo $0 dest
    exit 1
fi
while [ $# -gt 0 ]; do
    if [ "$1" = "--build-threads" ]; then
	: ${CORES:=$2}
	shift
    else
        PREFIX="$1"
	break
    fi
    shift
done
mkdir -p "$PREFIX"
PREFIX="$(cd "$PREFIX" && pwd)"
export PATH=$PREFIX/bin:$PATH
export TOOLCHAIN_ARCHS="i686 x86_64 armv7 aarch64"

: ${CORES:=$(nproc 2>/dev/null)}
: ${CORES:=$(sysctl -n hw.ncpu 2>/dev/null)}
: ${CORES:=4}
: ${ARCHS:=${TOOLCHAIN_ARCHS-i686 x86_64 armv7 aarch64}}

if [ ! -d gcc-libssp/libssp ]; then
    git clone https://gcc.gnu.org/git/gcc.git gcc-libssp
#    svn checkout -q svn://gcc.gnu.org/svn/gcc/tags/gcc_7_3_0_release/libssp
fi
cd gcc-libssp/libssp
git checkout releases/gcc-10.3.0
#cp ../../libssp-Makefile ./Makefile

# gcc/libssp's configure script runs checks for flags that clang doesn't
# implement. We actually just need to set a few HAVE defines and compile
# the .c sources.
#cp config.h.in config.h
#for i in HAVE_FCNTL_H HAVE_INTTYPES_H HAVE_LIMITS_H HAVE_MALLOC_H \
#    HAVE_MEMMOVE HAVE_MEMORY_H HAVE_MEMPCPY HAVE_STDINT_H HAVE_STDIO_H \
#    HAVE_STDLIB_H HAVE_STRINGS_H HAVE_STRING_H HAVE_STRNCAT HAVE_STRNCPY \
#    HAVE_SYS_STAT_H HAVE_SYS_TYPES_H HAVE_UNISTD_H HAVE_USABLE_VSNPRINTF \
#    HAVE_HIDDEN_VISIBILITY; do
#    cat config.h | sed 's/^#undef '$i'$/#define '$i' 1/' > tmp
#    mv tmp config.h
#done
#cat ssp/ssp.h.in | sed 's/@ssp_have_usable_vsnprintf@/define/' > ssp/ssp.h

for arch in $ARCHS; do
    export CC=$arch-w64-mingw32-clang
    
    mkdir -p build-$arch-w64-mingw32
    cd build-$arch-w64-mingw32
#    make -f ../Makefile -j$CORES CROSS=$arch-w64-mingw32-
#    mkdir -p $PREFIX/$arch-w64-mingw32/bin
    ../configure --host=$arch-w64-mingw32 \
                --enable-version-specific-runtime-libs \
		--prefix=$PREFIX \
		--with-toolexeclibdir=$PREFIX/$arch-w64-mingw32
		
     make -j$CORES		
     cp .libs/libssp.a $PREFIX/$arch-w64-mingw32/lib
     cp .libs/libssp_nonshared.a $PREFIX/$arch-w64-mingw32/lib
     cp .libs/libssp.dll.a $PREFIX/$arch-w64-mingw32/lib
     cp .libs/libssp-0.dll $PREFIX/$arch-w64-mingw32/bin
     mkdir -p $PREFIX/include/$arch-w64-mingw32/ssp
     cp ssp/ssp.h $PREFIX/include/$arch-w64-mingw32/ssp
    cd ..
done
