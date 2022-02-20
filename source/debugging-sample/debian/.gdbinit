# Sample script for GDB initializing.
# Copyright Feb 20, 2022 Kyuma Ohta <whatisthis.sowhat@gmail.com>

## Note: These versions are someday's source code of Debian GNU/Linux, sid.
## Usage:
## 1. Extract source code of libraries to any directory (i.e. ~/src .)
## 2. Modify ~/.gdbinit (mostly add a below line)
### add-auto-load-safe-path $HOME/src/common_source_project-fm7/source/build/.gdbinit
## 3. Copy this to build root (mostly $HOME/src/common_source_project-fm7/source/build/ )
## 4. Enjoy Debug!! (^ω^)

# Uncomment if you don't include source code to debug build of CSP.
directory ~/src/common_source_project-fm7/source/src
# libc
directory ~/src/glibc-2.33
# Standard C++ libraries.
directory ~/src/gcc-11-11.2.0

# Uncomment below if you built with Qt 5.x.
#directory ~/src/qtbase-opensource-src-5.15.2+dfsg
# Uncomment below if you built with Qt 6.x.
directory ~/src/qt6-base-6.2.2+dfsg

directory ~/src/libsdl2-2.0.20+dfsg

# Below is dirty hack (WHY?)
directory ~/src/glib2.0-2.70.4/debian/patches/debian

directory ~/src/harfbuzz-2.7.4
# Uncomment below if you built with FFmpeg-4.x.
directory ~/src/ffmpeg-4.4.1
# Uncomment below if you built with FFmpeg-5.x.
#directory ~/src/ffmpeg-5.0

# OpenGL 
directory ~/src/mesa-21.3.5
directory ~/src/libdrm-2.4.110

