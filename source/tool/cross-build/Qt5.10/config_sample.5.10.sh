#/bin/sh
export PATH="$PATH:\
            /usr/local/i586-mingw-msvc/icu/bin:\
	    /usr/local/i586-mingw-msvc/Angle/bin:\
	    /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/bin: \
            /usr/local/i586-mingw-msvc/DirectX_June_2010/Developer\ Runtime/x86 \
            "
export PKG_CONFIG_LIBDIR=/usr/i686-w64-mingw32/lib



#wine ./qtbase/configure.exe \
./configure \
	    -release \
	    -optimized-tools \
            -platform linux-g++ \
            -prefix /usr/local/i586-mingw-msvc/5.10.1/mingw_73x \
            -opensource -confirm-license \
	    -xplatform win32-g++ \
	    -qt-libpng -qt-libjpeg \
	    -qt-freetype \
	    -device-option CROSS_COMPILE=i686-w64-mingw32- \
	    -no-compile-examples \
	    -icu \
	    -nomake examples \
	    -nomake tests \
	    -D GL_GLEXT_PROTOTYPES \
	    -I /usr/local/i586-mingw-msvc/Angle/include \
	    -L /usr/local/i586-mingw-msvc/Angle/lib \
	    -I /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/include \
	    -L /usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32/lib \
	    -I /usr/local/i586-mingw-msvc/DirectX_June_2010/Include \
	    -L /usr/local/i586-mingw-msvc/DirectX_June_2010/Lib/x86 \
	    -device-option ANGLE_PREFIX=/usr/local/i586-mingw-msvc/Angle \
	    -device-option SDL_PREFIX=/usr/local/i586-mingw-msvc/SDL/i686-w64-mingw32 \
	    -device-option ICU_PREFIX=/usr/local/i586-mingw-msvc/icu \
	    -device-option DIRECTX_PREFIX=/usr/local/i586-mingw-msvc/DirectX_June_2010 \
	    -opengl dynamic \
	    -no-eglfs \
	    -no-pkg-config \
	    $@ \


#           -device-option QMAKE_CFLAGS+=-mno-rdrnd \
#	    -device-option QMAKE_CXXFLAGS+=-mno-rdrnd \
#            -pkg-config \
#	    -angle \
#	    -combined-angle-lib \
#	    -qt-zlib \
#	    -pkg-config \
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


