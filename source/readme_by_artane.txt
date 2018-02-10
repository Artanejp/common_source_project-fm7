** Qt porting for Common Source Code Project **
                                         February 11, 2018
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20180211

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

   a. Qt5 ツールキット。Qt 5.5以降を推奨します。
   
   b. OpenGL, 多分、最低OpenGL 2.1は必要です。（注：ひょっとしたら、OpenGLES2以降ならば動くように変えるかも知れない）
   
   c. gcc / g++ (5.0以降？)もしくは llvm clang / clang++ (3.5以降?)
      コンパイラツールチェーン。
      
   d. SDL2 (SDL 1.xではないので注意)
   
   e. CMake 2.8以降。
   
   f. ffmpegから、libavとlibswが必要です。 http://ffmpeg.org/ より。
   
   g. ffmpegは、それぞれのランタイムに必要なものをバンドルしてありますので、動かない時はインストールしてみてください。
      
   h. GNU/Linuxビルドでは、Qt5.5(Ubuntu 16.04LTS向け)もしくはQt5.9(Debian GNU/Linux sid向け)でビルドしてあります。
   
   * Windows もしくは GNU/Linux のcross tool chain (要Wine)で、MinGW (gcc6) と Qt 5.7 でのビルドができることを確認しました。
     
   * TIPS:
   
     * Windows等で動かした時に、画面の書き替えが表示されない場合は、環境変数 QT_OPENGL を software にしてみてください。（例えば、
       WindowsをVirtualBoxのゲストで使ってる場合など）
       
     * Windows版バイナリには、ソフトウェアレンダリングのopengl32.dllが添付されてますが、最近のパソコンの専用GPUドライバなら、
       もっと程度のいいOpenGLが入ってるはずです。
       添付版opengl32.dllを適当な名前に変更して動くかどうか試してみて下さい。
     
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

4. Qt固有の話

   * 設定ファイル(scancode.cfg と foo.ini)は、"~/.config/CommonSourceCodeProject/emufoo/" (Windowsの場合は".\CommonSourceCodeProject\emudfoo\" ) におかれます（移動しました）。

   * BIOSや効果音WAVやセーブステートは、、"~/CommonSourceCodeProject/emufoo/" (Windowsの場合は".\CommonSourceCodeProject\emudfoo\" ) におかれます（移動しました）。
   
   * 全ての記録物(スクリーンショットや動画や録音WAV）は、*当面の間* "~/CommonSourceCodeProject/emufoo/" (Windowsの場合は".\CommonSourceCodeProject\emudfoo\" ) におかれます。

   * ToolTipsを付けました。(2017-01-24)
      
   * 日本語に翻訳しました。(2017-01-24)
   
   * R@Mを $HOME/emu{Machine Name}/　に配置してください。(Windowsの場合は今の所 .\emu{Machine Name}\)。なお、このディレクトリは最初起動した後で作成されます。
   
   * 設定ファイルは、$HOME/.config/emu{Machine Name}/ に書き込まれます。(Windowsの場合は今の所 .\.config\emu{Machine Name}\)
   
   * ステートセーブファイルは、$HOME/emu{Machine Name}/{Machine Name}.sta に書き込まれます。
   
   * キーコード変換テーブルファイルが、$HOME/.config/emu{Machine Name}/scancode.cfg に書き込まれます。
   
     書式は、カンマで区切られた16進データです(10進ではないので注意) .
     
     1カラム目はM$ ヴァーチャルキーコード。
     
     2カラム目はQtネィティブのスキャンキーコードです。
     
   * UI部分の共通コンポーネント (src/qt/gui) を共有ライブラリlibCSPgui.soにまとめました。
   
   * インストール用のBASHスクリプトを用意しました。src/tool/installer_unix.shです。
   
   * ROMと同じところに、特定のWAVファイル(VMによって異なる)を入れると、FDDのシーク音やテープのボタン音・リレー音を鳴らすことが出来ます。
   
   * ローマ字カタカナ変換支援機構が一部の機種に実装されてます。romaji_kana.ja.txt をお読みください。
    
5. 移植状況
   
   a.現在、Debian GNU/Linux "sid"と、Ubuntu Linux 16.04LTS "Xenial"
     の AMD64版でしかテストしていません。
   　が、多分他のGNU/Linux OSやBSD系のOS (Mac含む) でもビルドすれば
   　動くでしょう。
     Windows もしくは GNU/Linux(要Wineとbinfmt-support)上でのMinGWと
     Qt community edition でのビルドが通るようになりました。
      
   b. 今は、Qtの開発側が「Qt4おわりね」とアナウンスしたので、Qt4ではなく
      Qt5を使っています。
      添付してあるバイナリは、Qt 5.5でビルドしました(が、Qt 5.1以降なら動くはずです)。

   c. Linux用ビルドでは、GCCをリンク時最適化(LTO)モードで使っています。
   d. MZ-2500のソケット機能を実装してみていますが、マトモにテストできてません(；´Д｀)
   
6. Upstream repositry:
      https://github.com/Artanejp/common_source_project-fm7
      
      https://osdn.net/projects/csp-qt/scm/git/common_source_project-fm7

7. Project Page:
      https://osdn.jp/projects/csp-qt/

8. Upstream (Takeda Toshiyaさんのオリジナル) 
      http://takeda-toshiya.my.coocan.jp/


Special thanks to:
  Ryu Takegamiさん : eFM-8/7/77/AV/40/EX のデバッグに協力していただいています。
  はせりんさん     : eFM-8/7/77/AV/40/EX のデバッグに協力していただいています。
Changes:

ChangeLog:
* 前の変更点をお読みになる場合には、ChangeLogと000_gitlog.txtをお読み下さい。

* SNAPSHOT Feb 11, 2018
  * Upstream 2017-12-27 .
  * [CONFIG] IMPORTANT: Now, "~/emufoo/" moved to "~/CommonSourceCodeProject/emufoo/"
	                (or ".\CommonSourceCodeProject\emufoo\" ).
             Please move settings to here.
  * [CONFIG] IMPORTANT: Now, "~/emufoo/foo.ini" has moved to "~/.config/CommonSourceCodeProject/emufoo/"
	                      (or ".\CommonSourceCodeProject\emufoo\ at Windows").
             Please move ini and scancode.conf to new place.
  * [VM/MB8877] Enable to boot FLEX/FM-8.
                Thanks to https://matsuri.5ch.net/test/read.cgi/i4004/1483504365/800- .
  * [UI/Qt] Add option: Emulate cursor keys as "2468" or "1235".
  * [UI/KEYBOARD] Add option, "Numpad's Enter is Fullkey's".Useful for FM-77AV series, etc.
  * [UI/VM] MB8877/T3444/uPD765A: Add ON/OFF debug logging for FDCs.
  * [Qt/EMU] Seriarize queueing both KEY_DOWN and KEY_UP.
             Expect to fix  key-in issues with slower host machine.
	     See https://matsuri.5ch.net/test/read.cgi/i4004/1483504365/841 .
  * [UI/Qt] BUG: Fix crash with BOOT_MODEs >= 8.
  * [UI/Qt] FM7: Extend boot menu.
  * [UI/KEYBOARD] Update VM table.
  * [UI/KEYBOARD] Fix load/save sequence of scancode.cfg.
  * [UI/Qt] Update translate for menu.Split section per menu-bar-entry.
  * [MENU/Qt] DISK: Fix not affect of some entries:"Correct disk timing","Ignore CRC error" and "Immediate transfer".
  * [Draw/OpenGL] FM77AV: Fix blinking with some situation.
  * [VM/MC6809] Fix around CWAI with interrupts.
  * [VM/MC6809] Make set/reset E flag at interrupt.
  * [VM/MC6809] More accurate emulation(maybe).
  * [VM/MC6809] Fix clock count with some situation, FM-8/7/77/AV don't need dummycpu.
  * [VM/FM7] Remove DUMMY CPU.Reduce host-cpu usage a lot.
             Related to commit a4e1a7cfef59e08f31675c6608871068d3f2c4ef.
  * [EMU/FM7] emu.cpp : No longer use hack with some VMs.
  * [VM/FM7] KEYBOARD: Keep BREAK key when special reset (hot start).
  * [VM/FM7] MAINIO: Reduce hitting PCM1BIT:: when not sound buzzer.
  * [VM/FM7] DISK: Fix RPM value of 2D/2DD drive to 300rpm.
  * [VM/FM7] Read some (bulk) roms for FM-8/7/NEW7/77.This is WORK-IN-PROGRESS.
  * [VM/FM8/FM77] Add delay FIRQ/NMI from 2HD/SFD FDC.This is temporally implement.
  * [VM/FM8] MAINIO: Move FM-8 specified features to FM8_MAINIO:: .
  * [VM/FM77] Add initial support of 2HD (for FM-77).
  * [VM/FM77] Fix access around BOOT-RAM.Thanks to Ryu Takegami.
  * [VM/FM77] Extra memories should start $00000, not $10000.Thanks to Ryu Takegami.
  * [VM/FM77] Fix wrong answer at $FD04.Thanks to Ryu Takegami.
  * [VM/FM77AV] DISPLAY: Separate event process around VSYNC/HSYNC.
  * [VM/FM77AV40] Fix FTBFS with Fedora linux.
  * [VM/FM77AV40EX] Do not effect offset registers to VRAM-ACTIVE-BLOCK 1.Thanks to Ryu Takegami.
  * [VM/MB8877] No need to hack with PSY-O-BLADE for FM77AV.This is test delete.
  * [VM/MB8877] Force down DRQ when LOST-DATA happens.
  * [VM/MB8877] Fix not read status data at the endof SEEK and verifying.
  * [VM/MB8877] Update structure of MB8877->fdc.
  * [VM/MB8877] Try: Improbe result quality of FORCE_INTERRUPT command.
                 I expect to boot OS-9 Level1 for FM-77 with 2HD drive.Thanks to Haserin-san.
  * [VM/DISK] Fix default value ignore_crc_error and correct_disk_timing (mostly "to false").
  * [DOC] Add README.md (written in Japanese) and README.en.md (written in English) for GITHUB's cover page.
  * Special thanks to Ryu Takegami, to assist debugging FM-7/77/AV/40/EX/8 .
  * Special thanks to Haserin, to assist debugging FM-7/77.AV/40/EX/8.
  * Built with commit 221fcef002f101b8dffd2a63edfc11a86e2c688a or later.

-- Feb 11, 2018 02:29:55 +0900 K.Ohta <whatisthis.sowhat@gmail.com>


本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

12/27/2017

[EMU] support to restore sound frequency/latency settings when load state

[VM/DISK] support T98-NEXT nfd r0 floppy disk image
[VM/DISK] support BKDSK hdm/hd5/hd4/hdb/dd9/dd6 floppy disk image


12/15/2017

[COMMON/FILEIO] add Fcompare function

[VM/*] improve save/load state function to check device class name
[VM/YM2151] fix save/load state function


12/14/2017

[RESOURCE] change accelerators for switching full speed and roman/kana input
[WINMAIN] fix to call ImmAssociateContext() in WM_ACTIVATE (thanks PC8801MA改)

[VM/HD46505] fix to force update event timing when R0-R9 are modified
[VM/HUC6280] support to show clocks since starting scanline in debugger
[VM/I286] support to show clocks since starting scanline in debugger
[VM/I386] support to show clocks since starting scanline in debugger
[VM/M6502] support to show clocks since starting scanline in debugger
[VM/MC6800] support to show clocks since starting scanline in debugger
[VM/MC6809] support to show clocks since starting scanline in debugger
[VM/MCS48] support to show clocks since starting scanline in debugger
[VM/TMS9995] support to show clocks since starting scanline in debugger
[VM/UPD7220] fix to force update event timing when sync are modified
[VM/UPD7801] support to show clocks since starting scanline in debugger
[VM/UPD7810] support to show clocks since starting scanline in debugger
[VM/Z80] support to show clocks since starting scanline in debugger

[X1TURBOZ/DISPLAY] fix pcg/analog palette access wait again (thanks Mr.Sato)


12/12/2017

[VM/DISK] fix crc in id/data field (thanks Mr.Sato)
[VM/HD46505] fix not to raise DISPTMG signal if bit2,3 of R8 are 11

[X1TURBOZ/DISPLAY] fix pcg/analog palette access wait (thanks Mr.Sato)
[X1TURBOZ/DISPLAY] fix to draw each line at start of hblank (thanks Mr.Sato)

-----

お楽しみあれ!
-- Ohta.
