#!/bin/sh
export PATH="$PATH:\
            /usr/local/i586-mingw-msvc/icu/bin:\
	    /usr/local/i586-mingw-msvc/Angle/bin:\
	    /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/bin: \
            /usr/local/i586-mingw-msvc/DirectX_June_2010/Developer Runtime/x86 \
            "
export PKG_CONFIG_LIBDIR=/usr/i686-w64-mingw32/lib

make CROSS_COMPILE=i686-w64-mingw32- PKG_CONFIG_LIBDIR=/usr/i686-w64-mingw32/lib $@
