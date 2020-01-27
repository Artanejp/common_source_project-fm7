; version 2003.03.04.1
;---------------------------------------------------------------------
;
; FM TOWNS �݊� ROM �V���[�Y
;
; FMT_SYS.ROM : CMOS BIOS
;
; by Kasanova
;
;---------------------------------------------------------------------
; ���P�Ƃł̓A�Z���u�����܂���


;---------------------------------------------------------------------

cmos_command_xx:
	jmp	$

;---------------------------------------------------------------------

cmos_command_fd:
	jmp	$

;---------------------------------------------------------------------

; init. cmos
cmos_command_00:
	mov	ax,cs
	mov	ds,ax

	; �܂��N���A
	xor	dx,dx
	xor	al,al
	mov	cx,0a8h
.loop1:
	call	cmos_write1byte
	inc	dx
	loop	.loop1

	; �����f�[�^�]��
	xor	dx,dx
	mov	si,.initdata
	mov	cx,.initdata_end-.initdata
.loop2:
	lodsb
	call	cmos_write1byte
	inc	dx
	loop	.loop2

	mov	dx,0a0h
	mov	si,.initdata2
	mov	cx,8
.loop3:
	lodsb
	call	cmos_write1byte
	inc	dx
	loop	.loop3

	mov	dx,0a0h
	call	cmos_read2byte
	mov	dx,ax
.loop4:
	push	dx
	xor	ax,ax
	call	cmos_write1byte
	pop	dx
	inc	dx
	cmp	dx,7c1h
	jc	.loop4

	call	cmos_init_blocknotable

	call	cmos_get_cmosheadersum
	mov	dx,0a4h
	call	cmos_write2byte

	; �`�F�b�N�T���e�[�u��������
	xor	ax,ax
	xor	cl,cl
.loop5:
	push	ax
	call	cmos_write_3f82
	pop	ax
	inc	al
	cmp	al,10h
	jc	.loop5

	xor	ah,ah
	ret

	; cmos��̃f�[�^�u���b�N���`�f�[�^
.initdata:
	db	1,0ffh       ; �u���b�N���݃t���O�{�u���b�N�ԍ��H
	db	'BOOT'       ; ���ʎq
	dw	00a8h, 0040h ; cmos��̃A�h���X�ƃT�C�Y

	db	1,0feh
	db	'SETU'
	dw	00e8h, 0100h

	db	1,0fdh
	db	'LOG '
	dw	01e8h, 0310h
	
	db	1,0fch
	db	'OASY'
	dw	04f8h, 0020h

	db	1,0fbh
	db	'XENI'
	dw	0518h, 0010h

	db	1,0fah
	db	'TOWN'
	dw	0528h, 0100h
.initdata_end:

.initdata2:
	db	28h,06h,99h,01h, 00h,00h,79h,41h

;---------------------------------------------------------------------

cmos_command_01:
	jmp	$

;---------------------------------------------------------------------

cmos_command_02:
	jmp	$

;---------------------------------------------------------------------

cmos_command_03:
	jmp	$

;---------------------------------------------------------------------

cmos_command_04:
	jmp	$

;---------------------------------------------------------------------

; CMOS��a2h�Ԓn�̏���bx�ɕԂ�����
cmos_command_05:
	mov	dx,0a2h
	call	cmos_read2byte
	mov	[bp+4],ax      ; bx
	xor	ah,ah
	ret

;---------------------------------------------------------------------

cmos_command_06:
	jmp	$

;---------------------------------------------------------------------

; transfer block to cmos
cmos_command_10:
	; �͈̓`�F�b�N
	mov	cl,[bp+6] ; cl
	mov	al,[bp+2] ; al
	call	cmos_check_blockvalidity
	or	ah,ah
	jnz	.exit1
	call	cmos_check_transferrange
	or	ah,ah
	jnz	.exit1

	; �]���ʂ� 0 �Ȃ牽�����Ȃ�
	or	bx,bx
	jnz	.starttransfer
.exit1:
	ret

.starttransfer:
	mov	si,[bp+0eh] ; di
	mov	ds,[bp+10h] ; ds
	mov	cx,bx
.loop:
	lodsb
	call	cmos_write1byte
	inc	dx
	loop	.loop

	movzx	ax,byte [bp+2]
	mov	dx,ax
	add	dx,dx ; dx<-ax*10
	add	dx,dx
	add	dx,dx
	add	dx,ax
	add	dx,ax
	call	cmos_read1byte
	or	al,al
	jns	.exit2

	call	cmos_calc_checksum
	mov	cl,al
	mov	al,[bp+2]
	call	cmos_write_3f82
.exit2:
	xor	ah,ah
	ret

;---------------------------------------------------------------------

; transfer block from cmos
cmos_command_11:
	; �͈̓`�F�b�N
	mov	cl,[bp+6] ; cl
	mov	al,[bp+2] ; al
	call	cmos_check_blockvalidity
	or	ah,ah
	jnz	.exit1
	call	cmos_check_transferrange
	or	ah,ah
	jnz	.exit1

	; �]���ʂ� 0 �Ȃ牽�����Ȃ�
	or	bx,bx
	jnz	.starttransfer
.exit1:
	ret

.starttransfer:
	mov	di,[bp+0eh] ; di
	mov	es,[bp+10h] ; ds
	mov	cx,bx
.loop:
	call	cmos_read1byte
	stosb
	inc	dx
	loop	.loop
	xor	ah,ah
	ret

;---------------------------------------------------------------------
; cmos�w�b�_�Ɗe�u���b�N�̃`�F�b�N
; out: ah != 0 : �w�b�_�ُ�
;      ah == 0 : �w�b�_����Abx�Ƀ`�F�b�N�T��������Ȃ�����
;                �u���b�N���r�b�g�P�ʂŃZ�b�g�����
cmos_command_20:
	mov	dx,0a6h
	call	cmos_read2byte
	cmp	ax,4179h        ; �w�b�_�̎��ʎq�H�l���̂ɈӖ������邩�s��
	jz	.next
	mov	ah,3
	ret
.next:
	call	cmos_get_cmosheadersum
	mov	bx,ax
	mov	dx,0a4h
	call	cmos_read2byte
	cmp	bx,ax
	mov	cx,20h
	mov	ah,80h
	jnz	.j1
	call	cmos_check_allblocks
.j1:
	mov	[bp+4],bx ; bx
	ret

;---------------------------------------------------------------------
; CMOS BIOS ������

; CMOS�A�h���X��I/O�A�h���X�ɕϊ�
; in dx:cmos address -> out dx:i/o address
cmos_getaddress:
	cmp	dx,7c0h
	ja	.over

	add	dx,dx
	add	dx,3000h
	xor	ax,ax
	ret
.over:
	push	cx
	sub	dx,7c1h
	mov	cx,800h
	mov	ax,dx
	xor	dx,dx
	div	cx
	inc	ax
	add	dx,dx
	add	dx,3000h
	pop	cx
	ret

;--------------------------------------

cmos_read1byte:
	push	dx
	call	cmos_getaddress
	in	al,dx
	pop	dx
	ret

;--------------------------------------

cmos_read2byte:
	push	cx
	push	dx
	call	cmos_read1byte
	mov	cl,al
	inc	dx
	call	cmos_read1byte
	mov	ah,cl
	xchg	al,ah
	pop	dx
	pop	cx
	ret

;--------------------------------------

cmos_write1byte:
	push	dx
	push	ax
	call	cmos_getaddress
	pop	ax
	out	dx,al
	pop	dx
	ret

;--------------------------------------

cmos_write2byte:
	push	dx
	push	ax
	call	cmos_write1byte
	inc	dx
	mov	al,ah
	call	cmos_write1byte
	
	pop	ax
	pop	dx
	ret

;--------------------------------------
; �`�F�b�N�T���e�[�u���ǂݍ���

cmos_read_3f82:
	movsx	dx,al
	add	dx,dx
	add	dx,3f82h
	in	al,dx
	mov	cl,al
	ret

;--------------------------------------
; �`�F�b�N�T���e�[�u����������

cmos_write_3f82:
	movsx	dx,al
	add	dx,dx
	add	dx,3f82h
	mov	al,cl
	out	dx,al
	ret

;--------------------------------------
; �u���b�N�ԍ��e�[�u���ǂݍ���

cmos_read_3fa2:
	push	dx
	add	dx,dx
	add	dx,3fa2h
	in	al,dx
	pop	dx
	ret

;--------------------------------------
; �u���b�N�ԍ��e�[�u����������

cmos_write_3fa2:
	push	dx
	add	dx,dx
	add	dx,3fa2h
	out	dx,al
	pop	dx
	ret

;--------------------------------------

; �w�肳�ꂽcmos�u���b�N�̃`�F�b�N�T����Ԃ�
; in al: block no
cmos_calc_checksum:
	push	bx
	call	cmos_getaddlength
	xor	bl,bl
.loop:
	call	cmos_read1byte
	add	bl,al
	inc	dx
	loop	.loop
	xor	ax,ax
	sub	al,bl
	pop	bx
	ret

;--------------------------------------

; �w�肳�ꂽcmos�u���b�N�̃A�h���X�ƒ�����Ԃ�
; in : al: no
; out: cx:length, dx:cmos address
cmos_getaddlength:
	xor	ah,ah ; dx<-ax*10
	mov	dx,ax
	add	dx,dx
	add	dx,dx
	add	dx,dx
	add	dx,ax
	add	dx,ax

	add	dx,8
	call	cmos_read2byte
	mov	cx,ax ; length

	sub	dx,2
	call	cmos_read2byte
	mov	dx,ax ; address
	ret

;--------------------------------------

cmos_check_blockrange:
	cmp	al,10h
	jc	.j1
	mov	ah,2
	ret
.j1:
	xor	ah,ah
	ret

;--------------------------------------

; �u���b�N�̗L�������`�F�b�N
; in: al, cl
cmos_check_blockvalidity:
	call	cmos_check_blockrange
	or	ah,ah
	jz	.j1
	ret
.j1:
	xor	ah,ah ; dx<-ax*10
	mov	dx,ax
	add	dx,dx
	add	dx,dx
	add	dx,dx
	add	dx,ax
	add	dx,ax

	mov	bl,cl
	push	dx
	call	cmos_read2byte
	; �u���b�N���L�����H
	test	al,1
	jz	.error

	mov	cx,8
	cmp	ah,bl
	jnz	.error

	xor	ah,ah
	pop	dx
	ret
.error:
	mov	cx,40h
	mov	ah,80h
	pop	dx
	ret

;--------------------------------------

; �]���͈̗͂L�������`�F�b�N
cmos_check_transferrange:
	mov	al,[bp+2] ; al
	call	cmos_getaddlength
	mov	di,[bp+8] ; dx
	add	di,[bp+4] ; bx
	jc	.error
	cmp	cx,di
	jc	.error
	mov	ax,[bp+8] ; dx
	add	dx,ax
	mov	bx,[bp+4] ; bx
	xor	ah,ah
	ret
.error:
	mov	cx,4
	mov	ah,80h
	ret

;--------------------------------------

; �u���b�N�ԍ��e�[�u��������
cmos_init_blocknotable:
	mov	si,.initdata
	xor	dx,dx
.loop:
	lodsb
	call	cmos_write_3fa2
	inc	dx
	cmp	dx,10h
	jc	.loop
	ret

.initdata:
	db	0,1,2,3,4, 255,255,255
	db	255,255,255,255, 255,255,255,255

;--------------------------------------

; 0-a4�܂ł̒l�����Z�����l�𓾂�
cmos_get_cmosheadersum:
	xor	dx,dx
	mov	cx,52h
	xor	bx,bx
.loop:
	push	cx
	call	cmos_read2byte
	pop	cx
	add	bx,ax
	add	dx,2
	loop	.loop
	mov	ax,bx
	ret

;--------------------------------------

; �S�u���b�N���`�F�b�N
cmos_check_allblocks:
	xor	ax,ax
	xor	di,di
	xor	si,si
.loop:
	mov	dx,si
	add	dx,dx
	add	dx,dx
	add	dx,dx
	add	dx,si
	add	dx,si
	call	cmos_read1byte
	or	al,al
	jns	.next

	mov	ax,si
	call	cmos_read_3f82
	mov	ax,si
	push	cx
	call	cmos_calc_checksum
	pop	cx

	cmp	al,cl
	jz	.next

	; �`�F�b�N�T���G���[�̂������u���b�N��bit��on
	mov	cx,si
	mov	ax,1
	shl	ax,cl
	or	di,ax
.next:
	inc	si
	cmp	si,10h
	jc	.loop
	mov	bx,di
	xor	ah,ah
	ret


