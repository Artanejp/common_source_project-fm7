#!/bin/sh
make CROSS_COMPILE=i686-w64-mingw32- PKG_CONFIG_LIBDIR=/usr/i686-w64-mingw32/lib $@
