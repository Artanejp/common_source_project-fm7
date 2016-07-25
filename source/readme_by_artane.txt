** Qt porting for Common Source Code Project **
                                          July 25, 2016
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20160621

   追加情報:
    　各機種バイナリーは、osdn.jp　もしくはミラーサイトより入手可能です。
   　https://osdn.jp/projects/csp-qt/  
     https://osdn.jp/projects/csp-qt/releases/　をチェックしてください。

　   Win32: 
  　 GNU/Linux(amd64) : 


1. 背景
   CSPは、非常に優れた構造のエミュレータです（しかし、些か重くてコンパイラ
   がいい最適化をしないと重めですが）。
   しかし、このコードはM$ Visual C++依存の部分が非常に多いです。
   そこで、GNU/Linuxでこれを動かすためにQtに色々と移植していきましょう。
   と言う感じで作業をはじめました。

2. 最低限必要なもの(Qt版)
   a. Qt5 ツールキット
   b. OpenGL, 多分、最低OpenGL 3.0は必要です。 (New!)
   c. gcc / g++ (4.7以降？)もしくは llvm clang / clang++ (3.5以降?)
      コンパイラツールチェーン。
   d. SDL2 (SDL 1.xではないので注意)
   e. CMake 2.8以降。
   f. ffmpegから、libavとlibswが必要です。 http://ffmpeg.org/ より。
   
   * Windows もしくは GNU/Linux のcross tool chain (要Wine)で、
     MinGW と Qt 5.5.1 でのビルドができることを確認しました。
     
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
   ・R@Mを $HOME/emu{Machine Name}/ に配置してください。(Windowsの場合は今の所 .\emu{Machine Name}\)
   　なお、このディレクトリは最初起動した後で作成されます。
   ・設定ファイルは、$HOME/.config/emu{Machine Name}/ に書き込まれます。(Windowsの場合は今の所 .\.config\emu{Machine Name}\)
   ・ステートセーブファイルは、$HOME/emu{Machine Name}/{Machine Name}.sta に書き込まれます。
   ・キーコード変換テーブルファイルが、$HOME/.config/emu{Machine Name}/scancode.cfg に書き込まれます。
     書式は、カンマで区切られた16進データです(10進ではないので注意) .
     1カラム目はM$ ヴァーチャルキーコード。
     2カラム目はQtネィティブのスキャンキーコードです。
   ・UI部分の共通コンポーネント (src/qt/gui) を共有ライブラリlibCSPgui.soにまとめました。
   ・インストール用のBASHスクリプトを用意しました。src/tool/installer_unix.shです。
   
     
5. 移植状況
   a. 現在、Debian GNU/Linux "sid" の AMD64版でしかテストしていません。
   　 が、多分他のGNU/Linux OSやBSD系のOS (Mac含む) でもビルドすれば
   　 動くでしょう。
      Windows もしくは GNU/Linux(要Wineとbinfmt-support)上でのMinGWと
      Qt community edition でのビルドが通るようになりました。
      安定したWindowsビルドを必要な方は、Visual Studio 2013 か 2015 のCommunity Edition
      でビルドしてください。（もう少ししたら、MinGWに切り替えようとは思ってます。)
      
   b. 今は、Qtの開発側が「Qt4おわりね」とアナウンスしたので、Qt4ではなく
      Qt5を使っています。
      添付してあるバイナリは、Qt 5.5でビルドしました(が、Qt 5.1以降なら動くはずです)。

   c.上流の2016-04-17現在でのPX7以外の全ての仮想マシンがQtに移植出来ています。
   d. Linux用ビルドでは、GCC 6をリンク時最適化(LTO)モードで使っています。
   
6. Upstream repositry:
      https://github.com/Artanejp/common_source_project-fm7
      https://www.pikacode.com/Artanejp/common_source_project-fm7/

7. Project Page:
      https://osdn.jp/projects/csp-qt/

8. Upstream (Takeda Toshiyaさんのオリジナル) 
      http://homepage3.nifty.com/takeda-toshiya/

Changes:

* SNAPSHOT July 25, 2016
  * Upstream 2016-04-13
  * Bugfix only.
  * [Win32] Build with Qt5.7.0 and GCC-6.1 (cross).
  * [MOVIE_SAVER/OSD] Fix wrong framerate when recording ; this is related by frame skipping. Maybe fixed (3) of http://hanabi.2ch.net/test/read.cgi/i4004/1430836648/705
  * [MOVIE_SAVER/Qt] Fix duplicate opening before closing. Maybe fixed (5) of http://hanabi.2ch.net/test/read.cgi/i4004/1430836648/708 .
  * Build with 8265a0a859ac69ef7c17548851902eab2de6f7f4 (or later).
  
-- Jul 25, 2016 21:59:41 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT June 29, 2016
  * Upstream 2016-04-13
  * Build with FFMPEG 3.0.2 supported libmp3lame.
  * [MOVIE_SAVER/Win32] Fix wrong value of left frames for audio when stopping to record.
  * [OSD/Qt] Fix wrong FPS with not drawing.
  * Build with 3752c12b4b08f9910f3e3b6ad6f7dfbd76342cbb (or later).
  
-- Jun 29, 2016 13:05:15 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT June 21, 2016
  * Upstream 2016-04-13
  * Build with FFMPEG 3.0.2 supported libmp3lame.
  * [Linux] Build with qt.io's official Qt 5.3.1 expect to work with 
    Ubuntu 16.04 LTS and Debian 8 "Jessie".
  * [MOVIE_SAVER] (Maybe mostly?) Fix asynchronous both video and audio.
  * [MOVIE_SAVER] Add libmp3lame and vorbis (disable from UI) for audio codec.
  * [Qt] Not linking config and using_flags directly from libCSPgui , libCSPavio and libCSPosd .
  * [MOVIE_SAVER][EMU] Fix stop/restart recording movie when changing/ejecting CARTRIDGEs.
                       Now, don't stop when changing/eject cartridge.
    See, http://hanabi.2ch.net/test/read.cgi/i4004/1430836648/699 .
  * [MOVIE_SAVER] Re-enable (and selectable) H.264 for video codec.
  * Build with 85c331b7635ca713e819218b86d65a877b7478f3 (or later).
  
-- Jun 21, 2016 01:07:44 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT June 18, 2016
  * Upstream 2016-04-13 .
  * Build with FFMPEG 3.0.2 .
  * [MOVIE_SAVER] More safer open/close.
  * [MOVIE_SAVER] Fix deadlock when closing movie.
  * [MOVIE_SAVER] Selectable both MPEG4v1 and H.264(libx264) for video codec.
  * [MOVIE_SAVER][Qt] Temporally set video-codec to MPEG4 (not AVC).
    This is issue of discard frames with libx264 by ffmpeg's libavcodec/libavformat.
    This seems to be ffmpeg's issue, not my program.
  * Build with 629f7d70816c04b38c3ab8cc277147a6bd2c2d2a (or later).
  
-- Jun 18, 2016 03:32:06 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT June 11, 2016
  * Upstream 2016-04-13 .
  * Use osdn.jp to distibute binaries.
    https://osdn.jp/projects/csp-qt .
  * [MOVIE_SAVER] Use ffmpeg internal aac codec, because faac or fdk_aac are not *Free* .
  * [VM/MOVIE_SAVER] Fix stopping when changing cartridge, now refer to upstream (split movies).
  * [Qt/Win32] Fix not work with WindowsXP, using homebrew version of ffmpeg-2.8.7.
  * Build with 89d31ce8daa733ea4a0c38f0a1890d3a0fcfce38 (or later).
  
-- Jun 11, 2016 05:09:55 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT June 09, 2016
  * Upstream 2016-04-13 .
  * Now, all of binaries are built with ffmpeg-2.8.7 at GNU/Linux,  with ffmpeg-3.0 at Windows.
  * Please read README.ffmpeg.txt .
  * [FM7/Disk] Add exceptions for Xanadu Scenario 2, this has not booted with changes 49dceaca9401d3c6037cb51ec013ca032ff0e83c .
  * Build with 64df71cd492be91289f883224640f42cace090ed (or later).

-- Jun 09, 2016 05:50:50 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT June 08, 2016
  * Upstream 2016-04-13 .
  * Add movie saver, using libav with x264.
  * Now, all of binaries are built with ffmpeg-3.0.
  * Build with 2142d5c7426e21cfeedbaea0450f238f8b4d7d38 (or later).

-- Jun 08, 2016 07:34:45 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT May 23-2, 2016
  * Win32 binary only RELEASE
  * Fix unable to run with some natibve Windows environment.
  * Build with c1448dc84f5439c7c8931614a8397dbefb6383da .

-- May 24, 2016 22:42:00 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT May 23, 2016
  * Upstream 2016-04-13 .
  * [Win32] Now built with -msse -msse2 . You can run only later than Pentium4 .
  * Move FILEIO:: FIFO:: COMMON:: to libCSPemu_utils .
  * Make OSD:: to inherited by OSD_BASE:: .
  * [Win32] Disable LINKFLAGS with "-static-libgcc -static-libstdc++" to reduce size of executions, now, bundled libstc++ is for gcc-5 .
  * [Win32/Build] Support build with MSYS2 (but not display because MSYS2's Qt was build without OpenGL).
  * [Linux] Build shared libraries with Link-Time-Optimize, reduce size of libs.
  * [X1/VM] Configurable buttons for X1/Turbo/Turbo Z.
  * [FM7/FDC] Set MB8877_NO_BUSY_AFTER_SEEK. Fixed unable to boot OS0. Thanks to Anna_Wu.
  * [FM77AV/MB61VH010] ALULINE: Reduce CPU usage (at delta X > delta Y).
  * [UI/Qt/DRAW] Fix crash sometimes when exit emulator.
  * Build with 1c1ddc85dfa7456b1ce48662c2e2930dcc4fc9d8 (or later).

-- May 23, 2016 02:22:07 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

Upstream changes:
-----
4/13/2016

[WINMAIN] improve auto key for the case to switch upper/lowercase with capslock
[EMU/DEBUGGER] fix issue that u command may cause the infinite loop

[VM/DATAREC] support to detect the frequency of signal
[VM/DATAREC] fix to always adjust zero position of wave signal
[VM/UPD1990A] fix not to check clk signal is low when stb signal is raised
[VM/UPD7810] support debugger
[VM/UPD7810] support MOV A,S opecode
[VM/UPD7810] fix not to change V register (thanks PockEmul)

[PC2001] support NEC PC-2001
[PC2001] fix cpu clock
[PC2001] support beep sound
[PC2001/IO] support rtc control signals
[YALKY] support Yuasa Kyouiku System YALKY
[YALKY/IO] improve data recorder controller

-----


お楽しみあれ!
-- Ohta.
