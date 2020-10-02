cmake .. -DCMAKE_TOOLCHAIN_FILE="$PWD/../build-cmake/cmake/toolchain_native_gcc.cmake" \
      -DCMAKE_BUILD_TYPE=Relwithdebinfo \
      -DCMAKE_C_FLAGS_RELWITHDEBINFO=" \
      		-g2 \
		-gno-inline-points \
		-O3 \
		-msse2 \
		-mfpmath=sse \
		" \
      -DCMAKE_CXX_FLAGS_RELWITHDEBINFO=" \
      		-g2 \
		-O3 \
		-msse2 \
		-mfpmath=sse \
		-gno-inline-points \
		" \
      -DCMAKE_EXE_LINKER_FLAGS="\
      		-g2 \
		-ggdb \
		-gz=zlib \
		" \
      -DCMAKE_MODULE_LINKER_FLAGS="\
      		-g2 \
		-gz=zlib \
		-gno-inline-points \
		" \
