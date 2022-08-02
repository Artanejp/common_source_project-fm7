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

: ${CORES:=$(nproc 2>/dev/null)}
: ${CORES:=$(sysctl -n hw.ncpu 2>/dev/null)}
: ${CORES:=4}
: ${ARCHS:=${TOOLCHAIN_ARCHS-i686 x86_64 armv7 aarch64}}

cd mingw-w64/mingw-w64-libraries
for lib in winpthreads winstorecompat; do
    cd $lib
    for arch in $ARCHS; do
        mkdir -p build-$arch
        cd build-$arch
        ../configure --host=$arch-w64-mingw32 --prefix=$PREFIX/$arch-w64-mingw32
        make -j$CORES
        make install
        cd ..
    done
    cd ..
done
