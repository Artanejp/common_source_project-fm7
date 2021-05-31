#!/bin/sh
PATH="/opt/llvm-mingw-12/bin:$PATH"
#export CC=i686-w64-mingw32-clang
#export CXX=i686-w64-mingw32-clang++
export CFLAGS="--stdlib=libc++ -msse2"
export LDFLAGS="-L/opt/llvm-mingw-12/lib -lc++ -lc++abi -lunwind -lssp"

../configure \
	    --host=i686-w64-mingw32 \
	    --enable-pic \
	    --enable-shared \
	    --enable-static \
	    --prefix=/usr/local/i586-mingw-msvc/lame
