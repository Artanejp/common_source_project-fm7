#!/bin/sh
PATH="/opt/llvm-mingw/bin:/usr/local/i586-mingw-msvc/lame-3.100/bin:$PATH"
CC=clang
CXX=clang++
#export LD=/usr/bin/i686-w64-mingw32-ld
export PKG_CONFIG_PATH=/usr/local/i586-mingw-msvc/pkgconfig/lib/pkgconfig

./configure --cross-prefix=i686-w64-mingw32- \
            --prefix=/usr/local/i586-mingw-msvc/ffmpeg-4.2 \
            --pkgconfigdir=/usr/local/i586-mingw-msvc/pkgconfig/lib/pkgconfig \
            --disable-static --enable-shared \
	    --target-os=mingw32 --arch=i686 \
	    --disable-inline-asm \
	    --disable-sse4 \
	    --disable-ssse3 \
	    --host-cc=gcc \
	    --enable-gpl \
	    --enable-libx264 \
	    --enable-libvorbis \
	    --enable-libmp3lame \
	    --enable-dxva2 \
	    --extra-cflags=" \
	    -I/opt/llvm-mingw/include \
	    -I/usr/local/i586-mingw-msvc/lame-3.100/include  \
	    -m32" \
	    --extra-ldflags=" \
	    -L/opt/llvm-mingw/lib \
	    -L/opt/llvm-mingw/i686-w64-mingw32/lib \
	    -L/usr/i686-w64-mingw32/lib \
	    -L/usr/local/i586-mingw-msvc/lame-3.100/lib \
	    "
#	    -I/usr/local/i586-mingw-msvc/libx264-158/include \
#	    -I/usr/local/i586-mingw-msvc/libvorbis/include \
#	    -I/usr/local/i586-mingw-msvc/libogg/include \
#	    -L/usr/local/i586-mingw-msvc/libvorbis/lib \
#	    -L/usr/local/i586-mingw-msvc/libogg/lib \
#	    -L/usr/local/i586-mingw-msvc/libx264-158/lib \
