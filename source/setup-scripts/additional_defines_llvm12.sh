#!/bin/bash

if [ __x__"${BUILD_TYPE}" = __x__Debug ] ; then
	USE_LTO=0
	OPTIMIZE_LEVEL="-O0"
	DEBUGFLAGS+=(-gdwarf)
	DEBUGFLAGS+=(-gz)
elif [ __x__"${BUILD_TYPE}" = __x__Relwithdebinfo ] ; then
	OPTIMIZE_LEVEL="-O3"
	DEBUGFLAGS+=(-gdwarf)
	DEBUGFLAGS+=(-gz)
else
	OPTIMIZE_LEVEL="-O3"
fi

BASICOPTS+=(${OPTIMIZE_LEVEL})
if [ "__x__${OPTIMIZE_LEVEL}" != "__x__-O0" ] ; then
	COPTS+=(-fslp-vectorize)
	COPTS+=(-fvectorize)
	COPTS+=(-fstrict-vtable-pointers)
	COPTS+=(-fstrict-enums)
fi

if [ $USE_LTO -ne 0 ] ; then
	BASICOPTS+=(-flto)
fi

. ${SCRIPTS_DIR}/additional_defines_simd_types_llvm.sh


BASICOPTS+=(-Wreserved-user-defined-literal)

DLL_LDOPTS+=(-fuse-ld=lld-${TOOLCHAIN_VERSION})
EXE_LDOPTS+=(-fuse-ld=lld-${TOOLCHAIN_VERSION})

if [ $USE_LTO -ne 0 ] ; then
	DLL_LDOPTS+=(-flto=jobserver)
	EXE_LDOPTS+=(-flto=jobserver)
fi

case ${BUILD_TYPE} in
	Relwithdebinfo )
		DLL_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
		EXE_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
		if [ $USE_LTO -ne 0 ] ; then
			DLL_LDOPTS+=(-Wl,--lto-O3)
			EXE_LDOPTS+=(-Wl,--lto-O3)
		fi
		EXE_LDOPTS+=(-fwhole-program-vtables)
		;;
	Debug )
		DLL_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
		EXE_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
		;;
	Release | * )
		DLL_LDOPTS+=(-s)
		EXE_LDOPTS+=(-s)
		if [ $USE_LTO -ne 0 ] ; then
			DLL_LDOPTS+=(-Wl,--lto-O3)
			EXE_LDOPTS+=(-Wl,--lto-O3)
		fi
		EXE_LDOPTS+=(-fwhole-program-vtables)
		;;
esac
