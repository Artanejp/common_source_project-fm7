#!/bin/sh
export PATH="/opt/llvm-mingw-12/bin:$PATH"
export CC=i686-w64-mingw32-clang
export CXX=i686-w64-mingw32-clang++
#export LD=/usr/bin/i686-w64-mingw32-ld
#export PKG_CONFIG_PATH=/usr/local/i586-mingw-msvc/pkgconfig/lib/pkgconfig

../configure --host=i686-w64-mingw32 \
            --prefix=/usr/local/i586-mingw-msvc/vorbis \
            --enable-static --enable-shared \
	    --disable-oggtest \
	    --with-ogg=/usr/local/i586-mingw-msvc/ogg
#    --with-ogg=../../libogg-1.3.2/build

#	    -I/usr/local/i586-mingw-msvc/libx264-158/include \
#	    -I/usr/local/i586-mingw-msvc/libvorbis/include \
#	    -I/usr/local/i586-mingw-msvc/libogg/include \
#	    -L/usr/local/i586-mingw-msvc/libvorbis/lib \
#	    -L/usr/local/i586-mingw-msvc/libogg/lib \
#	    -L/usr/local/i586-mingw-msvc/libx264-158/lib \
