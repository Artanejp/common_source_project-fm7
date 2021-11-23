#!/bin/sh
ADDITIONAL_STD_LIBS=""
ADDITIONAL_DEFINES=""
I_LIB_CLANG_FLAGS=""
L_LIB_CLANG_FLAGS=""
I_LIB_CLANGPP_FLAGS=""
L_LIB_CLANGPP_FLAGS=""
ARCH_TRIPLE=i686-w64-mingw32
LIBS_PREFIX="/usr/local/i586-mingw-msvc"

# Issue; 
# 1. clang-12 emits stpcpy() as __builtin_stpcpy() with some case of sprintf().
#    But, MinGW-w64 has no stpcpy().
ADDITIONAL_DEFINES=" \
		-fno-builtin-stpcpy \
		-Dstpcpy\(d,s\)=__builtin_stpcpy\(d,s\) \
		"

# 2. clang-12/libc++12 for MinGW calls _aligned_ prefixed
#    memory allocation/free functions,
#    but these MinGW-w64 has no them.Should use __mingw_aigned_ prefix. 
ADDITIONAL_DEFINES=" \
		${ADDITIONAL_DEFINES} \
		-D_aligned_malloc\(s,a\)=__mingw_aligned_malloc\(s,a\) \
		-D_aligned_free\(m\)=__mingw_aligned_free\(m\) \
		-D_aligned_offset_realloc\(m,s,a,o\)=__mingw_aligned_offset_realloc\(m,s,a,o\) \
		-D_aligned_realloc\(m,s,o\)=__mingw_aligned_realloc\(m,s,o\) \
		"
		
#I_LIB_CLANG_FLAGS=" \
#                     --sysroot=/opt/llvm-mingw-11 \
#		     "
#L_LIB_CLANG_FLAGS=" \
#                     --sysroot=/opt/llvm-mingw-11 \
#		     "

#I_LIB_CLANGPP_FLAGS=" \
#                     --sysroot=/opt/llvm-mingw-11 \
#		     "
#L_LIB_CLANGPP_FLAGS=" \
#                     --sysroot=/opt/llvm-mingw-11 \
#		     "

I_ADDITIONAL_DEFINES="${I_LIB_CLANG_FLAGS} ${I_LIB_CLANGPP_FLAGS} ${ADDITIONAL_DEFINES}"
L_ADDITIONAL_DEFINES="${I_LIB_CLANG_FLAGS} ${I_LIB_CLANGPP_FLAGS} \
                      ${L_LIB_CLANG_FLAGS} ${L_LIB_CLANGPP_FLAGS} \
                      ${ADDITIONAL_DEFINES}"

FFMPEG_DIR="${LIBS_PREFIX}/ffmpeg-4.4"
QT5_DIR="${LIBS_PREFIX}/Qt5.15/mingw_82x"

PATH=/opt/llvm-mingw-12/bin:$PATH
cmake .. \
	-DCMAKE_TOOLCHAIN_FILE="$PWD/../cmake/toolchains/toolchain_mingw_cross_llvm12.cmake" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_FLAGS_RELWITHDEBINFO=" \
      		-g2 \
		-ggdb \
		-gz=zlib \
		-O3 \
		-march=i686 \
		-msse -msse2 \
		-mfpmath=sse \
 	        -Wreserved-user-defined-literal \
 	        -fslp-vectorize \
 	        -fvectorize \
		-fstrict-vtable-pointers \
		-fstrict-enums \
		${I_ADDITIONAL_DEFINES} \
		" \
	-DCMAKE_CXX_FLAGS_RELWITHDEBINFO=" \
      		-g2 \
		-ggdb \
		-gz=zlib \
		-O3 \
		-march=i686 \
		-msse -msse2 \
		-mfpmath=sse \
 	        -Wreserved-user-defined-literal \
 	        -fslp-vectorize \
 	        -fvectorize \
		-fstrict-vtable-pointers \
		-fstrict-enums \
		${I_ADDITIONAL_DEFINES} \
		" \
	-DCMAKE_C_FLAGS_RELEASE=" \
		-O3 \
		-march=i686 \
		-msse -msse2 \
		-mfpmath=sse \
 	        -Wreserved-user-defined-literal \
 	        -fslp-vectorize \
 	        -fvectorize \
		-fstrict-vtable-pointers \
		-fstrict-enums \
		${I_ADDITIONAL_DEFINES} \
		" \
	-DCMAKE_CXX_FLAGS_RELEASE=" \
		-O3 \
		-march=i686 \
		-msse -msse2 \
		-mfpmath=sse \
 	        -Wreserved-user-defined-literal \
 	        -fslp-vectorize \
 	        -fvectorize \
		-fstrict-vtable-pointers \
		-fstrict-enums \
		${I_ADDITIONAL_DEFINES} \
		" \
	-DCMAKE_EXE_LINKER_FLAGS_RELEASE="\
		${L_ADDITIONAL_DEFINES} \
		" \
	-DCMAKE_MODULE_LINKER_FLAGS_RELEASE="\
		${L_ADDITIONAL_DEFINES} \
		" \
	-DCMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO="\
      		-g2 \
		-ggdb \
		-gz=zlib \
		-L/usr/i686-w64-mingw32/lib \
		${L_ADDITIONAL_DEFINES} \
		" \
	-DCMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO="\
      		-g2 \
		-ggdb \
		-gz=zlib \
		-L/usr/${ARCH_TRIPLE}/lib \
		${L_ADDITIONAL_DEFINES} \
		" \
	-DLIBAV_ROOT_DIR="${FFMPEG_DIR}" \
	-DQT5_ROOT_PATH="${QT5_DIR}"	\
	-DTARGET_ARCH="${ARCH_TRIPLE}" \
	-DLIBS_PREFIX="${LIBS_PREFIX}" \
	-DUSE_DEVICES_SHARED_LIB=ON \
	-DCSP_BUILD_WITH_CXX20=ON 
	
	
	
	