<H2>** Qt porting for Common Source Code Project **</H2>
<div align="right">
<H3>Sep 09, 2016<BR>
K.Ohta <whatisthis.sowhat _at_ gmail.com></H3>
</div>

*If you can't read Japanese, [english writing is here](/README.en.md/).*
======

概要
======

   このパッケージは、Common Source Code Project (以下、CSP)をQt5に移植したものです。
   
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/ 以下

## 追加情報:
   
各機種バイナリーは、osdn.net　もしくはミラーサイトより入手可能です。
    
    https://osdn.net/projects/csp-qt/  
   
    https://osdn.net/projects/csp-qt/releases/

をチェックしてください。

## 【おねがい】

doc/以下の文書で日本語しかなかったものを英語に翻訳していますが、機械翻訳を使ってるのであやしいです。英語の上手い方、校正などお願いします m(_ _)m

LICENCE
======

[GPL Version 2](http://www.opensource.jp/gpl/gpl.ja.html).

背景
====

   CSPは、非常に優れた構造のエミュレータです（しかし、些か重くてコンパイラがいい最適化をしないと重めですが）。
   
   しかし、このコードはM$ Visual C++依存の部分が非常に多いです。

   そこで、GNU/Linuxでこれを動かすためにQtに色々と移植していきましょう。と言う感じで作業をはじめました。

最低限必要なもの(Qt版)
====

   * Qt5 ツールキット。Qt 5.5以降を推奨します。
   
   * OpenGL, 多分、最低OpenGL 2.1は必要です。（注：ひょっとしたら、OpenGLES2以降ならば動くように変えるかも知れない）
   
   * gcc / g++ (5.0以降？)もしくは llvm clang / clang++ (3.5以降?)コンパイラツールチェーン。MS Visual StudioのC++でも大体はビルドできると思いますが、未確認。
      
   * SDL2 (SDL 1.xではないので注意)
   
   * CMake 2.8以降。
   
   * ffmpegから、libavとlibswが必要です。 http://ffmpeg.org/ より。
   
   * ffmpegは、それぞれのランタイムに必要なものをバンドルしてありますので、動かない時はインストールしてみてください。
      
   * GNU/Linuxビルドでは、Qt5.5(Ubuntu 16.04LTS向け)もしくはQt5.9(Debian GNU/Linux sid向け)でビルドしてあります。
   
    ** Windows もしくは GNU/Linux のcross tool chain (要Wine)で、MinGW (gcc6) と Qt 5.7 でのビルドができることを確認しました。
     
## TIPS:
   
     * Windows等で動かした時に、画面の書き替えが表示されない場合は、環境変数 QT_OPENGL を software にしてみてください。（例えば、WindowsをVirtualBoxのゲストで使ってる場合など）
       
     * Windows版バイナリには、ソフトウェアレンダリングのopengl32.dllが添付されてますが、最近のパソコンの専用GPUドライバなら、もっと程度のいいOpenGLが入ってるはずです。添付版opengl32.dllを適当な名前に変更して動くかどうか試してみて下さい。
     
ビルドの方法
==

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

## Qt固有の話(Windows除く)
==

   * ToolTipsを付けました。(2017-01-24)
      
   * 日本語に翻訳しました。(2017-01-24)
   
   * R@Mを $HOME/emu{Machine Name}/　に配置してください。(Windowsの場合は今の所 .\emu{Machine Name}\)。なお、このディレクトリは最初起動した後で作成されます。
   
   * 設定ファイルは、$HOME/.config/emu{Machine Name}/ に書き込まれます。(Windowsの場合は今の所 .\.config\emu{Machine Name}\)
   
   * ステートセーブファイルは、$HOME/emu{Machine Name}/{Machine Name}.sta に書き込まれます。
   
   * キーコード変換テーブルファイルが、$HOME/.config/emu{Machine Name}/scancode.cfg に書き込まれます。
   
     ** 書式は、カンマで区切られた16進データです(10進ではないので注意) .
     
     ** 1カラム目はM$ ヴァーチャルキーコード。
     
     ** 2カラム目はQtネィティブのスキャンキーコードです。
     
   * UI部分の共通コンポーネント (src/qt/gui) を共有ライブラリlibCSPgui.soにまとめました。
   
   * インストール用のBASHスクリプトを用意しました。src/tool/installer_unix.shです。
   
   * ROMと同じところに、特定のWAVファイル(VMによって異なる)を入れると、FDDのシーク音やテープのボタン音・リレー音を鳴らすことが出来ます。
   
   * ローマ字カタカナ変換支援機構が一部の機種に実装されてます。
    
移植状況
==

   * 現在、Debian GNU/Linux "sid"と、Ubuntu Linux 16.04LTS "Xenial"の AMD64版、後はWindowsのMinGWでしかテストしていません。
   
   　が、多分他のGNU/Linux OSやBSD系のOS (Mac含む) でもビルドすれば動くでしょう。
   
     Windows もしくは GNU/Linux(要Wineとbinfmt-support)上でのMinGWとQt community edition でのビルドが通るようになりました。
      
   * 今は、Qtの開発側が「Qt4おわりね」とアナウンスしたので、Qt4ではなくQt5を使っています。
   
      添付してあるバイナリは、Qt 5.5でビルドしました(が、Qt 5.1以降なら動くはずです)。

   * Linux用ビルドでは、GCCをリンク時最適化(LTO)モードで使っています。
   
   * MZ-2500のソケット機能を実装してみていますが、マトモにテストできてません(；´Д｀)
   
Upstream repositry:
==

      https://github.com/Artanejp/common_source_project-fm7
      
      https://osdn.net/projects/csp-qt/scm/git/common_source_project-fm7

Project Page:
==

      https://osdn.jp/projects/csp-qt/

Upstream (Takeda Toshiyaさんのオリジナル)
==

      http://takeda-toshiya.my.coocan.jp/


Special thanks to:
==

  Ryu Takegami : eFM-8/7/77/AV/40/EX のデバッグに協力していただいています。



Have fun!

--- Ohta.
 
&copy 2018 Toshiya Takeda
&copy 2018 K.Ohta <whatisthis.sowhat _at_ gmail.com>

