#!/bin/sh

set -e

SRC_DIR=../lib/builtins
BUILD_SUFFIX=

while [ $# -gt 0 ]; do
    if [ "$1" = "--build-sanitizers" ]; then
        SRC_DIR=..
        BUILD_SUFFIX=-sanitizers
        SANITIZERS=1
    elif [ "$1" = "--build-threads" ]; then
	: ${CORES=$2}
	shift
    elif [ "$1" = "--workload" ]; then
	: ${WORKLOADS=$2}
	shift
    else
        PREFIX="$1"
    fi
    shift
done
if [ -z "$PREFIX" ]; then
    echo $0 [--build-sanitizers] dest
    exit 1
fi

mkdir -p "$PREFIX"
PREFIX="$(cd "$PREFIX" && pwd)"
export PATH=$PREFIX/bin:$PATH

: ${CORES:=$(nproc 2>/dev/null)}
: ${CORES:=$(sysctl -n hw.ncpu 2>/dev/null)}
: ${CORES:=4}
: ${WORKLOADS:=99.9}
: ${ARCHS:=${TOOLCHAIN_ARCHS-i686 x86_64 armv7 aarch64}}

CLANG_VERSION=$(basename $(dirname $(dirname $(dirname $($PREFIX/bin/clang --print-libgcc-file-name -rtlib=compiler-rt)))))

if [ ! -d llvm-project/compiler-rt ] || [ -n "$SYNC" ]; then
    CHECKOUT_ONLY=1 ./build-llvm.sh
fi

# Add a symlink for i386 -> i686; we normally name the toolchain
# i686-w64-mingw32, but due to the compiler-rt cmake peculiarities, we
# need to refer to it as i386 at this stage.
if [ ! -e $PREFIX/i386-w64-mingw32 ]; then
    ln -sfn i686-w64-mingw32 $PREFIX/i386-w64-mingw32 || true
fi

cd llvm-project/compiler-rt

for arch in $ARCHS; do
    buildarchname=$arch
    libarchname=$arch
    if [ -n "$SANITIZERS" ]; then
        case $arch in
        i686|x86_64)
            # Sanitizers on windows only support x86.
            ;;
        *)
            continue
            ;;
        esac
    fi
    case $arch in
    armv7)
        libarchname=arm
        ;;
    i686)
        buildarchname=i386
        libarchname=i386
        ;;
    esac

    case $(uname) in
    MINGW*)
        CMAKE_GENERATOR="MSYS Makefiles"
        ;;
    *)
        ;;
    esac

    mkdir -p build-$arch$BUILD_SUFFIX
    cd build-$arch$BUILD_SUFFIX
    cmake \
        ${CMAKE_GENERATOR+-G} "$CMAKE_GENERATOR" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=$PREFIX/$arch-w64-mingw32 \
        -DCMAKE_C_COMPILER=$arch-w64-mingw32-clang \
        -DCMAKE_CXX_COMPILER=$arch-w64-mingw32-clang++ \
        -DCMAKE_SYSTEM_NAME=Windows \
        -DCMAKE_AR=$PREFIX/bin/llvm-ar \
        -DCMAKE_RANLIB=$PREFIX/bin/llvm-ranlib \
        -DCMAKE_C_COMPILER_WORKS=1 \
        -DCMAKE_CXX_COMPILER_WORKS=1 \
        -DCMAKE_C_COMPILER_TARGET=${buildarchname}-windows-gnu \
        -DCOMPILER_RT_DEFAULT_TARGET_ONLY=TRUE \
        -DCOMPILER_RT_USE_BUILTINS_LIBRARY=TRUE \
        $SRC_DIR
    make -j$CORES -l ${WORKLOADS}
    mkdir -p $PREFIX/lib/clang/$CLANG_VERSION/lib/windows
    mkdir -p $PREFIX/$arch-w64-mingw32/bin
    for i in lib/windows/libclang_rt.builtins-${buildarchname}.a; do
        if [ -f $i ]; then
            cp $i $PREFIX/lib/clang/$CLANG_VERSION/lib/windows/$(basename $i | sed s/$buildarchname/$libarchname/)
	fi
    done
    for i in lib/windows/libclang_rt.builtins-${libarchname}.a; do
        if [ -f $i ]; then
            cp $i $PREFIX/lib/clang/$CLANG_VERSION/lib/windows/
	fi
    done
    for i in lib/windows/libclang_rt.builtins-${libarchname}*.dll; do
        if [ -f $i ]; then
            cp $i $PREFIX/$arch-w64-mingw32/bin
        fi
    done
    if [ -n "$SANITIZERS" ]; then
        make install-compiler-rt-headers
    fi
    cd ..
done
