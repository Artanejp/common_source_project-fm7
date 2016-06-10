#!/bin/sh
./configure --cross-prefix=i686-w64-mingw32- \
            --disable-static --enable-shared \
	    --target-os=mingw32 --arch=i686 \
	    --enable-nonfree --enable-gpl \
	    --enable-libx264 --enable-libfaac \
	    --extra-cflags="-I/usr/local/i586-mingw-msvc/libfaac/include -I/usr/local/i586-mingw-msvc/libx264-148/include" \
	    --extra-ldflags="-L/usr/local/i586-mingw-msvc/libfaac/lib -L/usr/local/i586-mingw-msvc/libx264-148/lib"
	    
	    