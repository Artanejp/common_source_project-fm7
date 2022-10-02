#!/bin/bash


function detect_llvm_version () {
	if [ "__xx__${TOOLCHAIN_VERSION}" = "__xx__" ] ; then
		TEST_CC="clang"
	else
		TEST_CC="clang-${TOOLCHAIN_VERSION}"
	fi
	CC_VERSION=`${TEST_CC} -dumpversion | cur -d "." -f 1`
	echo "${CC_VERSION}"
}

C_MAJOR_VERSION=`detect_llvm_version`

if [ __x__"${BUILD_TYPE}" = __x__Debug ] ; then
	USE_LTO=0
	OPTIMIZE_LEVEL="-O0"
elif [ __x__"${BUILD_TYPE}" = __x__Relwithdebinfo ] ; then
	OPTIMIZE_LEVEL="-O3"
else
	OPTIMIZE_LEVEL="-O3"
fi

BASICOPTS+=(${OPTIMIZE_LEVEL})

if [ "__x__${OPTIMIZE_LEVEL}" != "__x__-O0" ] ; then
	if [ $C_MAJOR_VERSION -le 12 ] ; then
		COPTS+=(-fslp-vectorize)
		COPTS+=(-fvectorize)
		COPTS+=(-fstrict-vtable-pointers)
		COPTS+=(-fstrict-enums)
	fi
fi

if [ $USE_LTO -ne 0 ] ; then
	BASICOPTS+=(-flto)
fi

. ${SCRIPTS_DIR}/additional_defines_simd_types_llvm.sh

BASICOPTS+=(-Wreserved-user-defined-literal)

if [ "__xx__${TOOLCHAIN_VERSION}" != "__xx__" ] ; then
	DLL_LDOPTS+=(-fuse-ld=lld-${TOOLCHAIN_VERSION})
	EXE_LDOPTS+=(-fuse-ld=lld-${TOOLCHAIN_VERSION})
else
	DLL_LDOPTS+=(-fuse-ld=lld)
	EXE_LDOPTS+=(-fuse-ld=lld)
fi
if [ $USE_LTO -ne 0 ] ; then
	DLL_LDOPTS+=(-flto=jobserver)
	EXE_LDOPTS+=(-flto=jobserver)
fi
if [ __x__"${BUILD_TYPE}" != __x__Release ] ; then
	DEBUGFLAGS+=(-gdwarf)
	DEBUGFLAGS+=(-gz)
	DLL_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
	EXE_LDOPTS+=(-Wl,--compress-debug-sections=zlib)
	if [ $C_MAJOR_VERSION -le 11 ] ; then
		COPTS+=(-Wa,--compress-debug-sections=zlib)
	fi
else
	DLL_LDOPTS+=(-s)
	EXE_LDOPTS+=(-s)
fi

if [ __x__"${BUILD_TYPE}" != __x__Debug ] ; then
	if [ $USE_LTO -ne 0 ] ; then
		DLL_LDOPTS+=(-Wl,--lto-O3)
		EXE_LDOPTS+=(-Wl,--lto-O3)
	fi
	EXE_LDOPTS+=(-fwhole-program-vtables)
fi
