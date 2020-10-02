cmake .. -DCMAKE_TOOLCHAIN_FILE="$PWD/../build-cmake/cmake/toolchain_native_llvm.cmake" \
      -DCMAKE_BUILD_TYPE=Relwithdebinfo \
      -DCMAKE_C_FLAGS_RELWITHDEBINFO="-g2 -flto -O2" \
      -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-g2 -flto -O2" \
      -DCMAKE_EXE_LINKER_FLAGS="-g2 -gz -flto -O2"
