#/bin/sh
export PATH="$PATH:/usr/local/i586-mingw-msvc/icu/bin:/usr/local/i586-mingw-msvc/Angle/bin"
export PKG_CONFIG_LIBDIR=/usr/i686-w64-mingw32/lib
./configure \
            -prefix /usr/local/i586-mingw-msvc/5.7.1/mingw_621 \
            -release -opensource -confirm-license \
	    -xplatform win32-g++ \
	    -qt-zlib -qt-libpng -qt-libjpeg \
	    -qt-freetype \
	    -device-option CROSS_COMPILE=i686-w64-mingw32- \
	    -device-option QMAKE_INCDIR_EGL=/usr/local/i586-mingw-msvc/Angle/include \
	    -device-option QMAKE_LIBDIR_EGL=/usr/local/i586-mingw-msvc/Angle/lib \
	    -device-option QMAKE_LIBS_EGL=-lEGL -lGLESv2 \
	    -no-pch \
	    -no-compile-examples \
	    -icu \
	    -nomake examples \
	    -nomake tests \
	    -skip qtactiveqt \
	    -L /usr/local/i586-mingw-msvc/Angle/lib \
	    -I /usr/local/i586-mingw-msvc/Angle/include \
	    -largefile \
	    $@ \
	    -pkg-config \
#	    -skip qtactiveqt -skip qtcanvas3d \

