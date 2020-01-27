; version 2003.03.04.1
;---------------------------------------------------------------------
;
; FM TOWNS �݊� ROM �V���[�Y
;
; FMT_SYS.ROM : �v���e�N�g���[�h�E�v���V�W��(EXT-BOOT�����ɑ���)
;
; by Kasanova
;
;---------------------------------------------------------------------
; ���P�Ƃł̓A�Z���u�����܂���

; �R�[�h�������Ȃ��Ă�����A���@�Ɠ��l�� 0xFFFE0000-0xFFFE7FFF ��
; �ڂ��Ă���������

[BITS 32]
pmode_entry:
	movzx	esp,sp
	mov	ebp,esp
	mov	ax,ss
	movzx	eax,ax
	shl	eax,4
	add	esp,eax

	mov	ax,10h
	mov	ss,ax
	mov	ds,ax
	mov	es,ax

	push	ebp
	movzx	eax,bl
	call	[cs:pmode_jmptable+BOOTCODE_BASE+eax*4]
	pop	ebp

	mov	esp,ebp
	mov	ax,20h
	mov	ds,ax
	mov	es,ax
	mov	ss,ax

	db	0eah
	dd	.flush
	dw	18h
.flush:

[BITS 16]
	mov	eax,cr0
	and	al,0xfe
	mov	cr0,eax
	jmp	short $+2

	db	0eah
	dw	return_from_pmode
	dw	0fc00h


pmode_jmptable:
	dd	BOOTCODE_BASE+ pm_puticon
	dd	BOOTCODE_BASE+ pm_memorycheck
	dd	BOOTCODE_BASE+ pm_drawlogo
	dd	BOOTCODE_BASE+ pm_setpalette
	dd	BOOTCODE_BASE+ pm_transfermemory


[BITS 32]

;---------------------------------------------------------------------
; 32x32�A�C�R���\��
;
; cl = 0-127 : �V�X�e���A�C�R��
; cl = 128-  : �N���p�A�C�R��
pm_puticon:
	movzx	ecx,cl
	cmp	cl,128
	jc	.sysicon

	; �N���p�A�C�R���̏ꍇ
	sub	ecx,128
	shl	ecx,7
	lea	esi,[0ffffbc00h+ecx]
	jmp	.draw

	; �V�X�e���A�C�R���̏ꍇ
.sysicon:
	shl	ecx,8
	lea	esi,[0fffe8000h+ecx]
.draw:
	movzx	edi,dx
	add	edi,0c0000h
	mov	ecx,20h
.loop:
	movsd
	add	edi,VRAM_PITCH-4
	loop	.loop
	ret


;---------------------------------------------------------------------
; �������`�F�b�N��CMOS�ւ̏����o��
;
; ���������J�E���g�������ōs����悤�ɁA32�r�b�g�R�[�h�ŏ����Ă܂�
pm_memorycheck:
	; 3150h-317eh�́A�������̎�����Ԃ������炵��
	xor	al,al
	mov	dx,3150h
	mov	ecx,30h/2
.loop:
	out	dx,al
	add	dx,2
	loop	.loop

	; 5e8h�́A����Âł͕K�����p�\
	mov	dx,5e8h
	in	al,dx
	and	al,7fh
	mov	dx,3a5ch
	out	dx,al
	movzx	ecx,al

	mov	al,1
	mov	dx,3186h
	out	dx,al

	mov	eax,ecx
	shl	eax,4
	dec	eax
	mov	dx,318ah
	out	dx,al
	sub	dx,2
	mov	al,ah
	out	dx,al

	dec	ecx
	jz	.zero ; ��������1MB�����Ȃ��I

	mov	al,0ffh
	mov	dx,3150h
.loop2:
	out	dx,al
	dec	ecx
	jz	.zero
	add	dx,2
	cmp	dx,3180h
	jc	.loop2
.zero:
	mov	dx,31ach
	out	dx,al
	add	dx,2
	out	dx,al

	; TOWNS�����������J�E���g������Ȃ�A����

	ret


;---------------------------------------------------------------------
; �N�����S�\��
pm_drawlogo:
	; ds = es
	mov	esi,LOGO_ADDRESS
	mov	edi,0c0000h+VRAM_PITCH*130
	movzx	ebx,word [esi] ; �����h�b�g��
	add	ebx,7
	shr	ebx,3
	mov	ecx,VRAM_PITCH ; �\���ʒu����ʒ�����
	sub	ecx,ebx
	shr	ecx,1
	add	edi,ecx
	movzx	ecx,word [esi+2] ; �����h�b�g��

	add	esi,4

	mov	edx,0cff81h
	mov	al,[edx]
	and	al,0cfh
.loop:
	push	ecx
	; plane B
	push	edi
	mov	ecx,ebx
	mov	byte [edx],1
	rep movsb
	pop	edi

	; plane R
	push	edi
	mov	ecx,ebx
	mov	byte [edx],2
	rep movsb
	pop	edi

	; plane G
	push	edi
	mov	ecx,ebx
	mov	byte [edx],4
	rep movsb
	pop	edi

%if(LOGO_USEPLANES==4)
	; plane I
	push	edi
	mov	ecx,ebx
	mov	byte [edx],8
	rep movsb
	pop	edi
%endif

	lea	edi,[edi+VRAM_PITCH]
	pop	ecx
	loop	.loop

	mov	[edx],al
	ret

;---------------------------------------------------------------------
; �p���b�g������
;
; �����{���̊G�̃p���b�g��������̂ŁA�f�t�H���g�Œ�ɂ����ق�������
;   ��������Ȃ��E�E�E

pm_setpalette:
	mov	esi,LOGO_PAL_ADDRESS
	cmp	dword [esi], 0ffffffffh ; ����Ó��L�̃p���b�g�����邩�H
	jnz	.palexist
	mov	esi,pm_def_palette+BOOTCODE_BASE
.palexist:
	mov	ecx,16
.loop:
	mov	al,16
	sub	al,cl
	mov	dx,0fd90h
	out	dx,al
	mov	al,[esi]
	add	dx,2
	out	dx,al
	mov	al,[esi+1]
	add	dx,2
	out	dx,al
	mov	al,[esi+2]
	add	dx,2
	out	dx,al
	add	esi,3
	loop	.loop
	ret

pm_def_palette: ; B  - R  - G      B  - R  - G
	db	  0h,  0h,  0h,   80h,  0h,  0h
	db	  0h, 80h,  0h,   80h, 80h,  0h
	db	  0h,  0h, 80h,   80h,  0h, 80h
	db	  0h, 80h, 80h,   80h, 80h, 80h

	db	  0h,  0h,  0h,  0ffh,  0h,  0h
	db	  0h,0ffh,  0h,  0ffh,0ffh,  0h
	db	  0h,  0h,0ffh,  0ffh,  0h,0ffh
	db	  0h,0ffh,0ffh,  0ffh,0ffh,0ffh


;---------------------------------------------------------------------
; �������ԓ]��(sys_osr.asm����Ă΂��)
pm_transfermemory:
	movzx	ebp, byte [esi+1] ; ah�̒l�B�]������������Ŕ��肷��

	movzx	ebx,word [edi+4]
	shl	ebx,16
	mov	bx,[esi+2]
	shl	ebx,10           ; source

	push	esi
	push	edi

	movzx	esi,word [edi+0ch]
	shl	esi,4
	movzx	ecx,word [edi+0ah]
	add	esi,ecx          ; dest.

	mov	ecx,400h
	movzx	eax,word [edi+2] ; block count
	mul	ecx
	mov	ecx,eax

	mov	edi,esi
	mov	esi,ebx

	cmp	ebp,5
	jz	.noxchg
	xchg	esi,edi ; �������݃R�}���h�Ȃ�]�����Ɠ]��������ւ���
.noxchg:

	; �]������ۂ́A�o�C�g�A�N�Z�X�̂݉\�ȗ̈���l�����邱��
	rep movsb

	pop	edi
	pop	esi
	ret


