#!/usr/bin/bash

TOOLCHAIN_PREFIX=/opt/llvm-mingw-12
TOOLCHAIN_ARCHS="i686 x86_64 armv7 aarch64"
TOOLCHAIN_TARGET_OSES="mingw32 mings32uwp"
WORKDIR_2=$PWD
LLVM_VERSION=llvmorg-12.0.0
FORCE_THREADS=6

typeset -i CLEANING
CLEANING=0

if [ -e ./buildvars.dat ] ; then
    . ./buildvars.dat
fi

export TOOLCHAIN_ARCHS
export TOOLCHAIN_PREFIX
export TOOLCHAIN_TARGET_OSES
export WORKDIR_2

typeset -i ncount
ncount=1

THREAD_PARAM=""
if [ -n "${FORCE_THREADS}" ] ; then
   THREAD_PARAM="--build-threads ${FORCE_THREADS}"
fi

mkdir -p "${WORKDIR_2}/"
mkdir -p "${WORKDIR_2}/scripts"
mkdir -p "${WORKDIR_2}/scripts/wrappers"
mkdir -p "${WORKDIR_2}/build"

cp -ar scripts/*.sh  "${WORKDIR_2}/build/"
cp -ar ${WORKDIR_2}/scripts/wrappers ${WORKDIR_2}/build/wrappers
cp -ar ${WORKDIR_2}/scripts/test ${WORKDIR_2}/build/test/
pushd .

cd "${WORKDIR_2}/build"
echo "BUILD LLVM Toolchain for Win32/64 to ${WORKDIR_2}/build/"

${WORKDIR_2}/scripts/build-llvm.sh \
               --llvm-version ${LLVM_VERSION} \
	       ${THREAD_PARAM} \
	       ${TOOLCHAIN_PREFIX}

#if [ $? -ne 0 ] ; then
#   echo "PHASE ${ncount}: script: ${_sc} failed. Abort building"
#   exit $?
#fi

#${WORKDIR_2}/scripts/strip-llvm.sh \
#		       ${TOOLCHAIN_PREFIX}

#if [ $? -ne 0 ] ; then
#   echo "PHASE ${ncount}: script: ${_sc} failed. Abort building"
#   exit $?
#fi

ncount=$((${ncount}+1))

${WORKDIR_2}/scripts/install-wrappers.sh \
		       ${TOOLCHAIN_PREFIX}
		       
if [ $? -ne 0 ] ; then
   echo "PHASE ${ncount}: script: ${_sc} failed. Abort building"
   exit $?
fi
ncount=$((${ncount}+1))

export PATH="${TOOLCHAIN_PREFIX}/bin:${PATH}"

mkdir -p "${WORKDIR_2}/build/build"
cd "${WORKDIR_2}/build/build"
${WORKDIR_2}/scripts/build-mingw-w64.sh \
           ${THREAD_PARAM} \
	       ${TOOLCHAIN_PREFIX}
	       
cd "${WORKDIR_2}/build"
${WORKDIR_2}/scripts/build-compiler-rt.sh \
            ${THREAD_PARAM} \
            ${TOOLCHAIN_PREFIX} 


cd "${WORKDIR_2}/build/build"
${WORKDIR_2}/scripts/build-mingw-w64-libraries.sh \
            ${THREAD_PARAM} \
 	        ${TOOLCHAIN_PREFIX}



cd "${WORKDIR_2}/build"
${WORKDIR_2}/scripts/build-libcxx.sh \
               ${THREAD_PARAM} \
	       ${TOOLCHAIN_PREFIX}


#cd "${WORKDIR_2}/build/test"
#for arch in $TOOLCHAIN_ARCHS; do
#    mkdir -p $arch
#    for test in hello hello-tls crt-test setjmp; do
#        $arch-w64-mingw32-clang $test.c -o $arch/$test.exe || exit 1;
#    done;
#    for test in autoimport-lib; do
#        $arch-w64-mingw32-clang $test.c -shared -o $arch/$test.dll -Wl,--out-implib,$arch/lib$test.dll.a || exit 1; \
#    done;
#    for test in autoimport-main; do
#        $arch-w64-mingw32-clang $test.c -o $arch/$test.exe -L$arch -l${test%-main}-lib || exit 1;
#    done;
#done

# Build sanitizers. Ubsan includes <typeinfo> from the C++ headers, so
# we need to build this after libcxx.
# Sanitizers on windows only support x86.
cd "${WORKDIR_2}/build"
${WORKDIR_2}/scripts/build-compiler-rt.sh \
           ${THREAD_PARAM} \
           ${TOOLCHAIN_PREFIX}

#  		   --build-sanitizers \


#cd "${WORKDIR_2}/build/test"
#for arch in $TOOLCHAIN_ARCHS; do
#        case $arch in
#        i686|x86_64)
#	    ;;
#        *)
#            continue
#            ;;
#        esac 
#        for test in stacksmash; do
#            $arch-w64-mingw32-clang $test.c -o $arch/$test-asan.exe -fsanitize=address -g -gcodeview -Wl,-pdb,$arch/$test-asan.pdb || exit 1;
#        done
#        for test in ubsan; do
#            $arch-w64-mingw32-clang $test.c -o $arch/$test.exe -fsanitize=undefined || exit 1;
#        done
#done

# Build libssp

cd "${WORKDIR_2}/build"
cp "${WORKDIR_2}/scripts/libssp-Makefile" "${WORKDIR_2}/build"
_sc=""
${WORKDIR_2}/scripts/build-libssp.sh \
                   ${THREAD_PARAM} \
		   ${TOOLCHAIN_PREFIX} \
		   #--build-sanitizers


#cd "${WORKDIR_2}/build/test"
#for arch in $TOOLCHAIN_ARCHS; do
#    mkdir -p $arch
#    for test in stacksmash; do
#        $arch-w64-mingw32-clang $test.c -o $arch/$test.exe -fstack-protector-strong || exit 1
#    done
#done
#cd "${WORKDIR_2}/build/test"
#for arch in $TOOLCHAIN_ARCHS; do
#     cp $TOOLCHAIN_PREFIX/$arch-w64-mingw32/bin/*.dll $arch || exit 1
#done

# Clear build data
if [ ${CLEANING} -ne 0 ] ; then
   rm -rf ./build/*
fi
