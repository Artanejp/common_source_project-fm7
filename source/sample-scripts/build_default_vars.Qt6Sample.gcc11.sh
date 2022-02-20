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
#CSP_LLVM_TOOLCHAIN_VERSION=13
CSP_SIMD_FLAGS=" \
	-msse2 \
	-mfpmath=sse \
	"
	
CSP_BASIC_DEBUGFLAGS=" \
	-ggdb \
	-gz \
	"
#	-Wa,--compress-debug-sections=zlib \

CSP_BASIC_CFLAGS=" \
	-O3 \
	-fno-fat-lto-objects \
	-flto \
	-flto-compression-level=19 \
	-pthread \
	"
#	-flto=thin \

CSP_ARCH_CFLAGS=" \
	${CSP_SIMD_FLAGS} \
	"

CSP_ARCH_LDFLAGS=" \
	${CSP_SIMD_FLAGS} \
	"
	
CSP_ADDITIONAL_LDFLAGS_DLL=" \
	-flto=jobserver \
	-flto-compression-level=19 \
	-fuse-ld=gold \
	-fuse-linker-plugin \
	-Wl,--compress-debug-sections=zlib \
	"

CSP_ADDITIONAL_LDFLAGS_EXE=" \
	-fwhole-program \
	-flto-compression-level=19 \
	-fuse-ld=gold \
	-fuse-linker-plugin \
	-Wl,--compress-debug-sections=zlib \
	"


cmake .. -DCMAKE_TOOLCHAIN_FILE="$PWD/../cmake/toolchains/toolchain_native_gcc11.cmake" \
      -DCMAKE_BUILD_TYPE=Relwithdebinfo \
      -DCMAKE_C_FLAGS_RELWITHDEBINFO=" \
		${CSP_BASIC_DEBUGFLAGS} \
      		${CSP_BASIC_CFLAGS} \
		${CSP_ARCH_CFLAGS} \
		${SANITIZER_FLAGS} \
		${I_LIB_CLANG_FLAGS} \
		${L_LIB_CLANG_FLAGS} \
		" \
      -DCMAKE_CXX_FLAGS_RELWITHDEBINFO=" \
		${CSP_BASIC_DEBUGFLAGS} \
      		${CSP_BASIC_CFLAGS} \
		${CSP_ARCH_CFLAGS} \
		${I_LIB_CLANGPP_FLAGS} \
		${L_LIB_CLANGPP_FLAGS} \
		${SANITIZER_FLAGS} \
		" \
      -DCMAKE_EXE_LINKER_FLAGS="\
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
	
