** Qt porting for Common Source Code Project **
                                         February 22, 2019
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20190222

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

* SNAPSHOT February 22, 2019
  * Upstream 2019-02-19.
  * [VM] Fix crash when end of emulation at various(!) VMs.
  * [DEBUGGER/EMU] Some functions at debugger.cpp moved (and modified) to emu.cpp. This workaround needed by libCSPcommon .
  * [EMUUTIL/WIN32] Temporally disable SSE2.
  * [VM/I8080] I8085: Fix around SID instruction.FP200 works.
  * [VM/MSM5205] Add new API: pause_w().
  * [VM/MSM5205] Adjust ADPCM's sound level due to be too small sound.
  * [VM/UPD71071] Add 16bits transfer mode (needs to emulate FM-Towns).
  * [VM/PCENGINE] Separate around ADPCM from pce.cpp.
  * [VM/PCENGINE] Mostly works CD-ROM^2 softwares, excepts (at least) Valis2 and R-TYPE. Some softwares still contain wrong working.
  * [VM/SCSI_CDROM] CDDA: Fix interpreting cue sheet.Lasersoft's brand softwares may works.
  * [VM/SCSI_CDROM] CDDA:Don't update track when setting end position.
  * [OSD/Sound] Update OSD API, initialize_sound() has 4 args, not 2.
  * [OSD/Sound] SDL_MixAudioFormat() *MUST* use for SDL2, shouldn't use SDL_MixAudio for SDL2.
  * [Qt/OpenGL] Asynchronous pixel transfer with OpenGL 4.5 (and Core profile renderer).
  * [Qt/OpenGL] Now, core profile needs less than OpenGL 4.5.
  * [Qt/OpenGL] Fix not save screenshot with OpenGL renderers.(This issue didn't happen with OpenGL ES).
  * [Qt/AVIO] Fix wrong color at one-board-computers.
  * Built with b52a6f59c59908523e27e81911d17ba37dd2c1ac (or later).
  
-- February 21, 2019 21:35:48 +0900 K.Ohta <whatisthis.sowhat@gmail.com>
  
本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

2/19/2019

[VM/DEVICE] add is_primary_cpu() and update_extra_event()
[VM/EVENT] support to udpate event while cpu is running one opecode
[VM/Z80] improve to update event in every read/write cycle

[MZ2500/MEMORY] improve pcgram wait in display timing


2/16/2019

[EMU/DEBUGGER] improve to enumerate devices that is debugger available
[EMU/DEBUGGER] improve to show all breaked reasons
[EMU/DEBUGGER] support breakpoint of cpu selected by "! device" command
[EMU/*] simplify code for debugger
[VM/*] simplify code for debugger

[VM/I8237] support break point for direct memory access
[VM/MB8877] fix not to wait 60ms to start read/write after seek is finished
[VM/MC6844] support break point for direct memory access
[VM/TMS9918A] support break point for registers and vram access
[VM/UPD71071] support break point for direct memory access
[VM/Z80DMA] support break point for direct i/o and memory access


2/14/2019

[EMU/DEBUGGER] support break point for non-cpu device
[EMU/DEBUGGER] change command length from 64 to 1024

[VM/AY_3_891X] support break point
[VM/DEVICE] add get_debug_data_addr_space()
[VM/DEVICE] change type of get_debug_regs_info() from void to bool
[VM/MB8877] fix to decrease first seek time by 500us (2D/2DD) or 250us (2HD)
[VM/TMS9918A] support break point
[VM/YM2151] support break point
[VM/YM2203] support break point
[VM/Z80CTC] fix to reset interrupt req/in service by bit2 of control register
[VM/Z80DMA] fix to reset interrupt req/in service by reset command

[X1TURBO/EMM] support to access vram as memory space from debugger
[X1TURBO/IOBUS] support to access vram as memory space from debugger
[X1TURBO/IOBUS] support break point


2/9/2019

[EMU/DEBUGGER] enlarge text buffer size

[VM/DEVICE] add get_context_child() to enumerate daisy-chained devices
[VM/DISK] add get_sector_info()
[VM/MB8877] improve debugger to show current head position and disk geometry
[VM/MB8877] fix not to abort command when eject disk in unselected drive
[VM/UPD765A] improve debugger to show current head position and disk geometry
[VM/Z80*] add get_context_child() to enumerate daisy-chained devices

[X1TURBO] fix to force clear iei/oei of z80 family devices when reset vm
[X1TURBO/DISPLAY] fix to check bit0/2 of port 0x1fd0 in draw_text()


2/8/2019

[EMU/*] simplify code to draw screen while debugging cpu
[OSD/*] simplify code to draw screen while debugging cpu
[VM/*] simplify code to draw screen while debugging cpu

[BUBCOM80/DISPLAY] improve dmac
[HC80/IO] fix slave-cpu command 0x27 and 0x29 (thanks Mr.Stefano Bodrato)


2/7/2019

[EMU/DEBUGGER] improve to draw screen while debugging cpu
[EMU] add override/restore/run_wndproc() for debugger
[EMU] add create_bank_floppy_disk()
[OSD/WIN32] add override/restore/run_wndproc() for debugger

[VM/315_5124] improve draw_screen() for debugger
[VM/H6280] improve to run window procedure while suspending for debugger
[VM/I286] improve to run window procedure while suspending for debugger
[VM/I386] improve to run window procedure while suspending for debugger
[VM/I8080] improve to run window procedure while suspending for debugger
[VM/M6502] improve to run window procedure while suspending for debugger
[VM/MC6800] improve to run window procedure while suspending for debugger
[VM/MC6809] improve to run window procedure while suspending for debugger
[VM/MCS48] improve to run window procedure while suspending for debugger
[VM/TMS9918A] improve draw_screen() for debugger
[VM/TMS9995] improve to run window procedure while suspending for debugger
[VM/UPD7801] improve to run window procedure while suspending for debugger
[VM/UPD7810] improve to run window procedure while suspending for debugger
[VM/V9938] improve draw_screen() for debugger
[VM/V99X8] improve draw_screen() for debugger
[VM/Z80] improve to run window procedure while suspending for debugger

[BUBCOM80/DISPLAY] improve draw_screen() for debugger
[FAMILYBASIC/PPU] improve draw_screen() for debugger
[MZ80K/DISPLAY] improve draw_screen() for debugger
[MZ1500/MEMORY] improve draw_screen() for debugger
[PC8801/PC88] improve draw_screen() for debugger
[PCENGINE/PCE] improve draw_screen() for debugger
[SMC777/MEMORY] improve draw_screen() for debugger
[X1/DISPLAY] improve draw_screen() for debugger


1/29/2019

[MZ80K/MEMORY] support COLOR GAL 5 (thanks Mr.Suga)
[PC8001/PC88] fix issue that cursor is mistakenly hidden


1/18/2019

[PC8001/PC88] clear ram[0xff33] for DEMPA Galaxian
[SMC777/MEMORY] improve to render screen in each scan line


1/16/2019

[EMU] improve to reinitialize vm in reset when dipswitch is changed

[VM/UPD765A] fix st3 in sence devstat command to set two-side bit (temporary)

[PC8801] support GSX-8800
[PC8801] support to enable/disable PC-8801-11/GSX-8800/PCG-8100
[PC8801] fix some degradations

-----

お楽しみあれ!
-- Ohta.
