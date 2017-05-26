#!/bin/bash

CMAKE=/usr/bin/cmake

BUILD_TYPE="Relwithdebinfo"
CMAKE_APPENDFLAG=""
export WINEDEBUG="-all"
CMAKE_LINKFLAG=""
CMAKE_APPENDFLAG=""
MAKEFLAGS_GENERAL="-j4"
MAKE_STATUS_FILE="./000_make_status_config_build_cross_win32.log"

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
    _STATUS=${PIPESTATUS[0]}
    echo -e "$1 at `date --rfc-2822`:" "${_STATUS}" >> ../../${MAKE_STATUS_FILE}
   case ${_STATUS} in
     0 )
          ;;
     * ) 
     	  echo -e "Abort at `date --rfc-2822`." >> ../../${MAKE_STATUS_FILE}
	  #exit ${_STATUS}
	  ;;
    esac
    cd ../../
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
   ;;
   * )
   CMAKE_FLAGS4=""
   ;;
esac

echo "Make status." > ${MAKE_STATUS_FILE}
echo "Started at `date --rfc-2822`:" >> ${MAKE_STATUS_FILE}
build_dll libCSPemu_utils
echo $PWD
cp  ./libCSPemu_utils/build-win32/qt/emuutils/*.h   ./bin-win32/
cp  ./libCSPemu_utils/build-win32/qt/emuutils/*.dll ./bin-win32/
cp  ./libCSPemu_utils/build-win32/qt/emuutils/*.a   ./bin-win32/

build_dll libCSPfmgen
cp ./libCSPfmgen/build-win32/vm/fmgen/*.h   ./bin-win32/
cp ./libCSPfmgen/build-win32/vm/fmgen/*.dll ./bin-win32/
cp ./libCSPfmgen/build-win32/vm/fmgen/*.a   ./bin-win32/


build_dll libCSPavio
#cp ./libCSPavio/build-win32/qt/avio/*.h   ./bin-win32/
#cp ./libCSPavio/build-win32/qt/avio/*.dll ./bin-win32/
cp ./libCSPavio/build-win32/qt/avio/*.a   ./bin-win32/

build_dll libCSPosd
cp ./libCSPosd/build-win32/qt/osd/*.h   ./bin-win32/
cp ./libCSPosd/build-win32/qt/osd/*.dll ./bin-win32/
cp ./libCSPosd/build-win32/qt/osd/*.a   ./bin-win32/

case ${USE_COMMON_DEVICE_LIB} in
   "Yes" | "yes" | "YES" )
   build_dll libCSPcommon_vm
   cp ./libCSPcommon_vm/build-win32/vm/common_vm/*.h   ./bin-win32/
   cp ./libCSPcommon_vm/build-win32/vm/common_vm/*.dll ./bin-win32/
   cp ./libCSPcommon_vm/build-win32/vm/common_vm/*.a   ./bin-win32/
   ;;
   * )
   ;;
esac

build_dll libCSPgui
cp ./libCSPgui/build-win32/qt/gui/*.h   ./bin-win32/
cp ./libCSPgui/build-win32/qt/gui/*.dll ./bin-win32/
cp ./libCSPgui/build-win32/qt/gui/*.a   ./bin-win32/

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
    _STATUS=${PIPESTATUS[0]}
    echo -e "${SRCDATA} at `date --rfc-2822`:" "${_STATUS}" >> ../../${MAKE_STATUS_FILE}
    case ${_STATUS} in
      0 ) cp ./qt/common/*.exe ../../bin-win32/ ;;
      * )
     	  echo -e "Abort at `date --rfc-2822`." >> ../../${MAKE_STATUS_FILE}
	  #exit ${_STATUS}
	  ;;
    esac
    
    make clean
    cd ../..
done

echo -e "End at `date --rfc-2822`." >> ../../${MAKE_STATUS_FILE}

for ii in libCSPavio libCSPgui libCSPosd libCSPemu_utils; do
    cd $ii/build-win32
    make clean
    cd ../..
done

exit 0

