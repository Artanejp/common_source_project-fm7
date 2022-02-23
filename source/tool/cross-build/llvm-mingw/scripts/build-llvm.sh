#!/bin/sh

set -e

ASSERTS=OFF
BUILDDIR=build
FORCE_THREADS=OFF
OVERRIDE_THREADS=0
FORCE_WORKLOADS=OFF
OVERRIDE_WORKLOADS=0

while [ $# -gt 0 ]; do
    if [ "$1" = "--disable-asserts" ]; then
        ASSERTS=OFF
        BUILDDIR=build
    elif [ "$1" = "--enable-asserts" ]; then
        ASSERTS=ON
        BUILDDIR=build-asserts
    elif [ "$1" = "--full-llvm" ]; then
        FULL_LLVM=1
    elif [ "$1" = "--build-threads" ]; then
        FORCE_THREADS=ON
	OVERRIDE_THREADS="$2"
	: ${CORES="$2"}
	shift
    elif [ "$1" = "--workload" ]; then
        FORCE_WORKLOADS=ON
	OVERRIDE_WORKLOADS="$2"
	: ${WORKLOADS="$2"}
	shift
    elif [ "$1" = "--llvm-version" ]; then
        LLVM_VERSION="$2"
	shift
    else
        PREFIX="$1"
    fi
    shift
done

 mkdir -p "$PREFIX"
PREFIX="$(cd "$PREFIX" && pwd)"

if [ -z "$PREFIX" ]; then
    echo $0 [--enable-asserts] [--full-llvm] dest
    exit 1
fi

#: ${CORES:=$(nproc 2>/dev/null)}
#: ${CORES:=$(sysctl -n hw.ncpu 2>/dev/null)}
: ${CORES:=8}

: ${WORKLOADS:=16.0}

if [ ! -d llvm-project ]; then
    # When cloning master and checking out a pinned old hash, we can't use --depth=1.
    git clone https://github.com/llvm/llvm-project.git
    CHECKOUT=1
fi

if [ -n "$SYNC" ] || [ -n "$CHECKOUT" ]; then
    cd llvm-project
    [ -z "$SYNC" ] || git fetch
    git checkout ${LLVM_VERSION}
    cd ..
fi

[ -z "$CHECKOUT_ONLY" ] || exit 0

#if [ -n "$(which ninja)" ]; then
#    CMAKE_GENERATOR="Ninja"
#    NINJA=1
#else
    case $(uname) in
    MINGW*)
        CMAKE_GENERATOR="MSYS Makefiles"
        ;;
    *)
        ;;
    esac
#fi

if [ -n "$HOST" ]; then
    find_native_tools() {
        if [ -d llvm-project/llvm/build/bin ]; then
            echo $(pwd)/llvm-project/llvm/build/bin
        elif [ -d llvm-project/llvm/build-asserts/bin ]; then
            echo $(pwd)/llvm-project/llvm/build-asserts/bin
        elif [ -d llvm-project/llvm/build-noasserts/bin ]; then
            echo $(pwd)/llvm-project/llvm/build-noasserts/bin
        elif [ -n "$(which llvm-tblgen)" ]; then
            echo $(dirname $(which llvm-tblgen))
        fi
    }

    CMAKEFLAGS="$CMAKEFLAGS -DCMAKE_SYSTEM_NAME=Windows"
    CMAKEFLAGS="$CMAKEFLAGS -DCMAKE_CROSSCOMPILING=TRUE"
    CMAKEFLAGS="$CMAKEFLAGS -DCMAKE_C_COMPILER=$HOST-gcc"
    CMAKEFLAGS="$CMAKEFLAGS -DCMAKE_CXX_COMPILER=$HOST-g++"
    CMAKEFLAGS="$CMAKEFLAGS -DCMAKE_RC_COMPILER=$HOST-windres"
    CMAKEFLAGS="$CMAKEFLAGS -DCROSS_TOOLCHAIN_FLAGS_NATIVE="

    native=$(find_native_tools)
    if [ -n "$native" ]; then
        CMAKEFLAGS="$CMAKEFLAGS -DLLVM_TABLEGEN=$native/llvm-tblgen"
        CMAKEFLAGS="$CMAKEFLAGS -DCLANG_TABLEGEN=$native/clang-tblgen"
        CMAKEFLAGS="$CMAKEFLAGS -DLLVM_CONFIG_PATH=$native/llvm-config"
    fi

    CROSS_ROOT=$(cd $(dirname $(which $HOST-gcc))/../$HOST && pwd)
    CMAKEFLAGS="$CMAKEFLAGS -DCMAKE_FIND_ROOT_PATH=$CROSS_ROOT"
    CMAKEFLAGS="$CMAKEFLAGS -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER"
    CMAKEFLAGS="$CMAKEFLAGS -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY"
    CMAKEFLAGS="$CMAKEFLAGS -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY"

    # Custom, llvm-mingw specific defaults. We normally set these in
    # the frontend wrappers, but this makes sure they are enabled by
    # default if that wrapper is bypassed as well.
    CMAKEFLAGS="$CMAKEFLAGS -DCLANG_DEFAULT_RTLIB=compiler-rt"
    CMAKEFLAGS="$CMAKEFLAGS -DCLANG_DEFAULT_CXX_STDLIB=libc++"
    CMAKEFLAGS="$CMAKEFLAGS -DCLANG_DEFAULT_LINKER=lld"
    BUILDDIR=$BUILDDIR-$HOST
fi

TOOLCHAIN_ONLY=ON
if [ -n "$FULL_LLVM" ]; then
    TOOLCHAIN_ONLY=OFF
fi

cd llvm-project/llvm

case $(uname) in
MINGW*)
    EXPLICIT_PROJECTS=1
    ;;
*)
    # If we have working symlinks, hook up other tools by symlinking them
    # into tools, instead of using LLVM_ENABLE_PROJECTS. This way, all
    # source code is under the directory tree of the toplevel cmake file
    # (llvm-project/llvm), which makes cmake use relative paths to all source
    # files. Using relative paths makes for identical compiler output from
    # different source trees in different locations (for cases where e.g.
    # path names are included, in assert messages), allowing ccache to speed
    # up compilation.
    cd tools
    for p in clang lld; do
        if [ ! -e $p ]; then
            ln -s ../../$p .
        fi
    done
    cd ..
    ;;
esac

mkdir -p $BUILDDIR
cd $BUILDDIR
cmake \
    ${CMAKE_GENERATOR+-G} "$CMAKE_GENERATOR" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_CCACHE_BUILD=ON \
    -DLLVM_ENABLE_PIC=OFF \
    -DLLVM_ENABLE_ASSERTIONS=$ASSERTS \
    -DLLVM_ENABLE_PROJECTS="clang;lld;parallel-libs;polly;pstl" \
    -DLLVM_TARGETS_TO_BUILD="ARM;AArch64;X86" \
    -DLLVM_INSTALL_TOOLCHAIN_ONLY=$TOOLCHAIN_ONLY \
    -DLLVM_TOOLCHAIN_TOOLS="llvm-ar;llvm-ranlib;llvm-objdump;llvm-rc;llvm-cvtres;llvm-nm;llvm-strings;llvm-readobj;llvm-dlltool;llvm-pdbutil;llvm-objcopy;llvm-strip;llvm-cov;llvm-profdata;llvm-addr2line" \
    ${HOST+-DLLVM_HOST_TRIPLE=$HOST} \
    $CMAKEFLAGS \
   ..

cmake ..


#if [ -n "$NINJA" ]; then
#    ninja -j$CORES
#     ninja -j$CORES install/strip
#else
#    make -j$CORES 
     make -j$CORES -l ${WORKLOADS} 
     make -j$CORES -l ${WORKLOADS} runtimes
     
     make -j$CORES -l ${WORKLOADS} install/strip
#fi

exit 0

-DLLVM_ENABLE_RUNTIMES="compiler-rt;libunwind;libcxxabi;libcxx;openmp" \
    -DLLVM_RUNTIME_TARGETS="x86_64-w64-mingw32;i686-w64-mingw32;aarch64-w64-mingw32;armv7-w64-mingw32" \
    -DCOMPILER_RT_DEFAULT_TARGET_ONLY=TRUE \
    -DCOMPILER_RT_USE_BUILTINS_LIBRARY=TRUE \
    -DLIBUNWIND_USE_COMPILER_RT=TRUE \
    -DLIBUNWIND_ENABLE_THREADS=TRUE \
    -DLIBCXXABI_USE_COMPILER_RT=ON \
    -DLIBCXXABI_ENABLE_EXCEPTIONS=ON \
    -DLIBCXXABI_ENABLE_THREADS=ON \
    -DLIBCXXABI_ENABLE_NEW_DELETE_DEFINITIONS=ON \
    -DLIBCXXABI_USE_LLVM_UNWINDER=ON \
    -DLIBCXX_USE_COMPILER_RT=ON \
    -DLIBCXX_INSTALL_HEADERS=ON \
    -DLIBCXX_ENABLE_EXCEPTIONS=ON \
    -DLIBCXX_ENABLE_THREADS=ON \
    -DLIBCXX_HAS_WIN32_THREAD_API=ON \
    -DLIBCXX_ENABLE_MONOTONIC_CLOCK=ON \
    -DLIBCXX_SUPPORTS_STD_EQ_CXX11_FLAG=TRUE \
    -DLIBCXX_HAVE_CXX_ATOMICS_WITHOUT_LIB=TRUE \
    -DLIBCXX_ENABLE_EXPERIMENTAL_LIBRARY=OFF \
    -DLIBCXX_ENABLE_FILESYSTEM=OFF \
    -DLIBCXX_CXX_ABI=libcxxabi \
    -DLIBCXX_LIBDIR_SUFFIX="" \
    -DLIBCXX_ENABLE_NEW_DELETE_DEFINITIONS=OFF \
    -DLIBCXX_INCLUDE_TESTS=FALSE \
    -DLIBCXX_ENABLE_ABI_LINKER_SCRIPT=FALSE \
