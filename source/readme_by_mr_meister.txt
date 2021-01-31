# eX1改

2020-09-26 by Meister

## 概要

Common Source Code Project( http://takeda-toshiya.my.coocan.jp/ ) のX1エミュレータ eX1に
若干の機能追加を施したものです。

本家が取り込んでくれることを期待し，差分のみ公開します。

## 追加した機能

### BASIC ROMボード CZ-8RB

クリーンコンピュータのX1にROM起動のBASICを追加するものです。
2DDのべたイメージ(320KB)をCZ8RB.ROMというファイル名で置いておくとIPLが読み込んで起動します。
正規品のROMの中身はCZ-8CB01の起動ロゴだけが変更されたCZ-8RB01らしいのでCZ-8CB01の起動ディスクから
イメージを作ればほぼ同じものが手に入るでしょう。

BASICにこだわらず，S-OSやLSX-Dodgersなど任意のOSをFDDよりも高速に起動するために活用することができます。

拙作の互換IPL開発のために必要となり，実装しました。実機を持っていませんがX1センター( http://www.x1center.org/ )や
http://www.retropc.net/mm/x1/CZ-8RB/index.html で調査されている内容を参考にしています。


### 偽スーパーインポーズ

ネタ機能です。エミュレータの背景(黒色)を透かしてスーパーインポーズします。
プログラミングをしながらテレビが見られる！これぞパソコンテレビ X1！(専用ディスプレイ CZ-800D は不要です)

![shot2a](https://user-images.githubusercontent.com/17338071/94328733-cbd19c80-ffef-11ea-9e23-de07fe316f23.jpg)

(NHK+って便利ですね)

![shot2](https://user-images.githubusercontent.com/17338071/94328735-dc821280-ffef-11ea-9b4d-4d051b7ad04b.jpg)

ソースコードの#ifdef USE_SUPERIMPOSE周辺が変更点です。
SetLayeredWindowAttributesで透明色を指定するだけですが，RGB=(0,0,0)を指定するとGUI操作ができなくなる不具合があり
黒の描画色をわざわざRGB=(1,1,1)にしています。

