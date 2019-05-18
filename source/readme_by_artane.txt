** Qt porting for Common Source Code Project **
                                         May 18, 2019
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20190518

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

* SNAPSHOT May 18, 2019
  * Upstream 2019-04-30.
  * [General] Add Hino Electronics CEFUCOM-21.
  * [Build/GCC] Fix FTBFS without setting LTO_THREADS value and selecting USE_LTO.
  * [Build/Linux] buildvars.dat: Add "USE_SHRINK_DEBUG_SYMBOL" flag to save disk space.
  * [COMMON] Add high pass/low pass filter function.
  * [VM/PCM1BIT,AY3_891x] Add LPF feature.See initialize_sound().
  * [VM/EVENT] Add sound sampling (from host machine) feature framework.
  * [COMMON] Add RINGBUFFER:: class, extend of FIFO::.
  * [FIFO] Add [fill|empty] warning feature.This may be useful for some devices. i.e. 16550 UART, variants of i8251.
  * [FILEIO] Add StateVector() classes.
  * [VM/DEVICE] Add CPU pseudo SIGNALS, 108 to 110.
  * [VM/COMMON] Delete unneeded __builtin_assume_aligned().MinGW32 build works built with "-msse2 -O3".
  * [VM/DMA] Add NEW signal, notify to write-changed address.
  * [VM/EVENT] Add new APIs for recording sound.
  * [VM/PC9801RA] Note: Still not running Microsoft's EMM386.EXE and FreeBSD(98).Will fix.
  * [VM/PC9801] [SASI_BIOS] Fix wrong sectors at FORMAT command.Fix run out at DOS installation.
  * [VM/PC9801] Add some DIPSWITCH features.
  * [VM/PC9801,FMR50,FMR30][SASI_BIOS] Add translate_address(segment,offset) to DEVICE:: .
  * [VM/PC9801,FMR50] Add pseudo-cycles args to  pseudo-bios for i86/286/386.
  * [VM/PC9801] Truely bootable MS-DOS 6.20,excepts EMM386.EXE.
  * [VM/PC9801][SOUND] Playable PCM of PC-9801-86.
  * [VM/PC9801][PC-9801-86] Mostly implement PC-9801-86 sound board.Touhou-Huumaroku (partly) works.
  * [VM/PC9801][MEMBUS] Remove shadow_memory, this includes ram[0xc0000]-ram[0xe7fff].
  * [VM/PC9801][MEMBUS] Maybe complete to set mapping bus to 32bit VMs except Hi-Reso.
  * [VM/PC9801][MEMBUS] Faster memory access.
  * [VM/PC9801][EGC] Make EGC faster (maybe...).
  * [VM/PC9801][CPUREG] Add ARCTIC (a.k.a Timestamp) and 0.6uS? Wait.
  * [VM/PC9801][SASI_BIOS] Improve SENSE command.Write results to 0000:0585h at INITIALIZE.
  * [VM/PC9801][DISPLAY] EGC: Add write protect register (03h).
  * [VM/PC9801][FLOPPY] Improve drive setting via 2DD <-> 2HD.
  * [VM/PC9801][MOUSE] Set clock to 120Hz when resetting.
  * [VM/PC9801] Support low resolution monitor.This is WIP.
  * [VM/PC9801][VM/I386,I286] Add variable (main) CPU clock via CPU's write_signal().Add cpu_wait_foo() with CPU_EXECUTE(foo) .
  * [VM/PC9801RA] Enable to boot contains i386 CPU with (about) correctness ITF and IPL.
  * [VM/PC9801RA] Enable EGC.
  * [VM/I386] Separate I386_OP(int) to I386_OP(int_16) and I386_OP(int_32) because pseudo bios int call (maybe 1Bh) should be in 16bit mode.
  * [VM/I386] Enable debug log (logerror()).
  * [VM/I386] Improve CALL ABS SELECTOR:OFFSET.
  * [VM/I386] Call PSEUDO-BIOS even VM86 mode, EMM386.EXE for FreeDOS(98) and VEM486 (at MS-DOS 6.2) may work(still unstable a bit).
              Some games, i.e. Touhou-KaikiDan work now.
  * [VM/I386] Faster FETCH/READ.
  * [VM/I386] Make some functions around address translation INLINE.
  * [VM/I386] Fix unexpected page fault when accessing memories.
  * [VM/I386] Try to call pseudo-bios even within protected mode (inside of i386_trap()).
  * [VM/DEVICE,I386,I286] Add total_icount via read_signal().To implement PC-9801's clock counter.
  * [VM/DEVICE] Add address_translate() API to generic devices.
  * [VM/I8237] Debug register dump: Add "BANK" registers.
  * [VM/I286][PC9801] Add SIG_I386_FORCE_RESET to notify resetting to external devices.Fix "A20 ERROR" at rebooting PC-9801(32BIT MODELS) with rebooting from program.
  * [VM/UPD7220][PC9801] Add GDC clock feature. UPD7220::set_clock_freq().
  * [VM/UPD7220] Implement position limiter for drawing (line etc).You may set gdc_foo->set_screen_width() and gdc_foo->set_screen_height().
  * [VM/UPD7220] Apply "uPD7220 Design manual".make FIFO as ring buffer.
  * [VM/UPD7220] More precision emulation.
  * [VM/UPD7220][COMMON_VM] Integrate UPD7220_BASE:: and UPD7220:: to UPD7220:: .
  * [VM/UPD7220] Fix not blink cursor.
  * [VM/UPD7220] Improve around command interpret.This makes apply *real* command-queue-empty to IO 00h:BIT2.
  * [VM/UPD7220][PC9801] Add UGLY PC98 HACK to UPD7220:: from NP2.
  * [VM/UPD7220] Update around SYNC and drawing(WIP).
  * [VM/YM2203] Fix wrong status reply at address #3.
  * [VM/MEMORY] Faster memory access.
  * [VM/MC6809][COMMON] Improve disassembler.Thanks to HASERIN-San.
  * [VM/MC6809][DISASM] Fix offset address around INDEX addressing.
  * [VM/PC9801][MEMORY] Add new API at copy_table_[r|w|rw](to, start, end).
  * [VM/PC9801][DISPLAY] Faster accessable GDC.
  * [VM/PC9801][DISPLAY] More faster VRAM access (via EGC/GRCG).
  * [VM/PC9801][SASI_BIOS] Fix status values of SASI commands.
  * [VM/LIBCPU_NEWDEV] Remove deprecated classes.
  * [BUILD/WIN32] Adjust GCC option for MinGW.
  * [QT/MENU_FLAGS] Fix TYPO.
  * [QT/EMU,OSD] Adjust emulation sequence.
  * Built with 88b18e84a6e8049da97d15ddc96d5acf30b1fcb5 (or later).

-- May 18, 2019 23:00:59 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

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
