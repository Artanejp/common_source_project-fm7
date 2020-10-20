#!/bin/sh

set -e

if [ $# -lt 1 ]; then
    echo $0 dir
    exit 1
fi
PREFIX="$1"
cd "$PREFIX"

if [ -n "$FULL_LLVM" ]; then
    exit 0
fi

case $(uname) in
MINGW*)
    EXEEXT=.exe
    ;;
*)
    ;;
esac

cd bin
for i in bugpoint c-index-test clang-* diagtool dsymutil git-clang-format hmaptool ld64.lld llc lli llvm-* obj2yaml opt sancov sanstats scan-build scan-view verify-uselistorder wasm-ld yaml2obj libclang.dll LTO.dll *Remarks.dll *.bat; do
    basename=$i
    if [ -n "$EXEEXT" ]; then
        # Some in the list are expanded globs, some are plain names we list.
        case $i in
        *$EXEEXT)
            basename=$(echo $i | sed s/$EXEEXT//)
            ;;
        esac
        i=$basename
        if [ -e $basename$EXEEXT ]; then
            i=$basename$EXEEXT
        fi
    fi
    # Basename has got $EXEEXT stripped, but any other suffix kept intact.
    case $basename in
    *.sh)
        ;;
    clang++|clang-*.*|clang-cpp)
        ;;
    clang-*)
        suffix="${basename#*-}"
        # Test removing all numbers from the suffix; if it is empty, the suffix
        # was a plain number (as if the original name was clang-7); if it wasn't
        # empty, remove the tool.
        if [ "$(echo $suffix | tr -d '[0-9]')" != "" ]; then
            rm -f $i
        fi
        ;;
    llvm-ar|llvm-cvtres|llvm-dlltool|llvm-nm|llvm-objdump|llvm-ranlib|llvm-rc|llvm-readobj|llvm-strings|llvm-pdbutil|llvm-objcopy|llvm-strip|llvm-cov|llvm-profdata|llvm-addr2line|llvm-wrapper)
        ;;
    ld64.lld|wasm-ld)
        if [ -e $i ]; then
            rm $i
        fi
        ;;
    *)
        if [ -f $i ]; then
            rm $i
        elif [ -L $i ] && [ ! -e $(readlink $i) ]; then
            # Remove dangling symlinks
            rm $i
        fi
        ;;
    esac
done
if [ -n "$EXEEXT" ]; then
    # Convert ld.lld from a symlink to a regular file, so we can remove
    # the one it points to. On MSYS, and if packaging built toolchains
    # in a zip file, symlinks are converted into copies.
    if [ -L ld.lld$EXEEXT ]; then
        cp ld.lld$EXEEXT tmp
        rm ld.lld$EXEEXT
        mv tmp ld.lld$EXEEXT
    fi
    # lld-link isn't used normally, but can be useful for debugging/testing,
    # and is kept in unix setups. Removing it when packaging for windows,
    # to conserve space.
    rm -f lld$EXEEXT lld-link$EXEEXT
    # Remove superfluous frontends; these aren't really used.
    rm -f clang-cpp* clang++*
fi
cd ..
rm -rf share libexec
cd include
rm -rf clang clang-c lld llvm llvm-c
cd ..
cd lib
rm -rf lib*.a *.so* *.dylib* cmake
cd ..
