** Qt porting for Common Source Code Project **
                                          Apr 09, 2017
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20170409

   追加情報:
   
    　各機種バイナリーは、osdn.net　もしくはミラーサイトより入手可能です。
    
   　https://osdn.net/projects/csp-qt/  
   
     https://osdn.net/projects/csp-qt/releases/　をチェックしてください。

　   Win32: 

  　 GNU/Linux(amd64) : 

【おねがい】
     　doc/以下の文書で日本語しかなかったものを英語に翻訳していますが、機械翻訳を使ってるのであやしいです。
     
       英語の上手い方、校正などお願いします m(_ _)m

1. 背景

   CSPは、非常に優れた構造のエミュレータです（しかし、些か重くてコンパイラ
   がいい最適化をしないと重めですが）。
   しかし、このコードはM$ Visual C++依存の部分が非常に多いです。
   そこで、GNU/Linuxでこれを動かすためにQtに色々と移植していきましょう。
   と言う感じで作業をはじめました。

2. 最低限必要なもの(Qt版)

   a. Qt5 ツールキット。バイナリはQt 5.5基準でビルドしてあります。
   
   b. OpenGL, 多分、最低OpenGL 2.1は必要です。 (New!)
   
   c. gcc / g++ (5.0以降？)もしくは llvm clang / clang++ (3.5以降?)
      コンパイラツールチェーン。
      
   d. SDL2 (SDL 1.xではないので注意)
   
   e. CMake 2.8以降。
   
   f. ffmpegから、libavとlibswが必要です。 http://ffmpeg.org/ より。
   
   g. ffmpegは、それぞれのランタイムに必要なものをバンドルしてありますので、動かない時はインストールしてみてください。
      
   h. GNU/Linuxビルドでは、Qt5.5(Ubuntu 16.04LTS向け)もしくはQt5.7(Debian GNU/Linux sid向け)でビルドしてあります。
   
   * Windows もしくは GNU/Linux のcross tool chain (要Wine)で、MinGW (gcc6) と Qt 5.7 でのビルドができることを確認しました。
     
   * TIPS:
   
     Windows等で動かした時に、画面の書き替えが表示されない場合は、
     
     環境変数 QT_OPENGL を software にしてみてください。（例えば、
     
     WindowsをVirtualBoxのゲストで使ってる場合など）
     
3. ビルドの方法

   ソースコードを解凍するか、git clone / pull した後で:
   
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

4. Qt固有の話(Windows除く)

   *ToolTipsを付けました。(2017-01-24)
      
   *日本語に翻訳しました。(2017-01-24)
   
   *R@Mを $HOME/emu{Machine Name}/　に配置してください。(Windowsの場合は今の所 .\emu{Machine Name}\)。なお、このディレクトリは最初起動した後で作成されます。
   
   *設定ファイルは、$HOME/.config/emu{Machine Name}/ に書き込まれます。(Windowsの場合は今の所 .\.config\emu{Machine Name}\)
   
   *ステートセーブファイルは、$HOME/emu{Machine Name}/{Machine Name}.sta に書き込まれます。
   
   *キーコード変換テーブルファイルが、$HOME/.config/emu{Machine Name}/scancode.cfg に書き込まれます。
   
     書式は、カンマで区切られた16進データです(10進ではないので注意) .
     
     1カラム目はM$ ヴァーチャルキーコード。
     
     2カラム目はQtネィティブのスキャンキーコードです。
     
   *UI部分の共通コンポーネント (src/qt/gui) を共有ライブラリlibCSPgui.soにまとめました。
   
   *インストール用のBASHスクリプトを用意しました。src/tool/installer_unix.shです。
   
   *ROMと同じところに、特定のWAVファイル(VMによって異なる)を入れると、FDDのシーク音やテープのボタン音・リレー音を鳴らすことが出来ます。
   
   *ローマ字カタカナ変換支援機構が一部の機種に実装されてます。romaji_kana.ja.txt をお読みください。
    
5. 移植状況
   
   a.現在、Debian GNU/Linux "sid"と、Ubuntu Linux 16.04LTS "Xenial"
     の AMD64版でしかテストしていません。
   　が、多分他のGNU/Linux OSやBSD系のOS (Mac含む) でもビルドすれば
   　動くでしょう。
     Windows もしくは GNU/Linux(要Wineとbinfmt-support)上でのMinGWと
     Qt community edition でのビルドが通るようになりました。
     安定したWindowsビルドを必要な方は、Visual Studio 2013 か 2015 のCommunity Edition
     でビルドしてください。（もう少ししたら、MinGWに切り替えようとは思ってます。)
      
   b. 今は、Qtの開発側が「Qt4おわりね」とアナウンスしたので、Qt4ではなく
      Qt5を使っています。
      添付してあるバイナリは、Qt 5.5でビルドしました(が、Qt 5.1以降なら動くはずです)。

   c. Linux用ビルドでは、GCC 6をリンク時最適化(LTO)モードで使っています。
   d. MZ-2500のソケット機能を実装してみていますが、マトモにテストできてません(；´Д｀)
   
6. Upstream repositry:
      https://github.com/Artanejp/common_source_project-fm7
      https://www.pikacode.com/Artanejp/common_source_project-fm7/

7. Project Page:
      https://osdn.jp/projects/csp-qt/

8. Upstream (Takeda Toshiyaさんのオリジナル) 
      http://takeda-toshiya.my.coocan.jp/

Changes:

ChangeLog:

* SNAPSHOT April 09, 2017
  * Upstream 2017-04-02
  * [General] Add National JR-800.
  * [UI/Qt] Move drive status from status-bar to right dock.
  * [FILEIO] Fix FTBFS with ZLIB 1.2.8 or earlier.
  * [VM/General] EMU/COMMON : Fix bugs around handling filename and directories.
  * [VM/General] common.cpp : Use buffer sized functions for some string functions.
  * [VM/FM77AV] DISPLAY: HSYNC:Don't register event(s) per HDISP.
  * [VM/FM7] DISPLAY: Reduce address calculation.
  * [VM/FM7] DISPLAY: Fix auto skip feature.
  * [VM/FM7] Use function table(s) to access memories within display sub-system and main-system.
  * [VM/FM77AV40EX] Fix wrong display timing.
  * [Build/CMake] Add supporting for ZLIB.
  * [BUILD/LINUX] Fix linking order.
  * [General/BUILD] Linux: Use -D_UNICODE to build.

-- Apr 09, 2017 23:04:45 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT March 07, 2017
  * Upstream 2017-03-04
  * [VM] Some devices have prepared to move to libCSPcommon_vm .
  * [VM/FMGEN] Move FMGEN to libCSPfmgen .
  * [WIN32] Ready to build with DLLs contain GUI and AVIO and some features.
  * [BUILD] Selectable building.
  * [BUILD] Add logging build-status.
  * [Qt] Fix break device files when exiting emulator with some situations.
  * [Win32/OpenGL/WIP] TRY: Fixing crash with OpenGL3.0 at Corei5-2420M (and Windows7) PC. See Issues.txt or Issues.ja.txt.
  * [DOC] Update Issues.See http://hanabi.2ch.net/test/read.cgi/i4004/1483504365/30 .
  * [EMU/DEBUGGER] Use pthread_t instead of SDL_Thread.
  * [Qt/OSD] Remove do_call_debugger_command().
  * [SOUND/VM] Fix choppy sounds with some devices. i.e. PCM1BIT. Thanks to Takeda-San and Umaiboux-San.
  * [VM/FMGEN] Fix crash with GCC-5.This is issue of optimization, add "volatile" to any member(s).
  * [BUILD/LINUX] GCC5: (MAYBE) Enable to set "USE_RADICAL_OPTIMIZE" to "YES".
  * Built with 2dac70eb1743e2a0b778a57a1f520fce59aa6371 or newer.

-- Mar 07, 2017 15:55:25 +0900 K.Ohta <whatisthis.sowhat@gmail.com>


本家の変更:

4/2/2017

[RESOURCE] improve menu items
[WINMAIN] improve menu items

[EX80] support to show/hide crt monitor


3/30/2017

[COMMON] add functions to convert char, wchar_t, and _TCHAR to each other
[COMMON] add _fgetts, _ftprintf, my_ftprintf_s, and my_swprintf_s
[COMMON/FILEIO] add Fgetts and Ftprintf for _TCHAR
[COMMON/FILEIO] fix functions using ZLIB for _UNICODE case
[EMU/DEBUGGER] fix for _UNICODE case
[WINMAIN] improve to update status only when status is changed

[VM/HUC6280] improve disassembler to support symbols (thanks Mr.Kei Moroboshi)
[VM/I8080] improve disassembler to support symbols
[VM/M6502] support debugger and disassembler (thanks MAME)
[VM/MCS48] improve disassembler to support symbols
[VM/TMS9995] fix disassembler for _UNICODE case
[VM/UPD7810] improve disassembler to support symbols
[VM/UPD7810] fix disassembler for _UNICODE case

[BABBAGE2ND] support debugger and save/load state
[FAMILYBASIC] support debugger
[YS6464A] support debugger and save/load state


3/28/2017

[COMMON] add _tcscat and my_tcscat_s
[COMMON] add structure and functions to support symbols
[EMU/DEBUGGER] improve debugger to support symbols (thanks Mr.Kei Moroboshi)

[VM/MC6800] improve disassembler to support symbols (thanks Mr.Kei Moroboshi)
[VM/MC6809] improve disassembler to support symbols (thanks Mr.Kei Moroboshi)
[VM/TMS9995] improve disassembler to support symbols (thanks Mr.Kei Moroboshi)
[VM/UPD7801] improve disassembler to support symbols (thanks Mr.Kei Moroboshi)
[VM/Z80] improve disassembler to support symbols (thanks Mr.Kei Moroboshi)


3/26/2017

[VM/YM2413] support mute

[FAMILYBASIC] support Family BASIC MMC5/VRC7 MOD
[FAMILYBASIC] support correct scanlines
[FAMILYBASIC/MEMORY] support MMC5/ VRC7 based on unofficial nester
[FAMILYBASIC/MEMORY] fix data recorder signal (thanks MESS)


3/20/2017

[COMMON] suport to read ascii/binary file compressed by gzip
[RESOURCE] fix fm8/fm7/fm77 dipswitch menu items
[WINMAIN] fix to update status bar twice/second to supress flickar

[VM/SCSI_CDROM] support CD-ROM bin/img file compressed by gzip
[VM/DATAREC] support tape image file compressed by gzip
[VM/DATAREC] support to save FUJITSU FM-7/77 t77 format tape image

[BMJR/MEMORY] fix sound mixer not to clear previously mixed sound


3/18/2017

[WINMAIN] support multiple data recorder
[EMU] support multiple data recorder

[VM/DATAREC] support HITACH BASIC Master Jr bin format tape image
[VM/PTF20] support access lamp signal

[BMJR/MEMORY] fix reading cmt signal
[BMJR/MEMORY] fix sound mixer
[TK80BS/CMT] support TK-80 cmt i/f


3/15/2017

[WINMAIN] add status bar to draw access lamps
[EMU] add interfaces to get access status of floppy/quick/hard disk drives
[EMU] add interface to get cmt status message

[VM/DATAREC] support cmt status message
[VM/HD44102] fix build error
[VM/MC6800] fix M_RDOP and M_RDOP_ARG macros (thanks PockEmul)
[VM/LD700] support access lamp signal
[VM/SCSI_CDROM] support access lamp signal
[VM/SCSI_DEV] support access lamp signal
[VM/SCSI_HDD] support access lamp signal

[HC20] fix issue that we cannot debug TF-20 CPU
[JR800] support National JR-800 (thanks PockEmul)


3/12/2017

[VM/DATAREC] fix issue that fast forward noise may not stop


3/11/2017

[VM/DATAREC] support to play play/stop/fast-fwd noise
[VM/MB8877] support to play seek/load/unload noise
[VM/HD44102] support HD44102 based on MAME 0.171
[VM/T3444A] support to play seek/load/unload noise
[VM/UPD765A] support to play seek/load/unload noise
[VM/UPD765A] fix seek time (thanks Mr.Artane.)

[MZ80B/MEMORY80B] fix vgate signal (thanks Mr.Suga)


3/8/2017

[VM/DISK] improve to check if disk image is modified more strictly
[VM/DISK] import yaya2016-04-13plus1 (thanks Mr.umaiboux)
[VM/V9938] import yaya2016-04-13plus1 (thanks Mr.umaiboux and MAME)
[VM/YM2413] import yaya2016-04-13plus1 (thanks Mr.umaiboux)

[FMR30] split project for i86 and i286
[MSX/*] import yaya2016-04-13plus1 (thanks Mr.umaiboux)


3/7/2017

[WIN32/INPUT] improve to accept shift + caps/kana/kanji

[VM/DISK] support device name (thanks Mr.Aratane.)
[VM/EVENT] improve to check abnormal scanline number (thanks Mr.Sato)
[VM/EVENT] fix light weight sound rendering (thanks Mr.168)
[VM/*] add device name to all devices of each machine (thanks Mr.Aratane.)

[MZ80B] change floppy drive type from 2D to 2DD (thanks Mr.Suga)
[MZ80B/MEMORY80B] support I-O DATA PIO-3039 (thanks Mr.Suga)

-----


お楽しみあれ!
-- Ohta.
