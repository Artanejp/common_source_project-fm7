** Qt porting for Common Source Code Project **
                                          Oct 27, 2016
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20161027

   追加情報:
    　各機種バイナリーは、osdn.jp　もしくはミラーサイトより入手可能です。
   　https://osdn.jp/projects/csp-qt/  
     https://osdn.jp/projects/csp-qt/releases/　をチェックしてください。

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
   a. Qt5 ツールキット
   b. OpenGL, 多分、最低OpenGL 2.1は必要です。 (New!)
   c. gcc / g++ (4.7以降？)もしくは llvm clang / clang++ (3.5以降?)
      コンパイラツールチェーン。
   d. SDL2 (SDL 1.xではないので注意)
   e. CMake 2.8以降。
   f. ffmpegから、libavとlibswが必要です。 http://ffmpeg.org/ より。
   g. ffmpegは、それぞれのランタイムに必要なものをバンドルしてあります
      ので、動かない時はインストールしてみてください。
   h. GNU/Linuxビルドでは、Qt5.3でビルドしてあります(Ubuntu 16.04LTS向け)
   * Windows もしくは GNU/Linux のcross tool chain (要Wine)で、
     MinGW と Qt 5.7 でのビルドができることを確認しました。
     
   * TIPS:
     Windows等で動かした時に、画面の書き替えが表示されない場合は、
     環境変数 QT_OPENGL を software にしてみてください。（例えば、
     WindowsをVirtualBoxのゲストで使ってる場合など）
     
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

4. Qt固有の話(Windows除く)
   ・R@Mを $HOME/emu{Machine Name}/ に配置してください。(Windowsの場合は今の所 .\emu{Machine Name}\)
   　なお、このディレクトリは最初起動した後で作成されます。
   ・設定ファイルは、$HOME/.config/emu{Machine Name}/ に書き込まれます。(Windowsの場合は今の所 .\.config\emu{Machine Name}\)
   ・ステートセーブファイルは、$HOME/emu{Machine Name}/{Machine Name}.sta に書き込まれます。
   ・キーコード変換テーブルファイルが、$HOME/.config/emu{Machine Name}/scancode.cfg に書き込まれます。
     書式は、カンマで区切られた16進データです(10進ではないので注意) .
     1カラム目はM$ ヴァーチャルキーコード。
     2カラム目はQtネィティブのスキャンキーコードです。
   ・UI部分の共通コンポーネント (src/qt/gui) を共有ライブラリlibCSPgui.soにまとめました。
   ・インストール用のBASHスクリプトを用意しました。src/tool/installer_unix.shです。
   ・ROMと同じところに、特定のWAVファイル(VMによって異なる)を入れると、FDDのシーク音やテープのボタン音・リレー音を鳴らすことが出来ます。
    ・ローマ字カタカナ変換支援機構が一部の機種に実装されてます。romaji_kana.ja.txt をお読みください。
    
5. 移植状況
   a. 現在、Debian GNU/Linux "sid" の AMD64版でしかテストしていません。
   　 が、多分他のGNU/Linux OSやBSD系のOS (Mac含む) でもビルドすれば
   　 動くでしょう。
      Windows もしくは GNU/Linux(要Wineとbinfmt-support)上でのMinGWと
      Qt community edition でのビルドが通るようになりました。
      安定したWindowsビルドを必要な方は、Visual Studio 2013 か 2015 のCommunity Edition
      でビルドしてください。（もう少ししたら、MinGWに切り替えようとは思ってます。)
      
   b. 今は、Qtの開発側が「Qt4おわりね」とアナウンスしたので、Qt4ではなく
      Qt5を使っています。
      添付してあるバイナリは、Qt 5.5でビルドしました(が、Qt 5.1以降なら動くはずです)。

   c. Linux用ビルドでは、GCC 6をリンク時最適化(LTO)モードで使っています。
   d. MZ-2500のソケット機能を実装してみていますが、マトモにテストできてません(；´Д｀)
   
6. Upstream repositry:
      https://github.com/Artanejp/common_source_project-fm7
      https://www.pikacode.com/Artanejp/common_source_project-fm7/

7. Project Page:
      https://osdn.jp/projects/csp-qt/

8. Upstream (Takeda Toshiyaさんのオリジナル) 
      http://takeda-toshiya.my.coocan.jp/

Changes:

* SNAPSHOT October 27, 2016
  * Upstream 2016-04-13
  * [GENERAL] Add ROMAJI-KANA input assistant feature. see romakana.[en|ja].txt .
  * [VM/SOUND] Fix wrong rendering period for PCM1BIT with some VMs.  Maybe fixed issue of http://hanabi.2ch.net/test/read.cgi/i4004/1430836648/775 .
  * [VM/EVENT] Re-entrant set_realtime_render().
  * Built with FFMPEG(libav) 3.1.5 .
  * Built with 006cc1d851483ea84fc5a3f4fa58cbf03302c49b or later.

-- Oct 27, 2016 17:01:21 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT October 18, 2016
  * Upstream 2016-04-13
  * [VM/EVENT] Sound devices: Do mix_sound() dynamically, not do per a sample:
  *            To reduce usage of host CPU. See event.[cpp|h] and device.h .
  * [VM/FM7] Non-FM77AV*: Fix not clear active_page, fixed crash randomly.
  * Built with 004920711399d430ead55e59c948e7fb7a04a402 or later.

-- Oct 18, 2016 11:22:05 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT October 10, 2016
  * Upstream 2016-04-13
  * [VM] Add pseudo sounds (i.e. FDD seek), need sound files (even not WAV file at Qt porting), see doc/VMs/foo.txt . 
  * [VM][DATAREC][FDCs] Update STATE_VERSION of some devices.
  * [VM/FM7] VRAM: Make gcc using SIMD (when optimize options have set) to be faster rendering.
  * [VM/X1,FM7] load_state(): Keep backward compatibility to SNAPSHOT 20160923.
  * [VM/EVENT] Extend permitted sound events to 32 to use sound files (reserved feature).
  * [VM/EVENT] Update MAX_SOUND from 8 to 16.
  * [Qt/GUI] Win32: Fix crash launching with gdb/Mingw32 or gdb/Cygwin. See http://hanabi.2ch.net/test/read.cgi/i4004/1430836648/750 .
  * [Build/Linux] Fix wrong flags with LTO.
  * [Build] Non-Win32 : Ready to build with llvm clang (later than 3.7?).
  * [Build/GENERAL] Separate compiler depended params to source/build-cmake/param/ .
  * Build with commit d100ae8323d6657fe37ac44a69abc8da470b77ad (or later).

-- Oct 10, 2016 05:22:18 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT September 23, 2016
  * Upstream 2016-04-13
  * [DOC] Translated some documents written in Japanese only to English using Google-Translate, still not enaugh to be corrected.
  * [Qt/WIN32] Fix crash on startup (maybe true) caused by (´∀｀)＜ぬるぽ　ヽ(・∀・)ノ┌┛ガッΣ(ノ｀Д´)ノ
  * [VM] Add original name and role to all device and VM components.
  * [VM/FM7] JOYSTICK: Fix not redirect triggers.
  * [VM/FM7][MB8877/DISK] Workaround for RIGLAS/FM-7. Perhaps, below have side-effect, add special-disk to RIGLAS FM.
  ** Don't "NOT READY" even stopping motor.
  ** Transfer even stopping motor.
  * [EVENT] Remove logging on cancel event.
  * [Qt/Debugger] Not hang-up when Opening debugger and exit emu{foo}. This is temporally workaround.
  * [Qt/Debugger] Don't close with close button, this is temporally workaround.
  * [UI/Qt] Fix selecting printers.
  * [VM/PC9801] Add prepair of using ITF ROM, this is still only preparation.
  * Built with GIT 8cdb1dbebf95d7371e514886c23edc6c9401848a (or later).

-- Sep 23, 2016 19:48:12 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT September 09, 2016
  * Upstream 2016-04-13
  * [LOGGER] Add extend logger.
  * [LOGGER/Qt] Rename agar_logger.[cpp|h] to csp_logger.[cpp|h] .
  * [VM] Ready to enable log per device.
  * [VM] Update emu->out_debug_log to [DEVICE]->out_debug_log .
  * [UI/Qt] Add log viewer window.
  * [Qt/EMUTHREAD] Move commonly blocks to gui/ .
  * [Qt/KEY] Use queue for keyin/out.
  * [VM/OSD] Add socket (networking) featuers, but still not test (；´Д｀)
  * [VM/FM77AV] MB61VH010 : Faster drawing lines.
  * [VM/FM7] MAINIO: Comment out logging around FIRQ, temporally.
  * [OSD/KEYBOARD] Fix handling around SHIFT for some machines(i.e. PC8801).
  * [MOVIE_SAVER] Use SIMD to transfer a picture OSD(VM)->MOVIE_SAVER .
  * [DOC] Update updtream's URL, moved to takeda-toshiya.my.coocan.jp .
  * Built with GIT 1884b5247665d71c06fc6590b17434c5f3350ad5 (or later).

-- Sep 09, 2016 01:02:51 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT August 19, 2016
  * Upstream 2016-04-13
  * [WIN32] Replace libICU to homebrew, fix not starting.
  * [OSD/MOVIE_SAVER] Fix frames to enqueue to MOVIUE_SAVER, as if VIDEO FPS >= RECORD FPS.
  * [MOVIE_LOADER] Sync A/V on playing MOVIE.
  * Built with GIT 77380a77b25ca06965b912c84509e5c91085aeb1 (or later) .

-- Aug 19, 2016 23:12:36 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT August 15, 2016
  * Upstream 2016-04-13
  * [OSD][MOVIE_SAVER] Maybe correctness frame(s) counting with 60fps ヽ(=´▽=)ﾉ
  * [MOVIE_SAVER] Fix sometimes crashing when stop to save movie.
  * [PX7/MOVIE_LOADER] Sound Laser Disc, but not be smooth.
  * [MOVIE_SAVER][PC8801][PC9801] Adjust sound frequency when OVERRIDE_48000Hz , still choppy.
  * Built with GIT 3f4c809912dc92cdeb34d8ecdebe0087aa7f37b3 (or later) .
 
-- Aug 16, 2016 02:40:55 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

* SNAPSHOT August 09, 2016
  * Upstream 2016-04-13
  * [PX7] Add movie loader for LD, but not tested enough yet.
  * [MOVIE_SAVER/OpenGL] Add locking around drawing buffer.
  * [BUILD] Update FFMPEG to 3.1.1 .
  * [FFMPEG/LINUX] Enable OpenCL for ffmpeg @linux.You need libOpenCL.so.1 to use homebrew build.
  * [FFMPEG/WIN32] Enable DXVA2 for video-decoding accelerator.
  * [MOVIE_SAVER] Synchronous OPEN->ENCODEING->CLOSING.
  * [UI/MOVIE_SAVER] DO NOT change state of "Save as movie" without *real* starting/stopping.
  * [Draw/OpenGL] Optimize GLSL's by https://github.com/aras-p/glsl-optimizer .
  * Built with GIT fc7a03a9337287414e00777464ab273c4f44ea44 (or later) .
 
-- Aug 10, 2016 01:42:51 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

Upstream changes:
-----
4/13/2016

[WINMAIN] improve auto key for the case to switch upper/lowercase with capslock
[EMU/DEBUGGER] fix issue that u command may cause the infinite loop

[VM/DATAREC] support to detect the frequency of signal
[VM/DATAREC] fix to always adjust zero position of wave signal
[VM/UPD1990A] fix not to check clk signal is low when stb signal is raised
[VM/UPD7810] support debugger
[VM/UPD7810] support MOV A,S opecode
[VM/UPD7810] fix not to change V register (thanks PockEmul)

[PC2001] support NEC PC-2001
[PC2001] fix cpu clock
[PC2001] support beep sound
[PC2001/IO] support rtc control signals
[YALKY] support Yuasa Kyouiku System YALKY
[YALKY/IO] improve data recorder controller

-----


お楽しみあれ!
-- Ohta.
