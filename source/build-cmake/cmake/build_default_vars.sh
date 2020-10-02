cmake .. -DCMAKE_TOOLCHAINFILE=../build-cmake/cmake/toolchain_native_llvm.cmake \
      -DCMAKE_BUILD_TYPE=Relwithdebinfo \
      -DCMAKE_C_FLAGS="-g -gz -flto -O2" \
      -DCMAKE_CXX_FLAGS="-g -gz -flto -O2" \
      -DCMAKE_EXE_LINKER_FLAGS="-g -gz -flto -O2"
