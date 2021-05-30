#!/bin/bash

mkdir -p build
mkdir -p build-cross


sudo mkdir -p /usr/local/i586-mingw-msvc
sudo mkdir -p /usr/local/i586-mingw-msvc/icu
sudo mkdir -p /usr/local/i586-mingw-msvc/icu/include
sudo cp source/common/cmemory.h /usr/local/i586-mingw-msvc/icu/include

# Build for HOST
cd build/
../source/configure 
make -j12
cd ..

# Build for Cross
cd build-cross/
PATH="/opt/llvm-mingw-12/bin:$PATH"
../source/configure --host=i686-w64-mingw32 --prefix=/usr/local/i586-mingw-msvc/icu  \
                    --enable-static --disable-strict \
		    CPPFLAGS='-I/usr/local/i586-mingw-msvc/icu/include' \
		    CXXFLAGS="-std=c++11" \
		    LDFLAGS="-lc++abi -lunwind -lssp -lmsvcrt" \
		    LD=/usr/bin/i686-w64-mingw32-ld \
		    AR=/usr/bin/i686-w64-mingw32-ar \
		    --disable-icuio \
		    --with-cross-build=`pwd`/../build \
		    --with-data-packaging=library
		    
#		    LDFLAGS="-L$PWD/build-cross/stubdata" \

make install
cd ..
echo DONE.