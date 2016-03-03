#!/bin/bash

CMAKE=cmake
CCMAKE_CC=gcc
CCMAKE_CXX=g++

MAKEFLAGS_CXX="-g -O2 -DNDEBUG"
MAKEFLAGS_CC="-g -O2 -DNDEBUG"
BUILD_TYPE="Relwithdebinfo"
CMAKE_APPENDFLAG=""

mkdir -p ./bin-win32/
if [ -e ./buildvars_mingw.dat ] ; then
    . ./buildvars_mingw.dat
fi


MAKEFLAGS_CXX="${MAKEFLAGS_CXX} -DWINVER=0x501"
MAKEFLAGS_CC="${MAKEFLAGS_CXX} -DWINVER=0x501"

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

for SRCDATA in $@ ; do\

    mkdir -p ${SRCDATA}/build
    cd ${SRCDATA}/build
    
    echo ${CMAKE_FLAGS1} ${CMAKE_FLAGS2}
    ${CMAKE} -G "MinGW Makefiles" \
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

exit 0

