** Qt porting for Common Source Code Project **
                                         October 28, 2018
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20181028

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

* SNAPSHOT October 28, 2018
  * Upstream 2018-10-14 .
  * Update state framework to upstream, excepts scrntype_t.
  * [VM] Use namespace {VMNAME} to separate around VMs.
  * [STATE/FILEIO] Change StateValue(scrntype_t&) to StateValueScrnType_t(&) due to compiler not detect differ of scrntipe_t and (uint32_t | uint16_t);
  * [OSD/Qt] Remove some APIs.
  * [GUI/Qt] DIALOG/ABOUT:Fix not displaying version of libOSD.
  * [GENERAL] Update gitignore.
  * [MISC/TOOL] Add function extraction script.
  * [COMMON/FILEIO] common.h : Fix pair16_t and pair64_t to fileio.cpp .
  * [COMMON/FILEIO] Re-Add FILEIO::Fflush().
  * Built with 5cdfe7e27393edaecd445ac9b315d837dd697654 (or later).

-- October 28, 2018 03:36:00 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

10/14/2018

[COMMON/FILEIO] improve functions to load/save state file for big-endian

[VM/*] improve process_state for big-endian


10/13/2018

[EMU/DEBUGGER] increase breakpoint number from 8 to 16

[VM/VM_TEMPLATE] fix issue that virtual machine is not correctly released


10/10/2018

[VM/SCSI_DEV] fix ack signal issue when multiple devices are attached
[VM/SCSI_HOST] support to output cd/io/msg/req signals to other devices

[MZ2800/SASI] support SASI I/F and HDD (thanks Mr.Oh!Ishi)


10/7/2018

[COMMON] add pair16_t and pair64_t (thanks Mr.Artane.)
[COMMON] rename pair_t to pair32_t
[COMMON] add functions for endians (thanks Mr.Artane.)
[EMU] fix roman/kana conversion when uppercase alphabet is input (thanks Mr.Artane.)
[EMU] abolish SUPPORT_VARIABLE_TIMING and USE_TAPE_BUTTON
[WINMAIN] abolish USE_ALT_F10_KEY
[WIN32/INPUT] abolish NOTIFY_KEY_DOWN and USE_SHIFT_NUMPAD_KEY
[WIN32/INPUT] improve key input for shift + numpad keys

[VM/*] introduce VM_TEMPLATE (thanks Mr.Artane.)


10/5/2018

[COMMON] combine load_state and save_statet of cur_time_t to process_state
[COMMON/FIFO] combine load_state and save_statet to process_state
[COMMON/FILEIO] add functions to load/save state file

[VM/*] combine load_state and save_statet to process_state
[VM/I386] fix to load/save vtlb state
[VM/I386] fix to rebuild tables when load state

[JX] support save/load state
[FM77AV] import Mr.Artane.'s fixes (Release in September 30, 2018)

-----

お楽しみあれ!
-- Ohta.
