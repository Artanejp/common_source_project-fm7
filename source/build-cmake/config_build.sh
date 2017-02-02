#!/bin/bash

CMAKE=/usr/bin/cmake
CCMAKE_CC=gcc-6
CCMAKE_CXX=g++-6
LIB_INSTALL="/usr/local/lib/x86_64-linux-gnu/"

BUILD_TYPE="Relwithdebinfo"
CMAKE_APPENDFLAG=""

if [ -e ./buildvars.dat ] ; then
    . ./buildvars.dat
fi
case ${BUILD_TOOLCHAIN} in
   "LLVM" | "llvm" | "CLANG" | "clang" )
          #TOOLCHAIN_SCRIPT="../../cmake/toolchain_win32_cross_linux_llvm.cmake"
	  . ./params/buildvars_linux_params_llvm.dat
	  echo "Setup for LLVM"
	  ;;
   "GCC" | "gcc" | "GNU" )
	  #TOOLCHAIN_SCRIPT="../../cmake/toolchain_mingw_cross_linux.cmake"
	  . ./params/buildvars_linux_params_gcc.dat
	  echo "Setup for GCC"
	  ;;
   * )
	  #TOOLCHAIN_SCRIPT="../../cmake/toolchain_mingw_cross_linux.cmake"
	  . ./params/buildvars_linux_params_gcc.dat
	  echo "ASSUME GCC"
	  ;;
esac   

case ${STRIP_SYMBOLS} in
   "Yes" | "yes" | "YES" )
     MAKEFLAGS_BASE2="-s ${MAKEFLAGS_BASE2}"
     MAKEFLAGS_LINK_BASE="-s ${MAKEFLAGS_LINK_BASE2}"
     MAKEFLAGS_DLL_LINK_BASE="-s ${MAKEFLAGS_LINK_BASE2}"
   ;;
   "No" | "no" | "NO" | * )
     MAKEFLAGS_BASE2="-g -ggdb ${MAKEFLAGS_BASE2}"
     MAKEFLAGS_LINK_BASE="-g -ggdb ${MAKEFLAGS_LINK_BASE2}"
     MAKEFLAGS_DLL_LINK_BASE="-g -ggdb ${MAKEFLAGS_LINK_BASE2}"
   ;;
esac
#################
#
#
MAKEFLAGS_CXX="${MAKEFLAGS_BASE2}"
MAKEFLAGS_CC="${MAKEFLAGS_BASE2}"
MAKEFLAGS_LIB_CXX="${MAKEFLAGS_BASE2}"
MAKEFLAGS_LIB_CC="${MAKEFLAGS_BASE2}"

###################
#
#
if [ -n "${FFMPEG_DIR}" ]; then \
   CMAKE_APPENDFLAG="${CMAKE_APPENDFLAG} -DLIBAV_ROOT_DIR=${FFMPEG_DIR}"
fi
if [ -n "${QT5_DIR}" ]; then \
   CMAKE_APPENDFLAG="${CMAKE_APPENDFLAG}  -DQT5_ROOT_PATH=${QT5_DIR}"
fi

#################################
#
#
function build_dll() {
    # $1 = dir
    mkdir -p $1/build
    cd $1/build
    echo ${CMAKE_FLAGS1} ${CMAKE_FLAGS2}
    ${CMAKE} -DCMAKE_C_COMPILER:STRING=${CCMAKE_CC}  \
             -DCMAKE_CXX_COMPILER:STRING=${CCMAKE_CXX} \
	     "-DLIBCSP_INSTALL_DIR:STRING=${LIB_INSTALL}" \
	     ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_LIB_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_LIB_CC}" \
	     ${CMAKE_APPENDFLAG} \
	     .. | tee make.log
    
    ${CMAKE} -DCMAKE_C_COMPILER:STRING=${CCMAKE_CC}  \
	     -DCMAKE_CXX_COMPILER:STRING=${CCMAKE_CXX} \
	     "-DLIBCSP_INSTALL_DIR:STRING=${LIB_INSTALL}" \
	     ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_LIB_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_LIB_CC}" \
	     ${CMAKE_APPENDFLAG} \
	     .. | tee -a make.log
    
    make clean
    make ${MAKEFLAGS_GENERAL} 2>&1 | tee -a ./make.log
    case $? in
	0 ) sudo make install 2>&1 | tee -a ./make.log ;;
	* ) exit $? ;;
    esac
    
    make clean
    cd ../..
}

case ${BUILD_TYPE} in
    "Debug" | "DEBUG" | "debug" ) 
            CMAKE_FLAGS1="-DCMAKE_BUILD_TYPE:STRING=debug"
	    CMAKE_FLAGS2="-DCMAKE_CXX_FLAGS_DEBUG:STRING"
	    CMAKE_FLAGS3="-DCMAKE_C_FLAGS_DEBUG:STRING"
	    ;;
    "Release" | "RELEASE" | "release" ) 
            CMAKE_FLAGS1="-DCMAKE_BUILD_TYPE:STRING=Release"
	    CMAKE_FLAGS2="-DCMAKE_CXX_FLAGS_RELEASE:STRING"
	    CMAKE_FLAGS3="-DCMAKE_C_FLAGS_RELEASE:STRING"
	    ;;
    "Relwithdebinfo" | "RELWITHDEBINFO" | "relwithdebinfo" ) 
            CMAKE_FLAGS1="-DCMAKE_BUILD_TYPE:STRING=Relwithdebinfo"
	    CMAKE_FLAGS2="-DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING"
	    CMAKE_FLAGS3="-DCMAKE_C_FLAGS_RELWITHDEBINFO:STRING"
	    ;;
     * )
            echo "Specify BUILD_TYPE in buildvars.dat to Debug, Release, Relwithdebinfo."
	    exit -1
	    ;;
esac

# libCSPGui

build_dll libCSPcommon_vm
build_dll libCSPfmgen
build_dll libCSPavio
build_dll libCSPgui
build_dll libCSPosd
build_dll libCSPemu_utils

for SRCDATA in $@ ; do\

    mkdir -p ${SRCDATA}/build
    cd ${SRCDATA}/build
    
    echo ${CMAKE_FLAGS1} ${CMAKE_FLAGS2}
    ${CMAKE} -D CMAKE_C_COMPILER:STRING=${CCMAKE_CC}  \
             -D CMAKE_CXX_COMPILER:STRING=${CCMAKE_CXX} \
	     ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_CC}" \
	     ${CMAKE_APPENDFLAG} \
	     "-DCMAKE_EXE_LINKER_FLAGS:STRING=${MAKEFLAGS_LINK_BASE}" \
	     .. | tee make.log

    ${CMAKE} -D CMAKE_C_COMPILER:STRING=${CCMAKE_CC}  \
             -D CMAKE_CXX_COMPILER:STRING=${CCMAKE_CXX} \
	     ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_CC}" \
	     ${CMAKE_APPENDFLAG} \
	     "-DCMAKE_EXE_LINKER_FLAGS:STRING=${MAKEFLAGS_LINK_BASE}" \
	     .. | tee -a make.log

    make clean
    
    make ${MAKEFLAGS_GENERAL} 2>&1 | tee -a ./make.log
    case $? in
      0 ) sudo make install 2>&1 | tee -a ./make.log ;;
      * ) exit $? ;;
    esac
    
    make clean
    cd ../..
done

exit 0

