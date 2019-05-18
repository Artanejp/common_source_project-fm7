#!/bin/sh
VULKAN_SDK="/usr/local/i586-mingw-msvc/Vulkan"
SDK_PREFIX="/usr/local/i586-mingw-msvc"
#export PATH="$PATH:~/src/fxc2"
export PATH="$PATH:$SDK_PREFIX/icu/bin"
export PATH="$PATH:$SDK_PREFIX/Angle/bin"
export PATH="$PATH:$SDK_PREFIX/SDL/i686-w64-mingw32/bin"
export PATH="$PATH:$SDK_PREFIX/DirectX_June_2010/Utilities/bin/x86"
export PATH="$PATH:$SDK_PREFIX/DirectX_June_2010/Developer Runtime/x86"
export PATH="$PATH:$VULKAN_SDK/bin"


export PKG_CONFIG_LIBDIR=/usr/local/i586-mingw-msvc/pkgconfig/lib
export PKG_CONFIG_PATH=/usr/local/i586-mingw-msvc/pkgconfig/lib/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=/usr/local/i586-mingw-msvc/pkgconfig
#export QMAKE_DXSDK_DIR=/usr/local/i586-mingw-msvc/DirectX_June_2010/

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
	    -nomake tests \
	    -c++std c++14 \
	    -I $SDK_PREFIX/SDL/i686-w64-mingw32/include/SDL2 \
	    -I $SDK_PREFIX/SDL/i686-w64-mingw32/include \
	    -L $SDK_PREFIX/SDL/i686-w64-mingw32/lib \
	    -I $SDK_PREFIX/DirectX_June_2010/Include \
	    -L $SDK_PREFIX/DirectX_June_2010/Lib/x86 \
	    -I $VULKAN_SDK/include \
	    -I $VULKAN_SDK/include/vulkan \
	    -L $VULKAN_SDK/lib \
	    -device-option SDL_PREFIX=$SDK_PREFIX/SDL/i686-w64-mingw32 \
	    -device-option SDL2_PREFIX=$SDK_PREFIX/SDL/i686-w64-mingw32 \
	    -device-option ICU_PREFIX=$SDK_PREFIX/icu \
	    -device-option OPENGL_ES2_PREFIX=$SDK_PREFIX/Angle \
	    -device-option VULKAN_PREFIX=$VULKAN_SDK \
	    -device-option LIBS_OPENGL_ES2+=GLESv2 \
	    -device-option LIBS_OPENGL_ES2+=EGL \
	    -device-option LIBEGL_NAME=EGL.dll \
	    -device-option LIBGLESV2_NAME=GLESv2.dll \
	    -opengl dynamic \
	    -no-eglfs \
	    -no-evr \
	    -feature-direct3d9 \
	    -feature-vulkan \
            -pkg-config \
	    $@ \

#	    -device-option QMAKE_DXSDK_DIR=/usr/local/i586-mingw-msvc/DirectX_June_2010 \
#	    -device-option DXSDK_DIR=/usr/local/i586-mingw-msvc/DirectX_June_2010 \
#	    -angle \
#	    -feature-dxguid \
#	    -feature-direct3d9 \
#	    -feature-vulkan \
#
#	    -nomake examples \

#	    -I /usr/local/i586-mingw-msvc/Angle/include \
#	    -L /usr/local/i586-mingw-msvc/Angle/lib \
#	    -L /usr/local/i586-mingw-msvc/Angle/bin \
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


