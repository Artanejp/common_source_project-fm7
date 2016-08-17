#/bin/sh
export PATH="$PATH:/usr/local/i586-mingw-msvc/icu/bin:/usr/local/i586-mingw-msvc/Angle/bin"

./configure \
            -prefix /usr/local/i586-mingw-msvc/5.7/mingw_611 \
            -release -opensource -confirm-license \
	    -xplatform win32-g++ \
	    -qt-zlib -qt-libpng -qt-libjpeg \
	    -qt-freetype \
	    -device-option CROSS_COMPILE=i686-w64-mingw32- \
	    -no-pch \
	    -no-compile-examples \
	    -icu \
	    -nomake examples \
	    -skip qtactiveqt \
	    $@
#	    -skip qtactiveqt -skip qtcanvas3d \

