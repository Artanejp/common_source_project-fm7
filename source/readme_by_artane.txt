** Qt porting for Common Source Code Project **
                                         August 07, 2018
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20180807

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
   
   g. ffmpegは、Windowsに関してはバンドルしてありますので、動かない時はインストールしてみてください。
      
   h. Qt5.5(Ubuntu 16.04LTS向け)もしくはQt5.10(Win32とDebian GNU/Linux sid向け)でビルドしてあります。
   
   i. 表示基盤のデフォルトが、OpenGL ES2.0になりました。コマンドラインオプション --opengl で変更が可能です(--helpで参照)
   
   * Windows もしくは GNU/Linux のcross tool chain (要Wine)で、MinGW (gcc6) と Qt 5.10 でのビルドができることを確認しました。     
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

* 前の変更点をお読みになる場合には、ChangeLogと000_gitlog.txtをお読み下さい。

* SNAPSHOT August 07, 2018
  * Upstream 2018-05-24 .
  * [VM] Use VM_TEMPLATE, META-Class of VM::
  * [VM/FM7] Clear OPN/WHG/THG interrupt on resetting. Fix freeze when resetting after using RELICS.
  * [VM/FM7] DISPLAY: Not use CLR_HACK for MC6809 and delay using CLR to set busy flag.May be working MAGUS.Thanks to Anna_Wu.
  * [VM/MC6809] Remove dirty CLR insn hack.
  * [VM/DEVICE] Add DEVICE:: to libCSPcommon_vm.
  * [COMMON/Qt] Fix BYTEORDER of PIXEL@RGB888.
  * [UI/Qt][BUG] Fix crash with using LaserDisc(s).
  * [Qt/OpenGL] Initial support of OpenGL 4.3 Core profile.
  * [Qt/OpenGL] OpenGL 4.3 : Upload SCREEN using glMapBuffer().
  * [Qt/OpenGL] Use template class instead of GL2.0.
  * [UI/Qt] Separate some command line parsing to sub-routines.
  * [BUILD/WIN32] Update toolchain to Qt5.11.1.
  * [BUILD] Set SOVERSION and GIT hash automatically.
  * Built with df3888ca1199c6f75c1500dbb479d152ba6e2ad0 (or later).

-- August 07, 2018 03:58:14 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

5/24/2018

[COMMON] import Mr.Artane.'s fixes (Commits on May 10, 2018)
[COMMON/FIFO] import Mr.Artane.'s fixes (Commits on May 10, 2018)
[EMU] support set_vm_screen_lines() (thanks Mr.Artane.)
[EMU] fix interfaces for bubble cassette
[EMU] add interfaces for hard disk
[WINMAIN] add interfaces for hard disk

[VM/DEVICE] support read_debug_reg()
[VM/DISK] import Mr.Artane.'s fixes (Commits on May 10, 2018)
[VM/HARDDISK] support hard disk handler
[VM/I286] improve i286 core based on MAME 0.197
[VM/I286] support read_debug_reg()
[VM/I386] support read_debug_reg()
[VM/I8237] fix verify command
[VM/I8237] support to read bank register by read_signal()
[VM/MC6809] import Mr.Artane.'s fixes (Commits on May 10, 2018)
[VM/SCSI_DEV] support SASI specify command
[VM/SCSI_HDD] improve to use hard disk handler
[VM/SCSI_HOST] support to read ack signal

[BUBCOM80] support Systems Formulate BUBCOM80
[FM77AV] import Mr.Artane.'s fixes (Commits on May 10, 2018)
[FMR30] support to change hard disk image
[FMR50] support to change hard disk image
[FMR50/BIOS] improve to use hard disk handler
[MZ2500] support to select floppy drive type 2DD/2D
[MZ2500] support to change hard disk image
[MZ2500/MZ1E30] reimplent SASI I/F with general SCSI host/hard disk device
[PC9801/CPUREG] support NMI enable/disable
[PC9801/MEMBUS] improve for 24bit/32bit address
[PC9801/SASI] support SASI I/F and hard disk drives
[X1TURBO/SASI] support SASI I/F and hard disk drives

-----

お楽しみあれ!
-- Ohta.
