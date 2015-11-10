#!/bin/bash

CMAKE=/usr/bin/cmake
TOOLCHAIN_SCRIPT="../../cmake/toolchain_mingw_cross_linux.cmake"

#MAKEFLAGS_CXX="-g -O2 -DNDEBUG"
#MAKEFLAGS_CC="-g -O2 -DNDEBUG"
BUILD_TYPE="Relwithdebinfo"
CMAKE_APPENDFLAG=""
export WINEDEBUG="-all"

mkdir -p ./bin-win32/

#if [ -e ./buildvars.dat ] ; then
#    . ./buildvars.dat
#fi
#MAKEFLAGS_CXX="-g -O3 -ftree-vectorize -flto -DNDEBUG" 
#MAKEFLAGS_CC="-g -O3 -ftree-vectorize  -flto -DNDEBUG"
# To use MOC, please enable wine as interpreter of EXEs , below:
# $ sudo update-binfmts --install Win32_Wine /usr/bin/wine --extension exe . 
MAKEFLAGS_GENERAL="-j4"

#CMAKE_LINKFLAG="-DCMAKE_EXE_LINKER_FLAGS='${CMAKE_EXE_LINKER_FLAGS} -flto -O3 -ftree-vectorize -g'"
#CMAKE_LINKFLAG=""
#CMAKE_APPENDFLAG="-DCMAKE_AR=/usr/bin/gcc-ar -DCMAKE_NM=/usr/bin/gcc-nm -DCMAKE_RANLIB=/usr/bin/gcc-ranlib"

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

    mkdir -p ${SRCDATA}/build-win32
    cd ${SRCDATA}/build-win32
    
    echo ${CMAKE_FLAGS1} ${CMAKE_FLAGS2}
    ${CMAKE} -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_SCRIPT} \
	     ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_CC}" \
	     "-D USE_SDL2=OFF" \
	     ${CMAKE_APPENDFLAG} \
	     ${CMAKE_LINKFLAG} \
	     .. | tee make.log

    ${CMAKE} ${CMAKE_FLAGS1} \
	     "${CMAKE_FLAGS2}=${MAKEFLAGS_CXX}" \
	     "${CMAKE_FLAGS3}=${MAKEFLAGS_CC}" \
	     "-D USE_SDL2=OFF" \
	     ${CMAKE_APPENDFLAG} \
	     ${CMAKE_LINKFLAG} \
	     .. | tee -a make.log

    make clean
    
    make ${MAKEFLAGS_GENERAL} 2>&1 | tee -a ./make.log
    case $? in
      0 ) sudo cp ./qt/common/*.exe ../../bin-win32/ ;;
      * ) exit $? ;;
    esac
    
    make clean
    cd ../..
done

exit 0

