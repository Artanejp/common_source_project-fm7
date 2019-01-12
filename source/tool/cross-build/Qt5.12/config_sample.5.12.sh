#!/bin/sh
export PATH="$PATH:\
            /usr/local/i586-mingw-msvc/icu/bin:\
	    /usr/local/i586-mingw-msvc/Angle/bin:\
	    /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/bin: \
            /usr/local/i586-mingw-msvc/DirectX_June_2010/Developer\ Runtime/x86 \
            "
export PKG_CONFIG_LIBDIR=/usr/i686-w64-mingw32/lib
export PKG_CONFIG_SYSROOT_DIR=/usr/i686-w64-mingw32


#wine ./qtbase/configure.exe \
./configure \
	    -release \
	    -optimized-tools \
            -platform linux-g++ \
            -prefix /usr/local/i586-mingw-msvc/5.12/mingw_82x \
            -opensource -confirm-license \
	    -xplatform win32-g++ \
	    -qt-libpng \
	    -qt-libjpeg \
	    -qt-freetype \
	    -device-option CROSS_COMPILE=i686-w64-mingw32- \
	    -no-compile-examples \
	    -skip qtactiveqt \
	    -skip qtwebglplugin \
	    -skip qtwebengine \
	    -skip qtwebview \
	    -skip qtquickcontrols \
	    -skip qtlocation \
	    -icu \
	    -nomake examples \
	    -nomake tests \
	    -I /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/include/SDL2 \
	    -L /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/lib \
	    -I /usr/local/i586-mingw-msvc/DirectX_June_2010/Include \
	    -L /usr/local/i586-mingw-msvc/DirectX_June_2010/Lib/x86 \
	    -I /usr/local/i586-mingw-msvc/Angle/include \
	    -L /usr/local/i586-mingw-msvc/Angle/lib \
	    -L /usr/local/i586-mingw-msvc/Angle/bin \
	    -device-option SDL_PREFIX=/usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32 \
	    -device-option SDL2_PREFIX=/usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32 \
	    -device-option ICU_PREFIX=/usr/local/i586-mingw-msvc/icu \
	    -device-option OPENGL_ES2_PREFIX=/usr/local/i586-mingw-msvc/Angle \
	    -device-option LIBS_OPENGL_ES2+=GLESv2 \
	    -device-option LIBS_OPENGL_ES2+=EGL \
	    -device-option LIBEGL_NAME=EGL.dll \
	    -device-option LIBGLESV2_NAME=GLESv2.dll \
	    -opengl dynamic \
	    -no-eglfs \
	    -no-evr \
            -pkg-config \
	    $@ \

#	    -D GL_GLEXT_PROTOTYPES \
#	    -opengl es2 \
#	    -opengles3 \
#	    -device-option ANGLE_PREFIX=/usr/local/i586-mingw-msvc/Angle \
#           -device-option QMAKE_CFLAGS+=-mno-rdrnd \
#	    -device-option QMAKE_CXXFLAGS+=-mno-rdrnd \
#            -pkg-config \
#	    -angle \
#	    -combined-angle-lib \
#	    -qt-zlib \
#	    -no-pkg-config \
#	    -largefile \
#           -debug-and-release \
#	    -no-pch \
#	    -skip qtactiveqt \
#	    -skip qtcanvas3d \
#	    -skip qt3d \
#	    -opengl dynamic \
#	    -angle 
#	    -opengl es2 \
#	    -device-option LIBEGL_NAME=EGL.dll \
#	    -device-option LIBGLESV2_NAME=GLESv2.dll \


