** Qt porting for Common Source Code Project **
                                         January 29, 2018
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20180130_FLEX_TEST4

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
  Ryu Takegami : eFM-8/7/77/AV/40/EX のデバッグに協力していただいています。

Changes:

ChangeLog:
* 前の変更点をお読みになる場合には、ChangeLogと000_gitlog.txtをお読み下さい。
* SNAPSHOT Jan 19 2018
  * Upstream 2017-12-27 .
  * [EMU] Add set_vm_screen_lines(int lines) to notify changing vertical resolution
        to OSD and Hardware-Renderer(for scan lines). See vm/fm7/display.cpp .
  * [VM] Add EMU::set_vm_screen_lines() to some VMs.
  * [UI/Qt] Add "Click On Focus" feature option.This don't need to mouse-over to input from keyboard if set.
  * [VM/FM7] Add Japanese communication board (日本語通信カード) .
  * [VM/FM7] Add turning ON/OFF Z80 extra board.
  * [VM/FM7] Add UARTs.RS-232C, MODEM and MIDI.Temporally implements.
  * [FM7/JCOMMCARD] Add test program.Use asl ( http://john.ccac.rwth-aachen.de:8000/as/ ) to make from source code.
  * [VM/FM7] Use #define to define name of firmwares.
  * [VM/FM7] KANJIROM: Add JIS78 patch mode.From XM7 3.4L77a.
  * [VM/MB8877] Not clear status when force-interrupt with TYPE-1 Command(seek etc).
                Fix (Add hack) for FUKU*.d77 (えびふく's music disks for FM-7).
  * [VM/MB8877] Fix DRQ/IRQ timing.
  * [VM/MB8877] Make IRQ with disk not inserted or not connected when processing command(s).
  * [VM/MB8877] Make turning ON IRQ and turn OFF DRQ when seek completed.
  * [VM/MB8877] Adjust track parameter at initialize and CMD::RESTORE .
  * [VM/MB8877] OS-9 Level2 for FM77AV40 Works.
  * [VM/HD6844] FM77AV40EX has only one DMA channel.Not multiple channels.
  * [VM/HD6844] HD6844 has only a one interrupt line, not multiple.
  * [VM/HD6844] Rename drq_line to busreq_line.
  * Special thanks to Ryu Takegami, to assist debugging FM-7/77/AV/40/EX .
  * Built with commit 9376345bc1270e2a76db816636b645e744d09e82  or later.

-- Jan 19, 2018 01:49:25 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

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
