** Qt porting for Common Source Code Project **
                                         December 18, 2018
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20181218

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
  Ryu Takegamiさん     : eFM-8/7/77/AV/40/EX のデバッグに協力していただいています。
  はせりんさん         : eFM-8/7/77/AV/40/EX のデバッグに協力していただいています。
  Ootake 開発者の皆さん: ePCENGINEの改善のヒントをソースコードから勉強させていただいてます。

Changes:

* 前の変更点をお読みになる場合には、ChangeLogと000_gitlog.txtをお読み下さい。

* SNAPSHOT December 18, 2018
  * Upstream 2018-12-09 .
  * [VM/PC9801] Add pseudo SASI (HDD) BIOS for PC-9801.WIP.
  * [VM/EVENT] Improve error message at cancel_event().
  * [VM/HUC2680] Improve around timer.Thanks to  Ootake v2.83.
  * [VM/PCENGINE] Improve process around NEC extended command (0xd*).
  * [VM/PCENGINE] PCE_ADPCM: Fix not sound data length >= 65536 bytes.This still don't fix some softwares.i.e. Megami-Paradise.
  * [VM/PCENGINE] Porting from Ootake v2.83.This still WORK-IN-PROGRESS.
  * [VM/PCENGINE] CDROM: Don't reset ADPCM more than once at CDROM makes "NOT BUSY".
  * [VM/PCENGINE] Most of CD-ROM^2 softwares maybe work.Excepts LASERSOFT's products and using "ARCADE card".
  * [VM/SCSI_CDROM] Add write_signal() to control CDDA from MACHINE.
  * [VM/SCSI_CDROM] Fix CD-DA buffer handling.Reading buffer should be per 2352 bytes.
  * [VM/SCSI_CDROM] CUE: More correctness cue parsing.
  * [VM/SCSI_CDROM] CUE: Set default pre-gap to 2Sec (150frames).Fix audio problems of most softwares.Maybe fixed issues on Manhole.
  * [VM/SCSI_CDROM] More correctness SEEK/Interrupt timing.
  * [VM/FM7] DISPLAY: Fix for logging "[EVENT] EVENT: device (name=DISPLAY SUBSYSTEM, id=25) tries to cancel event 6 that is not its own (owned by (name=PRINTER I/F id=20))!!!"
  * [Draw/Qt] OpenGL: Abondon depth test.
  * [UI/Qt] OOPS: Fix OOPs around mounting virtual D88/D77 image(s).
  * [UI/MENU] HARDDISK: OOPS: I missed update directory.
  * [UI/MENU] Fix oops dialog of opening virtual HDD.
  * Built with ee880845ec85aa431df3c7a937611e9c20dd591d (or later).
  
-- December 18, 2018 16:31:55 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

12/9/2018

[VM/SCSI_CDROM] add vendor specific command for NEC CD-ROM^2

[PC8801/PC88] support CD-ROM drive (thanks M88/cdif)
[PC8801/PC88] support Video Art Boad (thanks X88000)


12/5/2018

[VM/MB8877] improve reset() to finish previous command and reset fdc completely
[VM/Z80] add read_signal() to read irq status

[PC8801/PC88] improve to render scan line with black if color graphic mode
[SMC70/MEMORY] support 640x400 and 160x100 graphic mode
[SMC777/MEMORY] fix issue that text blink is not working
[SMC777/MEMORY] improve inport 21h to read vsync irq status
[SMC777/MEMORY] improve inport 51h to read cursor and space keys as joystick #1
[VM/*] improve tape interfaces for each virtual machine


12/4/2018

[CONFIG] remove fmgen_dll_path and add mame2151_dll_path/mame2608_dll_path

[VM/YM2203] remove HAS_YM2608 and YM2203_PORT_MODE to simplify code

[PC8801] support HMB-20 sound board


12/2/2018-2

[PC8801/PC88] fix text/graph renderer again (thanks Mr.Bookworm)


12/2/2018

[PC8801/PC88] fix not to apply reverse attribute to monochrome graphic screen


12/1/2018

[PC8801] enable/disable drawing scan line when monitor type is hireso/standard
[PC8801/PC88] improve text attributes/rederer (thanks ePC-8801MA��)
[PC8801/PC88] fix analog back color
[PC8801/PC88] improve routine to change palette for each scan line


11/28/2018

[WIN32/INPUT] support joystick with multiple axes and hat key
[WIN32/INPUT] improve joystick to keyboard function for numpad key 5


11/27/2018

[WIN32/INPUT] support joystick to keyboard function
[WIN32/SCREEN] support Window x1.5 mode

[PC8801/PC88] support PC key, that is mapped to F11

-----

お楽しみあれ!
-- Ohta.
