#!/bin/sh
./configure --cross-prefix=i686-w64-mingw32- \
            --host=i686-w64-mingw32 \
	    --enable-pic \
	    --enable-shared \
	    --enable-static \
	    --prefix=/usr/local/i586-mingw-msvc/libx264-157
