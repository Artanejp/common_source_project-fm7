#!/bin/sh
./configure --prefix=/usr/local/ffmpeg-3.1.1 \
            --enable-shared --disable-static \
	    --enable-gpl --enable-libx264 \
	    --enable-libtheora --enable-libvorbis \
	    --enable-libmp3lame \
	    --enable-opencl \
	    --cc=gcc-6 --cxx=g++-6 \
	    --enable-lto
	    