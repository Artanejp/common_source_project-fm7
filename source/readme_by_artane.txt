** Qt porting for Common Source Code Project **
                                         October 15, 2019
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20191015

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

   j. Windows のビルドを、Docker環境上のLLVM CLANG (9) にしました。例外処理に関して、MinGW-w64のgccは非常に遅い方法を取ってるためです（Borlandが悪いのですが）。
     詳細は、 https://github.com/Artanejp/llvm-mingw と https://hub.docker.com/r/artanejp/llvm-mingw64-ubuntu-cosmic を参照して下さい。
   
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

* SNAPSHOT Oct 15, 2019
  * Upstream 2019-04-30.
  * This is point release, still exists some issues (i.e.EMM386(NEC/PC98) and FreeBSD(98) don't work) for PC-9801 and PC-Engine and some VMs, will fix them.
  * [Tools] Add DUMP LIST CHECKER.
  * [BUILD/Win32] Build with LLVM CLANG (for MinGW-w64).Because GCC for MinGW-w64/Win32 has very slow exception handling (due to Borland's patent). 
  * [BUILD/Win32] See https://github.com/Artanejp/llvm-mingw and https://hub.docker.com/r/artanejp/llvm-mingw64-ubuntu-cosmic for datails.
  * [BUILD/Win32] Build against FFMpeg 4.2.
  * [FM7/SOUND] Fix reading value of PSG register(s).Fix sound of FM-7's demonstration with FM-7/77 (not with 77AV variants).
  * [FM7/SOUND] Separate reset sequence for OPN/WHG/THG/PSG to a common function.
  * [VM/FM7] Replace printf debug messages to out_debug_log().
  * [VM/FAMILYBASIC] WIP: Fix wrong string for romaji-kana (and auto key).Still imcoplete, implementing DAKUION,will fix.
  * [VM/PC9801] CPUREG: V30 SUB CPU works.
  * [VM/PC9801] Re-define DIPSW, to work with V30@PC-9801RA.
  * [VM/PC8801] Fix double install DEBUGGER:: for OPN#1,#2.Thanks to https://matsuri.5ch.net/test/read.cgi/i4004/1526806551/598 .
  * [VM/MC6809] Fix duplicate signal; SIG_CPU_HALT.
  * [VM/DEBUGGER] Fix FTBFSs with LLVM CLANG.
  * [VM/AY_3_891X] Fix pop noise when enabling lpf or hpf.
  * [VM/Z80DMA] OOPS: Disable debug spam.
  * [VM/Ix86] More correctness wait.
  * [VM/Ix86] Implement wait by memory-wait-factor.
  * [VM/Ix86] Add SIG_CPU_HALTREQ.
  * [VM/I386][VM/V30] Fix cloick handling when BUSREQ/HALT.
  * [VM/I8259] PC9801: Fix crash when changing V30 Sub CPU <-> i286/i386 Main CPU.
  * [VM/EVENT] Update comment of scheduler (EVENT::drive()).
  * [EMU/ROMAJI_KANA] Some characters are enabled to input via ROMAJI-KANA conversion.
  * [EMU/AUTOKEY]  Some characters are enabled to input via pasting from clipboard.This using UCS-4(aka UTF-32) internal format.
  * [Qt/OpenGL] Don't makeCurrent()/doneCurrent() inside of resizeGL().Fixed crash running within Docker container.
  * [UI/Qt] ROMAJI_KANA: Some characters input from KANA mode (by host) are enabled.
  * [UI/Qt] VM: Add keycode - vk - name table listing features.See vm/fm7/keyboard.cpp and qt/osd_base.h and gui/dialog_set_key.cpp.
  * [Qt/LOGGER] Threaded console logging.
  * [Qt/AVIO] Update FFMPEG's API: Revoke to use functions marked as deprecate.
  * [Qt/AVIO] Drop to use deprecated functions for FFMpeg 4.x.
  * [Qt/AVIO] Add some codec entries (still not implement).
  * [OSD/Qt] Update sound APIs: for sound sampling.Still be imcomplete.Will implement.
  * [OSD/SOUND] Simplify sound callback for SDL.
  * [OSD/SOUND] SDL: Convert sound rate/bitwidth.
  * [OSD/General] Fix not reply version of libvCSPosd.Display OSD version at "about this software".
  * [QT/MOVIE_LOADER] Fix weird initilaizing memory.
  * [Qt/MOVIE_SAVER] CONFIG: Some functions make dllexport.
  * [Qt/HOMEBREW] Fix not detected SDL at configuration of Qt::Gamepad.
  * [Qt/JOY_THREAD] Fix memory leak when plug/unplug joystick.
  * [UI/Qt] Add font selection to debugger and log view dialogs.
  * [UI/Qt] AUTO_KEY: Copy and paste: Paste even Zenkaku KATAKANA/HIRAGNA/ASCII to VM (converted to Hankaku letters).
  * [Qt/CONFIG] Keep font metrics and window size of debugger and log viewer.Save to foo.ini file.
  * [Qt/OpenGL] GLES: Fix texture format for float/half float.May work "TV RENDERER" with Win32 build (via ANGLE).
  * [Qt/OpenGL] Use half float texture for float textures to reduce GPU usage.
  * Built with 74c7914381802640510c76f176b3c3ffeceb678d (or later).

-- Oct 15, 2019 02:40:49 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

4/30/2019

[VM/DEVICE] add is_primary_cpu() and update_extra_event()
[VM/EVENT] support to udpate event while cpu is running one opecode
[VM/I8259] fix reading isr register (thanks Mr.rednow)
[VM/SCSI_HOST] fix to raise irq at command/message phase
[VM/Z80] improve to update event in every read/write cycle

[CEFUCOM21] support Hino Electronics CEFUCOM-21 (not work)
[MZ2500/CRTC] apply crtc patch (thanks Mr.Koucha-Youkan)
[PC8801MA] improve to enable/disable cmdsing and pcg
[PC8801MA] improve to enable/disable changing palette for each scan line

-----

お楽しみあれ!
-- Ohta.
