** Qt porting for Common Source Code Project **
                                         August 11, 2017
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20170811

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
* SNAPSHOT Aug 11, 2017
  * Upstream 2017-08-10.
  * [EMU/ROMAKANA] Fix not convert with Qt.Use functions within EMU:: , not use original ROMAKANA functions.
  * [VM] Add PC-9801RA and PC-9801VX.
  * [VM] Add devices to libCSPcommon_vm mostly.
  * [VM] Fix FTBFS and bugs a lot.
  * [VM/FM7] Stop using DUMMYDEVICE:: . Use VM::get_extra_leds() to get led status.
  * [VM/MB8877] DISK:Fix not apply workaround to Gambler Jiko Chusin-ha for FM-7 series.
  * [VM/DATAREC] Fix crash with MZT data.
  * [Qt/UI] OpenGL: Display ICONs when accessing to virtual medias.
  * [Qt/UI]  Separate status bar display:Accessing to virtual medias.
  * [Qt/UI] Menu: Make macro to be easier constructing.
  * [Qt/UI] Ui_MainWindowBase:: Make private variables/functions not accessed from Ui_MainWindow:: .
  * [Qt/UI] Display LEDs with USE_EXTRA_LEDS or USE_KEY_LOCKED.See common/emu_thread.cpp for details.
  * [Qt/UI] Fix some memory leaks.
  * [Qt/OpenGL] Fix aspect ratio with some zoom type.
  * [Qt/FM7] Fix wrong scaling at VMs (only FM-8/7/77/AV) has only 200line (not have 400line). 
  * [BUILD/CMake] CCACHE: Fix SEGFAULT at linking after upgrade GNU toolchain.
  * Build with 6641d48a10c3d3b864f3b11f35a7e2ba07cef55d (or later).

-- Aug 11, 2017 19:40:03 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

本家の変更:

-----
8/10/2017

[EMU] support to enter 0-9 by numpad key while roman to kana is enabled
[EMU] support to enter shift + function key while roman to kana is enabled
[EMU/DEBUGGER] fix to wait until cpu is suspended and enters into waiting loop

[VM/I386] fix to flush vtlb after modifying address mask
[VM/UPD7220] support to specify device class to access vram
[VM/Z80] fix to run dma before checking interrupts

[PC8801/PC88] fix monocolor graph screen to get color index from text attribute
[PC8801/PC88] fix monocolor graph screen to render with graph palette

[PC9801VX] support NEC PC-9801VX
[PC9801/CPUREG] support address mask i/o
[PC9801/DISPLAY] support EGC (thanks Neko Project 2)
[PC9801/DISPLAY] support EGC/GRCG access from graphic GDC
[PC9801/MEMBUS] support 24bit/32bit address memory bus
[PC9801/MEMBUS] support ITF rom


6/22/2017

[EMU] move auto key codes from winmain to emu class
[EMU] support to convert roman letters to kana letters
[WINMAIN] support APPLICATION accelerator to enable/disable roman to kana
[WINMAIN] support CTRL+ALT+ENTER accelerator to enable/disable full speed

[VM/I386] improve i86/i286 core based on MAME 0.185
[VM/UPD765A] fix device status (thanks annonymus guy)
[VM/YM2203] fix to mask YM2608 ports in YM2203 case (thanks annonymus guy)

[PC8801/PC88] fix bank switch of extend ram (thanks annonymus guy)
[PC8801/PC88] fix crtc for dma underrun case (thanks annonymus guy)


5/28/2017

[WINMAIN] support to run simulation at full speed

[VM/I386] improve i386 core based on MAME 0.185


5/20/2017

[YIS/DISPLAY] support correct font rom (thanks Mr.Moriya)
[YIS/DISPLAY] support native graphic commands
[YIS/DISPLAY] include KST32B stroke font and its decoder (thanks Mr.Saka.N)

[X1TURBOZ/DISPLAY] fix 8 color mode with 4096 palette (thanks Mr.Sato)


5/17/2017

[X1TURBOZ/DISPLAY] fix 64/4096 color mode (thanks Mr.Sato)

-----


お楽しみあれ!
-- Ohta.
