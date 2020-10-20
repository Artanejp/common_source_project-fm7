This docker image is for building Windows variants of Common Source
Code Project for Qt (*).
This based on Ubuntu 18.10 "Cosmic Cuttlefish",AMD64.

Needed toolchains are exists below /opt/llvm-mingw/ (*2).
Needed libraries are placed at /usr/local/i586-mingw-msvc/ ,
but, you need to install M$ Direct X SDK to re-build Qt.
Also, Angle Project (OpenGL ES) is imported from Google's chromium.

Have Fun!
Sep 26, 2019 K.Ohta <whatisthis.sowhat _at_ gmail.com>

(*1) https://github.com/Artanejp/common_source_project-fm7
(*2) https://github.com/Artanejp/llvm-mingw
     https://github.com/mstorsjo/llvm-mingw
     