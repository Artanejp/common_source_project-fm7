#!/bin/sh
./configure --prefix=/usr/local/ffmpeg-2.8.7 \
            --enable-shared --disable-static \
	    --enable-gpl --enable-libx264 \
	    --enable-libtheora --enable-libvorbis \
	    --enable-libmp3lame \
	    --enable-lto
	    