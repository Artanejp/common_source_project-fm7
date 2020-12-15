<H2>** HowTo build/install Qt porting for Common Source Code Project **</H2>
<H3>-- CSP/Qt(略称)のビルドとインストールの仕方<H3>
<div align="right">
<H3>Oct 08, 2020<BR>
K.Ohta <whatisthis.sowhat _at_ gmail.com></H3>
</div>

はじめに
========
この文章では、Qt porting for Common Source Code Project (CSP/Qt)のビルド手順について記述します。


用意すべきもの(標準)
=======
--GCC又はCLANGなどの、コンパイラツールチェイン

--CMake (3.9以上推奨）

- 以下の開発ライブラリ(大抵のGNU/Linux OSやBSD系のOSなどではパッケージマネージャ(dfnやaptなど）で入るはずです。)が必要になります。**なお、現状、表示にOpenGL2.1以上かOpenGL ES2以上が必要になります**。
  - QT5 (QTCore, QtOpenGL, QtNetworkなど) <https://www.qt.io/>
  - SDL2 <https://libsdl.org/>
  - libAV <http://ffmpeg.org/>
  - zlib <http://zlib.net/>
  　その他、色々必要になります。
  
- Windows向けのビルドの場合、OpenGL ESを実装した[Angle Project](<https://github.com/Microsoft/angle>)が実行に必要になるかもしれません。これは、Google Chromeブラウザのオープンソース版である[Chromium Project](<http://www.chromium.org/>)のWindow (x86 32bit)ビルドの中にある、libEGL.dllとlibGLESv2.dllを使えばどうにかなります。

## なお、Windows向けのビルドをするための環境を、Dockerの形でビルドして使うと便利です。
## Dockerレポジトリは <https://hub.docker.com/r/artanejp/mingw-w64-llvm10-ubuntu20.04>
## 元のDockerfileは <https://github.com/Artanejp/llvm-mingw>

ビルド手法
=========

既にgitからcloneしたりリリースに添付されたソースコードを解凍してビルドする場合、

$ `cd ${SRCROOT}/source`
$ `mkdir build`
$ `cd build/`
$ `cmake ..`
$ `make`

とすればとにかくのビルドが可能ですが、**標準的な設定パラメータを収めたシェルスクリプトを、${SRCROOT}/source/sample-scripts/ 以下に入れてあります（まだまだ追加するかも）**。

このサンプルを使って、
$ `cd ${SRCROOT}/source`
$ `mkdir build`
$ `cd build/`
$ `cp ../sample-scripts/${SCRIPT_NAME}.sh .`
とビルドディレクトリに取って来た後で、
$ `sh ./${SCRIPT_NAME}.sh`
などとしてブートストラップ設定をして、CMakeがエラー起こさなかったら、
$ `make {色々オプション}`
としてビルドしてみましょう。

インストール手法
===============

普通は、 # `make install`で可能なはずです。Windowsビルドの場合は、まだToDoです。

                                      Last Update: Dec 16, 2020 00:07:53
