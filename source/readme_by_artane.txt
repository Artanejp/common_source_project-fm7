** Qt porting for Common Source Code Project **
                                         December 04, 2017
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20171204

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

Changes:

ChangeLog:
* 前の変更点をお読みになる場合には、ChangeLogをお読み下さい。

* SNAPSHOT Dec 04, 2017
  * Upstream 2017-12-03.
  * [VM/MC6809] Fixing hangup with F-BASIC v3.0ε.Thanks to Ryu Takegami-San. 
                (see https://matsuri.5ch.net/test/read.cgi/i4004/1483504365/641-645)
  * [VM/FM7] DISPLAY: Implement software scan line(s).
  * [VM/FM77AV] VRAM: More use SIMD to be faster.
    [VM/FM7] Try: Add suuporting for OPpenMP.But be slower than not using OpenMP (/_;)
  * Build with commit 9eb246b375699752a898d8be79a227f58e473d8e (or later).

-- Dec 04, 2017 19:29:05 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

12/3/2017

[VM/UPD765A] fix read diagnostic to set ND when 1st sector's id are not match


11/28/2017

[EMU/DEBUGGER] fix ut command to show correct range of cpu trace

[VM/DEVICE] rename bios_call_i86() to bios_call_far_i86()
[VM/HUC6280] support to show total cpu clocks in debugger
[VM/HUC6280] support cpu trace
[VM/I286] support to show total cpu clocks in debugger
[VM/I286] support cpu trace
[VM/I386] support to show total cpu clocks in debugger
[VM/I386] support cpu trace
[VM/M6502] support to show total cpu clocks in debugger
[VM/M6502] support cpu trace
[VM/MC6800] support to show total cpu clocks in debugger
[VM/MC6800] support cpu trace
[VM/MC6800] fix issue that can not break at instruction following tap/cli/sei
[VM/MC6809] support to show total cpu clocks in debugger
[VM/MC6809] support cpu trace
[VM/MCS48] fix to add clocks for interrupt to total cpu clocks
[VM/TMS9995] support to show total cpu clocks in debugger
[VM/TMS9995] support cpu trace
[VM/UPD7801] support to show total cpu clocks in debugger
[VM/UPD7801] support cpu trace
[VM/UPD7810] support to show total cpu clocks in debugger
[VM/UPD7810] support cpu trace
[VM/Z80] fix to add clocks for interrupt to total cpu clocks


11/26/2017

[PC8801MA] fix text attribute when dma underrun occurs


11/25/2017

[EMU/DEBUGGER] support command history
[EMU/DEBUGGER] support ut command (unassemble cpu trace)
[EMU/DEBUGGER] fix < command to check if command file is correctly opened

[EMU/OSD] fix initialize_screen_buffer() for RGB565 (thanks PC8801MA��)
[EMU/OSD] improve read_console_input() to get cursor key

[VM/MCS48] support to show total cpu clocks in debugger
[VM/MCS48] support cpu trace
[VM/UPD7220] improve vsync/hsync timing
[VM/UPD7220] improve status register to select vblank/hblank by sync command
[VM/YM2151] improve not to load mamefm.dll again (thanks PC8801MA��)
[VM/YM2203] improve not to load mamefm.dll again (thanks PC8801MA��)
[VM/Z80] support to show total cpu clocks in debugger
[VM/Z80] support cpu trace

[PC8801MA] support OPN+OPN, OPNA+OPNA, and OPNA+OPN (thanks PC8801MA��)
[PC8801MA] fix back space key issue (thanks PC8801MA��)
[PC8801MA] fix bank switch of extrom (thanks PC8801MA��)
[PC8801MA] fix not to apply digital palette in monochrome and digital mode
[PC8801MA] fix to apply analog palette in monochrome and analog mode
[PC8801MA] fix to apply reverse in text attribute to monochrome graphic
[PC8801MA] fix cpu mode "Z80 8MHz (FE2/MC)"

[X1/KEYBOAD] support phantom keys (thanks Mr.Sato)
[X1/SUB] fix not to check iei status when sub cpu sends ack (thanks Mr.Sato)

-----

お楽しみあれ!
-- Ohta.
