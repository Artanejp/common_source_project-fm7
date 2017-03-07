** Qt porting for Common Source Code Project **
                                          Mar 07, 2017
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20170307

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

* SNAPSHOT January 30, 2017
  * Upstream 2016-12-31
  * [VM/SOUND] Fix problems voice with PCM (at least MSX and FM-7(Reviver)).
  
               See http://hanabi.2ch.net/test/read.cgi/i4004/1483504365/37 .
	       
. * [VM/MB8877] Fix accessing 2D disk from 2DD mode drive (not 2D mode).

                Fixed booting issue of REVIVER for FM-7/77/AV with FM77AV40/EX/SX.
		
  * [Draw/Qt] Fix wrong aspect when using variable screen size.i.e.)emufm7/emufm77av40ex.
  * [UI/OpenGL] Add selection of draw infrastracture; OpenGLv3(MAIN) and OpenGLv2 (and OpenGLv3.2(Core): Still not implemented).
  * [Qt/OpenGL] Fix backgrounds not display at one-board-computer drawing with OpenGLv2.
  * [Qt/LOG] Enable syslog *truely*.
  * Built with cef2d4601438fd1fa680485ddba3434690e58451 or newer.

-- Jan 30, 2017 05:04:55 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

本家の変更:

3/4/2017

[WINMAIN] add interfaces for bubble casette (thanks Mr.Artane.)

[EMU] add interfaces for bubble casette (thanks Mr.Artane.)

[VM/*] import Mr.Artane.'s fixes (Commits on Feb 18, 2017)

[VM/AY_3_891X] split YM2203 and AY-3-891X (thanks Mr.Artane.)

[VM/DISK] fix trim buffer routine for 2D disk image (thanks Mr.Artane.)

[VM/EVENT] support light weight sound rendering (thanks Mr.Artane.)

[VM/MB8877] fix seek command for 2D disk image (thanks Mr.Artane.)

[FM77AV] import Mr.Artane.'s fixes (Commits on Feb 18, 2017)


3/1/2017

[EMU/DEBUGGER] support < command (input commands from file)

[X1/MEMORY] support to unmap rom by reading port 1Exxh (thanks Mr.Sato)


2/9/2017

[WINMAIN] support to draw bitmap image onto buttons

[WINMAIN] fix to redraw pcb bitmap image when window is invalidated

[WIN32/SCREEN] fix to invalidate rects for 7-seg leds and buttons

[BABBAGE2ND] add bitmap images for buttons

[EX80] add bitmap images for buttons

[SMB80TE] add bitmap images for buttons

[TK80BS] add bitmap images for buttons

[TK85] support NEC TK-85

[YS6464A] add bitmap images for buttons

12/31/2016

[COLECOVISION/KEYBOARD] fix not to raise irq when joystick is pressed

[COLECOVISION/KEYBOARD] fix joystick/tenkey inputs

[COLECOVISION/KEYBOARD] fix to save/load tenky enabled status

[SC3000/MEMORY] support 32KB+16KB or 128KB ROM carts


12/30/2016

[COLECOVISION] support COLECO ColecoVision (thanks Mr.tanam)

-----


お楽しみあれ!
-- Ohta.
