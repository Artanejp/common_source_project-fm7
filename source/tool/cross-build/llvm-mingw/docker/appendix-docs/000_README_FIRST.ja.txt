*** UTF-8 ***
このDockerイメージは、Common Source Code Project for Qt (*1)
のWindowsビルドを作成するための物です。
Ubuntu 18.10 "Cosmic Cuttlefish"のAMD64ベースです。

必要なツールチェインは/opt/llvm-mingw/ にあります(*2)。
ライブラリなどは、/usr/local/i586-mingw-msvc/ に揃えてありま
すが、Qtの再ビルドに関しては、DirectX SDKを別途インストールする
必要があります。
又、Angleproject(OpenGL ES)に関しては、Google chromiumから
持ち込んでいます。

お楽しみあれ。

Sep 26, 2019 K.Ohta <whatisthis.sowhat _at_ gmail.com>

(*1) https://github.com/Artanejp/common_source_project-fm7
(*2) https://github.com/Artanejp/llvm-mingw
     https://github.com/mstorsjo/llvm-mingw
     