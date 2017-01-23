** Qt porting for Common Source Code Project **
                                          Jan 24, 2017
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_20170124

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
   c. gcc / g++ (4.7以降？)もしくは llvm clang / clang++ (3.5以降?)
      コンパイラツールチェーン。
   d. SDL2 (SDL 1.xではないので注意)
   e. CMake 2.8以降。
   f. ffmpegから、libavとlibswが必要です。 http://ffmpeg.org/ より。
   g. ffmpegは、それぞれのランタイムに必要なものをバンドルしてあります
      ので、動かない時はインストールしてみてください。
   h. GNU/Linuxビルドでは、Qt5.5.1でビルドしてあります(Ubuntu 16.04LTS向け)
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
   ・ToolTipsを付けました。(2017-01-24)
   ・日本語に翻訳しました。(2017-01-24)
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
12/31/2016

[COLECOVISION/KEYBOARD] fix not to raise irq when joystick is pressed
[COLECOVISION/KEYBOARD] fix joystick/tenkey inputs
[COLECOVISION/KEYBOARD] fix to save/load tenky enabled status
[SC3000/MEMORY] support 32KB+16KB or 128KB ROM carts


12/30/2016

[COLECOVISION] support COLECO ColecoVision (thanks Mr.tanam)


12/29/2016

[RESOURCE] recompress png files in https://tinypng.com/

[EMU/DEBUGGER] support p command (trace one opcode, step-over)

[MZ80K/MEMORY] support V-GATE signal (thanks Mr.Suga)
[PASOPIA/DISPLAY] fix graphic color in screen 2 mode (thanks Mr.Kamei)
[SMB80TE] support SHARP SM-B-80TE


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
