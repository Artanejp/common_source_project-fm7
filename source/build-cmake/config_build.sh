#!/bin/bash

CMAKE=/usr/bin/cmake
CCMAKE_CC=gcc-5
CCMAKE_CXX=g++-5

MAKEFLAGS_CXX="-g -O3 -DNDEBUG"
MAKEFLAGS_CC="-g -O3 -DNDEBUG"
BUILD_TYPE="Relwithdebinfo"
CMAKE_APPENDFLAG=""

. ./buildvars.dat

case ${BUILD_TYPE} in
    "Debug" | "DEBUG" | "debug" ) 
            CMAKE_FLAGS1="-D CMAKE_BUILD_TYPE:STRING=\"debug\""
	    CMAKE_FLAGS2="-D CMAKE_CXX_FLAGS_DEBUG"
	    CMAKE_FLAGS3="-D CMAKE_C_FLAGS_DEBUG"
	    ;;
    "Release" | "RELEASE" | "release" ) 
            CMAKE_FLAGS1="-D CMAKE_BUILD_TYPE:STRING=\"release\""
	    CMAKE_FLAGS2="-D CMAKE_CXX_FLAGS_RELEASE"
	    CMAKE_FLAGS3="-D CMAKE_C_FLAGS_RELEASE"
	    ;;
    "Relwithdebinfo" | "RELWITHDEBINFO" | "relwithdebinfo" ) 
            CMAKE_FLAGS1="-D CMAKE_BUILD_TYPE:STRING=\"Relwithdebinfo\""
	    CMAKE_FLAGS2="-D CMAKE_CXX_FLAGS_RELWITHDEBINFO"
	    CMAKE_FLAGS3="-D CMAKE_C_FLAGS_RELWITHDEBINFO"
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
    ${CMAKE} -D CMAKE_C_COMPILER:STRING=${CCMAKE_CC}  \
             -D CMAKE_CXX_COMPILER:STRING=${CCMAKE_CXX} \
	     "${CMAKE_FLAGS1}" \
	     "${CMAKE_FLAGS2}=\"${MAKEFLAGS_CXX}\"" \
	     "${CMAKE_FLAGS3}=\"${MAKEFLAGS_CC}\"" \
	     "${CMAKE_APPENDFLAG}" \
	     ..

    ${CMAKE} -D CMAKE_C_COMPILER:STRING=${CCMAKE_CC}  \
             -D CMAKE_CXX_COMPILER:STRING=${CCMAKE_CXX} \
	     "${CMAKE_FLAGS1}" \
	     "${CMAKE_FLAGS2}=\"${MAKEFLAGS_CXX}\"" \
	     "${CMAKE_FLAGS3}=\"${MAKEFLAGS_CC}\"" \
	     "${CMAKE_APPENDFLAG}" \
	     ..

    make clean
    make -j12 2>&1 | tee -a ./make.log
    
    sudo make install
    
    cd ../..
done

exit 0

