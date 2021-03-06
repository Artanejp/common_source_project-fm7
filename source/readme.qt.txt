** Qt porting for Common Source Code Project **
                                        March 03, 2020
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

0. About
   This package is Qt5 porting of Common Source Code Project (CSP).
   
   Building with GNU/Linux(64bit) and MinGW(32bit Windows).
   
1. Background

   Common Source Code Project (CSP) is good emulator writing.
   But codes are specified to M$ Visual C.
   I'm using GNU/Linux, and I starting to apply FM-7(or later).
   So, I start to port to GNU/Linux and using Qt4.
   
   * Note: You can build with MinGW32 and Qt 5.7(for MinGW).

   * TIPS: If emufoo.exe don't show screen drawing, set environment 
           variable QT_OPENGL to software (i.e. Using Windows as VirtualBox's gueat OS).
     
2. What you need at least:

   a. Qt5 (neither Qt3 and Qt4) toolkit. Recommend to use Qt5.5 or later.
   b. Some OpenGL implementation, maybe at leaset OpenGL v2.1.
   c. gcc / g++ (4.7 or later? ) or llvm clang / clang++ (3.5 or later?) toolchain.
   d. SDL2 (not SDL1.x).
   e. CMake 2.8 or later.
   f. libav and libsw from ffmpeg (optional) http://ffmpeg.org/ . 

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
   ・Config file (scancode.cfg and foo.ini) has placed (moved) to
     "~/.config/CommonSourceCodeProject/emufoo/"
     (for Windows, ".\CommonSourceCodeProject\emudfoo\" ).
   ・BIOS, WAVS, BINS and SAVED STATES have placed (moved) to
     "~/CommonSourceCodeProject/emufoo/"
     (for Windows, ".\CommonSourceCodeProject\emudfoo\" ).
   ・All of recorded products (PNG Screenshots, MOVIES, SOUNDS) 
     have *temporally* written to "~/.config/CommonSourceCodeProject/emufoo/"
     (for Windows, ".\CommonSourceCodeProject\emudfoo\" ).
   ・Added ToolTips and Japanese translations.(2017-01-24)
   ・Place R@Ms under $HOME/CommonSourceCodeProject/emu{Machine Name}/ , this directory has made
     after first using.
   ・Config file, {foo}.ini is written on $HOME/.config/CommonSourceCodeProject/emu{Machine Name}/ .
   ・Saved state file, {foo}.sta is written on $HOME/CommonSourceCodeProject/emu{Machine Name}/ .
   ・Key code conversion file is written on $HOME/.config/CommonSourceCodeProject/emu{Machine Name}/scancode.cfg .
     This file contains comma separated fields, written at hex-decimal (not decimal), 
     first is M$ Virtual Key Code,
     second is Qt's scan code.
   ・Common UI components (at src/qt/gui ) are moved to shared lib. libCSPgui.so .
   ・Installer (bash) script is available now; src/tool/installer_unix.sh .
   ・When you place specified WAV files(differ per VM) same directory as ROMs, you can sound sounds: i.e. FDD seek, CMT relay and buttons.
   
5.Status
a. I tested to build only under Debian GNU/Linux "sid" and 
   Ubuntu Linux 16.04LTS (Xenial).
   But, perhaps, will succed to build another GNU/Linux OSs or BSD OS variants.
   * On windows, using MinGW is already okay.
   * Cross building with GNU/Linux's MinGW32 (GCC6) and Qt5.7 (for MinGW) is available. 
     Modify and use build-cmake/config_build_cross_win32.sh and related *.cmake files.
   * And, you can also build with M$ Visual Studio 2013 or 2015.
   
  b. Now, I using Qt5 as toolkit, because authors of Qt announced
     "Qt4 is obsolete, will be updated no longer".

  c. All of virtual machines of upstream (@Apr 13, 2016) are ported to Qt.
  d. Implement socket feature using by MZ-2500, but not enough to test (；´Д｀)
  e. Now using GCC-6 with Link Time Optimize to build for distrubuted binaries.

6. Upstream repositry:

      https://osdn.jp/projects/csp-qt/
      
      https://github.com/Artanejp/common_source_project-fm7

*Note: Repositry of pikacode has dead X-).
      
7. Upstream (Takeda Toshiya San's original code) 
      http://takeda-toshiya.my.coocan.jp/

Have fun!
-- Ohta.
