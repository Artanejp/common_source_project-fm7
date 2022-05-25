#!/bin/bash

# Maybe WORK-IN-PROGRESS 20220526 K.O

USE_CXX20=0
USE_LTO=0
USE_QT6=0

if [ __x__"${BUILD_TYPE}" = __x__Debug ] ; then
	USE_LTO=0
	OPTIMIZE_LEVEL="-O0"
	DEBUGFLAGS+=(-g2)
elif [ __x__"${BUILD_TYPE}" = __x__Relwithdebinfo ] ; then
	OPTIMIZE_LEVEL="-O3"
	DEBUGFLAGS+=(-g2)
else
	OPTIMIZE_LEVEL="-O3"
fi

BASICOPTS+=(${OPTIMIZE_LEVEL})
#if [ __x__"${BUILD_TYPE}" != __x__Release ] ; then
#	COPTS+=(-Wa,--compress-debug-sections=zlib)
#fi


# ToDo: Determine SIMD TYPES
. ${SCRIPTS_DIR}/additional_defines_simd_types_gcc.sh

#if [ $USE_LTO -ne 0 ] ; then
#	DLL_LDOPTS+=(-flto=auto)
#	EXE_LDOPTS+=(-flto=auto)
#fi

case ${BUILD_TYPE} in
	Relwithdebinfo )
		DLL_LDOPTS+=(-g2)
		EXE_LDOPTS+=(-g2)
		;;
	Debug )
		DLL_LDOPTS+=(-g2)
		EXE_LDOPTS+=(-g2)
		;;
	Release | * )
		DLL_LDOPTS+=(-s)
		EXE_LDOPTS+=(-s)
		;;
esac
