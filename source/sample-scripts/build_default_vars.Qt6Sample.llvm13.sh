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
CSP_SIMD_FLAGS=" \
        -msse2 \
	-mfpmath=sse \
	"
	
CSP_BASIC_DEBUGFLAGS=" \
        -gdwarf \
	-gz \
	"

CSP_BASIC_CFLAGS=" \
	-flto=thin \
        -Wreserved-user-defined-literal \
        -fslp-vectorize \
        -fvectorize \
	-fstrict-vtable-pointers \
	-fstrict-enums \
	"
	
CSP_ARCH_CFLAGS=" \
	${CSP_SIMD_FLAGS} \
	"

CSP_ARCH_LDFLAGS=" \
	${CSP_SIMD_FLAGS} \
	"
	
CSP_ADDITIONAL_LDFLAGS_DLL=" \
	-flto=jobserver \
	-Wl,--compress-debug-sections=zlib \
	-Wl,--lto-O3 \
	"
#	-Wl,--compress-debug-sections=zlib \
#	-Wl,--lto-O3 \

CSP_ADDITIONAL_LDFLAGS_EXE=" \
	-flto=jobserver \
	-fwhole-program-vtables \
	-Wl,--compress-debug-sections=zlib \
	-Wl,--lto-O3 \
	"

cmake .. -DCMAKE_TOOLCHAIN_FILE="$PWD/../cmake/toolchains/toolchain_native_llvm-versioned.cmake" \
      -DCMAKE_CSP_LLVM_VERSION=${CSP_LLVM_TOOLCHAIN_VERSION} \
      -DCMAKE_BUILD_TYPE=Relwithdebinfo \
      -DCMAKE_C_FLAGS_RELWITHDEBINFO=" \
		-O3 \
		${CSP_BASIC_DEBUGFLAGS} \
      		${CSP_BASIC_CFLAGS} \
		${CSP_ARCH_CFLAGS} \
		${SANITIZER_FLAGS} \
		${I_LIB_CLANG_FLAGS} \
		${L_LIB_CLANG_FLAGS} \
		" \
      -DCMAKE_CXX_FLAGS_RELWITHDEBINFO=" \
		-O3 \
		${CSP_BASIC_DEBUGFLAGS} \
      		${CSP_BASIC_CFLAGS} \
		${CSP_ARCH_CFLAGS} \
		${I_LIB_CLANGPP_FLAGS} \
		${L_LIB_CLANGPP_FLAGS} \
		${SANITIZER_FLAGS} \
		" \
      -DCMAKE_EXE_LINKER_FLAGS="\
		-O3 \
		-fuse-ld=lld-${CSP_LLVM_TOOLCHAIN_VERSION} \
		${CSP_BASIC_DEBUGFLAGS} \
		${CSP_BASIC_LDFLAGS} \
		${CSP_ARCH_LDFLAGS} \
		${I_LIB_CLANG_FLAGS} \
		${L_LIB_CLANG_FLAGS} \
		${I_LIB_CLANGPP_FLAGS} \
		${L_LIB_CLANGPP_FLAGS} \
		${SANITIZER_FLAGS} \
		${CSP_ADDITIONAL_LDFLAGS_EXE} \
		" \
      -DCMAKE_MODULE_LINKER_FLAGS="\
		-O3 \
		-fuse-ld=lld-${CSP_LLVM_TOOLCHAIN_VERSION} \
		${CSP_BASIC_DEBUGFLAGS} \
		${CSP_BASIC_LDFLAGS} \
		${CSP_ARCH_LDFLAGS} \
		${I_LIB_CLANG_FLAGS} \
		${L_LIB_CLANG_FLAGS} \
		${I_LIB_CLANGPP_FLAGS} \
		${L_LIB_CLANGPP_FLAGS} \
		${SANITIZER_FLAGS} \
		${CSP_ADDITIONAL_LDFLAGS_DLL} \
		" \
	-DUSE_QT_6=ON \
	-DUSE_LTO=ON \
	-DCSP_BUILD_WITH_CXX20=ON
	
