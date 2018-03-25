** Qt porting for Common Source Code Project **
                                         March 26, 2018
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20180326

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
   
   
   * キーコード変換テーブルファイルが、$HOME/.config/CommonSourceCodeProject/emu{Machine Name}/scancode.cfg に書き込まれます。
   
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

* SNAPSHOT Mar 26, 2018
  * [General/Qt] Add some command line options.
  * [COMMON/FM7] Add __DECL_VECTORIZE_LOOP decl. to indicate expect to use vectorize (a.k.a. SIMD instructions).
  * [VM/MB8877] Fix verify timing on SEEK command.Fix not booting Sylpheed for FM77AV.
  * [VM/MC6809] Fix clock using.
  * [VM/EVENT] Specify CPU per VM.
  * [VM/FM7] Use template and static_cast<T *> to expect to be faster.
  * [VM/FM7] Add Green display for FM-7/8/77 .
  * [VM/FM77L4] Add FM77L4.Maybe 400lines board still not working.
  * [Qt/SCREEN] Add turning on/off virtual media Icons on screen (OSD).
  * [MOVIE_LOADER] Fix scaling factor.Displaying video as correct width and height.
  * [MOVIE_LOADER] Fix hang up at end of video.
  * Built with f8f16ac6f19fe2dcab250ad50d96cf0b30c8903e or later.

-- Mar 26, 2018 01:34:20 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

3/1/2018

[PC98RL] support NEC PC-98RL
[PC9801/DISPLAY] update for hireso mode
[PC9801/DISPLAY] fix for the case gdc scroll parameters are invalid
[PC9801/MEMBUS] update for hireso mode
[PC9801/MEMBUS] support outport 053Dh
[PC9801/MEMBUS] move memory map routine from VM class to MEMBUS class


2/28/2018

[VM/I286] fix not to clear cycles in reset()
[VM/I386] fix not to clear cycles in reset()
[VM/I386] improve mov_r16_rm16 instruction to check limit
[VM/I386] fix debugger

[PC9801RA] support NEC PC-9801RA


2/27/2018

[VM/I8237] fix bank register and inc mask register

[PC9801/DISPLAY] fix array length of analog palette
[PC9801/MEMBUS] improve memory bus for i386 or later (partial)


2/25/2018

[VM/DISK] improve for case 2D/2DD disk is inserted to 2DD/2D drive

[FMR30/FLOPPY] support to change drive type 2DD/2HD
[FMR30/FLOPPY] support to get media type 2D/2DD/2HD
[FMR50/BIOS] suppoert int 93h, ah=00h/01h to set/get drive type
[FMR50/BIOS] improve int 93h, ah=02h to get sector size and media type
[FMR50/FLOPPY] support to change drive type 2DD/2HD
[MZ80A] support to select floppy drive type 2D/2DD
[MZ80B] support to select floppy drive type 2D/2DD
[MZ80B] support to select cpu clock 4MHz/6MHz
[MZ800] support to select floppy drive type 2D/2DD
[MZ1500] support to select floppy drive type 2D/2DD
[MZ2200] support to select floppy drive type 2D/2DD
[MZ2200] support to select cpu clock 4MHz/6MHz
[MZ2800/FLOPPY] support to change drive type 2DD/2HD
[PC100] support to select floppy drive type 2D/2DD
[PC100/IOCTRL] improve dipswitch value for floppy drive type 2D/2DD
[X1TURBO] support to select floppy drive type 2D/2DD/2HD
[X1TURBO/FLOPPY] support to change drive type 2D/2DD/2HD


2/23/2018

[VM/DISK] support two side
[VM/I8237] support address mask
[VM/I8237] fix interface to connect tc signal for ch.2-4
[VM/IO] support to create multiple instances with different address range
[VM/MC6840] fix issue for the case address range is not 0-7
[VM/MEMORY] support to create multiple instances with different address ranges
[VM/UPD765A] fix st3 in sence devstat command

[FM16BETA] support FUJITSU FM16beta (not work)
[FMR50/MEMORY] fix memset issue
[PC9801] fix to connect terminal count signal from dmac to fdc

-----

お楽しみあれ!
-- Ohta.
