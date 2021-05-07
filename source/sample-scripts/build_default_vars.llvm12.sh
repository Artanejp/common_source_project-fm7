#!/bin/sh
SANITRIZER_FLAGS=""
#SANITRIZER_FLAGS="-fsanitize=address"

cmake .. -DCMAKE_TOOLCHAIN_FILE="$PWD/../cmake/toolchains/toolchain_native_llvm12.cmake" \
      -DCMAKE_BUILD_TYPE=Relwithdebinfo \
      -DCMAKE_C_FLAGS_RELWITHDEBINFO=" \
      		-g2 \
		-gz=zlib \
		-O3 \
		-flto \
		-msse2 \
		-mfpmath=sse \
 	        -Wreserved-user-defined-literal \
		${SANITIZER_FLAGS} \
		" \
      -DCMAKE_CXX_FLAGS_RELWITHDEBINFO=" \
      		-g2 \
		-gz=zlib \
		-flto \
		-O3 \
		-msse2 \
		-mfpmath=sse \
		${SANITIZER_FLAGS} \
		" \
      -DCMAKE_EXE_LINKER_FLAGS="\
      		-g2 \
		-gz=zlib \
		-flto \
		-O3 \
		-msse2 \
 	        -fslp-vectorize \
 	        -fvectorize \
		-fstrict-vtable-pointers \
		-fstrict-enums \
		${SANITIZER_FLAGS} \
		-Wl,--compress-debug-sections=zlib \
		" \
      -DCMAKE_MODULE_LINKER_FLAGS="\
      		-g2 \
		-gz=zlib \
		-O3 \
		-flto \
		-msse2 \
 	        -fslp-vectorize \
 	        -fvectorize \
		-fstrict-vtable-pointers \
		-fstrict-enums \
		-Wl,--compress-debug-sections=zlib \
		${SANITIZER_FLAGS} \
		" \
