#!/bin/sh
SANITRIZER_FLAGS=""

I_LIB_CLANG_FLAGS=""
L_LIB_CLANG_FLAGS=""
I_LIB_CLANGPP_FLAGS=""
L_LIB_CLANGPP_FLAGS=""

#SANITRIZER_FLAGS="-fsanitize=address"
#I_LIB_CLANG_FLAGS=""
#L_LIB_CLANG_FLAGS=""
#I_LIB_CLANGPP_FLAGS="-stdlib=libc++"
#L_LIB_CLANGPP_FLAGS="-stdlib=libc++"
#I_LIB_CLANGPP_FLAGS="-stdlib=libstdc++"
#L_LIB_CLANGPP_FLAGS="-stdlib=libstdc++"

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
 	        -fslp-vectorize \
 	        -fvectorize \
		-fstrict-vtable-pointers \
		-fstrict-enums \
		${SANITIZER_FLAGS} \
		${I_LIB_CLANG_FLAGS} \
		${L_LIB_CLANG_FLAGS} \
		-Wa,--compress-debug-sections=zlib \
		" \
      -DCMAKE_CXX_FLAGS_RELWITHDEBINFO=" \
      		-g2 \
		-gz=zlib \
		-O3 \
		-flto \
		-msse2 \
		-mfpmath=sse \
 	        -Wreserved-user-defined-literal \
 	        -fslp-vectorize \
 	        -fvectorize \
		-fstrict-vtable-pointers \
		-fstrict-enums \
		${I_LIB_CLANGPP_FLAGS} \
		${L_LIB_CLANGPP_FLAGS} \
		${SANITIZER_FLAGS} \
		-Wa,--compress-debug-sections=zlib \
		" \
      -DCMAKE_EXE_LINKER_FLAGS="\
      		-g2 \
		-gz=zlib \
		-O3 \
		-flto \
		-msse2 \
		${I_LIB_CLANG_FLAGS} \
		${L_LIB_CLANG_FLAGS} \
		${I_LIB_CLANGPP_FLAGS} \
		${L_LIB_CLANGPP_FLAGS} \
		${SANITIZER_FLAGS} \
		-Wl,--compress-debug-sections=zlib \
		" \
      -DCMAKE_MODULE_LINKER_FLAGS="\
      		-g \
		-gz \
		-O3 \
		-flto \
		-msse2 \
		${I_LIB_CLANG_FLAGS} \
		${L_LIB_CLANG_FLAGS} \
		${I_LIB_CLANGPP_FLAGS} \
		${L_LIB_CLANGPP_FLAGS} \
		${SANITIZER_FLAGS} \
		-Wl,--compress-debug-sections=zlib \
		" \
		-DCSP_BUILD_WITH_CXX20=ON
