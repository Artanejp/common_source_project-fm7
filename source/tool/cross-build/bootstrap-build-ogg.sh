#!/bin/sh
export PATH="/opt/llvm-mingw/bin:$PATH"
#export CC=i686-w64-mingw32-clang
#export CXX=i686-w64-mingw32-clang++
#export LD=/usr/bin/i686-w64-mingw32-ld
#export PKG_CONFIG_PATH=/usr/local/i586-mingw-msvc/lib/pkgconfig

../configure --host=i686-w64-mingw32 \
            --prefix=/usr/local/i586-mingw-msvc \
            --enable-static --enable-shared \

#	    -I/usr/local/i586-mingw-msvc/libx264-158/include \
#	    -I/usr/local/i586-mingw-msvc/libvorbis/include \
#	    -I/usr/local/i586-mingw-msvc/libogg/include \
#	    -L/usr/local/i586-mingw-msvc/libvorbis/lib \
#	    -L/usr/local/i586-mingw-msvc/libogg/lib \
#	    -L/usr/local/i586-mingw-msvc/libx264-158/lib \
