#!/bin/bash

function detect_gcc_version () {
	if [ "__xx__${TOOLCHAIN_VERSION}" = "__xx__" ] ; then
		TEST_CC="gcc"
	else
		TEST_CC="gcc-${TOOLCHAIN_VERSION}"
	fi
	CC_VERSION=`${TEST_CC} -dumpversion`
	echo "${CC_VERSION}"
}

C_MAJOR_VERSION=`detect_gcc_version`

if [ $C_MAJOR_VERSION -lt 8 ] ; then
	USE_CXX20=0
	USE_LTO=0
	USE_QT6=0
fi

if [ __x__"${BUILD_TYPE}" = __x__Debug ] ; then
	USE_LTO=0
	OPTIMIZE_LEVEL="-O0"
elif [ __x__"${BUILD_TYPE}" = __x__Relwithdebinfo ] ; then
	OPTIMIZE_LEVEL="-O3"
else
	OPTIMIZE_LEVEL="-O3"
fi

BASICOPTS+=(${OPTIMIZE_LEVEL})

if [ __x__"${BUILD_TYPE}" != __x__Release ] ; then
	
	if [ $C_MAJOR_VERSION -lt 8 ] ; then
		DEBUGFLAGS+=(-g2)
	else
		DEBUGFLAGS+=(-ggdb)
		DEBUGFLAGS+=(-gz)
		if [ $C_MAJOR_VERSION -ge 12 ] ; then
			COPTS+=(-Wa,--compress-debug-sections=zlib)
			DEBUGFLAGS+=(-fmerge-debug-strings)
			DEBUGFLAGS+=(-feliminate-unused-debug-symbols)
			DEBUGFLAGS+=(-feliminate-unused-debug-types)
		else
			COPTS+=(-Wa,--compress-debug-sections=zlib)
		fi
	fi
fi

if [ $USE_LTO -ne 0 ] ; then
	BASICOPTS+=(-flto)
	DLL_LDOPTS+=(-flto=auto)
	EXE_LDOPTS+=(-flto=auto)
	
	if [ $C_MAJOR_VERSION -ge 12 ] ; then
		COPTS+=(-flto-compression-level=19)
		COPTS+=(-fno-fat-lto-objects)
	else
		COPTS+=(-flto-compression-level=9)
		COPTS+=(-fno-fat-lto-objects)
	fi
fi

# ToDo: Determine SIMD TYPES
. ${SCRIPTS_DIR}/additional_defines_simd_types_gcc.sh

case ${BUILD_TYPE} in
	Relwithdebinfo | Debug)
		if [ $C_MAJOR_VERSION -lt 8 ] ; then
			DLL_LDOPTS+=(-g2)
			EXE_LDOPTS+=(-g2)
		else
			DLL_LDOPTS+=(-ggdb)
			DLL_LDOPTS+=(-gz)
			DLL_LDOPTS+=(-Wl,--compress-debug-sections=zlib)

			EXE_LDOPTS+=(-ggdb)
			EXE_LDOPTS+=(-gz)
			EXE_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
		fi
		;;
	Release | * )
		DLL_LDOPTS+=(-s)
		EXE_LDOPTS+=(-s)
		;;
esac
