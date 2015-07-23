** Qt porting for Common Source Code Project **
                                           Jul 23, 2015
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

4.Qt specified notice:
   ・Place R@Ms under $HOME/emu{Machine Name}/ , this directory has made
     after first using.
   ・Config file, {foo}.ini is writteon on $HOME/.config/emu{Machine Name}/ .
   
5.Status
  a. I tested to build only under Debian GNU/Linux "sid".
     But, perhaps, will succed to build another GNU/Linux OSs or
     BSD OS variants.
     On windows, using MinGW or Cygwin is not tested yet,
     still use M$ Visual Studio 2013 (Community edition).
     
  b. Now, I using Qt5 as toolkit, because authors of Qt announced
     "Qt4 is obsolete, will be updated no longer".

  c. These machines are already ported to Qt (On Jul 23, 2015):
     ・Ascii MSX1/MSX2 (not PX-7).
     ・Casio FP-1100 .
     ・Casio FP-200 .
     ・Epson HC-20/40/80.
     ・Fujitsu FM-7/77/AV .(→ READ readme_fm7.txt)
     ・Fujitsu FM16pi .
     ・Fujitsu FM-R50(i286/i386/i486)/R60/R70/R80/R250/R280 (Not tested enough).
     ・Gijyutu hyouronsya babbage2nd.
     ・NEC PC-6001/mk2/mk2SR .
     ・NEC PC-6601/SR .
     ・NEC PC8001mk2SR (Not tested enough).
     ・NEC PC8801MA .
     ・NEC PC-9801/E/U/VF/VM .
     ・NEC PC98DO .
     ・NEC PC98LT/HA .
     ・NEC HE PC-ENGINE.
     ・NEC TK-80BS .
     ・NEC HE PC Engine.
     ・Tomy PYUTA.
     ・Sega Game Gear/Master System (Mark3).
     ・Sharp X1/turbo/turboZ/Twin.
     ・Sharp MZ-700/800/1500 .
     ・Sharp MZ-80A/K/1200 .
     ・Sharp MZ-80B/2200/2500 .

  c. Now using GCC-5.1 with Link Time Optimize to build for distrubuted binaries.

6. Upstream repositry:
      https://github.com/Artanejp/common_source_project-fm7

7. Upstream (Takeda Toshiya San's original code) 
      http://homepage3.nifty.com/takeda-toshiya/

Have fun!
-- Ohta.
   
   
