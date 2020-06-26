** Qt porting for Common Source Code Project **
                                         June 27, 2020
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_2020627

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

* SNAPSHOT June 27, 2020
 * Upstream 2020-02-21.
 * [EMULATION] Now, emulation period is half of one frame.
               Because some devices (i.e. mouse) seems to need a short period.
               This may change to 1/4 of one frame (or not).
	       See event.cpp and qt/common/emu_thread.cpp (& more).
 * [VM/EMU] Important: now EMU:: class inherits EMU_TEMPLATE:: .
 * [VM/EVENT][Qt] execute event->drive() (or event->run()) by half frame.
                  This is workaround for choppy mouse pointer/joystick.
 * [VM/FMTOWNS] Still works initially.
                See source/src/vm/fmtowns/00_status.ja.md,
                STATUS section of doc/VMs/fmtowns.txt
		and 000_gitlog.txt .
 * [VM/FMTOWNS] CDROM: Implement CMD 00h (SEEK) correctness.
                May Fractal engine works..
 * [VM/DEVICE][DEBUGGER] Add Call trace feature to I386_NP21.
                         DEVICE::'s API HAS CHANGED.
 * [VM/DEBUGGER] Add logging to "UCT" (Call trace) command.
 * [VM][CONFIG] Add variable memory size feature to some VMs.See eFMTOWNS.
 * [Qt/OpenGL4_5] Draw: Fix crash with external mapping (immutable storage).
                        Still not implement reading buffer.
 * [COMMON/FIFO] Add FIFO::fifo_size() and FIFO::left().Update SOVERSION.
 * [BUILD/CMake] Win32: Update toolchain for LLVM10.x and Ubuntu20.04
                 (Has uploaded to
		 https://hub.docker.com/repository/docker/artanejp/mingw-w64-llvm10-ubuntu20.04/general ).
 * [BUILD/Linux] Debian Sid: Now, build with CLANG 10 and Qt5.14.
 * [VM/FMGEN][VM/YM2612][VM/FMTOWNS] Fix prescaler value, calculating for own OPN2.
 * [VM/I386_NP21] Merge Upstream 2020-04-06 's I386::/NP21.Belows are differ from upstream:
      - Make some memory access functions inline (these are bottoleneck of emulation).
      - And some modifies are same as SNAPSHOT March 03, 2020.
 * [VM/I386_NP21] Optimise for speed, make some functions __inline__ .
 * [VM/I386_NP21] Fix EDX value at resetting.
 * [VM/I386_NP21] Temporally enable FPU for i386.
 * [VM/I386_NP21][DEBUGGER] WIP: Adding exception handling.
 * [VM/I386_NP21] Log when made panic.
 * [VM/I386_NP21] Add undefined instruction "0F A6".
                  This may act as "int 6".Thanks to Soji Yamakawa-San.
 * [VM/I386_NP21] FPU: FISTTP INSNs (prefix DF) are only later than Pentium 4,
                  not exists I386/486/Pentium.
 * [VM/I386_NP21] Disable FPU with I386, enable with I486SX.
 * [VM/I386_NP21] Change FPUemul to DOSBOX2 (temporally).
 * [VM/I386_NP21] Initialize CR0 to 0x00000000 (+some bits) for i386.
 * [VM/I386_NP21] *Perhaps* REPNE MOVS[B|W|D] don't dedicate Z flag,
                  Thanks to Soji Yamakawa-San.
 * [VM/I386_NP21] Fix FTBFS with LLVM CLANG++.
 * [VM/I386_NP21] Add interrupt within rep prefix'ed ops.
 * [VM/UPD71071] Modify handling of 16bit transfer mode.
 * [VM/UPD71071] TOWNS_DMAC: Implement *truely* 16bit transfer feature
                             from Renesas(NEC Electronics)'s DATA SHEET.
 * [VM/UPD71071] TOWNS_DMAC: Ugly workaround for 16bit transfer DMAC command.
                             Will fix.
 * [VM/UPD71071] Change mean of TC bus bits (per channel).See mz2800.cpp.
 * [VM/UPD71071] TOWNS_DMAC: Fix mandatory name with "mask" variable/arg.
 * [VM/UPD71071] Adjust status of on-demand-mode.
 * [VM/I8253] Add debugger feature, still reading only.
 * [VM/DEBUGGER] Add "RH" command to debugger and
                 bool get_debug_regs_description(_TCHAR *, size_t) to API.
 * [VM/FMTOWNS] FONTROMS: Add API read_direct_data8() to reading faster by CRTC.
 * [VM/FM8] Fix warning from EVENT:: when resetting.
 * [VM/SCSI] Add new (pseudo) SIGNAL for preparing to use buffered transfer.
 * [Qt/LOGGER] Shrink redundant title.
 * [VM/LOGGER][OSD][VM_TEMPLATE] Add API to log with VM's time.
 * [OSD/Qt]Remove mouse position limiter.
 * [UI/Qt] Virtual media: Adjust width of "HDx:".
 * [UI/Qt] Add filename feature to Virtual-Media indicator.
 * [UI/Qt] Adjust width for HDD.
 * [UI/Qt][OSD] Add tooltip for virtual medias.
 * [UI/Qt] CDROM: Add "SWAP BYTE ORDER for AUDIO" config entry.
 * [OSD/Qt][LOGGER] Fix linkage error for LLD 10.x and Win32 cross.
 * Built with d2322eb3793c06a3056ed10245d49c6a865a79d4 (or later).

-- Jun 27, 2020 01:51:03 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

4/6/2020

[VM/I386_NP21] update to Neko Project 21/W ver0.86 rev72


-----

お楽しみあれ!
-- Ohta.
