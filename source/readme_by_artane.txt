** Qt porting for Common Source Code Project **
                                         December 16, 2020
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20201216

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
  Soji Yamakawaさん    : eFM-Townsの開発で「津軽」を参考にさせていただいたり、
                         アドバイスを頂いたりしています。
Changes:

* 前の変更点をお読みになる場合には、ChangeLogと000_gitlog.txtをお読み下さい。

* SNAPSHOT December 16, 2020
  * Upstream 2020-08-16.
  * Important: Build system moved to CMake *perfectly*.See INSTALL.md or INSTALL.en.md.
  * eFM-Towns: Works more softwares.See 00_status_ja.md .
  * [Qt/JOYSTICK] Add mapping gamecontroller settings.
	Read from $CONFIG_DIR/joydb.ini as SDL_GAMECONTROLLER format.
	See https://wiki.libsdl.org/SDL_GameControllerAddMapping .
  * [VM/I386_NP21] Improve CPU registers message.
  * [VM/I386_NP21] Print PC address on PANIC.
  * [VM/DEVICE] Add APIs to DEVICE::; clear_event(), force_register_event(), check_and_update_event() and variants.
  * [VM/FMTOWNS] CDROM: Implement around commands.
  * [VM/FMTOWNS] CDROM: Fix around command 80h (SET STATE).May work RANCE III and SUPER REAL MAHJONG PIV.
  * [VM/FMTOWNS] CDROM: Command A0h: TOWNS_CD_ACCEPT_WAIT must be only after CDROM_COMMAND_STOP_CDDA.
  * [VM/FMTOWNS] CDROM: Reply status immediately with PLAY/PAUSE/UNPAUSE without STATUS BIT (44h/C5h/C7h).
  * [VM/FMTOWNS] CDROM: Maybe working with CCD image.
  * [VM/FMTOWNS] CDROM: Rename delay/status methods to unique name.
  * [VM/FMTOWNS] CDROM: Don't occure duplicated EOT.
  * [VM/FMTOWNS] CDROM: Fix not play CDDA with command SPAM.
  * [VM/FMTOWNS] CDROM: Fix wrong sector size with single track.
  * [VM/FMTOWNS] CDROM: Implement ISO file feature, MODE1/2/RAW read feature.
  * [VM/FMTOWNS] CDROM: Fix wrong response at PAUSE/RESUME CDDA (85h/87h).
  * [VM/FMTOWNS] SPRITE: Fix around zooming and rotating.
  * [VM/FMTOWNS] CRTC: Available to display LOW RESOLUTION.
  * [VM/FMTOWNS] MEMORY: Disable shadow write at F8000h-FFFFFh.
  * [VM/FMTOWNS] MEMORY: Reset memory map when reset from CPU (i.e.Triple fault).
  * [VM/FMTOWNS] SPRITE: Implement correct offset handling and clipping feature.
  * [VM/FMTOWNS] SPRITE: Event driven sprite.
  * [VM/FMTOWNS] VRAM: TRY: Don't wrap around boundary of VRAM (i.e.8107ffff).
  * [VM/UPD71071] eFM-Towns works without SINGLE_MODE_DMA.
  * [VM/UPD71071/TOWNS_DMAC] Add debug message for issues, i.e.Bubble Bobble for FM-Towns.
  * [VM/UPD71071/TOWNS_DMAC] More correctness addressing on R/W.
  * [VM/FM7] DISPLAY: Optimize to be faster.
  * [VM/FM7] DISPLAY: Adjust alignment of some variables.
  * [VM/FM7] MAINMEM: Fix crash at DISPLAY::initialize().
             Seems to break memory at FM7_MAINMEM::initialize().
  * [VM/FM7] MAINMEM: Fixing MEMORY LEAK.
  * [VM/FM7] DISPLAY: Fix not resume digital palette on loading state.
  * [VM/PCM1BIT][COMMON] Fix memory access violation in high-pass/low-pass filter.
  * [VM/MB8877] Fix memory leak on state saving/loading.
  * [VM/COMMON_VM] Move AD78820KR::, RF6C68::, YM2612:: to libCSPcommon_vm.
  * [VM] Make event() and mix() (and some functions) with __FASTCALL.
  * [EMU/Qt] Block execution EMU until prepering GUIs.
  * [EMU/Qt] Don't out LOG until logger set up.
  * [Qt/Draw] DO not start thread at Ui_MainWindow::LaunchEmuThread().
  * [Qt/OSD] Inherit OSD_BASE to QObject.Maybe not needed threading.
  * [Qt/EMU] MOVE a lot of methods to EMU_TEMPLATE:: and EmuThreadClassBase::.
  * [UI/Qt] MOUSE: Add mouse sensitivities config GUI.
  * [UI/Qt] Integrate to single MainWindow object.
            Fix https://matsuri.5ch.net/test/read.cgi/i4004/1601147054/80 .
  * [UI/Qt] Add confirm on quitting emulator.
  * [Qt/LOGGER] Add mutex locking to some functions called from logger.
                Fix crash on quitting a emulator.
  * [UI/Qt] Change orders of "Emulator Menu".
  * [Qt/MOUSE] Fix mouse clicking on one-board-computers.
  * [COMMON] Make method of pairXX_t with __FASTCALL.
  * [UI/Qt] FLOPPY: Use 5inch icon when using 5inch floppy.
  * [OSD/Qt] Fix using pointer after freeing.
  * [DOC] Add INSTALL.md and INSTALL.en.md.
  * [NET/Qt] Re-implement around TCP/IP, UDP/IP.MZ-2500 works.
  * [Qt/OpenGL] SHADER: Stop to use discard.
  * [Qt/OpenGL] Make shaders abstraction.
  * [Qt/OpenGL 4.5] Reduce create/destroy buffers at same screen size.
  * [Qt/OpenGL4_5] Fix around TV-Rendering.
  * [QT/OpenGL4.5] Prepare to implement screen distortion.
  * [Qt/OpenGL] Integrate shaders beyond GL version and GL/GLES.
  * [UI/Qt] Fix not update around virtual media display.
  * [UI/Qt] Fix not found disk file include kanji (or another non-latin character)
             as filename.This issue maybe happen at non-UTF-8 locale.
  * [Qt/COMMAND_LINE] Fix failure starting args "--fdx" "--bubx" with  D88/D77/B77 file.
                      Maybe fixed https://matsuri.5ch.net/test/read.cgi/i4004/1601147054/30 .
  * [BUILD/CMAKE] Support multiarch LIBDIR.
	          Maybe fixed https://matsuri.5ch.net/test/read.cgi/i4004/1601147054/21-24.
  * [BUILD/CMAKE] Try to resolve https://matsuri.5ch.net/test/read.cgi/i4004/1601147054/33.
  * [BUILD/Win32] Make CSPcommon_vm DLL.
  * [BUILD/CMAKE] FORCE SET C++ STANDARD to C++11.
  * [TOOLCHAIN/WIN32] Update cross-build X264 script.
  * [TOOLCHAIN/WIN32] Update cross building scripts for some external libraries.
  * Built with 46662e26995354caf47739a7723cd5b96dc67a26 (or later).

-- Dec 16, 2020 00:40:21 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

8/16/2020

[VM/SCSI_DEV] improve to specify data req signal delay (thanks Mr.Sato)
[VM/SCSI_DEV] fix read6/write6 command in 0 length case (thanks Mr.Sato)
[VM/SISI_HDD] change drive type to hot swappable
[VM/SISI_HDD] improve seek time (thanks Mr.Sato)
[VM/SASI_HDD] support winchester drive parameters command
[VM/Z80DMA] improve to sense rdy signal as level, not edge (thanks Mr.Sato)

[MZ2500/CRTC] fix sub plane address in 640x200 16colors mode (thanks Mr.856)
[SVI3X8] support SPECTRAVIDEO SVI-3x8 (thanks Mr.tanam)
[X1] add menu items for FD2/FD3 (thanks Mr.Sato)
[X1/DISPLAY] fix high speed pcg definition (thanks Mr.YAT)
[X1TURBOZ/DISPLAY] improve palette update timing (thanks Mr.Sato)


8/14/2020

[OSD/WIN32] support x64 build (thank Mr.Marukun)
[OSD/WIN32] support Direct2D

[MICOM_MAHJONG] support Nippon Mail Service MICOM MAHJONG (thanks Mr.GORRY)
[TVBOY] support GAKKEN TV BOY (thanks Mr.tanam)

-----

お楽しみあれ!
-- Ohta.
