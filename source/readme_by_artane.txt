** Qt porting for Common Source Code Project **
                                         March 03, 2020
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20200302

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

* SNAPSHOT March 03, 2020
 * Upstream 2020-02-21.
 * [VM/FMTOWNS] Work initially.
                See STATUS section for doc/VMs/fmtowns.txt and
		000_gitlog.txt .
 * [VM/I386_NP21] Merge Upstream 2020-02-21 's I386::/NP21.Belows are differ from upstream:
      - Implement memory wait to change CPU speed.
      - Implement extra reset wire to notify CPU reset.
      - Some headers are changed due to cause FTBFS with GCC.
      - Character encoding chenged to UTF-8 at most of source files(not all?)
 * [VM/I386] IMPORTANT: libcpu_newdev/i386 has removed.I386:: porting from NP21 seems to be working nice, no need to porting from MAME/C++.
 * [VM/HARDDISK] Calculate correctness C/H/S of HDD.
 * [VM/SCSI_HDD][WIP] Implement RECALIBRATE SCSI command.
 * [VM/SCSI_HDD][VM/SCSI_DEV] Implement some command.But still not active.
 * [VM/BMJr] Fix Break sequence. Thanks to https://matsuri.5ch.net/test/read.cgi/i4004/1526806551/648 .
 * [VM/BMJr] Fix reset-key (EIKIGOU + BREAK) sequence.
 * [VM/BMJr] Alse assume Back Space key to DELETE key.
 * [VM/BMJr] Also assume Esc key to BREAK key.
 * [VM/FMGEN] Initial implemantation of YM2612 (OPN2).
 * [VM/UPD71071] Make some functions make virtual to prepare overwrap by TOWNS_DMAC.
 * [VM/DEVICE] Add update_signal_mask() to modify signal mask for SIG_SCSI_DAT for SCSI/SASI devices.
 * [VM/COMMON_VM] Include SCSI devices to libCSPcommon_vm, excepts SCSI_HOST::.
 * [VM/PCENGINE] ADPCM: Fix em-bugged freeze ADPCM DMA after CDC STATUS (write to I/O 1800h.)
 * [BUILD/Windows] LLVM: Update Qt version to 5.14.
 * [VM/PC9801] DISPLAY: Re-Backport from Upstream 2020-02-01.Kakinoki Syougi works fine.
 * [VM/SCSI_CDROM] Fix freeze some PC-Engine's CD-ROM^2 games and SCSI HDD for FM-Towns.
 * [Qt/OpenGL_ES] Win32: Fix shader compilation errors with Angle Project.
 * [Qt/OpenGL] Correctness texture magnitude calculating.
 * [UI/Qt] Win32: Fix closing D77/D88 image when select another slot.
 * [UI/Qt] Harddisk: Add *.h0-*.h9 , they are Unz (Towns emulator)'s virtual harddisk images.
 * [OSD/SOUND] Fix crash when effective sound sink don't exists.
 * [Qt/OpenGL] Fix FTBFS if don't have libglu.
 * [UI/Qt] Add "USE_CUSTOM_SCREEN_ZOOM_FACTOR" flag to fooVM.h.
 * [UI/Qt] Try: Make GUI core (QApplication -> QCoreApplication) to be non-Global.
   Thanks to https://matsuri.5ch.net/test/read.cgi/i4004/1526806551/719 .
 * Built with 177db8ccb3765bf7f49ef3d9f25738bb15348e2b (or later).

-- Mar 03, 2020 01:44:00 +0900 K.Ohta <whatisthis.sowhat@gmail.com>


本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

2/21/2020

[VM/I386_NP21] update to Neko Project 21/W ver0.86 rev71 beta4

[PC9801VX] support to switch cpu mode to V30

[PC9801RA] support to switch cpu mode to V30


2/17/2020

[EMU] add is_floppy_disk_connected() and is_quick_disk_connected()

[WINMAIN] disable floppy/quick disk drive menus when drives are disconnected

[VM/I8080] improve disassembler to distinguish 8080 and 8085

[VM/I86] split i86/i88/i186/v30 from I286 class

[VM/I86] fix aam in v30

[VM/I86] support 8080 emulation mode in V30

[VM/I386_NP21] improve not to explicitly accept irq while executing opecode

[VM/I386_DASM] split i386 disassembler from I286/I386 class

[VM/V30_DASM] split v30 disassembler from I286 class

[VM/V30_DASM] add 8080 disassebler for 8080 emulation mode

[VM/VM_TEMPLATE] add is_floppy_disk_connected() and is_quick_disk_connected()

[PC9801] support to enable/disable 1MB/640K/320KB-FDD interfaces


2/1/2020

[EMU] support to create blank hard disk image (*.hdi/*.nhd)

[WINMAIN] add menu items to mount blank hard disk image

[VM/DEVICE] improve memory/io bus interfaces for 16/32bit access

[VM/DEVICE] add get_event_clocks() and get_cpu_clocks()

[VM/EVENT] add get_event_clocks() and get_cpu_clocks()

[VM/I386_NP21] support 80386 (based on Neko Project 21/W i386 core)

[VM/I8259] make update_intr() public

[VM/MEMORY] improve memory bus interfaces for 16/32bit access

[VM/MEMORY] make rd_table/wr_table/addr_shift public

[VM/UPD7220] fix stop command (thanks Neko Project 21/W)

[FMR50] change i386 core from MAME to Neko Project 21/W

[FMR50] support to mount blank hard disk image

[MZ2500] support to mount blank hard disk image

[MZ2800] support to mount blank hard disk image

[PC9801] change i386 core from MAME to Neko Project 21/W

[PC9801] support to mount blank hard disk image

[PC9801/DISPLAY] improve code for big-endian host machine

[PC9801/FMSOUND] support 86-type PCM (thanks Neko Project 21/W)

[PC9801/MEMBUS] improve memory bus interfaces for 16/32bit access

[PC9801/MOUSE] fix irq number in hireso mode

[PC9801/SASI] improve irq/drq signals to generate from sasi bus signals

[X1TURBO] support to mount blank hard disk image

[X1TURBO/IOBUS] fix not to clear vram in reset()


1/23/2020

[VM/Z80DMA] fix byte counter read by read mask follows command (thanks Mr.Sato)

-----

お楽しみあれ!
-- Ohta.
