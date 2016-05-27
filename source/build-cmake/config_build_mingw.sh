#!/bin/bash

CMAKE=cmake
CCMAKE_CC=gcc
CCMAKE_CXX=g++

MAKEFLAGS_CXX="-g -O2 -DNDEBUG"
MAKEFLAGS_CC="-g -O2 -DNDEBUG"
BUILD_TYPE="Relwithdebinfo"

CMAKE_LINKFLAG=""
#CMAKE_GENTYPE="\"MinGW Makefiles\""
#CMAKE_APPENDFLAG=""
CMAKE_GENTYPE="\"MSYS Makefiles\""
CMAKE_GENFLAGS="-DCMAKE_MAKE_PROGRAM=mingw32-make"
mkdir -p ./bin-win32/
if [ -e ./buildvars_mingw.dat ] ; then
    . ./buildvars_mingw.dat
fi


MAKEFLAGS_CXX="${MAKEFLAGS_CXX} -DWINVER=0x501"
MAKEFLAGS_CC="${MAKEFLAGS_CC} -DWINVER=0x501"
MAKEFLAGS_LIB_CXX="${MAKEFLAGS_LIB_CXX} -DWINVER=0x501"
MAKEFLAGS_LIB_CC="${MAKEFLAGS_LIB_CC} -DWINVER=0x501"

function build_dll() {
    mkdir -p $1/build-win32
    cd $1/build-win32
    echo ${CMAKE_FLAGS1} ${CMAKE_FLAGS2}
    ${CMAKE} -G "${CMAKE_GENTYPE}" \
	     ${CMAKE_GENFLAGS} \
             -D CMAKE_C_COMPILER:STRING=${CCMAKE_CC}  \
	     -D CMAKE_CXX_COMPILER:STRING=${CCMAKE_CXX} \
	     ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_LIB_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_LIB_CC}" \
	     ${CMAKE_APPENDFLAG} \
	     ${CMAKE_LINKFLAG} \
	     .. | tee make.log
    
    ${CMAKE} -D CMAKE_C_COMPILER:STRING=${CCMAKE_CC}  \
             -D CMAKE_CXX_COMPILER:STRING=${CCMAKE_CXX} \
	     ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_LIB_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_LIB_CC}" \
	     ${CMAKE_APPENDFLAG} \
	     ${CMAKE_LINKFLAG} \
	     .. | tee -a make.log
    
    mingw32-make clean
    mingw32-make ${MAKEFLAGS_GENERAL} 2>&1 | tee -a ./make.log
    
    case $? in
	0 ) 
	#      cp ./qt/gui/libqt_gui.a ../../bin-win32/ 
	#      cp ./qt/gui/*.lib ../../bin-win32/ 
	#      cp ./qt/gui/*.dll ../../bin-win32/ 
	;;
	* ) exit $? ;;
    esac
    #mingw32-make clean
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
build_dll libCSPavio
build_dll libCSPgui
build_dll libCSPosd
build_dll libCSPemu_utils

for SRCDATA in $@ ; do\

    mkdir -p ${SRCDATA}/build
    cd ${SRCDATA}/build
    
    echo ${CMAKE_FLAGS1} ${CMAKE_FLAGS2}
    ${CMAKE} -G "${CMAKE_GENTYPE}" \
	     ${CMAKE_GENFLAGS} \
             -D CMAKE_C_COMPILER:STRING=${CCMAKE_CC}  \
             -D CMAKE_CXX_COMPILER:STRING=${CCMAKE_CXX} \
	     ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_CC}" \
	     ${CMAKE_APPENDFLAG} \
	     ${CMAKE_LINKFLAG} \
	     .. | tee make.log

    ${CMAKE} -D CMAKE_C_COMPILER:STRING=${CCMAKE_CC}  \
             -D CMAKE_CXX_COMPILER:STRING=${CCMAKE_CXX} \
	     ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_CC}" \
	     ${CMAKE_APPENDFLAG} \
	     ${CMAKE_LINKFLAG} \
	     .. | tee -a make.log

    mingw32-make clean
    
    mingw32-make ${MAKEFLAGS_GENERAL} 2>&1 | tee -a ./make.log
    case $? in
      0 ) cp ./qt/common/*.exe ../../bin-win32/ ;;
#     0 ) sudo make install 2>&1 | tee -a ./make.log ;;
      * ) exit $? ;;
    esac
    
    mingw32-make clean
    cd ../..
done

for ii in libCSPavio libCSPgui libCSPosd libCSPemu_utils; do
    cd $ii/build-win32
    make clean
    cd ../..
done

exit 0

