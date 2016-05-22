** FM-7 series  emulator for common source code project. **
                                               May 23, 2016
		   K.Ohta <whatisthis.sowhat _at_ gmail.com>


1.背景
  今主流のFM-7系エミュレータであるXM7は非常に優れていますが、
  FOSSではなくクローズドソースコードです。
  しかし、これを許諾を取ってSDL/Agar Toolkitに移植してきま
  した[1]。
  このXM7/SDLは、Agar Toolkitに起因する多くのバグに悩まされ
  てきました。
  そうしてる間に、MESSがXM7を盗用していた問題が浮上して、オープン
  ソースなFM-7系エミュレータが欲しくなってきました。
  しかたないから、私が新規に作ろうと思いましたが、その時に
  Common Source Code Project(CSP)と言うGPLv2の枠組み [2] がある
  のを知ったので、使わせていただくことにしました。

  [1] https://github.com/Artanejp/XM7-for-SDL
  [2] http://homepage3.nifty.com/takeda-toshiya/

2.移植状況
  a. FM-7はほぼ動いてます。
  b. FM-77に関してもほぼ同様ですが、400ラインカードと2HD周りは
     全く実装できていません（現物がないのでどうしょうもない）
  c. FM-8は一応実装できましたが、テストが十分ではありません。
     バブルカセットも一応実装しましたが、まだ微妙に挙動が変です。
     B77でも読めるようにはしましたが、テストができてません。
     後、8インチFDは実装がまだです。
  d. FM77AVはほぼ動くようになりました。
  e. FM77AV40/EXについてもほぼ動くようになりました。
  f. AV系キーボードの隠しメッセージも入れました[3]。
  g. ステートセーブ機能を入れてみました。大体動くようです。
  h. プリンタエミュレーションで、単なるテキスト出力を実装しました。
     ついでに、電波新聞社のジョイスティック（ゼビウスについてきた奴）
     を実装しました。
  i. AV系キーボードの隠しブザーも入れました。
  j. FDDのつけ外しをサポートしました。最初に使うときは、"Connect 320KB FDD" にチェックを入れて、再起動してください。
  k. FM-8/7でテンキーで移動するゲームについて、自動で「5」もしくは「8」を入力するモードを新設しました。

  その他のことについては、RELEASENOTEやreadme_by_artane.txtを。

  [3] 私のほうがXM7よりも先にインプリメントしました(笑)

3. 動かし方
  以下のR@Mイメージが必要です
  以下のイメージがない場合には、代用R@Mが使用可能です。
  　 http://retropc.net/apollo/download/xm7/romset/index.htm
  
  FM-7 以降で必要なもの:
  BOOT_BAS.ROM : 512 bytes, BASIC ブートROM.
  BOOT_DOS.ROM : 512 bytes, DOS(NOT MS-DOS)ブートROM.
  FBASIC302.ROM
  FBASIC300.ROM
  FBASIC30.ROM : 31744 bytes, F-BASIC 3.0（上記３つの内いずれか）。
                 代用ROMが用意されています。.
  SUBSYS_C.ROM : 10240 bytes, サブシステムモニタ タイプC.

  追加ROMとして:
  KANJI.ROM
  KANJI1.ROM   : 131072 bytes, JIS第一水準漢字パターン.
  BOOT_MMR.ROM : 512 bytes, FM-77用の隠しブートROM(未チェック).

  FM77AVでは、更に以下のものが必要です:
  INITIATE.ROM : 8192 bytes, イニシエータROM.
                 これは、確実にFM77AVの物である必要があります。
		 77AV20/40系の物では動きません
  SUBSYSCG.ROM : 8192 bytes, キャラクターデータROM.
  SUBSYS_A.ROM : 8192 bytes, サブシステムモニタ タイプA.
  SUBSYS_B.ROM : 8192 bytes, サブシステムモニタ タイプB.

  追加ROM (FM77AV20/40向けのみ):
  KANJI2.ROM   : 131072 bytes, JIS第二水準漢字パターン.
  DICROM.ROM   : 262144 bytes, かな漢字変換辞書ROM.
  EXTSUB.ROM   : 49152 bytes, 拡張サブモニタ(77AV20以降)

  FM77AV20/40/EX/SXでは、INITIATE.ROMは、AV20/40/EX/SX固有のものを
  使ってください。

  辞書ROMを使う場合、以下のデータが作成されます :
  USERDIC.DAT  : 8192 bytes, かな漢字変換の学習データ.

4. 上流リポジトリ:
      https://github.com/Artanejp/common_source_project-fm7


お楽しみを!
-- K.Ohta.
