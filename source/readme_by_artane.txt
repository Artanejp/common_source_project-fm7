** Qt porting for Common Source Code Project **
                                         January 05, 2020
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20200105

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

* SNAPSHOT Jan 05, 2020
 * Upstream 2019-12-31.
 * This is point release, still exists some issues (i.e.EMM386(NEC/PC98) and FreeBSD(98) don't work) for PC-9801 and PC-Engine and some VMs, will fix them.
 * [PCENGINE/CD-ROM^2] More games work.Except some games (i.e. Valis1/2).
 * [DOC] Add CrossBuild_Win32.ja.txt.
 * [VM/FM77AV] VRAM: Fix crash with opening of LUXOL.
 * [VM/FM7] DISPLAY: Simplify logic of draw_screen().
 * [VM/M5] Fix keyboard input, separate BOTH LSHIFT and RSHIFT.
           Expect to fix issue of https://matsuri.5ch.net/test/read.cgi/i4004/1526806551/617.
 * [VM/COMMON_VM] Add MEMORY:: (vm/memory.[cpp|h]) to libCSPcommon_vm.
 * [VM/COMMON_VM] MEMORY:: ; update API.
 * [VM/COMMON_VM] Include IO:: class to common_vm.
 * [VM/SCSI_DEV] Comment out duplicated writing to SIG_SCSI_DAT.
 * [VM/UPD71071] Prepare address extend for uPD71071 (for Towns).
 * [VM/UPD71071] do_dma() : Split to tiny functions.
 * [VM/SCSI_CDROM] CD-DA: Fix playing beyond multiple tracks.
              Fix freezing some CD-ROM^2 games for PC-Engine (i.e. Vails series).
 * [VM/PCENGINE] ADPCM: Add comments.
 * [VM/PCENGINE] ADPCM: Fix stopping with command 60h(REPEAT PLAY).Fix not sounding BOSS round at Valis2, but still hangs up.
 * [VM/PCENGINE] SCSI_CDROM: WIP: Fixing not play game at Valis 1.
 * [VM/I386] Reduce compiler warnings.
 * [Qt/OpenGL] Use half float textures if enabled.
 * [Qt/OpenGL] GL3: Fix using color space; yiq to ycrcb.
 * [Qt/OpenGL] GLPack:: Selective texture having (or not) ALPHA CHANNEL.
 * [Qt/OpenGL] GL4.5: Fix not capture GPU snapshot from RENDERDOC.
 * [Qt/OpenGL] GL4.5: Fix not map buffer to screen on most (not all) of emulators.
 * [Qt/OpenGL] GL4.5: Tuning around "TV Renderer".
 * [Qt/OpenGL] GL4.5: NTSC SHADER: Adjust scale factor.
 * [Qt/OpenGL] OpenGL ES: Add entry for OpenGL ES 3.1.
 * [Qt/DRAW_THREAD] New (more accurate) draw scheduler.
 * [Qt/ABOUT_DIALOG] Maybe improve credit displaying.
 * [Qt/AVIO] Replace fprintf() to logging functions.
 * [OSD/KEYBOARD] Remove some obsoleted defines.
 * Built with e327e2060d37129fed5cb150cbae8ef943fc03f8 (or later).

-- Jan 05, 2020 20:59:52 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

12/31/2019

[MZ2500/CRTC] apply crtc patch (thanks Mr.Koucha-Youkan)

11/3/2019

[PC98RL] fix horizontal frequency

[PC98RL/DISPLAY] fix not to change horizontal frequency

[PC98RL/DISPLAY] fix cg window

[PC98RL/DISPLAY] fix multiple vram planes accessing

[PC9801/DISPLAY] fix vram word accessing (thanks Mr.Artane.)


10/28/2019

[X1/PSUB] improve tape recording

[X1/SUB] improve tape stopping

[X1/SUB] improve tape write-protected signal


10/22/2019

[X1/PSUB] improve tape end signal

[X1/SUB] improve tape end signal

-----

お楽しみあれ!
-- Ohta.
