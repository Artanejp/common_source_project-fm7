; nasmw -O3 -f bin sys.asm -o fmt_sys6.prg
; modified by K.Ohta from 2020-01-27 from comp_sysrom/sys.asm, .
; version 2003.03.04.1 by Kasanova.
;  http://townsemu.world.coocan.jp/compatiroms.html
;---------------------------------------------------------------------
;
; FM TOWNS �݊� ROM �V���[�Y
;
; FMT_SYS.ROM : ���C���p�[�g
; 0FFFFC000h - 0FFFFFFFFh
;
; by Kasanova
;
;---------------------------------------------------------------------
; FMT_SYS.ROM �̍\��(�{����)
; 0FFFC0000h - 0FFFDFFFFh : 12�h�b�g�t�H���g
;                           �@��ɂ���Ă�ALL FFh�A�N�����S(�p�b�N�h
;                           �s�N�Z������)������@�������
; 0FFFE0000h - 0FFFE7FFFh : EXT-BOOT(32�r�b�g�v���O����)
; 0FFFE8000h - 0FFFEFFFFh : �V�X�e���A�C�R��
; 0FFFF0000h - 0FFFF7FFFh : �����̃p�^�[��?
; 0FFFF8000h - 0FFFFAFFFh ; �N�����S(�v���[������)
;                           �@��ɂ���Ă� Extention BIOS
; 0FFFFB000h - 0FFFFBFFFh : �u�[�g���Ɏg���A�C�R��
; 0FFFFC000h - 0FFFFFFFFh ; 16�r�b�g�v���O����
;---------------------------------------------------------------------
; FMT_SYS.ROM �̍\��(���̌݊�ROM��)
; 0FFFC0000h - 0FFFDFFFFh : 12�h�b�g�t�H���g
; 0FFFE0000h - 0FFFE7FFFh : EXT-BOOT(32�r�b�g�v���O����)�A�܂��g���Ă��Ȃ�
; 0FFFE8000h - 0FFFEFFFFh : �V�X�e���A�C�R��
; 0FFFF0000h - 0FFFF7FFFh : �_�~�[�f�[�^(0ffh)
; 0FFFF8000h - 0FFFFBBFFh ; �N�����S(�v���[�������A4�v���[����)
; 0FFFFBC00h - 0FFFFBFFFh : �u�[�g���Ɏg���A�C�R��
; 0FFFFC000h - 0FFFFFFFFh ; 16�r�b�g+32�r�b�g�v���O����
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

; ���₵���w�b�_
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

	; CMOS��񂪐��������H
	mov	ah,20h
	CALLFAR	cmos_bios
	jnc	.noinitcmos
	; CMOS������
	mov	ah,0
	CALLFAR cmos_bios
.noinitcmos:

	mov	al,PMODE_SETPALETTE
	call	call_pmode

	mov	al,PMODE_DRAWLOGO
	call	call_pmode

	mov	al,PMODE_MEMORYCHECK
	call	call_pmode

	; CD���ǂ߂邩�H
	mov	ah,0eh
	CALLFAR disk_bios
	jnc	.cdok

	; �蔲��(^^;
	mov	cl,ICON_CD
	mov	dx, (VRAM_PITCH*368)+(VRAM_PITCH-4)
	call	call_pmode
	mov	si,mes_cantboot
	mov	di,VRAM_PITCH*384
	call	textout
	jmp	$

.cdok:
	; IPL�ǂݍ���
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
	; �N���Ɏ��s����Ɩ߂��Ă���
	; ���������ꍇ�͂Q�x�Ɩ߂��Ă��Ȃ�
	mov	si,mes_wrongipl
	mov	di,VRAM_PITCH*384
	call	textout

	; ��
	jmp	$

mes_reading:
	db	'�V�X�e���ǂݍ��ݒ��ł��@�@�@�@',0
mes_wrongipl:
	db	'�V�X�e�����Ⴂ�܂��@�@�@�@�@�@',0
mes_setsys:
	db	'�V�X�e�����Z�b�g���Ă��������@',0
mes_cantboot:
	db	'�b�c���Z�b�g���ă��Z�b�g���Ă�',0

;---------------------------------------------------------------------
; IPL�̃o�[�W�������`�F�b�N

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
; GDT���Z�b�g

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
; �v���e�N�g���[�h�E�v���V�W�����Ă�

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
; PIC������
; ���E�F�C�g�����Ă��Ȃ��̂ŁA���@�ł͓��삵�Ȃ�

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
; �L�[�{�[�h������

init_keyboard:
	mov	dx,602h
	mov	al,0a1h ; reset
	out	dx,al

	; �o�b�t�@����ɂȂ�܂ő҂�
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
; CRTC�������AFMR�݊��̉�ʃ��[�h��

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

	; �S�v���[�����������ݑΏۂɐݒ�
	mov	dx,0ff81h
	mov	al,0fh
	out	dx,al

	; �S�v���[���\��
	mov	dx,0ff82h
	mov	al,67h
	out	dx,al

	; �`��Ώۃv���[����I��
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
; ������\��
;
; si = ������
; di = �\����VRAM�A�h���X

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

; �V�t�gJIS��JIS�ϊ�
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
; DISK-BIOS(�Ə���ɌĂ�ł���)
; ah�ɉ����Ď��̋@�\��񋟂���(ah = 2-0x11)
	align 2
disk_command_table:
	dw	disk_command_02 ; 2 : ������
	dw	disk_command_03 ; 3 : ���f�B�A�擪�փV�[�N�H
	dw	disk_command_04 ; 4 : ������
	dw	disk_command_05 ; 5 : ���[�h
	dw	disk_command_06 ; 6 : ���C�g
	dw	disk_command_xx ; 7 : ����
	dw	disk_command_08 ; 8 : �h���C�u���Z�b�g(FDD & HDD)
	dw	disk_command_xx ; 9 : ����
	dw	disk_command_xx ; a : ����
	dw	disk_command_xx ; b : ����
	dw	disk_command_xx ; c : ����
	dw	disk_command_xx ; d : ����
	dw	disk_command_0e ; e : �h���C�u�`�F�b�N
	dw	disk_command_xx ; f : ����
	dw	disk_command_xx ;10 : ����
	dw	disk_command_11 ;11 : ������
;
; ���^�[���R�[�h: ah(0:����I��)�A�G���[�̗L���̓L�����[�t���O�ɃZ�b�g

disk_bios:
	; �߂�ǂ������B�t���O���ς��Ȃ��悤����
	push	dx
	push	ax ; ���ꂪ���^�[���R�[�h�ɂȂ�

	; �܂��A���[�J���X�^�b�N�ɐ؂�ւ���
	; ���݂� SS:SP ��ޔ�
	SAVEREG_TO_CMOS 319ch, ss
	SAVEREG_TO_CMOS 31a0h, sp
	LOADREG_FROM_CMOS 31a8h, sp
	mov	ax,BOOT_SS
	mov	ss,ax
	; ���[�J���X�^�b�N�ɐ؂�ւ�����

	; �ďo���� SS:SP �� push
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

	; DS:SI �ŌĂяo�����X�^�b�N���������悤�ɂ���
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

	; ��ԍŏ��� push �������W�X�^�����[�h
	mov	ax,[si]
	mov	dx,[si+2]

	; �{���Ȃ�͈͔��肪���邪�ȗ�

	; �Ă�
	mov	al,ah
	xor	ah,ah
	sub	ax,2
	add	ax,ax
	mov	bx,ax
	call	[cs:disk_command_table+bx]

	; ���ʂ��i�[
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
	call	cd_command_0e ; �ꉞ����ő��
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
; CMOS-BIOS(�Ə���ɌĂ�ł���)
; ah�ɉ����Ď��̋@�\��񋟂���(ah = -3(0xfd)-0x20)
	align 2
	dw	cmos_command_fd ;fd : ������
	dw	cmos_command_xx ;fe : ����
	dw	cmos_command_xx ;ff : ����
cmos_command_table:
	dw	cmos_command_00 ; 0 : �C�j�V�����C�Y
	dw	cmos_command_01 ; 1 : ������
	dw	cmos_command_02 ; 2 : ������
	dw	cmos_command_03 ; 3 : ������
	dw	cmos_command_04 ; 4 : ������
	dw	cmos_command_05 ; 5 : ������
	dw	cmos_command_06 ; 6 : ������
	dw	cmos_command_xx ; 7 : ����
	dw	cmos_command_xx ; 8 : ����
	dw	cmos_command_xx ; 9 : ����
	dw	cmos_command_xx ; a : ����
	dw	cmos_command_xx ; b : ����
	dw	cmos_command_xx ; c : ����
	dw	cmos_command_xx ; d : ����
	dw	cmos_command_xx ; e : ����
	dw	cmos_command_xx ; f : ����
	dw	cmos_command_10 ;10 : �u���b�N��������
	dw	cmos_command_11 ;11 : �u���b�N�ǂݏo��
	dw	cmos_command_xx ;12 : ����
	dw	cmos_command_xx ;13 : ����
	dw	cmos_command_xx ;14 : ����
	dw	cmos_command_xx ;15 : ����
	dw	cmos_command_xx ;16 : ����
	dw	cmos_command_xx ;17 : ����
	dw	cmos_command_xx ;18 : ����
	dw	cmos_command_xx ;19 : ����
	dw	cmos_command_xx ;1a : ����
	dw	cmos_command_xx ;1b : ����
	dw	cmos_command_xx ;1c : ����
	dw	cmos_command_xx ;1d : ����
	dw	cmos_command_xx ;1e : ����
	dw	cmos_command_xx ;1f : ����
	dw	cmos_command_20 ;20 : �w�b�_�����킩�`�F�b�N
;
; ���^�[���R�[�h: ah(0:����I��)�A�G���[�̗L���̓L�����[�t���O�ɃZ�b�g

cmos_bios:
	; ����܂��߂�ǂ������B�t���O�͕ς��Ă������݂���
	push	bp
	mov	bp,dx

	; �܂��Aax��ޔ�
	SAVEREG_TO_CMOS 319ch,ax

	; ���[�J���X�^�b�N�ɐ؂�ւ���
	; ���݂� SS:SP ��ޔ�
	SAVEREG_TO_CMOS 31a0h, ss
	SAVEREG_TO_CMOS 31a4h, sp
	LOADREG_FROM_CMOS 31a8h, sp
	mov	ax,BOOT_SS
	mov	ss,ax
	; ���[�J���X�^�b�N�ɐ؂�ւ�����

	; �ďo���� SS:SP �� push
	LOADREG_FROM_CMOS 31a0h, ax ; ss
	push	ax
	LOADREG_FROM_CMOS 31a4h, ax ; sp
	push	ax

	; �ޔ����Ă�����ax�𕜌�
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

	; �͈̓`�F�b�N���āA�Ă�
	mov	al,[bp+3]
	mov	ah,1

	cmp	al,21h
	jnl	.error
	cmp	al,0fch
	jng	.error

	movsx	bx,al
	add	bx,bx
	call	[cs:cmos_command_table+bx]

	; ���ʂ��i�[
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

	; �ďo����SS:SP�̕���
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
; �e�f�o�C�X���L�̏������L�q�����R�[�h���C���N���[�h

%include "sys_cd.asm"
%include "sys_fd.asm"
%include "sys_hd.asm"
%include "sys_osr.asm"

%include "sys_cmos.asm"

%include "sys_p32.asm"

;---------------------------------------------------------------------
; �E�F�C�g(����Âł͂��܂�Ӗ��������̂ŏȗ�)

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

	JMPFAR invalid1 ; �f�f�G���[?
	JMPFAR invalid2 ; �f�f�G���[?
	JMPFAR invalid3 ; ?
	JMPFAR invalid4 ; ������\��(������)
	JMPFAR disk_bios
	JMPFAR cmos_bios
	JMPFAR invalid5 ; ������\��(������)
	JMPFAR waitloop

	dd 0,0, 0,0,0,0

	JMPFAR startall ; �������炷�ׂĂ��n�܂�

	db 0,0,0
	dd 0,0

