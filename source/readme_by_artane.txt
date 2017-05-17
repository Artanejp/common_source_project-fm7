** Qt porting for Common Source Code Project **
                                          May 18, 2017
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20170518

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

   a. Qt5 ツールキット。バイナリはQt 5.5基準でビルドしてあります。
   
   b. OpenGL, 多分、最低OpenGL 2.1は必要です。 (New!)
   
   c. gcc / g++ (5.0以降？)もしくは llvm clang / clang++ (3.5以降?)
      コンパイラツールチェーン。
      
   d. SDL2 (SDL 1.xではないので注意)
   
   e. CMake 2.8以降。
   
   f. ffmpegから、libavとlibswが必要です。 http://ffmpeg.org/ より。
   
   g. ffmpegは、それぞれのランタイムに必要なものをバンドルしてありますので、動かない時はインストールしてみてください。
      
   h. GNU/Linuxビルドでは、Qt5.5(Ubuntu 16.04LTS向け)もしくはQt5.7(Debian GNU/Linux sid向け)でビルドしてあります。
   
   * Windows もしくは GNU/Linux のcross tool chain (要Wine)で、MinGW (gcc6) と Qt 5.7 でのビルドができることを確認しました。
     
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

   *ToolTipsを付けました。(2017-01-24)
      
   *日本語に翻訳しました。(2017-01-24)
   
   *R@Mを $HOME/emu{Machine Name}/　に配置してください。(Windowsの場合は今の所 .\emu{Machine Name}\)。なお、このディレクトリは最初起動した後で作成されます。
   
   *設定ファイルは、$HOME/.config/emu{Machine Name}/ に書き込まれます。(Windowsの場合は今の所 .\.config\emu{Machine Name}\)
   
   *ステートセーブファイルは、$HOME/emu{Machine Name}/{Machine Name}.sta に書き込まれます。
   
   *キーコード変換テーブルファイルが、$HOME/.config/emu{Machine Name}/scancode.cfg に書き込まれます。
   
     書式は、カンマで区切られた16進データです(10進ではないので注意) .
     
     1カラム目はM$ ヴァーチャルキーコード。
     
     2カラム目はQtネィティブのスキャンキーコードです。
     
   *UI部分の共通コンポーネント (src/qt/gui) を共有ライブラリlibCSPgui.soにまとめました。
   
   *インストール用のBASHスクリプトを用意しました。src/tool/installer_unix.shです。
   
   *ROMと同じところに、特定のWAVファイル(VMによって異なる)を入れると、FDDのシーク音やテープのボタン音・リレー音を鳴らすことが出来ます。
   
   *ローマ字カタカナ変換支援機構が一部の機種に実装されてます。romaji_kana.ja.txt をお読みください。
    
5. 移植状況
   
   a.現在、Debian GNU/Linux "sid"と、Ubuntu Linux 16.04LTS "Xenial"
     の AMD64版でしかテストしていません。
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

ChangeLog:
* SNAPSHOT May 18, 2017
  * Upstream 2017-05-15
  * [UI/Qt] Fix unconnected event.
  * [UI/Qt] I forgot to add a "*.gz" extension to opening of Cartridges, CDs and Binaries.
  * [OSD/Qt] Add *_features_* functions. It needs for *NEW* device emulation.Pls. port to win32/ .
  * [EMU][Qt][MOUSE] Fix not effect with button-up.
  * [VM] LibCSPcommon_vm : Without #ifdef.
  * [VM] LibCSPcommon_vm : Add some devices.See src/vm/common_vm/CmakeLists.txt .
  * [VM] Fix FTBFSs with LLVM (4.0).
  * [VM/MC6809] Build shared library without libnewdev/. Use vm/mc6809.cpp and vm/mc6809_debugup.cpp . See, fm7.cpp.
  * [VM/FM7] Faster address transferring.
  * [VM/DEVICE] You should add "DECVICE::initialize()" to top of initialize().
  * [VM/DEVICE] Move some devices to shared lib.
  * [VM/DEVICE] Split MSM5832:: from MSM58321:: .
  * [VM/M6502] Split M6502 to N2A03 and M6502.
  * [VM/MB61VH010] Expect to be faster rendering.
  * [VM/MB61VH010][FM77AV] Re-order addrsss of read_data8() and write_data8(). You must re-build some VMs (FM77AV series etc.)
  * [CONFIG] Fix over loop.
  * Build with 5d424b2ecac58fe55570c2e9fc0db0edd0d94471 or later.

-- May 17, 2017 22:44:25 +0900 K.Ohta <whatisthis.sowhat@gmail.com>


本家の変更:

-----
5/15/2017

[X1TURBOZ] support SHARP X1turboZ
[X1TURBOZ/DISPLAY] support X1turboZ enhanced graphic modes
[X1TURBOZ/IOSUB] support analog palette access wait


5/13/2017

[WIN32/SCREEN] fix not to move window position unnecessarily

[VM/AM9511] support AM9511 (thanks Xep128)
[VM/DISK] support to specify raw track size
[VM/MB8877] support to specify raw track size
[VM/MC6844] support MC6844
[VM/MC6850] support MC6850
[VM/MSM58321] add read_signal()
[VM/MEMORY] improve that dma controller does not read/write memory mapped i/o
[VM/NOISE] fix not to restart from first sample if already playing noise

[YIS] support YAMAHA YIS


4/16/2017

[VM/IO] revert the fix in 4/15/2017
[VM/SN76489AN] revert the fix in 4/15/2017


4/15/2017

[COMMON] add muldiv_s32 and muldiv_u32 to multiple 32bit integer
[COMMON] fix functions to support symbols

[VM/IO] improve to use read/write_io8/16/32w() to get wait clock from device
[VM/SN76489AN] improve to return 32 + 16 * n for wait clock

[MZ700] improve correct cpu clock and frame rate
[MZ800] improve correct cpu clock and frame rate

-----


お楽しみあれ!
-- Ohta.
