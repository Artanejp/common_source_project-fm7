#!/bin/sh
VULKAN_SDK="/usr/local/i586-mingw-msvc/Vulkan"
SDK_PREFIX="/usr/local/i586-mingw-msvc"
LLVM_INSTALL_DIR="/opt/llvm-mingw"
#export PATH="$PATH:~/src/fxc2"
export PATH="/opt/llvm-mingw/bin:$PATH:$SDK_PREFIX"
export PATH="$PATH:$SDK_PREFIX/icu/bin"
export PATH="$PATH:$SDK_PREFIX/icu/lib"
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
            -opensource -confirm-license \
	    -device-option CROSS_COMPILE=i686-w64-mingw32- \
	    -optimized-tools \
            -platform linux-g++ \
            -prefix /usr/local/i586-mingw-msvc/5.15/mingw_82x \
	    -xplatform win32-clang-g++ \
	    -qt-libpng \
	    -qt-libjpeg \
	    -qt-freetype \
	    -I /usr/local/i586-mingw-msvc/Angle/include \
	    -L /usr/local/i586-mingw-msvc/Angle/lib \
	    -L /usr/local/i586-mingw-msvc/Angle/bin \
	    -I $SDK_PREFIX/SDL/i686-w64-mingw32/include/SDL2 \
	    -I $SDK_PREFIX/SDL/i686-w64-mingw32/include \
	    -L $SDK_PREFIX/SDL/i686-w64-mingw32/lib \
	    -I $SDK_PREFIX/DirectX_June_2010/Include \
	    -L $SDK_PREFIX/DirectX_June_2010/Lib/x86 \
	    -I $SDK_PREFIX/icu/include \
	    -L $SDK_PREFIX/icu/lib \
	    -I $VULKAN_SDK/include \
	    -I $VULKAN_SDK/include/vulkan \
	    -L $VULKAN_SDK/lib \
	    -L $VULKAN_SDK/bin \
	    -device-option SDL_PREFIX=$SDK_PREFIX/SDL/i686-w64-mingw32 \
	    -device-option SDL2_PREFIX=$SDK_PREFIX/SDL/i686-w64-mingw32 \
	    -device-option LIBS_SDL2+=SDLmain \
	    -device-option ICU_PREFIX=$SDK_PREFIX/icu \
	    -device-option VULKAN_PREFIX=$VULKAN_SDK \
	    -device-option LIBS_OPENGL_ES2+=GLESv2 \
	    -device-option LIBS_OPENGL_ES2+=EGL \
	    -device-option LIBEGL_NAME=EGL.dll \
	    -device-option LIBGLESV2_NAME=GLESv2.dll \
	    -device-option OPENGL_ES2_PREFIX=$SDK_PREFIX/Angle \
	    -device-option QMAKE_DXSDK_DIR=/usr/local/i586-mingw-msvc/DirectX_June_2010 \
	    -device-option DXSDK_DIR=/usr/local/i586-mingw-msvc/DirectX_June_2010 \
	    -device-option QSG_RHI=1 \
	    -opengl dynamic \
	    -no-eglfs \
	    -no-evr \
	    -feature-direct3d9 \
	    -feature-dxguid \
	    -feature-direct3d11 \
            -pkg-config \
	    -icu \
	    -skip qtactiveqt \
	    -skip qtlocation \
	    -skip qtwebglplugin \
	    -skip qtwebengine \
	    -skip qtwebview \
	    -skip qtconnectivity \
	    -nomake tests \
	    -c++std c++17 \
	    $@ 

#	    -feature-vulkan \

#            -platform linux-g++ \
#	    -xplatform win32-g++ \
#	    -feature-opengles3 \
#	    -no-compile-examples \
#	    -nomake examples \
#	    -no-pch \
#	    -angle 
#	    -opengl es2 \
#	    -combined-angle-lib \
#	    -opengl dynamic \

#	    -I /usr/local/i586-mingw-msvc/Angle/include \
#	    -L /usr/local/i586-mingw-msvc/Angle/lib \
#	    -L /usr/local/i586-mingw-msvc/Angle/bin \
#	    -D GL_GLEXT_PROTOTYPES \
#	    -device-option ANGLE_PREFIX=/usr/local/i586-mingw-msvc/Angle \
#           -device-option QMAKE_CFLAGS+=-mno-rdrnd \
#	    -device-option QMAKE_CXXFLAGS+=-mno-rdrnd \
#	    -qt-zlib \
#	    -no-pkg-config \
#	    -largefile \
#           -debug-and-release \




