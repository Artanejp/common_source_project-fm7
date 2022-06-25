#!/bin/bash

# Maybe WORK-IN-PROGRESS 20220526 K.O

if [ __x__"${BUILD_TYPE}" = __x__Debug ] ; then
	USE_LTO=0
	OPTIMIZE_LEVEL="-O0"
	DEBUGFLAGS+=(-ggdb)
	DEBUGFLAGS+=(-gz)
	DEBUGFLAGS+=(-fmerge-debug-strings)
	DEBUGFLAGS+=(-feliminate-unused-debug-symbols)
	DEBUGFLAGS+=(-feliminate-unused-debug-types)
elif [ __x__"${BUILD_TYPE}" = __x__Relwithdebinfo ] ; then
	OPTIMIZE_LEVEL="-O3"
	DEBUGFLAGS+=(-ggdb)
	DEBUGFLAGS+=(-gz)
	DEBUGFLAGS+=(-fmerge-debug-strings)
	DEBUGFLAGS+=(-feliminate-unused-debug-symbols)
	DEBUGFLAGS+=(-feliminate-unused-debug-types)
else
	OPTIMIZE_LEVEL="-O3"
fi

BASICOPTS+=(${OPTIMIZE_LEVEL})
if [ __x__"${BUILD_TYPE}" != __x__Release ] ; then
	COPTS+=(-Wa,--compress-debug-sections=zlib)
fi

if [ $USE_LTO -ne 0 ] ; then
	BASICOPTS+=(-flto)
	LDOPTS+=(-flto-compression-level=19)
	COPTS+=(-flto-compression-level=19)
	COPTS+=(-ffat-lto-objects)
fi

# ToDo: Determine SIMD TYPES
. ${SCRIPTS_DIR}/additional_defines_simd_types_gcc.sh

if [ $USE_LTO -ne 0 ] ; then
	DLL_LDOPTS+=(-flto=auto)
	EXE_LDOPTS+=(-flto=auto)
fi

case ${BUILD_TYPE} in
	Relwithdebinfo )
		DLL_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
		EXE_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
		;;
	Debug )
		DLL_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
		EXE_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
		;;
	Release | * )
		DLL_LDOPTS+=(-s)
		EXE_LDOPTS+=(-s)
		;;
esac
