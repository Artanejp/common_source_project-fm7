** Qt porting for Common Source Code Project **
                                           Oct 29, 2015
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

0. About
   This package is Qt5 porting of Common Source Code Project (CSP)
   and Built with Visual Studio 2015 + DirectX9 + Direct Input 8 for Win32.
   
1. Background
   Common Source Code Project (CSP) is good emulator writing.
   But codes are specified to M$ Visual C.
   I'm using GNU/Linux, and I starting to apply FM-7(or later).
   So, I start to port to GNU/Linux and using Qt4.

2. What you need at least:
   a. Qt5 (neither Qt3 and Qt4) toolkit.
   b. Some OpenGL implementation, maybe at leaset OpenGL v2.x .
   c. gcc / g++ (4.7 or later? ) or llvm clang / clang++ (3.5 or later?) toolchain.
   d. SDL2 (not SDL1.x).
   e. CMake 2.8 or later.

3. How to build:
   After extracting (or git pulled) sourcecodes:
   $ cd {srctop}/source/build-cmake/{Machine name}/
   $ mkdir build
   $ cd build
   
   To configure:
   $ cmake ..
   or
   $ ccmake ..

   To build:
   $ make

   To install:
   $ sudo make install

4.Qt specified notice (for non-Windows):
   ・Place R@Ms under $HOME/emu{Machine Name}/ , this directory has made
     after first using.
   ・Config file, {foo}.ini is written on $HOME/.config/emu{Machine Name}/ .
   ・Saved state file, {foo}.sta is written on $HOME/emu{Machine Name}/ .
   ・Key code conversion file is written on $HOME/.config/emu{Machine Name}/scancode.cfg .
     This file contains comma separated fields, written at hex-decimal (not decimal), 
     first is M$ Virtual Key Code,
     second is Qt's scan code.
   
5.Status
  a. I tested to build only under Debian GNU/Linux "sid".
     But, perhaps, will succed to build another GNU/Linux OSs or
     BSD OS variants.
     On windows, using MinGW or Cygwin is not tested yet,
     still use M$ Visual Studio 2013 (Community edition).
     
  b. Now, I using Qt5 as toolkit, because authors of Qt announced
     "Qt4 is obsolete, will be updated no longer".

  c. All of virtual machines of upstream (@Oct 02, 2015) are already ported to Qt.
  d. Now using GCC-5.2 with Link Time Optimize to build for distrubuted binaries.

6. Upstream repositry:
      https://github.com/Artanejp/common_source_project-fm7

7. Upstream (Takeda Toshiya San's original code) 
      http://homepage3.nifty.com/takeda-toshiya/

Have fun!
-- Ohta.
   
   
