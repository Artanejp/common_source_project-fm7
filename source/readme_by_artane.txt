** Qt porting for Common Source Code Project **
                                         September 26, 2020
	      K.Ohta <whatisthis.sowhat _at_ gmail.com>

* If you can't read Japanese, read readme.qt.txt .

0. 概要
   このパッケージは、Common Source Code Project (以下、CSP)
   をQt5に移植したものです。
   バイナリはGNU/Linux(64bit)用とMinGW (32bit Windows)用を
   用意しています。
   
   ソースコード：
   
     https://github.com/Artanejp/common_source_project-fm7/releases/tag/SNAPSHOT_2020926

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

   a. Qt5 ツールキット。Qt 5.5以降を推奨します。
   
   b. OpenGL, 多分、最低OpenGL 2.1は必要です。（注：ひょっとしたら、OpenGLES2以降ならば動くように変えるかも知れない）
   
   c. gcc / g++ (5.0以降？)もしくは llvm clang / clang++ (3.5以降?)
      コンパイラツールチェーン。
      
   d. SDL2 (SDL 1.xではないので注意)
   
   e. CMake 2.8以降。
   
   f. ffmpegから、libavとlibswが必要です。 http://ffmpeg.org/ より。
   
   g. ffmpegは、Windowsに関してはバンドルしてありますので、動かない時はインストールしてみてください。
      
   h. Qt5.5(Ubuntu 16.04LTS向け)もしくはQt5.10(Win32とDebian GNU/Linux sid向け)でビルドしてあります。
   
   i. 表示基盤のデフォルトが、OpenGL ES2.0になりました。コマンドラインオプション --opengl で変更が可能です(--helpで参照)

   j. Windows のビルドを、Docker環境上のLLVM CLANG (9) にしました。例外処理に関して、MinGW-w64のgccは非常に遅い方法を取ってるためです（Borlandが悪いのですが）。
     詳細は、 https://github.com/Artanejp/llvm-mingw と https://hub.docker.com/r/artanejp/llvm-mingw64-ubuntu-cosmic を参照して下さい。
   
   * Windows もしくは GNU/Linux のcross tool chain (要Wine)で、MinGW (gcc6) と Qt 5.10 でのビルドができることを確認しました。

   * TIPS:
   
     * Windows等で動かした時に、画面の書き替えが表示されない場合は、環境変数 QT_OPENGL を software にしてみてください。（例えば、
       WindowsをVirtualBoxのゲストで使ってる場合など）
       
     * Windows版バイナリには、ソフトウェアレンダリングのopengl32.dllが添付されてますが、最近のパソコンの専用GPUドライバなら、
       もっと程度のいいOpenGLが入ってるはずです。
       添付版opengl32.dllを適当な名前に変更して動くかどうか試してみて下さい。
     
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

   * 設定ファイル(scancode.cfg と foo.ini)は、"~/.config/CommonSourceCodeProject/emufoo/" (Windowsの場合は".\CommonSourceCodeProject\emudfoo\" ) におかれます（移動しました）。

   * BIOSや効果音WAVやセーブステートは、、"~/CommonSourceCodeProject/emufoo/" (Windowsの場合は".\CommonSourceCodeProject\emudfoo\" ) におかれます（移動しました）。
   
   * 全ての記録物(スクリーンショットや動画や録音WAV）は、*当面の間* "~/CommonSourceCodeProject/emufoo/" (Windowsの場合は".\CommonSourceCodeProject\emudfoo\" ) におかれます。

   * ToolTipsを付けました。(2017-01-24)
      
   * 日本語に翻訳しました。(2017-01-24)
   
   
   * キーコード変換テーブルファイルが、$HOME/.config/CommonSourceCodeProject/emu{Machine Name}/scancode.cfg に書き込まれます。
   
     書式は、カンマで区切られた16進データです(10進ではないので注意) .
     
     1カラム目はM$ ヴァーチャルキーコード。
     
     2カラム目はQtネィティブのスキャンキーコードです。
     
   * UI部分の共通コンポーネント (src/qt/gui) を共有ライブラリlibCSPgui.soにまとめました。
   
   * インストール用のBASHスクリプトを用意しました。src/tool/installer_unix.shです。
   
   * ROMと同じところに、特定のWAVファイル(VMによって異なる)を入れると、FDDのシーク音やテープのボタン音・リレー音を鳴らすことが出来ます。
   
   * ローマ字カタカナ変換支援機構が一部の機種に実装されてます。romaji_kana.ja.txt をお読みください。
    
5. 移植状況
   
   a.現在、Debian GNU/Linux "sid"と、Ubuntu Linux 16.04LTS "Xenial"
     の AMD64版でしかテストしていません。
   　が、多分他のGNU/Linux OSやBSD系のOS (Mac含む) でもビルドすれば
   　動くでしょう。
     Windows もしくは GNU/Linux(要Wineとbinfmt-support)上でのMinGWと
     Qt community edition でのビルドが通るようになりました。
      
   b. 今は、Qtの開発側が「Qt4おわりね」とアナウンスしたので、Qt4ではなく
      Qt5を使っています。
      添付してあるバイナリは、Qt 5.5でビルドしました(が、Qt 5.1以降なら動くはずです)。

   c. Linux用ビルドでは、GCCをリンク時最適化(LTO)モードで使っています。
   d. MZ-2500のソケット機能を実装してみていますが、マトモにテストできてません(；´Д｀)
   
6. Upstream repositry:
      https://github.com/Artanejp/common_source_project-fm7
      
      https://osdn.net/projects/csp-qt/scm/git/common_source_project-fm7

7. Project Page:
      https://osdn.jp/projects/csp-qt/

8. Upstream (Takeda Toshiyaさんのオリジナル) 
      http://takeda-toshiya.my.coocan.jp/


Special thanks to:
  Ryu Takegamiさん     : eFM-8/7/77/AV/40/EX のデバッグに協力していただいています。
  はせりんさん         : eFM-8/7/77/AV/40/EX のデバッグに協力していただいています。
  Ootake 開発者の皆さん: ePCENGINEの改善のヒントをソースコードから勉強させていただいてます。
  Soji Yamakawaさん    : eFM-Townsの開発で「津軽」を参考にさせていただいたり、
                         アドバイスを頂いたりしています。
Changes:

* 前の変更点をお読みになる場合には、ChangeLogと000_gitlog.txtをお読み下さい。

* SNAPSHOT September 26, 2020
 * Upstream 2020-04-06.
 * [FMTOWNS/DMAC] Bootable TownsOS v1.1L30 based softwares.
                  Fix around DMA address mask.
		  See source/src/vm/fmtowns/00_status.ja.md.
 * [General] Now, version of all DLLs/SOLIBs are 3.0.x.		  
 * [DEVICE] Change API: special_reset(num).
            This aimes to support FM-Towns's multiple special resetting.
 * [I18N] Prepare to support multiple languages.
 * [Draw/GL4_5] Wait until complete to mapping.
                Fix crash with QUAZZLE (FMTOWNS;FSW Collection 10).
 * [VM/FMTOWNS][OOPs] Fix fallthroughs.
 * [VM/FMTOWNS] Add IC CARD feature.
 * [FMTOWNS/CRTC] More simple logic at rendering.
 * [FMTOWNS/CDROM] RESTORE/SEEK COMMAND (00h) must seek to lba0, then aimed lba.
 * [FMTOWNS/CDROM] PAUSE COMMAND (85h) : Return extra status even isn't audio track.
 * [FMTOWNS/CDROM] READ MODE1: May not need extra status, integrated after reading.
 * [FMTOWNS/MEMORY] Integrate memory accessing to primitive inline functions.
 * [FMTOWNS/CDROM][WIP] Status around CMD A0h. This is working-in-progress.
 * [FMTOWNS/CDROM][WIP] TRY: Implement PIO transfer.
 * [FMTOWNS/CDROM] Should read per a sector, not variable length.
 * [FMTOWNS/CDROM] Implement pseudo burst transfer for DMA.
 * [FMTOWNS/CDROM] Set CDDA_STATUS=CDDA_OFF before reading data.
                   Fix スーパーリアル麻雀PIV.
 * [FMTOWNS/SPRITE] Initially works.
 * [FMTOWNS/VRAM] Faster write access via write_memory_mapped_io[16|32]() .
 * [FMTOWNS/TIMER] Disable free run counter before 1H/2H/1F/2F.
 * [FMTOWNS/FLOPPY] Implement some bits and disk changed feature 
                    (0208h:bit0:R after Towns2H/2F/1H/1F).
 * [FMTOWNS/TIMER] Didable 1uS wait feature wait before xxF/xxH.
 * [FMTOWNS/KEYBOARD] TRY: Boot with 'CD' 'H0' etc.Still works only with 'DEBUG'.
 * [FMTOWNS/SCSI] Add SIG_SCSI_EOT signal.
 * [FMTOWNS/SCSI] Set ctr_reg after sending command to host.
 * [Qt/LOGGER] Fix not initialize (internal)osd_pointer;
               wish to fix below issue (@Fedora Linux) 
	       https://matsuri.5ch.net/test/read.cgi/i4004/1526806551/935
               by this.
 * [VM/I386_NP21] Memory access:Make functions inline to be faster processing.
 * [VM/COMMON_VM] Fix warining of 'set_context_intr' hides overloaded 
                  virtual function [-Woverloaded-virtual] with LLVM Clang++.
 * [VM/MC6809] Remove MC6809_BASE::, integrated to MC6809:: .
 * [VM/Z80] Remove Z80_BASE::, integrate to Z80:: .
 * [VM/UPD7220] Limit address of PSET.More correctness clock feature.
 * [VM/UPD71071] Fix tc bit down.
 * [VM/UPD71071] Add some signals.
 * [VM/UPD71071][FMTOWNS][MZ2800] Update API; Separate TC signals per a channel.
 * [VM/UPD71071] SREQ is prior than MASK.Don't auto transfer at demand mode.
 * [VM/UPD71071] Implement ENDx signal for stopping DMA from some devices.

 * [VM/I8259] Initialize registers by reset().
 * [EMU][UI/FLOPPY] Implement 1sec delayed open() for floppy, 
                    fix not detect when changing from HISTORY.
 * [X1/DRAW] Fix spending a lot of host CPU usage on draw_screen().
             This issue has happened at only X1 (not turbo) due to
             memory aligns and cache lines.
             Set alignment of RAM and some values.
 * Built with 97db8d7a26eb8eeb7722b009456d7c9bcadda0f7 (or later).

-- Sep 26, 2020 18:29:40 +0900 K.Ohta <whatisthis.sowhat@gmail.com>

本家の変更:
* 前の変更点をお読みになる場合には、history.txtをお読み下さい。

4/6/2020

[VM/I386_NP21] update to Neko Project 21/W ver0.86 rev72


-----

お楽しみあれ!
-- Ohta.
