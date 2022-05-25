#!/bin/bash

# Maybe WORK-IN-PROGRESS 20220526 K.O

if [ "__x__${CROSS_LIBS_PREFIX}" = "__x__" ] ; then
	CROSS_LIBS_PREFIX="/usr/local/lib/i586-mingw-msvc"
fi
if [ "__x__${CROSS_TOOLCHAIN_PREFIX}" = "__x__" ] ; then
	CROSS_TOOLCHAIN_PREFIX="/opt/llvm-${TOOLCHAIN_VERSION}"
fi
if [ "__x__${CROSS_TOOLCHAIN_TRIPLE}" = "__x__" ] ; then
	case ${TARGET_ARCH} in
		i386 | i686 )
			CROSS_TOOLCHAIN_TRIPLE=i686-w64-mingw32
			;;
		x86_64 | amd64 )
			CROSS_TOOLCHAIN_TRIPLE=x86_64-w64-mingw32
			;;
		armv7 )
			CROSS_TOOLCHAIN_TRIPLE=armv7-w64-mingw32
			;;
		aarch64 )
			CROSS_TOOLCHAIN_TRIPLE=aarch64-w64-mingw32
			;;
		* )
			CROSS_TOOLCHAIN_TRIPLE=${TARGET_ARCH}-w64-mingw32
			;;
	esac
fi

if [ "__x__${FFMPEG_NAME}" = "__x__" ] ; then
	FFMPEG_NAME="ffmpeg-4.4"
fi
if [ "__x__${FFMPEG_DIR}" = "__x__" ] ; then
	FFMPEG_DIR="${CROSS_LIBS_PREFIX}/${FFMPEG_NAME}"
fi
if [ "__x__${QT_PATH_SUFFIX}" = "__x__" ] ; then
	QT_PATH_SUFFIX="Qt5.15/mingw_82x"
fi
if [ "__x__${QT_DIR}" = "__x__" ] ; then
	QT_DIR="${CROSS_LIBS_PREFIX}/${QT_PATH_SUFFIX}"
fi


PATH=${CROSS_TOOLCHAIN_PREFIX}/${CROSS_TOOLCHAIN_TRIPLE}:$PATH

if [ __x__"${BUILD_TYPE}" = __x__Debug ] ; then
	USE_LTO=0
	OPTIMIZE_LEVEL="-O0"
	DEBUGFLAGS+=(-g2)
	DEBUGFLAGS+=(-ggdb)
	DEBUGFLAGS+=(-gz=zlib)
elif [ __x__"${BUILD_TYPE}" = __x__Relwithdebinfo ] ; then
	OPTIMIZE_LEVEL="-O3"
	DEBUGFLAGS+=(-g2)
	DEBUGFLAGS+=(-ggdb)
	DEBUGFLAGS+=(-gz=zlib)
else
	OPTIMIZE_LEVEL="-O3"
fi

case ${TARGET_ARCH} in
	i386 | i686 )
		BASICOPTS+=(-march=i686)
		;;
	* )
		;;
esac


BASICOPTS+=(${OPTIMIZE_LEVEL})

if [ $USE_LTO -ne 0 ] ; then
	BASICOPTS+=(-flto)
fi

# ToDo: Determine SIMD TYPES
. ${SCRIPTS_DIR}/additional_defines_simd_types_llvm.sh

COPTS+=(-fno-builtin-stpcpy)
COPTS+=(-Dstpcpy\(d,s\)=__builtin_stpcpy\(d,s\))

COPTS+=(-D_aligned_malloc\(s,a\)=__mingw_aligned_malloc\(s,a\))
COPTS+=(-D_aligned_free\(m\)=__mingw_aligned_free\(m\))
COPTS+=(-D_aligned_offset_realloc\(m,s,a,o\)=__mingw_aligned_offset_realloc\(m,s,a,o\))
COPTS+=(-D_aligned_realloc\(m,s,o\)=__mingw_aligned_realloc\(m,s,o\))


BASICOPTS+=(-Wreserved-user-defined-literal)

#DLL_LDOPTS+=(-fuse-ld=lld-${TOOLCHAIN_VERSION})
#EXE_LDOPTS+=(-fuse-ld=lld-${TOOLCHAIN_VERSION})
#if [ $USE_LTO -ne 0 ] ; then
#	DLL_LDOPTS+=(-flto=jobserver)
#	EXE_LDOPTS+=(-flto=jobserver)
#fi

CMAKE_OPTS+=(-DLIBAV_ROOT_DIR="${FFMPEG_DIR}")
CMAKE_OPTS+=(-DQT5_ROOT_PATH="${QT_DIR}")
CMAKE_OPTS+=(-DTARGET_ARCH="${CROSS_TOOLCHAIN_TRIPLE}")
CMAKE_OPTS+=(-DLIBS_PREFIX="${CROSS_LIBS_PREFIX}")

LDOPTS+=(-L${CROSS_TOOLCHAIN_PREFIX}/${CROSS_TOOLCHAIN_TRIPLE}/lib)
LDOPTS+=(-L/usr/${CROSS_TOOLCHAIN_TRIPLE}/lib)
case ${BUILD_TYPE} in
	Relwithdebinfo )
		BASICOPTS+=(-fslp-vectorize)
		BASICOPTS+=(-fslp-vectorize)
		BASICOPTS+=(-fstrict-vtable-pointers)
		BASICOPTS+=(-fstrict-enums)
		#DLL_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
		#EXE_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
		#if [ $USE_LTO -ne 0 ] ; then
		#	DLL_LDOPTS+=(-Wl,--lto-O3)
		#	EXE_LDOPTS+=(-Wl,--lto-O3)
		#fi
		#EXE_LDOPTS+=(-fwhole-program-vtables)
		;;
	Debug )
		#DLL_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
		#EXE_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
		;;
	Release | * )
		BASICOPTS+=(-fslp-vectorize)
		BASICOPTS+=(-fslp-vectorize)
		BASICOPTS+=(-fstrict-vtable-pointers)
		BASICOPTS+=(-fstrict-enums)
		DLL_LDOPTS+=(-s)
		EXE_LDOPTS+=(-s)
		#if [ $USE_LTO -ne 0 ] ; then
		#	DLL_LDOPTS+=(-Wl,--lto-O3)
		#	EXE_LDOPTS+=(-Wl,--lto-O3)
		#fi
		#EXE_LDOPTS+=(-fwhole-program-vtables)
		;;
esac
