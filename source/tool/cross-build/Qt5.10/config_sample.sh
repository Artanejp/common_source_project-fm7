#/bin/sh
export PATH="$PATH:/usr/local/i586-mingw-msvc/icu/bin:/usr/local/i586-mingw-msvc/Angle/bin"
export PKG_CONFIG_LIBDIR=/usr/i686-w64-mingw32/lib

#wine ./qtbase/configure.exe \
./configure \
            -release \
	    -sse2 -sse3 \
	    -optimized-tools \
            -platform linux-g++ \
            -prefix /usr/local/i586-mingw-msvc/5.10.0/mingw_720 \
            -opensource -confirm-license \
	    -xplatform win32-g++ \
	    -qt-zlib -qt-libpng -qt-libjpeg \
	    -qt-freetype \
	    -device-option CROSS_COMPILE=i686-w64-mingw32- \
	    -device-option LIBEGL_NAME=EGL.dll \
	    -device-option LIBGLESV2_NAME=GLESv2.dll \
	    -no-pch \
	    -no-compile-examples \
	    -icu \
	    -nomake examples \
	    -nomake tests \
	    -opengl dynamic \
	    -skip qtactiveqt \
	    -D GL_GLEXT_PROTOTYPES \
	    -I /usr/local/i586-mingw-msvc/SDL/include \
	    -L /usr/local/i586-mingw-msvc/SDL/lib/x86 \
	    -I /usr/local/i586-mingw-msvc/DirectX_June_2010/Include \
	    -L /usr/local/i586-mingw-msvc/DirectX_June_2010/Lib/x86 \
	    $@ \
	    | tee > configure_status.log
	    
#	    -I /usr/local/i586-mingw-msvc/Angle/include \
#	    -L /usr/local/i586-mingw-msvc/Angle/lib \

#            -pkg-config \
#	    -skip qtcanvas3d \
#	    -skip qt3d \
#	    -opengl dynamic \
#	    -angle 
#	    -opengl es2 \
#	    -pkg-config \
#	    -no-eglfs \


