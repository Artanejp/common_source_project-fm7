#!/bin/sh
PATH=/opt/llvm-mingw/bin:$PATH
CFLAGS="-I/usr/i686-w64-mingw32/include"
LDFLAGS="-L/usr/i686-w64-mingw32/lib"
./configure --host=i686-w64-mingw32 \
            --enable-pcre2-16 --enable-pcre2-32 \
            --enable-pic --enable-shared \
	    --prefix=/usr/local/i586-mingw-msvc/pcre2
	    
	    #--enable-static 
	    
