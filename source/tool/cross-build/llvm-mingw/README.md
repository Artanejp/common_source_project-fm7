**Sorry still not writing in English, this written only Japanese.**

<H2>Scripts for LLVM cross toolchain for Windows (ix86, x86_64, armv7 aarch64)</H2>
<div align="right">
<H3>Oct 21, 2020<BR>
K.Ohta <whatisthis.sowhat@gmail.com></H3>
</div>


## About

    このディレクトリは、MinGW-w64用のクロスツールチェインをビルドするスクリプトです。
    
    LLVM CLANGベースのGNU/Linux上で動くものが、ビルド可能です(BSD他は未確認)

## How to build

     依存するファイルは以下のとおりです(ToDo):
     
     
     これらをインストールした後、
     
     $ cp -ar . foo/
     
     $ cd foo
     
     $ sudo ./bootstrap.sh
     
     **現在、sudo(又はfakerootなど)が必要になります**。

     で、　/opt/llvm-mingw-11/ 以下に自動的にインストールされます。


## TIPS     
   
     buildvars.dat である程度の設定が可能です。
     
     詳しくは、bootstrap.shをお読みください。