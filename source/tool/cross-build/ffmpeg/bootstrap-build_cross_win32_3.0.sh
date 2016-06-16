#!/bin/sh
./configure --cross-prefix=i686-w64-mingw32- \
            --prefix=/usr/local/i586-mingw-msvc/ffmpeg-3.0.2 \
            --disable-static --enable-shared \
	    --target-os=mingw32 --arch=i686 \
	    --enable-gpl \
	    --enable-libx264 \
	    --enable-libvorbis \
	    --extra-cflags="-I/usr/local/i586-mingw-msvc/libx264-148/include -I/usr/local/i586-mingw-msvc/libvorbis/include -I/usr/local/i586-mingw-msvc/libogg/include" \
	    --extra-ldflags="-L/usr/local/i586-mingw-msvc/libx264-148/lib -L/usr/local/i586-mingw-msvc/libvorbis/lib -L/usr/local/i586-mingw-msvc/libogg/lib"
	    
	    