#!/bin/sh

DIRECTX_ARCH=x86
#DIRECCX_ARCH=x64

SDK_PREFIX="/usr/local/i586-mingw-msvc"
SDK_PREFIX_DIRECTX="/usr/local/i586-mingw-msvc/DirectX_June_2010"

VULKAN_SDK="${SDK_PREFIX}/Vulkan"
LLVM_INSTALL_DIR="/opt/llvm-mingw-12"

ADDITIONAL_FLAGS=""
BUILD_WITH_VULKAN=1

#export PATH="$PATH:~/src/fxc2"
export PATH="${LLVM_INSTALL_DIR}/bin:$PATH:$SDK_PREFIX"
export PATH="$PATH:$SDK_PREFIX/icu/bin"
export PATH="$PATH:$SDK_PREFIX/icu/lib"
export PATH="$PATH:$SDK_PREFIX/Angle/bin"
export PATH="$PATH:$SDK_PREFIX/SDL/i686-w64-mingw32/bin"
export PATH="$PATH:$SDK_PREFIX_DIRECTX/Utilities/bin/${DIRECTX_ARCH}"
export PATH="$PATH:$SDK_PREFIX_DIRECTX/Developer Runtime/${DIRECTX_ARCH}"
export PATH="$PATH:$VULKAN_SDK/bin"

export PKG_CONFIG_LIBDIR=${SDK_PREFIX}/pkgconfig/lib
export PKG_CONFIG_PATH=${SDK_PREFIX}/pkgconfig/lib/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=${SDK_PREFIX}/pkgconfig
#export QMAKE_DXSDK_DIR=${SDK_PREFIX}/DirectX_June_2010/

if [ ${BUILD_WITH_VULKAN} -ne 0 ] ; then
   ADDITIONAL_FLAGS="${ADDITIONAL_FLAGS} \
            -device-option VULKAN_PREFIX=$VULKAN_SDK \
	    -device-option QMAKE_INCDIR_VULKAN=$VULKAN_SDK/include \
	    -device-option QMAKE_LIBDIR_VULKAN=$VULKAN_SDK/lib \
	    -I $VULKAN_SDK/include \
	    -I $VULKAN_SDK/include/vulkan \
	    -L $VULKAN_SDK/lib \
	    -L $VULKAN_SDK/bin \
	    -vulkan "
fi

#wine ./qtbase/configure.exe \
./configure \
	    -release \
            -opensource -confirm-license \
	    -device-option CROSS_COMPILE=i686-w64-mingw32- \
	    -optimized-tools \
            -platform linux-g++ \
            -prefix ${SDK_PREFIX}/Qt5.15/mingw_82x \
	    -xplatform win32-clang-g++ \
	    -qt-libpng \
	    -qt-libjpeg \
	    -qt-freetype \
	    -device-option LLVM_INSTALL_DIR="${LLVM_INSTALL_DIR}" \
	    -I ${SDK_PREFIX}/Angle/include \
	    -L ${SDK_PREFIX}/Angle/lib \
	    -L ${SDK_PREFIX}/Angle/bin \
	    -I $SDK_PREFIX/SDL/i686-w64-mingw32/include/SDL2 \
	    -I $SDK_PREFIX/SDL/i686-w64-mingw32/include \
	    -L $SDK_PREFIX/SDL/i686-w64-mingw32/lib \
	    -I ${SDK_PREFIX_DIRECTX}/Include \
	    -L ${SDK_PREFIX_DIRECTX}/Lib/${DIRECTX_ARCH} \
	    -I $SDK_PREFIX/icu/include \
	    -L $SDK_PREFIX/icu/lib \
	    -device-option SDL_PREFIX=$SDK_PREFIX/SDL/i686-w64-mingw32 \
	    -device-option SDL2_PREFIX=$SDK_PREFIX/SDL/i686-w64-mingw32 \
	    -device-option LIBS_SDL2+=SDLmain \
	    -device-option ICU_PREFIX=$SDK_PREFIX/icu \
	    -device-option LIBS_OPENGL_ES2+=GLESv2 \
	    -device-option LIBS_OPENGL_ES2+=EGL \
	    -device-option LIBEGL_NAME=EGL.dll \
	    -device-option LIBGLESV2_NAME=GLESv2.dll \
	    -device-option OPENGL_ES2_PREFIX=$SDK_PREFIX/Angle \
	    -device-option QMAKE_DXSDK_DIR=${SDK_PREFIX_DIRECTX} \
	    -device-option DXSDK_DIR=${SDK_PREFIX_DIRECTX} \
	    -device-option QSG_RHI=1 \
	    -device-option QMAKE_CFLAGS+="-fno-builtin-stpcpy" \
	    -D"stpcpy\(d,s\)=__builtin_stpcpy\(d,s\)" \
	    -D"_aligned_malloc\(s,a\)=__mingw_aligned_malloc\(s,a\)" \
	    -D"_aligned_free\(m\)=__mingw_aligned_free\(m\)" \
	    -D"_aligned_offset_realloc\(m,s,a,o\)=__mingw_aligned_offset_realloc\(m,s,a,o\)" \
	    -D"_aligned_realloc\(m,s,o\)=__mingw_aligned_realloc\(m,s,o\)" \
	    \
	    -device-option QMAKE_CXXFLAGS+="-fno-builtin-stpcpy" \
	    -device-option QMAKE_LFLAGS+="-lc++abi -lunwind -lssp" \
	    \
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
	    -c++std c++17 \
	    -nomake tests \
	    ${ADDITIONAL_FLAGS} \
	    $@ 

#	    -c++std c++17 \
#	    -angle \
#	    -device-option QMAKE_INCDIR_OPENGL_ES2="${SDK_PREFIX}/Angle/include" \
#	    -device-option QMAKE_LIBDIR_OPENGL_ES2="${SDK_PREFIX}/Angle/lib" \
