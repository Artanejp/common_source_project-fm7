#!/bin/bash

# You can set variables before executing:
# SRCROOT				: STRING
# CMAKEFILES_DIR		: STRING : Set root directory of CMake scripts DEFAULT=${SRCROOT}/source/cmake .
# CMAKE_TOOLCHAINS_DIR	: STRING : Set root directory of CMake Toolchains. DEFAULT=${CMAKEFILES__DIR}/toolchains .
# BUILD_TYPE			: STRING : Set build type with CMAKE. DEFAULT="Relwithdebinfo".
# NO_INCLUDE_DEFAULTS	: STRING : SET "1" not try to include default values.

typeset -i USE_QT6
typeset -i USE_LTO
typeset -i USE_CXX20
USE_QT6=0
USE_LTO=1
USE_CXX20=0

BUILD_TYPE=""
TOOLCHAIN_TYPE=gcc
TOOLCHAIN_VERSION=

declare -a SCRIPT_LIST
declare -a COPTS
declare -a EXE_LDOPTS
declare -a DLL_LDOPTS
declare -a CMAKE_OPTS

# Below are defined by a variation.
declare -a COPTS_PREFIX
declare -a LDOPTS_PREFIX
declare -a EXE_LDOPTS_PREFIX
declare -a DLL_LDOPTS_PREFIX

declare -a DEBUGFLAGS
declare -a BASICOPTS


# Interpret arguments.
if [ __x__"${NO_INCLUDE_DEFAULTS}" != __x__1 ] ; then
	if [ -e build_setup_defaults.txt ] ; then
		. build_setup_defaults.txt
	fi
fi
if [ __x__"${TARGET_ARCH}" = __x__ ] ; then
	TARGET_ARCH=x86_64
fi


for x in $@ ; do
	case "$1" in
		--srcroot | -r )
			shift
			SRCROOT="$1"
			;;
		--scripts-dir | -s )
			shift
			SCRIPTS_DIR="$1"
			;;
		--build-type | -type | -t )
			shift
			BUILD_TYPE="$1"
			;;
		--copts )
			shift
			COPTS+=("$1")
			;;
		--ldopts )
			shift
			LDOPTS+=("$1")
			;;
		--dll-ldopts )
			shift
			DLL_LDOPTS+=("$1")
			;;
		--exe-ldopts )
			shift
			EXE_LDOPTS+=("$1")
			;;
		--cmakefiles-dir | -d )
			shift
			CMAKEFILES_DIR="$1"
			;;
		--cmake-toolchains-dir | -td )
			shift
			CMAKE_TOOLCHAINS_DIR="$1"
			;;
		--cmake-toolchain-name | --toolchain | -t )
			shift
			TOOLCHAIN_NAME="$1"
			;;
		--toolchain-type | --type )
			shift
			TOOLCHAIN_TYPE="$1"
			;;
		--toolchain-version | --V )
			shift
			TOOLCHAIN_VERSION="$1"
			;;
		--toolchain-path )
			shift
			TOOLCHAIN_PATH="$1"
			;;
		--cross-build-type | -c )
			shift
			CROSS_BUILD_TYPE="$1"
			;;
		--cross-libs-prefix )
			shift
			CROSS_LIBS_PREFIX="$1"
			;;
		--target-arch | -a )
			shift
			TARGET_ARCH="$1"
			;;
		--qt6 | -6 )
			USE_QT6=1
			;;
		--qt5 | --no-qt6 | -5)
			USE_QT6=0
			;;
		--lto )
			USE_LTO=1
			;;
		--no-lto )
			USE_LTO=0
			;;
		--cxx20 | --cxx-20 | -20 )
			USE_CXX20=1
			;;
		--no-cxx20 | --no-cxx-20 | -oldcxx )
			USE_CXX20=0
			;;
		--include | -I )
			shift
			if [ -x "$1" ] ; then
				. $1
			fi
			;;
	esac
	shift
done

# Determine soruce root
if [ __x__"${SRCROOT}" = __x__ ] ; then
	SRCROOT="${PWD}/../.."
fi
if [ __x__"${BUILD_TYPE}" = __x__ ] ; then
	BUILD_TYPE="Releithdebinfo"
fi
if [ __x__"${CMAKEFILES_DIR}" = __x__ ] ; then
	CMAKEFILES_DIR="${SRCROOT}/source/cmake"
fi
if [ __x__"${CMAKE_TOOLCHAINS_DIR}" = __x__ ] ; then
	CMAKE_TOOLCHAINS_DIR="${CMAKEFILES_DIR}/toolchains"
fi
if [ __x__"${SCRIPTS_DIR}" = __x__ ] ; then
	SCRIPTS_DIR="${SRCROOT}/source/setup-scripts"
fi

if [ __x__"${CROSS_BUILD_TYPE}" != __x__ ] ; then
	TOOLCHAIN_NAME_PREFIX="toolchain_${CROSS_BUILD_TYPE}_cross"
	ADITIONAL_DEFINES_PREFIX="additional_defines_${CROSS_BUILD_TYPE}_cross"
else
	TOOLCHAIN_NAME_PREFIX="toolchain_native"
	ADITIONAL_DEFINES_PREFIX="additional_defines"
fi

case "${TOOLCHAIN_TYPE}" in
	llvm* | llvm )
		if [ __x__"${TOOLCHAIN_VERSION}" != __x__ ] ; then
		    CMAKE_OPTS+=(-DCMAKE_CSP_LLVM_VERSION=${TOOLCHAIN_VERSION})
		else
		    CMAKE_OPTS+=(-UCMAKE_CSP_LLVM_VERSION)
		fi
		;;
	gcc* | gcc )
		if [ __x__"${TOOLCHAIN_VERSION}" != __x__ ] ; then
		    CMAKE_OPTS+=(-DCMAKE_CSP_GCC_VERSION=${TOOLCHAIN_VERSION})
		else
		    CMAKE_OPTS+=(-UCMAKE_CSP_GCC_VERSION)
		fi
		;;
	* )
		if [ __x__"${TOOLCHAIN_VERSION}" = __x__ ] ; then
			TOOLCHAIN_VERSION=11
		fi
		CMAKE_OPTS+=(-DCMAKE_CSP_GCC_VERSION=${TOOLCHAIN_VERSION})
		;;
esac
if [ __x__"${TOOLCHAIN_NAME}" = __x__ ] ; then
	case "${TOOLCHAIN_TYPE}" in
		llvm* | llvm )
			TOOLCHAIN_NAME="${TOOLCHAIN_NAME_PREFIX}_llvm-versioned.cmake"
			TOOLCHAIN_SHORT_NAME="llvm"
			;;
		gcc* | gcc )
			TOOLCHAIN_NAME="${TOOLCHAIN_NAME_PREFIX}_gcc-versioned.cmake"
			#TOOLCHAIN_NAME="${TOOLCHAIN_NAME_PREFIX}_gcc${TOOLCHAIN_VERSION}.cmake"
			TOOLCHAIN_SHORT_NAME="gcc"
			;;
		* )
			TOOLCHAIN_NAME="${TOOLCHAIN_NAME_PREFIX}_gcc-versioned.cmake"
			TOOLCHAIN_SHORT_NAME="gcc"
			;;
	esac
fi


case "${TOOLCHAIN_TYPE}" in
	llvm* | llvm | gcc* | gcc )
		if [ __x__"${CROSS_BUILD_TYPE}" != __x__ ] ; then
			ADDITONAL_DEFINES_FILE=" ${SCRIPTS_DIR}/${ADITIONAL_DEFINES_PREFIX}_${TOOLCHAIN_SHORT_NAME}${TOOLCHAIN_VERSION}.sh"
		else
			ADDITONAL_DEFINES_FILE=" ${SCRIPTS_DIR}/${ADITIONAL_DEFINES_PREFIX}_${TOOLCHAIN_SHORT_NAME}_generic.sh"
		fi
		;;
	* )
		ADDITONAL_DEFINES_FILE=" ${SCRIPTS_DIR}/${ADITIONAL_DEFINES_PREFIX}_${TOOLCHAIN_SHORT_NAME}${TOOLCHAIN_VERSION}.sh"
		;;
esac
echo ${ADDITONAL_DEFINES_FILE}

if [ $USE_QT6 -ne 0 ] ; then
	CMAKE_OPTS+=(-DUSE_QT_6=ON)
else
	CMAKE_OPTS+=(-DUSE_QT_6=OFF)
fi

if [ $USE_LTO -ne 0 ] ; then
	CMAKE_OPTS+=(-DUSE_LTO=ON)
else
	CMAKE_OPTS+=(-DUSE_LTO=OFF)
fi

if [ $USE_CXX20 -ne 0 ] ; then
	CMAKE_OPTS+=(-DCSP_BUILD_WITH_CXX20=ON)
else
	CMAKE_OPTS+=(-DCSP_BUILD_WITH_CXX20=OFF)
fi

if [ __x__"${SIMD_TYPE}" = __x__ ] ; then
	case ${TARGET_ARCH} in
		arm64 | armv9 )
			SIMD_TYPE=""
			;;
		x86_64 | i386 | i686 | * )
			SIMD_TYPE=sse2
			;;
	esac
fi

# ToDo: Default values
if [ -e ${ADDITONAL_DEFINES_FILE} ] ; then
	. ${ADDITONAL_DEFINES_FILE}
fi


CMAKE_ARG_LIST=""

COPTS_ARG_STR=`echo ${BASICOPTS[@]} ${DEBUGFLAGS[@]} ${COPTS_PREFIX[@]} ${COPTS[@]}`
EXE_LDOPTS_STR=`echo ${BASICOPTS[@]} ${DEBUGFLAGS[@]} ${LDOPTS_PREFIX[@]} ${EXE_LDOPTS_PREFIX[@]} ${LDOPTS[@]} ${EXE_LDOPTS[@]}`
DLL_LDOPTS_STR=`echo ${BASICOPTS[@]} ${DEBUGFLAGS[@]} ${LDOPTS_PREFIX[@]} ${DLL_LDOPTS_PREFIX[@]} ${LDOPTS[@]} ${DLL_LDOPTS[@]}`



# GO!
cmake "${SRCROOT}/source" \
	  -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAINS_DIR}/${TOOLCHAIN_NAME}" \
	  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
	  ${CMAKE_OPTS_PREFIX[@]} \
	  -DCMAKE_C_FLAGS="${COPTS_ARG_STR}" \
	  -DCMAKE_CXX_FLAGS="${COPTS_ARG_STR}" \
	  -DCMAKE_EXE_LINKER_FLAGS="${EXE_LDOPTS_STR}" \
	  -DCMAKE_MODULE_LINKER_FLAGS="${DLL_LDOPTS_STR}" \
	  ${CMAKE_OPTS[@]} \

	  
