** Qt porting for Common Source Code Project **
                                           Aug 02, 2015
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものと、Windows (Win32)向けに、Visual Studio 2015
   + Direct X9 + Direct Input 8でビルドしたもののセットです。

1. 背景
   CSPは、非常に優れた構造のエミュレータです（しかし、些か重くてコンパイラ
   がいい最適化をしないと重めですが）。
   しかし、このコードはM$ Visual C++依存の部分が非常に多いです。
   そこで、GNU/Linuxでこれを動かすためにQtに色々と移植していきましょう。
   と言う感じで作業をはじめました。

2. 最低限必要なもの(Qt版)
   a. Qt5 ツールキット
   b. OpenGL, 多分、最低OpenGL 2.0は必要です。
   c. gcc / g++ (4.7以降？)もしくは llvm clang / clang++ (3.5以降?)
      コンパイラツールチェーン。
   d. SDL2 (SDL 1.xではないので注意)
   e. CMake 2.8以降。

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
   ・R@Mを $HOME/emu{Machine Name/ に配置してください。
   　なお、このディレクトリは最初起動した後で作成されます。
   ・設定ファイルは、$HOME/.config/emu{Machine Name}/ に書き込まれます。

5. 移植状況
   a. 現在、Debian GNU/Linux "sid" の AMD64版でしかテストしていません。
   　 が、多分他のGNU/Linux OSやBSD系のOS (Mac含む) でもビルドすれば
   　 動くでしょう。WindowsとMinGWの組み合わせに関しては、CMakeがまだ
    　正しい設定をできてない状況です。
      Windowsの方は、Visual Studio 2013 か 2015 のCommunity Edition
      でビルドしてください。
   b. 今は、Qtの開発側が「Qt4おわりね」とアナウンスしたので、Qt4ではなく
      Qt5を使っています。
      添付してあるバイナリは、Qt 5.5でビルドしました。

   c.以下のマシンがQtに移植出来ています。(2015年7月23日現在)
     ・Ascii MSX1/MSX2 (not PX-7).
     ・Casio FP-1100 .
     ・Casio FP-200 .
     ・Epson HC-20/40/80.
     ・Fujitsu FM-7/77/AV . (→ READ readme_fm7.jp.txt)
     ・Fujitsu FM16pi .
     ・Fujitsu FM-R50(i286/i386/i486)/R60/R70/R80/R250/R280 (Not tested enough).
     ・Gijyutu hyouronsya babbage2nd.
     ・NEC PC-6001/mk2/mk2SR .
     ・NEC PC-6601/SR .
     ・NEC PC8001mk2SR (Not tested enough).
     ・NEC PC8801MA .
     ・NEC PC-9801/E/U/VF/VM .
     ・NEC PC98DO .
     ・NEC PC98LT/HA .
     ・NEC HE PC-ENGINE.
     ・NEC TK-80BS .
     ・NEC HE PC Engine.
     ・Tomy PYUTA.
     ・Sega Game Gear/Master System (Mark3).
     ・Sharp X1/turbo/turboZ/Twin.
     ・Sharp MZ-700/800/1500 .
     ・Sharp MZ-80A/K/1200 .
     ・Sharp MZ-80B/2200/2500 .
     ・Shinko Sangyou YS-6464a .
     ・Toshiba J-3100SL .
     ・Z80 TV Game (Hand made)

   d. Linux用ビルドでは、GCC 5.1をリンク時最適化(LTO)モードで使っています。
   
6. Upstream repositry:
      https://github.com/Artanejp/common_source_project-fm7

7. Upstream (Takeda Toshiyaさんのオリジナル) 
      http://homepage3.nifty.com/takeda-toshiya/

お楽しみあれ!
-- Ohta.
