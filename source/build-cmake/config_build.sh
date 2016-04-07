#!/bin/bash

CMAKE=/usr/bin/cmake
CCMAKE_CC=gcc-5
CCMAKE_CXX=g++-5

MAKEFLAGS_CXX="-g -O2 -DNDEBUG"
MAKEFLAGS_CC="-g -O2 -DNDEBUG"
LIB_INSTALL="/usr/local/lib/x86_64-linux-gnu/"

BUILD_TYPE="Relwithdebinfo"
CMAKE_APPENDFLAG=""

if [ -e ./buildvars.dat ] ; then
    . ./buildvars.dat
fi

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
mkdir -p libCSPgui/build
cd libCSPgui/build
    
echo ${CMAKE_FLAGS1} ${CMAKE_FLAGS2}
${CMAKE} -DCMAKE_C_COMPILER:STRING=${CCMAKE_CC}  \
         -DCMAKE_CXX_COMPILER:STRING=${CCMAKE_CXX} \
	 "-DLIBCSP_INSTALL_DIR:STRING=${LIB_INSTALL}" \
	 ${CMAKE_FLAGS1} \
	 "${CMAKE_FLAGS2}=${MAKEFLAGS_LIB_CXX}" \
	 "${CMAKE_FLAGS3}=${MAKEFLAGS_LIB_CC}" \
	 ${CMAKE_APPENDFLAG} \
	 ${CMAKE_LINKFLAG} \
	 .. | tee make.log
	 
${CMAKE} -DCMAKE_C_COMPILER:STRING=${CCMAKE_CC}  \
	 -DCMAKE_CXX_COMPILER:STRING=${CCMAKE_CXX} \
	 "-DLIBCSP_INSTALL_DIR:STRING=${LIB_INSTALL}" \
	 ${CMAKE_FLAGS1} \
	 "${CMAKE_FLAGS2}=${MAKEFLAGS_LIB_CXX}" \
	 "${CMAKE_FLAGS3}=${MAKEFLAGS_LIB_CC}" \
	 ${CMAKE_APPENDFLAG} \
	 ${CMAKE_LINKFLAG} \
	 .. | tee -a make.log

make clean
    
make ${MAKEFLAGS_GENERAL} 2>&1 | tee -a ./make.log
case $? in
      0 ) sudo make install 2>&1 | tee -a ./make.log ;;
      * ) exit $? ;;
    esac
    
make clean
cd ../..


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

