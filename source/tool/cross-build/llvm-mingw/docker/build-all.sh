#!/bin/sh

set -e

if [ $# -lt 1 ]; then
    echo $0 dest
    exit 1
fi

FORCE_THREADS=OFF
__NARG=""
while [ $# -gt 0 ]; do
    if [ "$1" = "--build-threads" ]; then
	${CORES:="$2"}
	__NARG="--build-threads $2"
	shift
     else
        PREFIX="$1"
	break
     fi
     shift
done

./build-llvm.sh ${__NARG} $PREFIX
./install-wrappers.sh $PREFIX
./build-mingw-w64.sh ${__NARG} $PREFIX
./build-compiler-rt.sh ${__NARG} $PREFIX
./build-mingw-w64-libraries.sh ${__NARG} $PREFIX
./build-libcxx.sh ${__NARG} $PREFIX
./build-compiler-rt.sh ${__NARG} $PREFIX --build-sanitizers
./build-libssp.sh ${__NARG} $PREFIX
