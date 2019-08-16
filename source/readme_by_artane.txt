** Qt porting for Common Source Code Project **
                                         August 16, 2019
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20190816

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

* SNAPSHOT Aug 16, 2019
  * Upstream 2019-04-30.
  * This is point release, still exists some issues (i.e.EMM386(NEC/PC98) and FreeBSD(98) don't work) for PC-9801 and PC-Engine and some VMs, will fix them.
  * [UI/Qt] DEBUGGER: Add history for debugger command line.
  * [UI/Qt] DEBUGGER: Add auto-completion for command-line.
  * [VM/DEVICE] Use __FASTCALL with interfaces, read_*() ,write_*(), fetch_op() and some functions.Make this faster emulation (i.e.PC-9801RA and EMM386 under FreeDOS).
  * [VM/PC9801] Separate EGC functions.
  * [VM/PC9801] Add V30@8.0MHz with some I286/I386 machines.
  * [VM/PC9801] Check differnt of system work area (0000:0400-0000:05FF) both mame(pc9801rs) and emupc9801ra .
  * [VM/PC9801] Add "UPPER_I386" flag for detect using later than HAS_I386.
  * [VM/PC9801] CPUREG: (Maybe) improve changing cpu sequence around I/O 00F0h.
  * [VM/PC9801] CPUREG: Redirect interrupt signal via CPUREG:: .VMs with V30 sub CPU (i.e.PC9801RA) work with V30.
  * [VM/PC9801] Fix wrong initialize SYS_PORT_B.
  * [VM/PC9801] Fix wrong initialize memory switch.
  * [VM/PC9801] Add DIPSWITCH object.
  * [VM/PC9801] Fix different value at [0000:0501].
  * [VM/PC9801] MEMBUS: Split update_bios() to functions.
  * [VM/FP1100] Fix lacking some key symbols.Thanks to https://matsuri.5ch.net/test/read.cgi/i4004/1526806551/540 .
  * [VM/AY_3_891X] Fix not supported defines, replace flags.
  * [VM/AY_3_891X] Add feature ; dump/set register via debugger.
  * [VM/YM2151] Add feature ; dump/set register via debugger.
  * [VM/YM2203] Add feature ; dump/set register via debugger.
  * [VM/SN74689AN] Add feature ; dump/set register via debugger.
  * [VM/BEEP] Add feature ; dump register via debugger.
  * [VM/PCM1BIT] Add feature ; dump register via debugger.
  * [VM/I80x86/V30] Start debugger even halting.
  * [VM/I80x86/8088/V30] Make i86/186/88/286 and V30 to common_vm.
  * [VM/I386] Fix WRONG flag mask at LMSW.
  * [VM/I386] MOV CR0 EyX : Fix wrong flags handling.
  * [VM/I386] Exitable when falling into infinite TRAP-Loop.
  * [VM/I386] mov CRx,R32/mov r32,CRx : Adjusting.
  * [VM/i8259] Add PIC HACKing flag for PC9801.
  * [VM/uPD7810/uPD7907] PC2001: Include uPD7810 variants and uPD7907 to libCSP_common_VM.
  * [VM/MB8877] Fix buffer overflow with logging.
  * [VM/Z80DMA] TODO/WIP: Workaround for https://tablacus.github.io/LSX-Dodgers/ .This still be not resolved issue.
  * [VM/EVENT] Add remove_context_cpu().This may not effect to MAIN_CPU(id==0).
  * [DOC/FM7] Fix typo (*ﾉω・*)てへぺろ
  * [Qt/LOGGER] Improve locking.
  * [UI/Qt] OOPs: Fix LACK of DATARECORDER BUTTONS(abolish of USE_TAPE_BUTTON): Lack of merging UPSTREAM 2018/10/07.
  * [UI/Qt] MENU: Split some methods (of Ui_MainMenuBase::) to menu_emulator.cpp and menu_machine.cpp .
  * [UI/Qt] MENU: Simplify menu creation.
  * [CONFIG/Qt] Fix bit order of logging configure.
  * [BUILD/CMAKE] Add CPU affinity mask when compiling.This may work only with GNU/Linux host.
  * [BUILD/CMAKE] Improve build message with finished.
  * [BUILD/MINGW] Update optimize parameter.
  * [BUILD] Separate definitions of archtecture flags.
  * [BUILD] Add ARM32/64 definitions (initial).Still not testing.
  * Built with bbbb75cdc4051269c60a5f7ba18881eda56e8fd3 (or later).

-- Aug 16, 2019 20:23:45 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

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
