cmake .. -DCMAKE_TOOLCHAIN_FILE="$PWD/../build-cmake/cmake/toolchain_native_gcc.cmake" \
      -DCMAKE_BUILD_TYPE=Relwithdebinfo \
      -DCMAKE_C_FLAGS_RELWITHDEBINFO=" \
      		-gz=zlib \
      		-g2 \
		-gno-inline-points \
		-O3 \
		-msse2 \
		-mfpmath=sse \
		-flto \
		" \
      -DCMAKE_CXX_FLAGS_RELWITHDEBINFO=" \
      		-gz=zlib \
      		-g2 \
		-O3 \
		-msse2 \
		-mfpmath=sse \
		-gno-inline-points \
		-flto \
		" \
      -DCMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO="\
      		-g2 \
		-ggdb \
		-gz=zlib \
		-O3 \
		-msse2 \
		-mfpmath=sse \
		-flto=8 \
		" \
      -DCMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO="\
      		-g2 \
		-gz=zlib \
		-gno-inline-points \
		-O3 \
		-msse2 \
		-mfpmath=sse \
		-flto=8 \
		" \
