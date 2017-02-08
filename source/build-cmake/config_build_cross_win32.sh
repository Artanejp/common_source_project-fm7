#!/bin/bash

CMAKE=/usr/bin/cmake

BUILD_TYPE="Relwithdebinfo"
CMAKE_APPENDFLAG=""
export WINEDEBUG="-all"
CMAKE_LINKFLAG=""
CMAKE_APPENDFLAG=""
MAKEFLAGS_GENERAL="-j4"
export WCLANG_FORCE_CXX_EXCEPTIONS=1

mkdir -p ./bin-win32/

if [ -e ./buildvars_mingw_cross_win32.dat ] ; then
    . ./buildvars_mingw_cross_win32.dat
fi
case ${BUILD_TOOLCHAIN} in
   "LLVM" | "llvm" | "CLANG" | "clang" )
          TOOLCHAIN_SCRIPT="../../cmake/toolchain_win32_cross_linux_llvm.cmake"
	  . ./params/buildvars_mingw_params_llvm.dat
	  echo "Setup for LLVM"
	  ;;
   "GCC" | "gcc" | "GNU" )
	  TOOLCHAIN_SCRIPT="../../cmake/toolchain_mingw_cross_linux.cmake"
	  . ./params/buildvars_mingw_params_gcc.dat
	  echo "Setup for GCC"
	  ;;
   * )
	  TOOLCHAIN_SCRIPT="../../cmake/toolchain_mingw_cross_linux.cmake"
	  . ./params/buildvars_mingw_params_gcc.dat
	  echo "ASSUME GCC"
	  ;;
esac   

CMAKE_APPENDFLAG="${CMAKE_APPENDFLAG} -DLIBAV_ROOT_DIR=${FFMPEG_DIR}"

case ${USE_LTO} in
   "Yes" | "yes" | "YES" )
     MAKEFLAGS_BASE2="-flto ${MAKEFLAGS_BASE2}"
     MAKEFLAGS_LINK_BASE="-flto ${MAKEFLAGS_BASE2}"
   ;;
   "No" | "no" | "NO" | * )
     MAKEFLAGS_BASE2="-flto ${MAKEFLAGS_BASE2}"
     MAKEFLAGS_LINK_BASE="-flto ${MAKEFLAGS_BASE2}"
   ;;
esac

case ${STRIP_SYMBOLS} in
   "Yes" | "yes" | "YES" )
     MAKEFLAGS_BASE2="-s ${MAKEFLAGS_BASE2}"
     MAKEFLAGS_LINK_BASE="-s ${MAKEFLAGS_LINK_BASE}"
   ;;
   "No" | "no" | "NO" | * )
     MAKEFLAGS_BASE2="-g -ggdb ${MAKEFLAGS_BASE2}"
     MAKEFLAGS_LINK_BASE="-g -ggdb ${MAKEFLAGS_LINK_BASE}"
   ;;
esac
###########################
#
#
MAKEFLAGS_CXX="${MAKEFLAGS_BASE2} "
MAKEFLAGS_CC="${MAKEFLAGS_BASE2}"
MAKEFLAGS_LIB_CXX="${ADDITIONAL_MAKEFLAGS_LINK_LIB} ${MAKEFLAGS_BASE2}"
MAKEFLAGS_LIB_CC="${ADDITIONAL_MAKEFLAGS_LINK_LIB}  ${MAKEFLAGS_BASE2}"

CMAKE_LINKFLAG="${ADDITIONAL_MAKEFLAGS_LINK_EXE}  ${MAKEFLAGS_LINK_BASE}"
CMAKE_DLL_LINKFLAG="${ADDITIONAL_MAKEFLAGS_LINK_DLL}  ${MAKEFLAGS_LINK_BASE}"

# To use MOC, please enable wine as interpreter of EXEs , below:
# $ sudo update-binfmts --install Win32_Wine /usr/bin/wine --extension exe . 
# Compatible with GCC-4.9 (-fabi-version=8)
MAKEFLAGS_CXX="${MAKEFLAGS_CXX} -DWINVER=0x501"
MAKEFLAGS_CC="${MAKEFLAGS_CC} -DWINVER=0x501"

MAKEFLAGS_LIB_CXX="${MAKEFLAGS_LIB_CXX} -DWINVER=0x501"
MAKEFLAGS_LIB_CC="${MAKEFLAGS_LIB_CC} -DWINVER=0x501"

function build_dll() {
    mkdir -p $1/build-win32
    cd $1/build-win32
    echo ${CMAKE_FLAGS1} ${CMAKE_FLAGS2}
    ${CMAKE} -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_SCRIPT} \
	     ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_LIB_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_LIB_CC}" \
	     "${CMAKE_FLAGS4}" \
	     "-DUSE_SDL2=ON" \
	     ${CMAKE_APPENDFLAG} \
	     "-DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_LINKFLAG}" \
	     "-DCMAKE_CROSSCOMPILING=true"\
	.. | tee make.log
    
    ${CMAKE} ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_LIB_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_LIB_CC}" \
	     "${CMAKE_FLAGS4}" \
	     "-DUSE_SDL2=ON" \
	     ${CMAKE_APPENDFLAG} \
	     "-DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_LINKFLAG}" \
	     .. | tee -a make.log
    
    make clean
    
    make ${MAKEFLAGS_GENERAL} 2>&1 | tee -a ./make.log
#    case $? in
#         0 ) 
#          cp ./qt/gui/libqt_gui.a ../../bin-win32/ 
#          cp ./qt/gui/*.lib ../../bin-win32/ 
#          cp ./qt/gui/*.dll ../../bin-win32/ 
#         ;;
#          * ) exit $? ;;
#    esac
    #make clean
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
case ${USE_COMMON_DEVICE_LIB} in
   "Yes" | "yes" | "YES" )
   CMAKE_FLAGS4="-DUSE_DEVICES_SHARED_LIB=ON"
   build_dll libCSPcommon_vm
   ;;
   * )
   CMAKE_FLAGS4=""
   ;;
esac
build_dll libCSPfmgen
build_dll libCSPavio
build_dll libCSPgui
build_dll libCSPosd
build_dll libCSPemu_utils


for SRCDATA in $@ ; do\

    mkdir -p ${SRCDATA}/build-win32
    cd ${SRCDATA}/build-win32
    
    echo ${CMAKE_FLAGS1} ${CMAKE_FLAGS2}
    ${CMAKE} -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_SCRIPT} \
	     ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_CC}" \
	     "${CMAKE_FLAGS4}" \
	     "-DUSE_SDL2=ON" \
	     "-DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_LINKFLAG}" \
	     ${CMAKE_APPENDFLAG} \
	     .. | tee make.log

    ${CMAKE} ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_CC}" \
	     "${CMAKE_FLAGS4}" \
	     "-DUSE_SDL2=ON" \
	     "-DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_LINKFLAG}" \
	     ${CMAKE_APPENDFLAG} \
	     .. | tee -a make.log

    make clean
    
    make ${MAKEFLAGS_GENERAL} 2>&1 | tee -a ./make.log
    case $? in
      0 ) cp ./qt/common/*.exe ../../bin-win32/ ;;
      * ) exit $? ;;
    esac
    
    make clean
    cd ../..
done

exit 0

#for ii in libCSPavio libCSPgui libCSPosd libCSPemu_utils; do
#    cd $ii/build-win32
#    make clean
#    cd ../..
#done

exit 0

