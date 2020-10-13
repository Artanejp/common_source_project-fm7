#!/bin/sh
TOOLCHAIN_ROOT="/opt/llvm-mingw"
INSTALL_ROOT="/usr/local/i586-mingw-msvc"
TOOLCHAIN_PREFIX="i686-w64-mingw32"
export PATH="${TOOLCHAIN_ROOT}/bin:${INSTALL_ROOT}/lame/bin:$PATH"
export CC=clang
export CXX=clang++
export LD=${TOOLCHAIN_ROOT}/bin/${TOOLCHAIN_PREFIX}-ld
export PKG_CONFIG_PATH=${INSTALL_ROOT}/pkgconfig/lib/pkgconfig

sudo ln -s -f /usr/bin/pkg-config ${TOOLCHAIN_ROOT}/bin/${TOOLCHAIN_PREFIX}-pkg-config

./configure \
            --cross-prefix=${TOOLCHAIN_PREFIX}- \
            --prefix=${INSTALL_ROOT}/ffmpeg-4.3 \
            --pkgconfigdir=${INSTALL_ROOT}/pkgconfig/lib/pkgconfig \
            --disable-static --enable-shared \
	    --target-os=mingw32 \
	    --arch=i686 \
	    --host-cc=${TOOLCHAIN_PREFIX}-gcc \
	    --disable-mediafoundation \
	    --disable-mediacodec \
	    --disable-inline-asm \
	    --disable-amf \
	    --enable-gpl \
	    --enable-libx264 \
	    --enable-libmp3lame \
	    --enable-dxva2 \
	    --enable-libvorbis \
            --enable-cross-compile \
	    --disable-doc \
	    --extra-cflags=" \
         	    	     -I${TOOLCHAIN_ROOT}/${TOOLCHAIN_PREFIX}/include \
	                     -I${INSTALL_ROOT}/libx264/include \
	                     -I${INSTALL_ROOT}/lame/include \
			     -I${INSTALL_ROOT}/vorbis/include \
			     -I${INSTALL_ROOT}/ogg/include\
			     " \
	    --extra-cxxflags=" \
         	    	     -I${TOOLCHAIN_ROOT}/${TOOLCHAIN_PREFIX}/include \
	                     -I${INSTALL_ROOT}/libx264/include \
	                     -I${INSTALL_ROOT}/lame/include \
			     -I${INSTALL_ROOT}/vorbis/include \
			     -I${INSTALL_ROOT}/ogg/include \
			     " \
	    --extra-ldflags=" \
	                     -L${TOOLCHAIN_ROOT}/lib \
         	    	     -L${TOOLCHAIN_ROOT}/${TOOLCHAIN_PREFIX}/lib \
	                     -L/usr/${TOOLCHAIN_PREFIX}/lib \
	                     -L${INSTALL_ROOT}/libx264/lib \
	                     -L${INSTALL_ROOT}/lame/lib \
			     -L${INSTALL_ROOT}/vorbis/lin \
			     -L${INSTALL_ROOT}/ogg/lib \
			     "\

#

