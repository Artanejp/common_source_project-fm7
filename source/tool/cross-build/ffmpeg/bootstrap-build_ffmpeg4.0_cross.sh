#!/bin/sh
./configure --cross-prefix=i686-w64-mingw32- \
            --prefix=/usr/local/i586-mingw-msvc/ffmpeg-4.0 \
            --disable-static --enable-shared \
	    --target-os=mingw32 --arch=i686 \
	    --enable-gpl \
	    --enable-libx264 \
	    --enable-libvorbis \
	    --enable-libmp3lame \
	    --enable-dxva2 \
	    --extra-cflags="-I/usr/local/i586-mingw-msvc/libx264-155/include -I/usr/local/i586-mingw-msvc/lame-3.99.5/include  -I/usr/local/i586-mingw-msvc/libvorbis/include -I/usr/local/i586-mingw-msvc/libogg/include" \
	    --extra-ldflags="-L/usr/local/i586-mingw-msvc/libx264-155/lib -L/usr/local/i586-mingw-msvc/lame-3.99.5/lib -L/usr/local/i586-mingw-msvc/libvorbis/lib -L/usr/local/i586-mingw-msvc/libogg/lib"

