<H2>** Qt porting for Common Source Code Project **</H2>
<div align="right">
<H3>Feb 10, 2018<BR>
K.Ohta <whatisthis.sowhat _at_ gmail.com></H3>
</div>

## *If you can read Japanese, [japanese(日本語) writing is here](/README.md/).*

About
====

   This package is Qt5 porting of Common Source Code Project (CSP).
   
   Building with GNU/Linux(64bit) and MinGW(32bit Windows).

## Source code
   
<https://github.com/Artanejp/common_source_project-fm7/> 

## Additional infomations
   
You can get pre-compiled binaries from [osdn.net](http://osdn.net) and their mirrors.
    
<https://osdn.net/projects/csp-qt/>  
   
<https://osdn.net/projects/csp-qt/releases/>


LICENSE
======

[GPL Version 2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html).

Background
==========

Common Source Code Project (CSP) is good emulator writing.
   
But codes are specified to M$ Visual C.
   
I'm using GNU/Linux, and I starting to apply FM-7(or later).
   
So, I start to port to GNU/Linux and using Qt4.
   
## Note:

You can build with MinGW32 and Qt 5.5.1(for MinGW).

TIPS:

* If emufoo.exe don't show screen drawing, set environment variable QT_OPENGL to software (i.e. Using Windows as VirtualBox's gueat OS).
     
What you need at least:
=====

* Qt5 (neither Qt3 and Qt4) toolkit.

* Some OpenGL implementation, maybe at leaset OpenGL v3.0 .

* gcc / g++ (5.4 or later? ) or llvm clang / clang++ (3.5 or later?) toolchain.

* SDL2 (not SDL1.x).

* CMake 2.8 or later.

How to build:
=====

After extracting (or git pulled) sourcecodes:

    $ cd {srctop}/source/build-cmake/{Machine name}/
    $ mkdir build
    $ cd build
   
*To configure:*
   
    $ cmake ..
	
 or
   
    $ ccmake ..

*To build:*

    $ make

*To install:*

    $ sudo make install

Qt specified notice :
====

* Config file (scancode.cfg and foo.ini) has placed (moved) to "~/.config/CommonSourceCodeProject/emufoo/" (for Windows, ".\CommonSourceCodeProject\emudfoo\" ).

* BIOS, WAVS, BINS and SAVED STATES have placed (moved) to  "~/CommonSourceCodeProject/emufoo/" (for Windows, ".\CommonSourceCodeProject\emudfoo\" ).

* All of recorded products (PNG Screenshots, MOVIES, SOUNDS) have *temporally* written to "~/.config/CommonSourceCodeProject/emufoo/" (for Windows, ".\CommonSourceCodeProject\emudfoo\" ).

* Added ToolTips.(2017-01-24)
      
* Translated menu entries to Japanese.(2017-01-24)

* Place R@Ms under $HOME/CommonSourceCodeProject/emu{Machine Name}/ , this directory has made after first using.

* Key code conversion file is written on $HOME/.config/CommonSourceCodeProject/emu{Machine Name}/scancode.cfg .

  ** This file contains comma separated fields, written at hex-decimal (not decimal),
 
  ** First is M$ Virtual Key Code,
 
  ** Second is Qt's scan code.
   
* Common components (i.e. source/src/qt/gui ) are packed to DLLs/shared libs.

* Bash script for installation is available: source/src/tool/installer_unix.sh .

* When you place some WAV files to directry same as R@M, you can some sounds; i.e. FDD's seek.See doc/VMs/*.txt .

* ROMAJI-KANA conversion assitance feature has implemented to some VMs.

Status
====

* I tested to build only under Debian GNU/Linux "sid".But, perhaps, will succed to build another GNU/Linux OSs or BSD OS variants.

  ** On windows, using MinGW is already okay.
 
  ** Cross building with GNU/Linux's MinGW32 and Qt5.5.1 (for MinGW) is available.
 
  ** Modify and use build-cmake/config_build_cross_win32.sh and related *.cmake files.
 
  ** And, you can also build with M$ Visual Studio 2013 or 2015.
   
* Now, I using Qt5 as toolkit, because authors of Qt announced "Qt4 is obsolete, will be updated no longer".

* All of virtual machines of upstream (@Dec 17, 2015) are already ported to Qt.

* Now using GCC-7.x with Link Time Optimize to build for distrubuted binaries.

* Implemented MZ-2500's socket, but, still not test enough(；´Д｀)

Repositry:
===

<https://github.com/Artanejp/common_source_project-fm7>
      
<https://osdn.net/projects/csp-qt/scm/git/common_source_project-fm7>

Project Page:
==

<https://osdn.jp/projects/csp-qt/>

Upstream (Takeda Toshiya San's original code)
====

<http://takeda-toshiya.my.coocan.jp/>

Special thanks to:
==

  Ryu Takegami : Assist to debug and give informations about eFM-8/7/77/AV/40/EX .

Have fun!

--- Ohta.
 
&copy; 2018 Toshiya Takeda

&copy; 2018 K.Ohta <whatisthis.sowhat _at_ gmail.com>

