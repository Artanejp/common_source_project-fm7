** FM-7 series  emulator for common source code project. **
                                               Jun 25, 2015
		   K.Ohta <whatisthis.sowhat _at_ gmail.com>

1.Background
  Major FM-7 series emulator, XM7 is closed source code, not FOSS.
  But, I was porting to SDL/Agar toolkit.[1]
  This has many of bugs inheritated from Agar Toolkit.
  And, I wish to distribute FM-7 emulator with FOSS license.
  So, I decided to build FM-7 emulator to Common Source Code
  Project.

  [1] https://github.com/Artanejp/XM7-for-SDL
  
2.Status
  a. FM-7 is working now. Excepts "Gambler Jiko Chusinha" or another.
  b. FM-77 is working now.
     Especially 400 line card is still not implement.
  c. FM-8 is *not* implement, I have no document, now.
  d. FM77AV is mostly implemented, some of softwares are working, 
     or not.
     Some of softwares don't detects Joystick, and some of another 
     softwares don't detect keyboard well.
     Some of softwares are not booting (i.e. DAIVA).
  e. Now, implementing FM77AV40SX, but mostly not working.
  f. FM-8 will be implemented, but not start.
  
3.How to Work
  You Need these R@M images to work FM-7.
  If you don't have these images, you can get substitution R@Ms
  from : http://retropc.net/apollo/download/xm7/romset/index.htm .
  
  At least for FM-7 or later:
  BOOT_BAS.ROM : 512 bytes, To boot as BASIC mode.
  BOOT_DOS.ROM : 512 bytes, To boot as DOS(NOT MS-DOS) mode.
  FBASIC302.ROM
  FBASIC300.ROM
  FBASIC30.ROM : 31744 bytes, F-BASIC 3.0 code,
                 Dummy (only BIOS) rom if you use substitution ROMS.
  SUBSYS_C.ROM : 10240 bytes, Monitor of SUBCPU.

  Optionally ROMS:
  KANJI.ROM
  KANJI1.ROM   : 131072 bytes, Kanji JIS class 1 patterns.
  BOOT_MMR.ROM : 512 bytes, hidden boot ROM for FM-77 (only).

  You need belows if you try to work FM-77AV:
  INITIATE.ROM : 8192 bytes, initiator ROM.
  SUBSYSCG.ROM : 8192 bytes, character data for subsystem.
  SUBSYS_A.ROM : 8192 bytes, monitor type A for sub system.
  SUBSYS_B.ROM : 8192 bytes, monitor type B for sub system.

  Optionally ROMS (For FM77AV20/40):
  KANJI2.ROM   : 131072 bytes, Kanji JIS class 2 patterns.
  DICROM.ROM   : 262144 bytes, Dictionary data for Kana-Kanji conversion.
  EXTSUB.ROM   : 49152 bytes, extra monitor for subsystem (77AV20 or later?)

  Making if you use DICROM :
  USERDIC.DAT  : 8192 bytes, learning data of Kana-Kanji conversion.


4. Upstream repositry:
      https://github.com/Artanejp/common_source_project-fm7


Enjoy!
-- K.Ohta.
