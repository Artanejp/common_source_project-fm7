#!/bin/sh

export LLVM_INSTALL_DIR="/opt/llvm-mingw-11"
export __QT_SDK_PREFIX="/usr/local/i586-mingw-msvc"
export __CROSS_ARCH=i686-w64-mingw32-
export __QT_SDK_PREFIX_DIRECTX="/usr/local/i586-mingw-msvc/DirectX_June_2010"

export PATH="${LLVM_INSTALL_DIR}/bin:\
            $PATH:\
            ${__QT_SDK_PREFIX}/icu/bin:\
	    ${__QT_SDK_PREFIX}/Angle/bin:\
	    ${__QT_SDK_PREFIX}/SDL/i686-w64-mingw32/bin: \
            ${__QT_SDK_PREFIX_DIRECTX}/Developer Runtime/x86 \
            "

export PKG_CONFIG_LIBDIR=${__QT_SDK_PREFIX}/pkgconfig/lib

#taskset 0xfbe make CROSS_COMPILE=i686-w64-mingw32- PKG_CONFIG_LIBDIR=/usr/i686-w64-mingw32/lib $@ 2>&1 
make \
     CROSS_COMPILE=${__CROSS_ARCH} \
     PKG_CONFIG_LIBDIR=${PKG_CONFIG_LIBDIR} \
     $@ 2>&1 