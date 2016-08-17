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
../source/configure --host=i686-w64-mingw32 --prefix=/usr/local/i586-mingw-msvc/icu  \
                    --enable-static --disable-strict \
		    CPPFLAGS='-I/usr/local/i586-mingw-msvc/icu/include' \
		    LDFLAGS=-L/usr/local/i586-mingw-msvc/bundle-libs \
		    --with-cross-build=`pwd`/../build \
		    --with-data-packaging=library
sudo make install
cd ..
echo DONE.