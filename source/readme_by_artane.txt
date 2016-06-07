** Qt porting for Common Source Code Project **
                                          June 08, 2016
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
1. 背景
   CSPは、非常に優れた構造のエミュレータです（しかし、些か重くてコンパイラ
   がいい最適化をしないと重めですが）。
   しかし、このコードはM$ Visual C++依存の部分が非常に多いです。
   そこで、GNU/Linuxでこれを動かすためにQtに色々と移植していきましょう。
   と言う感じで作業をはじめました。

2. 最低限必要なもの(Qt版)
   a. Qt5 ツールキット
   b. OpenGL, 多分、最低OpenGL 3.0は必要です。 (New!)
   c. gcc / g++ (4.7以降？)もしくは llvm clang / clang++ (3.5以降?)
      コンパイラツールチェーン。
   d. SDL2 (SDL 1.xではないので注意)
   e. CMake 2.8以降。
   f. ffmpegから、libavとlibswが必要です。 http://ffmpeg.org/ より。
   
   * Windows もしくは GNU/Linux のcross tool chain (要Wine)で、
     MinGW と Qt 5.5.1 でのビルドができることを確認しました。
     
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

   c.上流の2016-04-17現在でのPX7以外の全ての仮想マシンがQtに移植出来ています。
   d. Linux用ビルドでは、GCC 6をリンク時最適化(LTO)モードで使っています。
   
6. Upstream repositry:
      https://github.com/Artanejp/common_source_project-fm7
      https://www.pikacode.com/Artanejp/common_source_project-fm7/

7. Upstream (Takeda Toshiyaさんのオリジナル) 
      http://homepage3.nifty.com/takeda-toshiya/

お楽しみあれ!
-- Ohta.
