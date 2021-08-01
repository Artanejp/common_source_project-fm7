** Qt porting for Common Source Code Project **
                                         August 01, 2021
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20210801

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

* SNAPSHOT August 01, 2021
  * Upstream 2021-05-02.
  * [BUILD/CMAKE] Add *Very Experimental* C++20 build settings.
  * [OSD/EMU][MOUSE/JOYSTICK] Should LOCK/UNLOCK per referring buffers of joystick/mouse.
    Note: This is MAJOR API CHANGE around mouse and joystick.
  * [CONFIG/FMTOWNS] Add config.machine_features[32] to use machine configuration (w/o DIP SWITCH).Still not be bulidable.
    Note: See vm/fmtowns/joystick.cpp, FMTOWNS::JOYSTICK::update_config().
  * [FMTOWNS/KEYBOARD] SPECIAL_RESET: Available to boot with special key code (i.e. "CD" "DEBUG").
  * [FMTOWNS/CDROM] Re-implement parsing CCD image file.Still ugly.
  * [FMTOWNS/VRAM] Add __LIKELY_IF() and __UNLIKELY_IF() to assist compiler's optimization.
    See [read|write]_memory_mapped_io32() and read_memory_mapped_io16() at FMTOWNS::TOWNS_VRAM class.
  * [VM/I386_NP21] Add hints of branch-prediction whether accessing memory (and some of interrupts) is legal.
    This reduces HOST CPU USAGE especially high-VM-clocks.
  * [FMTOWNS/VRAM] More faster vram accessing.
  * [FMTOWNS/MEMORY/VRAM] Add features around CACHE for after i486.Still be dummy.
  * [FMTOWNS/CRTC] Use more SIMDs to be faster rendering.
  * [FMTOWNS/KEYBOARD] Implement AUTO REPEAT.
  * [FMTOWNS/TIMER] Clear OV and INTR when enabling interval timer.
  * [FMTOWNS/VRAM] More faster VRAM access for packed pixel mode.
  * [FMTOWNS/CDROM] Even reply without REQ_STATUS bit (0x20) with PAUSE/RESUME CDDA (85h/87h).
  * [FMTOWNS/CDROM] Falldown intr even stat_reply_intr (04C2:bit6) == 0 on MCU interrupt.
  * [FMTOWNS/CDROM] Reply error when reading beyond track on READ_SECTOR.
  * [FMTOWNS/CDROM] Fix around booting from some version(s?) of TOWNS OS.
	 i.e)AYAYO 4.
  * [FMTOWNS/CRTC/TIMER/MEMORY] Improve around I/O.
  * [FMTOWNS/DMAC] Remove some variables.Update state save version.
  * [FMTOWNS/SCSI] Add a PORT: 0C34h.
  * [FMTOWNS/MEMORY] Revert to use primitive_[read|write]. Fix CDROM's some issues (i.e. Crashing Ayayo4).
  * [FMTOWNS/MEMORY] Add dma read/write functions with wait.
  * [FMTOWNS/ICCARD] Improve logic around open/close.
  * [FMTOWNS/JOYSTICK] Fix confused mouse mask.
  * [FMTOWNS] Split MOUSE and JOYPADs to separate class.
  * [FMTOWNS/FLOPPY] Add support for 1.44MB 2HD (2HC).
  * [I286/I86/V30] Separate namespace CPUTYPE ENUMS.
  * [I286/I86] Fix weird bitmask default value.
  * [VM/UPD71071] Add read_io16(), write_io16() and ACK feature.
  * [PC9801/EGC] Fix FTBFS with C++20 ; "error: ISO C++17 does not allow 'register' storage class specifier".
  * [BUILD/CMake] Add samplescripts supports GCC-11 initially.
  * [BUILD/CMake] Prepare to support LLVM12, but now unable to build Win32 version with llvm12 due to _aligned_alloc() and _aligned_free().
  * [Build/Win32] LLVM12/MinGW-w64/Cross : Add libc++abi and others to toolchain.
  * [Build/Win32] Update revision of ffmpeg to 4.4.
  * [JOY_THREAD/SDL2/GAMECONTROLLER] Fix sample initial value.
  * Built with ab2601af3b0de22bd806ce5312ebf06823a16405 (or later).

-- August 01, 2021 22:34:13 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

5/2/2021

[VM/DATAREC] fix mixing sound track
[VM/HD46505] support smooth vertical scroll
[VM/MC6843] fix seek command
[VM/MC6844] fix data chain register to consider 2/4 channel select bit
[VM/MC6844] fix to transfer 64K when byte count register is zero
[VM/Z80CTC] fix to apply written time constant just after reset bit is cleared

[BX1] fix memory map around ram
[BX1] support cartridge rom images
[BX1/DISPLAY] add missing font patterns
[BX1/FLOPPY] support i/o ports around fdc
[BX1/KEYBOARD] support PROG.SELECT switch
[BX1/PRINTER] support AUTO PRINT switch
[MZ1500/JOTSTICK] support joystick (thanks Mr.Koucha-Youkan)
[PC6001] remove some codes from iP6 Plus
[X1TURBO/DISPLAY] support smooth vertical scroll


2/7/2021

[WINMAIN] improve WM_KEYDOW/WM_KEYUP events for VK_PROCESSKEY case
[WIN32/CONSLE] improve routine to change console size

[VM/DISK] fix density flag when loading solid image with fm sectors
[VM/MC6843] fix track zero flag in STRA
[VM/MC6843] fix seek error flag in STRB
[VM/MC6843] fix seek command

[BX1/DISPLAY] improve for drawing digitron display
[BX1/KEYBOARD] improve I/O ports for detecting key pressed/released
[BX1/PRINTER] add ugly patch for printer process

1/24/2021

[VM/I8279] support 8279 (based on MAME)

[MP85] support MITEC MP-85


1/17/2021

[VM/UPD765A] improve for the case tc is asserted while reading/writing sector

[PC8801/PC88] improve to render screen with port params at end of disp timing
[X1TURBO/FLOPPY] fix to change type of all drives (thanks Mr.Sato)


1/3/2021

[WINMAIN] improve for pressing shift key and numpad key

[MZ80K] fix roman/kana conversion
[MZ80K/KEYBOARD] improve for pressing right shift key and numpad key


12/31/2020

[PC8801/PC88] improve crtc to refer reverse setting in start display command


12/21/2020

[VM/SCSI_CDROM] fix start frame of CD-DA playing when track number is specified

[PCENGINE/PCE] fix issue that ADPCM is mistakenly looped


12/19/2020

[VM/SCSI_CDROM] fix pre-gap of first track when it is audio track


12/18/2020

[VM/SCSI_CDROM] improve routine to get start/end frame of CD-DA playing

[PC8801/PC88] support 8inch DMA-type floppy drives for PC-8001mkII/SR
[PC9801/DISPLAY] improve EGC (thanks Mr.Ryuji Okamoto)


12/16/2020

[PC8801/DISKIO] improve to read/write files in initial current directory
[PC8801/PC88] support force ready/drq mask register for DMA-type FDD
[PC8801/PC88] fix PC-8001mkIISR hiragana font when PCG-8100 is enabled


12/15/2020

[PC8801/DISKIO] support M88 DiskDrv (thanks Mr.CISC and Mr.apaslothy)
[PC8801/PC88] support PC-8001mkIISR hiragana font
[PC8801/PC88] support to disable 5inch/8inch-FDD interfaces
[PC8801/PC88] support to disable updating scan line setting automatically
[PC8801/PC88] fix mouse data when position is not latched


12/14/2020

[PC8801/PC88] support 8inch DMA-type floppy drives


12/13/2020

[VM/MC6843] support MC6843 (based on MAME)
[[VM/SCSI_CDROM] fix to reset logical block size in Test Unit Ready command

[BX1] support CANON BX-1 (not work)
[MZ1500/QUICKDISK] improve for BSD record (thanks Mr.Motochan1500)
[PC8801/PC88] fix hireso graphic screen when scan line is disabled


12/12/2020

[PC8801/PC88] fix to clear attibutes at starting new frame
[PC8801/PC88] fix to read status of 2nd OPNA


12/11/2020

[PC8801/PC88] fix to run dma from memory to crtc when (rd,wr)=(0,0)
[PC8801/PC88] fix to run dma from scsi to memory only when count > 0


12/8/2020

[COMMON] fix build error on VC++2019 (thanks Mr.Sato)

[VM/Z80CTC] fix not to clear in-service at software reset (thanks Mr.Sato)
[VM/Z80DMA] fix stall cycles at BUSACK in byte mode (thanks Mr.Sato)

[X1TURBOZ/DISPLAY] fix zpalette in 64 colors, 2 screens mode (thanks Mr.Sato)


12/6/2020-2

[MZ1500] fix inp(0xe8) to detect voice board is missing (thanks Mr.kitahei88)
[X1TURBOZ/DISPLAY] fix to update zpalette at vline=0 (thanks Mr.Sato)


12/6/2020

[OSD/WIN32] import Unity plug-in code (thanks Mr.Marukun)

[VM/I386_NP21] update to Neko Project 21/W ver0.86 rev79 beta4
[VM/UPD765A] fix transfer size to 128 << min(N, 7) (thanks Mr.Kugimoto)

[MZ1500/QUICKDISK] improve for QDF format (thanks Mr.kitahei88)
[MZ1500/QUICKDISK] improve for BSD record (thanks Mr.Yuushi)
[PC9801/MEMORY] fix switching BIOS ROM/RAM
[X1/CZ8RB] support CZ-8RB (thanks Mr.Meister)

-----

お楽しみあれ!
-- Ohta.
