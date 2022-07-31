#!/bin/sh

set -e

BUILD_STATIC=1
BUILD_SHARED=1

while [ $# -gt 0 ]; do
    if [ "$1" = "--disable-shared" ]; then
        BUILD_SHARED=
    elif [ "$1" = "--enable-shared" ]; then
        BUILD_SHARED=1
    elif [ "$1" = "--disable-static" ]; then
        BUILD_STATIC=
    elif [ "$1" = "--enable-static" ]; then
        BUILD_STATIC=1
    elif [ "$1" = "--build-threads" ]; then
	: ${CORES:=$2}
	shift
    else
        PREFIX="$1"
    fi
    shift
done

if [ -z "$PREFIX" ]; then
    echo $0 [--disable-shared] [--disable-static] dest
    exit 1
fi

mkdir -p "$PREFIX"
PREFIX="$(cd "$PREFIX" && pwd)"

export PATH=$PREFIX/bin:$PATH

: ${CORES:=$(nproc 2>/dev/null)}
: ${CORES:=$(sysctl -n hw.ncpu 2>/dev/null)}
: ${CORES:=4}
: ${ARCHS:=${TOOLCHAIN_ARCHS-i686 x86_64 armv7 aarch64}}

if [ ! -d llvm-project/libunwind ] || [ -n "$SYNC" ]; then
    CHECKOUT_ONLY=1 ./build-llvm.sh
fi

cd llvm-project

LIBCXX=$(pwd)/libcxx

case $(uname) in
MINGW*)
    CMAKE_GENERATOR="MSYS Makefiles"
    ;;
*)
    ;;
esac

build_all() {
    type="$1"
    if [ "$type" = "shared" ]; then
        SHARED=TRUE
        STATIC=FALSE
    else
        SHARED=FALSE
        STATIC=TRUE
    fi

    #cd libcxxabi
    for arch in $ARCHS; do

        cmake \
            ${CMAKE_GENERATOR+-G} "$CMAKE_GENERATOR" \
            -DCMAKE_BUILD_TYPE=Release \
 	    -S runtimes -B build-$arch-w64-mingw32-$type \
            -DCMAKE_C_COMPILER=$arch-w64-mingw32-clang \
            -DCMAKE_CXX_COMPILER=$arch-w64-mingw32-clang++ \
            -DLLVM_ENABLE_RUNTIMES="libunwind;libcxx;libcxxabi" \
	    -DLLVM_RUNTIME_TARGETS="$arch-w64-mingw32" \
            -DCMAKE_CROSSCOMPILING=TRUE \
            -DCMAKE_SYSTEM_NAME=Windows \
            -DCMAKE_CXX_COMPILER_TARGET=$arch-w64-mingw32 \
            -DCMAKE_C_COMPILER_WORKS=TRUE \
            -DCMAKE_CXX_COMPILER_WORKS=TRUE \
            -DLLVM_COMPILER_CHECKED=TRUE \
            -DCMAKE_AR=$PREFIX/bin/llvm-ar \
            -DCMAKE_RANLIB=$PREFIX/bin/llvm-ranlib \
            -DCMAKE_INSTALL_PREFIX=$PREFIX/$arch-w64-mingw32/ \
            -DLIBCXXABI_USE_COMPILER_RT=ON \
            -DLIBCXXABI_INSTALL_HEADERS=ON \
            -DLIBCXXABI_ENABLE_EXCEPTIONS=ON \
            -DLIBCXXABI_ENABLE_THREADS=OFF \
	    -DLIBCXXABI_ENABLE_NEW_DELETE_DEFINITIONS=ON \
	    -DLIBCXXABI_USE_LLVM_UNWINDER=ON \
            -DLIBCXXABI_ENABLE_SHARED=$SHARED \
            -DLIBCXXABI_ENABLE_STATIC=$STATIC \
            -DLIBCXX_USE_COMPILER_RT=ON \
            -DLIBCXX_INSTALL_HEADERS=ON \
            -DLIBCXX_ENABLE_EXCEPTIONS=ON \
            -DLIBCXX_ENABLE_THREADS=ON \
            -DLIBCXX_ENABLE_MONOTONIC_CLOCK=ON \
            -DLIBCXX_ENABLE_SHARED=$SHARED \
            -DLIBCXX_ENABLE_STATIC=$STATIC \
            -DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=OFF \
            -DLIBCXX_ENABLE_FILESYSTEM=ON \
            -DLIBUNWIND_USE_COMPILER_RT=TRUE \
            -DLIBUNWIND_ENABLE_THREADS=TRUE \
            -DLIBUNWIND_ENABLE_SHARED=$SHARED \
            -DLIBUNWIND_ENABLE_STATIC=$STATIC \
            -DLIBUNWIND_ENABLE_CROSS_UNWINDING=FALSE \
	    -DLIBCXX_CXX_ABI=libcxxabi \
            -DCMAKE_SHARED_LINKER_FLAGS="-lpsapi" \

        make -C build-$arch-w64-mingw32-$type -j$CORES
        make -C build-$arch-w64-mingw32-$type install
        if [ "$type" = "shared" ]; then
            mkdir -p $PREFIX/$arch-w64-mingw32/bin
            cp build-$arch-w64-mingw32-$type/lib/libc++abi.dll $PREFIX/$arch-w64-mingw32/bin
            cp build-$arch-w64-mingw32-$type/lib/libc++.dll $PREFIX/$arch-w64-mingw32/bin
            cp build-$arch-w64-mingw32-$type/lib/libunwind.dll $PREFIX/$arch-w64-mingw32/bin
        else
            # Merge libpsapi.a into the static library libunwind.a, to
            # avoid having to specify -lpsapi when linking to it.
            llvm-ar qcsL \
                $PREFIX/$arch-w64-mingw32/lib/libunwind.a \
                $PREFIX/$arch-w64-mingw32/lib/libpsapi.a
        fi
        #cd ..
    done
#    cd ..
    

}

# Build shared first and static afterwards; the headers for static linking also
# work when linking against the DLL, but not vice versa.
[ -z "$BUILD_SHARED" ] || build_all shared
[ -z "$BUILD_STATIC" ] || build_all static
#build_all2