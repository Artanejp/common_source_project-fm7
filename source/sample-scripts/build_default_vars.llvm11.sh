#!/bin/sh
SANITRIZER_FLAGS=""
#SANITRIZER_FLAGS="-fsanitize=address"

cmake .. -DCMAKE_TOOLCHAIN_FILE="$PWD/../cmake/toolchains/toolchain_native_llvm11.cmake" \
      -DCMAKE_BUILD_TYPE=Relwithdebinfo \
      -DCMAKE_C_FLAGS_RELWITHDEBINFO=" \
      		-g \
		-gz=zlib \
		-O3 \
		-msse2 \
		-mfpmath=sse \
 	        -Wreserved-user-defined-literal \
 	        -fslp-vectorize \
 	        -fvectorize \
		-fstrict-vtable-pointers \
		-fstrict-enums \
		${SANITIZER_FLAGS} \
		-Wa,--compress-debug-sections=zlib \
		" \
      -DCMAKE_CXX_FLAGS_RELWITHDEBINFO=" \
      		-g \
		-gz=zlib \
		-O3 \
		-msse2 \
		-mfpmath=sse \
 	        -Wreserved-user-defined-literal \
 	        -fslp-vectorize \
 	        -fvectorize \
		-fstrict-vtable-pointers \
		-fstrict-enums \
		${SANITIZER_FLAGS} \
		-Wa,--compress-debug-sections=zlib \
		" \
      -DCMAKE_EXE_LINKER_FLAGS="\
      		-g \
		-gz=zlib \
		-O3 \
		-msse2 \
		${SANITIZER_FLAGS} \
		-Wl,--compress-debug-sections=zlib \
		" \
      -DCMAKE_MODULE_LINKER_FLAGS="\
      		-g \
		-gz \
		-O3 \
		-msse2 \
		${SANITIZER_FLAGS} \
		-Wl,--compress-debug-sections=zlib \
		" \
