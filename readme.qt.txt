** Qt porting for Common Source Code Project **
                                           Mar 29, 2015
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

1. Background
   Common Source Code Project (CSP) is good emulator writing.
   But codes are specified to M$ Visual C.
   I'm using GNU/Linux, and I starting to apply FM-7(or later).
   So, I start to port to GNU/Linux and using Qt4.

2. What you need at least:
   a. Qt4 (neither Qt3 and Qt5) toolkit.
   b. gcc / g++ (4.7 or later? ) or llvm clang / clang++ (3.5 or later?) toolchain.
   c. SDL2 (not SDL1.x).
   d. CMake 2.8 or later.

3. Building:
   * After extracting sourcecodes:
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

4.Status
  a. I tested top build only under Debian GNU/Linux "jessie".
     But, perhaps, succeded to build another GNU/Linux OSs or
     BSD OS variants.
     On windows, using MinGW or Cygwin is not tested yet.

  b. These machines are already ported to Qt (On Mar 29, 2015):
     ・Epson HC-20
     ・Ascii MSX-1 (not PX-7).
     ・Fujitsu FM-7 .
     ・Sharp MZ-700/800/1500 .
     ・Sharp MZ-80A/K/1200 .
     ・NEC PC-6001/mk2/mk2SR .
     ・NEC PC-6601/SR .
     ・NEC PC8001mk2SR (Not tested enough).
     ・NEC PC8801MA .
     ・NEC PC-9801/E/U/VF/VM .
     ・NEC PC98DO .
     ・NEC PC98LT/HA .
     ・NEC HE PC-ENGINE.
     ・Shapr X1/turboZ.

5. Upstream repositry:
      https://github.com/Artanejp/common_source_project-fm7

Have fun!
-- Ohta.
   
   
