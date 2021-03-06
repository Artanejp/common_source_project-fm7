; nasmw -O3 -f bin sys.asm -o fmt_sys6.prg
; modified by K.Ohta from 2020-01-27 from comp_sysrom/sys.asm, .
; version 2003.03.04.1 by Kasanova.
;  http://townsemu.world.coocan.jp/compatiroms.html
;---------------------------------------------------------------------
;
; FM TOWNS 互換 ROM シリーズ
;
; FMT_SYS.ROM : メインパート
; 0FFFFC000h - 0FFFFFFFFh
;
; by Kasanova
;
;---------------------------------------------------------------------
; FMT_SYS.ROM の構造(本物の)
; 0FFFC0000h - 0FFFDFFFFh : 12ドットフォント
;                           機種によってはALL FFh、起動ロゴ(パックド
;                           ピクセル方式)がある機種もあり
; 0FFFE0000h - 0FFFE7FFFh : EXT-BOOT(32ビットプログラム)
; 0FFFE8000h - 0FFFEFFFFh : システムアイコン
; 0FFFF0000h - 0FFFF7FFFh : 何かのパターン?
; 0FFFF8000h - 0FFFFAFFFh ; 起動ロゴ(プレーン方式)
;                           機種によっては Extention BIOS
; 0FFFFB000h - 0FFFFBFFFh : ブート時に使うアイコン
; 0FFFFC000h - 0FFFFFFFFh ; 16ビットプログラム
;---------------------------------------------------------------------
; FMT_SYS.ROM の構造(この互換ROMの)
; 0FFFC0000h - 0FFFDFFFFh : 12ドットフォント
; 0FFFE0000h - 0FFFE7FFFh : EXT-BOOT(32ビットプログラム)、まだ使っていない
; 0FFFE8000h - 0FFFEFFFFh : システムアイコン
; 0FFFF0000h - 0FFFF7FFFh : ダミーデータ(0ffh)
; 0FFFF8000h - 0FFFFBBFFh ; 起動ロゴ(プレーン方式、4プレーン分)
; 0FFFFBC00h - 0FFFFBFFFh : ブート時に使うアイコン
; 0FFFFC000h - 0FFFFFFFFh ; 16ビット+32ビットプログラム
;---------------------------------------------------------------------

%define BOOTCODE_BASE 0ffffc000h

%define BOOT_SS       0f7a0h
%define BOOT_SP       057eh
%define LOCAL_SP      05feh

%define VRAM_PITCH 50h

%define LOGO_ADDRESS      0ffff8000h
%define LOGO_USEPLANES    4
%if(LOGO_USEPLANES==3)
%define LOGO_PAL_ADDRESS  0ffffaf00h
%else
%define LOGO_PAL_ADDRESS  0ffffbb80h
%endif

%define ICON_WAIT 81
%define ICON_FDD  64
%define ICON_CD   67
%define ICON_HDD  71

%define PMODE_PUTICON     0
%define PMODE_MEMORYCHECK 1
%define PMODE_DRAWLOGO    2
%define PMODE_SETPALETTE  3
%define PMODE_TRANSFERMEM 4

;---------------------------------------------------------------------

%macro JMPFAR 1
	db 0eah
	dw %1
	dw 0fc00h
%endmacro


%macro CALLFAR 1
	db 09ah
	dw %1
	dw 0fc00h
%endmacro


%macro SAVEREG_TO_CMOS 2
	mov	dx,%1
%ifidn %2,ax
%else
	mov	ax,%2
%endif
	out	dx,al
	mov	dx,%1+2
	mov	al,ah
	out	dx,al
%endmacro


%macro LOADREG_FROM_CMOS 2
	mov	dx,%1+2
	in	al,dx
	mov	ah,al
	mov	dx,%1
	in	al,dx
%ifidn %2,ax
%else
	mov	%2,ax
%endif
%endmacro

;---------------------------------------------------------------------

; あやしいヘッダ
	dd	0,0,0,0, 0,0,0,0
;

[BITS 16]

startall:
	cli
	cld
	mov	ax,dx
	mov	dx,3c26h
	out	dx,al
	mov	al,ah
	sub	dl,2
	out	dx,al

	; disable & reset DMAC
	mov	al,0fh
	out	0afh,al
	mov	al,3
	out	0a0h,al

	in	al,28h
	or	al,1
	out	28h,al

	; select ROM
	mov	dx,404h
	xor	al,al
	out	dx,al

	mov	cx,BOOT_SS
	mov	ss,cx
	mov	sp,BOOT_SP

	push	cs
	pop	ds

	; set local stack address
	SAVEREG_TO_CMOS 31a8h, LOCAL_SP

	mov	dx,3c22h
	xor	al,al
	out	dx,al ; non 386SX

	mov	dx,31b8h
	out	dx,al
	mov	dx,31b2h
	out	dx,al
	mov	dx,31cch
	out	dx,al

	call	set_gdt
	call	init_pic
	call	init_keyboard
	call	init_crtc

	; CMOS情報が正しいか？
	mov	ah,20h
	CALLFAR	cmos_bios
	jnc	.noinitcmos
	; CMOS初期化
	mov	ah,0
	CALLFAR cmos_bios
.noinitcmos:

	mov	al,PMODE_SETPALETTE
	call	call_pmode

	mov	al,PMODE_DRAWLOGO
	call	call_pmode

	mov	al,PMODE_MEMORYCHECK
	call	call_pmode

	; CDが読めるか？
	mov	ah,0eh
	CALLFAR disk_bios
	jnc	.cdok

	; 手抜き(^^;
	mov	cl,ICON_CD
	mov	dx, (VRAM_PITCH*368)+(VRAM_PITCH-4)
	call	call_pmode
	mov	si,mes_cantboot
	mov	di,VRAM_PITCH*384
	call	textout
	jmp	$

.cdok:
	; IPL読み込み
	push	ds
	mov	cx,0
	mov	dx,0
	mov	ax,0b000h
	mov	ds,ax
	mov	di,0
	mov	ax,05c0h ; read command + media no.
	mov	bx,1
	CALLFAR disk_bios
	pop	ds

	mov	cl,ICON_WAIT
	mov	al,PMODE_PUTICON
	mov	dx, (VRAM_PITCH*368)+(VRAM_PITCH-4)
	call	call_pmode

	mov	si,mes_reading
	mov	di,VRAM_PITCH*384
	call	textout

	call	check_iplvalidity
	jc	.wrongipl

	mov	ax,0ffffh
	mov	bx,0008h
	call	far [cs:si]

.wrongipl:
	; 起動に失敗すると戻ってくる
	; 成功した場合は２度と戻ってこない
	mov	si,mes_wrongipl
	mov	di,VRAM_PITCH*384
	call	textout

	; 死
	jmp	$

mes_reading:
	db	'システム読み込み中です　　　　',0
mes_wrongipl:
	db	'システムが違います　　　　　　',0
mes_setsys:
	db	'システムをセットしてください　',0
mes_cantboot:
	db	'ＣＤをセットしてリセットしてね',0

;---------------------------------------------------------------------
; IPLのバージョンをチェック

check_iplvalidity:
	push	es
	mov	si,0b000h
	mov	es,si

	mov	si,.ipl_type1
	cmp	dword [es:0],'IPL4'
	jz	.goodipl

	mov	si,.ipl_type2
	cmp	dword [es:3],'IPL4'
	jz	.goodipl

	stc
.goodipl:
	pop	es
	ret

.ipl_type1:
	dw	4,0b000h
.ipl_type2:
	dw	0,0b000h
;	dw	0,0c200h

;---------------------------------------------------------------------
; GDTをセット

set_gdt:
	lgdt	[cs:.lgdtr]
	ret

	align 8
		dw	0
.lgdtr:		dw	002fh ; GDT limit
		dd	0fc000h+.gdtentry

.gdtentry:	db	 00h, 00h,00h, 00h,00h,00h, 00h,00h
;		db	0ffh,0ffh,00h, 00h,00h,9bh,0c0h,00h
		db	0ffh,0ffh,00h, 00h,00h,9bh,0cfh,00h
		db	0ffh,0ffh,00h, 00h,00h,93h,0cfh,00h
		db	0ffh,0ffh,00h,0c0h,0fh,9bh,000h,00h
		db	0ffh,0ffh,00h,0c0h,0fh,93h,000h,00h
		db	0ffh,000h,00h,0c0h,0fh,9bh,0c0h,00h
;		db	0ffh,0ffh,00h,0c0h,0fh,9bh,0cfh,00h


;---------------------------------------------------------------------
; プロテクトモード・プロシジャを呼ぶ

call_pmode:
	push	ds
	push	es
	push	gs
	mov	bx,ss
	mov	gs,bx
	mov	bx,ax

	mov	eax,cr0
	or	al,1
	mov	cr0,eax
	jmp	short $+2

	db	0eah
	dw	.goto_pmode
	dw	28h
.goto_pmode:

	db	0eah
	dd	BOOTCODE_BASE+pmode_entry
	dw	8

return_from_pmode:
	mov	bx,gs
	mov	ss,bx

	pop	gs
	pop	es
	pop	ds
	ret

;---------------------------------------------------------------------
; PIC初期化
; ※ウェイトを入れていないので、実機では動作しない

init_pic:
	mov	al,19h
	out	0,al
	mov	al,40h
	out	2,al
	mov	al,80h
	out	2,al
	mov	al,1dh
	out	2,al
	mov	al,0feh
	out	2,al
	mov	al,19h
	out	10h,al
	mov	al,48h
	out	12h,al
	mov	al,87h
	out	12h,al
	mov	al,9
	out	12h,al
	mov	al,0ffh
	out	12h,al
	ret

;---------------------------------------------------------------------
; キーボード初期化

init_keyboard:
	mov	dx,602h
	mov	al,0a1h ; reset
	out	dx,al

	; バッファが空になるまで待つ
.loop:
	mov	dx,602h
	in	al,dx
	test	al,1
	jz	.exit
	sub	dx,2
	in	al,dx
	jmp	.loop
.exit:
	ret

;---------------------------------------------------------------------
; CRTC初期化、FMR互換の画面モードへ

init_crtc:
	mov	dx,0fda0h
	xor	al,al
	out	dx,al

	mov	si,crtcinitdata
	mov	cx,32
.loop:
	mov	al,32
	sub	al,cl
	mov	dx,440h
	out	dx,al
	mov	ax,[si]
	add	dx,2
	out	dx,ax
	add	si,2
	loop	.loop

	mov	dx,448h
	xor	al,al
	out	dx,al
	add	dx,2
	mov	al,15h
	out	dx,al

	mov	dx,448h
	mov	al,1
	out	dx,al
	add	dx,2
	mov	al,8
	out	dx,al

	mov	dx,0fda0h
	mov	al,8
	out	dx,al

	; 全プレーンを書き込み対象に設定
	mov	dx,0ff81h
	mov	al,0fh
	out	dx,al

	; 全プレーン表示
	mov	dx,0ff82h
	mov	al,67h
	out	dx,al

	; 描画対象プレーンを選択
	mov	dx,0ff83h
	xor	al,al
	out	dx,al

	ret


crtcinitdata:
	dw	0040h, 0320h, 0000h, 0000h, 035fh, 0000h, 0010h, 0000h
	dw	036fh, 009ch, 031ch, 009ch, 031ch, 0040h, 0360h, 0040h
	dw	0360h, 0000h, 009ch, 0000h, 0050h, 0000h, 009ch, 0000h
	dw	0050h, 004ah, 0001h, 0000h, 003fh, 0003h, 0000h, 0150h

;---------------------------------------------------------------------
; 文字列表示
;
; si = 文字列
; di = 表示先VRAMアドレス

textout:
	push	es
	push	bx
	mov	ax,0c000h
	mov	es,ax
	mov	bx,0ff94h

.textoutloop:
	mov	cx,[si]
	or	cl,cl
	jz	.exit

	call	sjistojis
	mov	[es:bx],cl
	mov	[es:bx+1],ch
	mov	cx,16
.onecharloop:
	mov	al,[es:bx+2]
	mov	ah,[es:bx+3]
	mov	[es:di],ax
	add	di,VRAM_PITCH
	loop	.onecharloop

	sub	di,VRAM_PITCH*16-2
	add	si,2
	jmp	.textoutloop
.exit:
	pop	bx
	pop	es
	ret

; シフトJIS→JIS変換
sjistojis:
	cmp	cl,0e0h
	jc	.j1
	sub	cl,40h
.j1:
	sub	cl,81h
	shl	cl,1
	add	cl,21h
	mov	al,ch
	cmp	ch,9fh
	jc	.j2
	inc	cl
	sub	ch,5eh
.j2:
	sub	ch,20h
	cmp	al,7eh
	ja	.j3
	test	cl,1
	jz	.j3
	inc	ch
.j3:
	ret

;---------------------------------------------------------------------
; DISK-BIOS(と勝手に呼んでいる)
; ahに応じて次の機能を提供する(ah = 2-0x11)
	align 2
disk_command_table:
	dw	disk_command_02 ; 2 : 未実装
	dw	disk_command_03 ; 3 : メディア先頭へシーク？
	dw	disk_command_04 ; 4 : 未実装
	dw	disk_command_05 ; 5 : リード
	dw	disk_command_06 ; 6 : ライト
	dw	disk_command_xx ; 7 : 無効
	dw	disk_command_08 ; 8 : ドライブリセット(FDD & HDD)
	dw	disk_command_xx ; 9 : 無効
	dw	disk_command_xx ; a : 無効
	dw	disk_command_xx ; b : 無効
	dw	disk_command_xx ; c : 無効
	dw	disk_command_xx ; d : 無効
	dw	disk_command_0e ; e : ドライブチェック
	dw	disk_command_xx ; f : 無効
	dw	disk_command_xx ;10 : 無効
	dw	disk_command_11 ;11 : 未実装
;
; リターンコード: ah(0:正常終了)、エラーの有無はキャリーフラグにセット

disk_bios:
	; めんどくさい。フラグも変えないよう注意
	push	dx
	push	ax ; これがリターンコードになる

	; まず、ローカルスタックに切り替える
	; 現在の SS:SP を退避
	SAVEREG_TO_CMOS 319ch, ss
	SAVEREG_TO_CMOS 31a0h, sp
	LOADREG_FROM_CMOS 31a8h, sp
	mov	ax,BOOT_SS
	mov	ss,ax
	; ローカルスタックに切り替え完了

	; 呼出し元 SS:SP を push
	LOADREG_FROM_CMOS 319ch, ax ; ss
	push	ax
	LOADREG_FROM_CMOS 31a0h, ax ; sp
	push	ax

	push	es
	push	ds
	push	di
	push	si
	push	bp

	LOADREG_FROM_CMOS 31a8h, bp

	; DS:SI で呼び出し元スタックをいじれるようにする
	LOADREG_FROM_CMOS 319ch, ds
	LOADREG_FROM_CMOS 31a0h, si

	push	cx
	push	bx
	clc
	pushf

	cli
	cld
	mov	ax,ss
	mov	es,ax
	mov	di,sp
	push	bp

	; 一番最初に push したレジスタをロード
	mov	ax,[si]
	mov	dx,[si+2]

	; 本来なら範囲判定があるが省略

	; 呼ぶ
	mov	al,ah
	xor	ah,ah
	sub	ax,2
	add	ax,ax
	mov	bx,ax
	call	[cs:disk_command_table+bx]

	; 結果を格納
	or	ah,ah
	setnz	al
	mov	[si+1],ah
	or	[es:di],al ; CF
	
	pop	ax
	popf
	pop	bx
	pop	cx
	pop	bp
	pop	si
	pop	di
	pop	ds
	pop	es

	mov	dx,bx
	pop	bx
	mov	ax,bx
	pop	bx
	mov	ss,bx
	mov	sp,ax
	mov	bx,dx
	pop	ax
	pop	dx
	retf


disk_command_xx:
	jmp	$

disk_command_02:
	jmp	$

disk_command_03:
	call	cd_command_0e ; 一応これで代替
	ret

disk_command_04:
	jmp	$

disk_command_05:
	mov	al,[si]
	and	al,0f0h
	cmp	al,040h
	jz	.rom
	call	cd_command_05
	ret
.rom:
	call	osrom_command_05
	ret

disk_command_06:
	mov	al,[si]
	and	al,0f0h
	cmp	al,040h
	jz	.rom
	jmp	$
	ret
.rom:
	call	osrom_command_06
	ret

disk_command_08:
	jmp	$

disk_command_0e:
	call	cd_command_0e
	ret

disk_command_11:
	jmp	$


;---------------------------------------------------------------------
; CMOS-BIOS(と勝手に呼んでいる)
; ahに応じて次の機能を提供する(ah = -3(0xfd)-0x20)
	align 2
	dw	cmos_command_fd ;fd : 未実装
	dw	cmos_command_xx ;fe : 無効
	dw	cmos_command_xx ;ff : 無効
cmos_command_table:
	dw	cmos_command_00 ; 0 : イニシャライズ
	dw	cmos_command_01 ; 1 : 未実装
	dw	cmos_command_02 ; 2 : 未実装
	dw	cmos_command_03 ; 3 : 未実装
	dw	cmos_command_04 ; 4 : 未実装
	dw	cmos_command_05 ; 5 : 未実装
	dw	cmos_command_06 ; 6 : 未実装
	dw	cmos_command_xx ; 7 : 無効
	dw	cmos_command_xx ; 8 : 無効
	dw	cmos_command_xx ; 9 : 無効
	dw	cmos_command_xx ; a : 無効
	dw	cmos_command_xx ; b : 無効
	dw	cmos_command_xx ; c : 無効
	dw	cmos_command_xx ; d : 無効
	dw	cmos_command_xx ; e : 無効
	dw	cmos_command_xx ; f : 無効
	dw	cmos_command_10 ;10 : ブロック書き込み
	dw	cmos_command_11 ;11 : ブロック読み出し
	dw	cmos_command_xx ;12 : 無効
	dw	cmos_command_xx ;13 : 無効
	dw	cmos_command_xx ;14 : 無効
	dw	cmos_command_xx ;15 : 無効
	dw	cmos_command_xx ;16 : 無効
	dw	cmos_command_xx ;17 : 無効
	dw	cmos_command_xx ;18 : 無効
	dw	cmos_command_xx ;19 : 無効
	dw	cmos_command_xx ;1a : 無効
	dw	cmos_command_xx ;1b : 無効
	dw	cmos_command_xx ;1c : 無効
	dw	cmos_command_xx ;1d : 無効
	dw	cmos_command_xx ;1e : 無効
	dw	cmos_command_xx ;1f : 無効
	dw	cmos_command_20 ;20 : ヘッダが正常かチェック
;
; リターンコード: ah(0:正常終了)、エラーの有無はキャリーフラグにセット

cmos_bios:
	; これまためんどくさい。フラグは変えてもいいみたい
	push	bp
	mov	bp,dx

	; まず、axを退避
	SAVEREG_TO_CMOS 319ch,ax

	; ローカルスタックに切り替える
	; 現在の SS:SP を退避
	SAVEREG_TO_CMOS 31a0h, ss
	SAVEREG_TO_CMOS 31a4h, sp
	LOADREG_FROM_CMOS 31a8h, sp
	mov	ax,BOOT_SS
	mov	ss,ax
	; ローカルスタックに切り替え完了

	; 呼出し元 SS:SP を push
	LOADREG_FROM_CMOS 31a0h, ax ; ss
	push	ax
	LOADREG_FROM_CMOS 31a4h, ax ; sp
	push	ax

	; 退避しておいたaxを復元
	LOADREG_FROM_CMOS 319ch,ax

	mov	dx,bp
	push	es ; [bp+12]
	push	ds ; [bp+10]
	push	di ; [bp+e]
	push	si ; [bp+c]
	push	bp ; [bp+a]
	push	dx ; [bp+8]
	push	cx ; [bp+6]
	push	bx ; [bp+4]
	push	ax ; [bp+2]
	clc
	pushf

	cli
	cld
	mov	bp,sp

	; 範囲チェックして、呼ぶ
	mov	al,[bp+3]
	mov	ah,1

	cmp	al,21h
	jnl	.error
	cmp	al,0fch
	jng	.error

	movsx	bx,al
	add	bx,bx
	call	[cs:cmos_command_table+bx]

	; 結果を格納
	or	ah,ah
	setnz	al
	jns	.noerror
.error:
	mov	[bp+6],cx
.noerror:
	mov	[bp+3],ah
	or	[bp],al ; CF

	popf
	pop	ax
	pop	bx
	pop	cx
	pop	dx
	pop	bp
	pop	si
	pop	di
	pop	ds
	pop	es

	mov	bp,dx

	SAVEREG_TO_CMOS 319ch,ax

	; 呼出し元SS:SPの復元
	pop	ax ; sp
	SAVEREG_TO_CMOS 31a0h,ax
	pop	ax ; ss
	mov	ss,ax
	LOADREG_FROM_CMOS 31a0h,ax
	mov	sp,ax

	LOADREG_FROM_CMOS 319ch,ax
	mov	dx,bp
	pop	bp
	retf


;---------------------------------------------------------------------
; 各デバイス特有の処理を記述したコードをインクルード

%include "sys_cd.asm"
%include "sys_fd.asm"
%include "sys_hd.asm"
%include "sys_osr.asm"

%include "sys_cmos.asm"

%include "sys_p32.asm"

;---------------------------------------------------------------------
; ウェイト(うんづではあまり意味が無いので省略)

waitloop:
	retf

;---------------------------------------------------------------------

invalid1:
	jmp	invalid1

invalid2:
	jmp	invalid2

invalid3:
	jmp	invalid3

invalid4:
	jmp	invalid4

invalid5:
	jmp	invalid5


;---------------------------------------------------------------------

	align 3000h, db 0

%rep 0fb0h
	db 0
%endrep

	JMPFAR invalid1 ; 診断エラー?
	JMPFAR invalid2 ; 診断エラー?
	JMPFAR invalid3 ; ?
	JMPFAR invalid4 ; 文字列表示(未実装)
	JMPFAR disk_bios
	JMPFAR cmos_bios
	JMPFAR invalid5 ; 文字列表示(未実装)
	JMPFAR waitloop

	dd 0,0, 0,0,0,0

	JMPFAR startall ; ここからすべてが始まる

	db 0,0,0
	dd 0,0

