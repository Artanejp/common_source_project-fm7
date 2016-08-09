Build Common Source Code Project (CSP) with FFMPEG.
                                              August 09, 2016 K.Ohta
                                 <whatisthis.sowhat _at_ gmail.com>
 
1. Background
  Now, emu{foo} (for Qt) have implemented saving screen and sound as movie.
  This using libav (a.k.a. FFMpeg).
  But, this has different versions some distributions, because FFMpeg
  has often changing major version (THIS IS VERY EVIL (#゜Д゜)).
  For example, in Debian sid built with 3.1.1, in Ubuntu 16.04 LTS with 2.8.6 .
  So, distributed binaries must be built with fixed version of FFMpeg.
  
  Note:
   For Windows, shared libs are not managed by distribution, you don't
   need below (or needs below).
   
2. Install ffmpeg and CSP (if you don't have or have only FFMpeg 3).
  a. Install libx264 and libfaac with development files (at least).
  b. Download source code of ffmpeg-2.8.7 (or another version) from mirrors.
  c. Extract this any folder.
  d. Run:
     $ cd ffmpeg-{version}
     $ ./coinfigure --prefix=/usr/local/ffmpeg-{version} \
        --disable-static --enable-shared --enable-gpl \
        --enable-libx264 --enable-libmp3lame
	(and enabling another options)
  e. Build and install
    $ make
    $ sudo make install
    $ cd /usr/local/ffmpeg-{version}
    $ cd lib/
    $ sudo cp lib*.so.* /usr/local/lib/{arch}/
    $ sudo ldconfig
    
  f. Edit source/build-cmake/buildvars.dat of CSP, set below lines:
     CMAKE_APPENDFLAG="-DLIBAV_ROOT_DIR=/usr/local/ffmpeg-{version}"
     CMAKE_APPENDFLAG="${CMAKE_APPENDFLAG} -DUSE_MOVIE_SAVER=ON -DUSE_MOVIE_LOADER=ON"
  
  e. Cleanup building directories, must remove {foo}/build .
  f. run config_build.sh to build and install CSP.
  
  Have fun,
  Ohta.
  
     
