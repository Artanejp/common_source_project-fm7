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
CSP_LLVM_TOOLCHAIN_VERSION=13

cmake .. -DCMAKE_TOOLCHAIN_FILE="$PWD/../cmake/toolchains/toolchain_native_llvm-versioned.cmake" \
      -DCMAKE_CSP_LLVM_VERSION=${CSP_LLVM_TOOLCHAIN_VERSION} \
      -DCMAKE_BUILD_TYPE=Relwithdebinfo \
      -DCMAKE_C_FLAGS_RELWITHDEBINFO=" \
      		-gdwarf \
		-gz \
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
		" \
      -DCMAKE_CXX_FLAGS_RELWITHDEBINFO=" \
      		-gdwarf \
		-gz \
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
		" \
      -DCMAKE_EXE_LINKER_FLAGS="\
		-fuse-ld=lld-${CSP_LLVM_TOOLCHAIN_VERSION} \
      		-gdwarf \
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
		-Wl,--lto-O3 \
		" \
      -DCMAKE_MODULE_LINKER_FLAGS="\
		-fuse-ld=lld-${CSP_LLVM_TOOLCHAIN_VERSION} \
      		-gdwarf \
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
		-Wl,--lto-O3 \
		" \
	-DUSE_QT_6=ON \
	-DCSP_BUILD_WITH_CXX20=ON
	
