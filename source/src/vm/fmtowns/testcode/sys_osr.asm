; version 2003.03.04.1
;---------------------------------------------------------------------
;
; FM TOWNS �݊� ROM �V���[�Y
;
; FMT_SYS.ROM : RAM/ROM�h���C�u�A�N�Z�X
;
; by Kasanova
;
;---------------------------------------------------------------------
; ���P�Ƃł̓A�Z���u�����܂���

;---------------------------------------------------------------------
;�����l��
; �E���f�B�A�ԍ� 0x40 ��RAM���邢��ROM�ɐU��ꂽ���f�B�A�ԍ��ł���
; �E�A�N�Z�X��̕����A�h���X�́A���f�B�A�ԍ��̉���4�r�b�g�ɂ���Č��܂�
; �E�����I�ɂ�386SX�n�̃������}�b�v���l�����邱��
;
;   0: 00000000h- 7fffffffh
;   1: ����
;   2: c2000000h- c207ffffh (�������݋֎~)
;   3: ����
;   4: ����
;   5: ����
;   6: ����
;   7: 00000000h- ffffffffh (BYTE�A�N�Z�X)
;   8: 00000000h- ffffffffh (DWORD�A�N�Z�X)
;   9: c2000000h- c207ffffh (�������݋֎~)
;   a: c0000000h- c007ffffh IC���������݃`�F�b�N����
;   b: fffc0000h- ffffffffh (�������݋֎~)
;   c: 80000000h- 8007ffffh
;   d: 80100000h- 8017ffffh
;   e: c2140000h- c2141fffh
;   f: ����
;
;---------------------------------------------------------------------

;---------------------------------------------------------------------
; �ǂݍ��݁���������

osrom_command_05:
osrom_command_06:
	mov	al,[si]
	and	al,0fh

	cmp	al,8 ; �Ƃ肠�����A���ꂾ���Ή�
	jz	.ok
	jmp $
.ok:
	; ���W�X�^�̏��16�r�b�g��ύX���Ă͂����Ȃ�
	pushad

	xor	edx,edx
	movzx	esi,si
	mov	dx,ds
	shl	edx,4
	add	esi,edx

	xor	edx,edx
	movzx	edi,di
	mov	dx,es
	shl	edx,4
	add	edi,edx

	; ���������X�}�[�g�ȕ��@���l���܂��傤
	mov	ax,PMODE_TRANSFERMEM
	sub	sp,6
	mov	bp,sp
	sgdt	[bp]
	push	bp
	call	set_gdt
	call	call_pmode
	pop	bp
	lgdt	[bp]
	add	sp,6

	popad
	ret


