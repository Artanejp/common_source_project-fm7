** FM-7 series  emulator for common source code project. **
                                               Mar 29, 2015
		   K.Ohta <whatisthis.sowhat _at_ gmail.com>

1.Background
  Major FM-7 series emulator, XM7 is closed source code.
  But, I was porting to SDL/Agar toolkit.[1]
  This has many of bugs inheritated from Agar Toolkit.
  And, I wish to distribute FM-7 emulator distributed by
  FOSS license.
  So, I decided to build FM-7 emulator to Common Source Code
  Project.

  [1] https://github.com/Artanejp/XM7-for-SDL
  
2.Status
  a. FM-7 is working now.
     Excepts : OS-9 Level1 is too heavy to use.
  b. FM-77 is *not* working now, partly implemented.
     Especially 400 line card is still not implement.
  c. FM-8 is *not* implement, I have no document, now.
  d. FM-77AV (or later) is *partly* implement, but not test to work.

3.How to Work
  You Need these R@M images to work FM-7.
  If you don't have these images, you can get substitution R@Ms
  from : http://retropc.net/apollo/download/xm7/romset/index.htm .
  
  At least for FM-7 or later:
  BOOT_BAS.ROM : 512 bytes, To boot as BASIC mode.
  BOOT_DOS.ROM : 512 bytes, To boot as DOS(NOT MS-DOS) mode.
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

  Optionally ROMS:
  KANJI2.ROM   : 131072 bytes, Kanji JIS class 2 patterns.
  DICROM.ROM   : 262144 bytes, Dictionary data for Kana-Kanji conversion.
  EXTSUB.ROM   : 49152 bytes, extra monitor for subsystem (77AV20 or later?)

  Making if you use DICROM :
  USERDIC.DAT  : 8192 bytes, learning data of Kana-Kanji conversion.

  FM-8 is not designed yet.

4. Upstream repositry:
      https://github.com/Artanejp/common_source_project-fm7


Enjoy!
-- K.Ohta.
