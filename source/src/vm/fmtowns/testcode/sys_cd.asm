; version 2003.03.04.1
;---------------------------------------------------------------------
;
; FM TOWNS �݊� ROM �V���[�Y
;
; FMT_SYS.ROM : CD�A�N�Z�X
;
; by Kasanova
;
;---------------------------------------------------------------------
; ���P�Ƃł̓A�Z���u�����܂���


%define CD_CMOS_PARA  3b60h
%define CD_CMOS_DATA  3b70h
%define CD_CMOS_DATA0 3b70h
%define CD_CMOS_DATA1 3b72h
%define CD_CMOS_DATA2 3b74h
%define CD_CMOS_DATA3 3b78h

;---------------------------------------------------------------------
; �ǂݍ���
; cl+dx : �ǂݍ��݊J�n�Z�N�^�ԍ�(16�i)
; bx    : �ǂݍ��ރZ�N�^��
; ds:di : �]����A�h���X
; [���^�[���R�[�h]
;  ah : 0(����I��)�Abx : �ǂݎc�����Z�N�^��

cd_command_05:
	call	cd_test_ready
	jc	.error1          ; �A�N�Z�X�ł����ԂłȂ�
	cmp	word [es:di+2],0 ; bx
	jz	.error2          ; �ǂݍ��ރZ�N�^�����s�K��
	
	; �ǂݍ��݊J�n�Z�N�^���Z�b�g
	mov	bx,CD_CMOS_PARA
	movzx	dx,byte [es:di+4] ; cl
	mov	ax,[si+2]         ; dx
	call	cd_set_sectorno

	; �ǂݍ��ݏI���Z�N�^���v�Z���A�Z�b�g
	movzx	dx,byte [es:di+4] ; cl
	mov	ax,[si+2]         ; dx
	mov	bx,[es:di+2]      ; bx
	dec	bx
	add	ax,bx
	adc	dx,0
	mov	bx,CD_CMOS_PARA+6
	call	cd_set_sectorno

	; DMA������
	call	cd_init_dma
	mov	ax,[es:di+0ah] ; di
	mov	dx,[es:di+0ch] ; es
	push	dx
	shl	dx,4
	add	ax,dx
	pop	dx
	shr	dx,12
	call	cd_dma_setaddress
	mov	ax,7ffh
	call	cd_dma_setlength
	in	al,0afh
	and	al,7
	out	0afh,al

	; �ǂݍ��݊J�n
	mov	al,022h
	call	cd_sendcommand
	call	cd_recieve4byte

	mov	dx,CD_CMOS_DATA0
	in	al,dx
	or	al,al
	jnz	.error3 ; �R�}���h���s�G���[

.readloop:
	call	cd_recieve4byte
	mov	dx,CD_CMOS_DATA0
	in	al,dx
	cmp	al,22h
	jz	.transfer
	cmp	al,6
	jz	.exit
	jmp	.error3 ; �G���[���A�ُ�ȃ��^�[���R�[�h

.transfer:
	call	cd_dma_transfer
	call	cd_dma_getaddress
	call	cd_dma_setaddress
	dec	word [es:di+2] ; bx �ǂݎc���Z�N�^�����炷
	jmp	.readloop

.exit:
	in	al,0afh
	or	al,8
	out	0afh,al
	xor	ah,ah
	call	cd_store_result
	ret

.error3:
	in	al,0afh
	or	al,8
	out	0afh,al
.error2:
	mov	ah,80h
	mov	cx,2
.error1:
	call	cd_store_result
	ret


;---------------------------------------------------------------------

; �h���C�u�̏�Ԃ��`�F�b�N
cd_command_0e:
	call	cd_test_ready
	call	cd_store_result
	ret


;---------------------------------------------------------------------
; ������

; �R�}���h���s���ʂ��i�[
cd_store_result:
	mov	[si+1],ah
	or	ah,ah
	jns	.noerrorcode
	mov	[es:di+4],cx
.noerrorcode:
	ret

;----------

; CD���ǂݍ��݉\�����`�F�b�N
cd_test_ready:
	call	cd_recieve
	call	cd_clear_parabuffer
	mov	al,0a0h
	call	cd_sendcommand
	call	cd_recieve4byte

	mov	dx,CD_CMOS_DATA1
	in	al,dx
	and	al,0fh

	mov	ah,80h
	cmp	al,9 ; �m�b�g���f�B
	jnz	.j1
	mov	cx,1
	stc
	ret
.j1:
	xor	ah,ah
	clc
	ret

;----------

; �p�����[�^�i�[�o�b�t�@���N���A
cd_clear_parabuffer:
	push	cx
	push	dx
	mov	dx,CD_CMOS_PARA
	xor	al,al
	mov	cx,8
.loop:
	out	dx,al
	add	dx,2
	loop	.loop
	pop	dx
	pop	cx
	ret

;----------

; CDC�R�}���h���s
cd_sendcommand:
	push	bx
	push	cx
	push	dx
	mov	ah,al
	mov	dx,4c0h
.waitready:
	in	al,dx
	test	al,1
	jz	.waitready

	mov	bx,CD_CMOS_PARA
	mov	cx,8
.commandloop:
	mov	dx,bx
	in	al,dx
	mov	dx,4c4h
	out	dx,al
	add	bx,2
	loop	.commandloop

	mov	al,ah
	mov	dx,4c2h
	out	dx,al
	pop	dx
	pop	cx
	pop	bx
	ret

;----------

; CDC�����4�o�C�g�̃X�e�[�^�X���擾
cd_recieve4byte:
	push	ax
	push	dx
	mov	dx,4c0h
.loop:
	in	al,dx
	test	al,2
	jz	.loop

	or	al,al

	mov	dx,4c2h
	in	al,dx
	mov	dx,CD_CMOS_DATA0
	out	dx,al
	mov	dx,4c2h
	in	al,dx
	mov	dx,CD_CMOS_DATA1
	out	dx,al
	mov	dx,4c2h
	in	al,dx
	mov	dx,CD_CMOS_DATA2
	out	dx,al
	mov	dx,4c2h
	in	al,dx
	mov	dx,CD_CMOS_DATA3
	out	dx,al

	jns	.exit

	mov	dx,4c0h ; clear irq
	mov	al,80h
	out	dx,al

.exit:
	pop	dx
	pop	ax
	ret

;----------

; �H�׎c�����N���A
cd_recieve:
	push	dx
	mov	dx,4c0h
	in	al,dx
	test	al,2
	jz	.exit

.loop:
	call	cd_recieve4byte
	in	al,dx
	test	al,2
	jnz	.loop
.exit:
	pop	dx
	ret

;----------

; 10�i�ϊ�
cd_hextodecimal:
	push	cx
	mov	ch,ah
	xor	ah,ah
	mov	cl,10
	div	cl
	shl	al,4
	add	al,ah
	mov	ah,ch
	pop	cx
	ret

;----------

; CD�̃Z�N�^�ԍ���10�i���ɕϊ����ĕۊ�
cd_set_sectorno:
	push	bx
	push	cx
	push	dx
	add	ax,150   ; CD�̐擪�Z�N�^�̓Z�N�^150
	adc	dx,0
	mov	cx,75*60 ; M-S-F �� M
	div	cx
	xchg	ax,dx
	; dx = M, ax = S-F
	mov	cl,75    ; S
	div	cl
	mov	cl,dl
	xchg	al,ah

	; cl-ah-al : M-S-F

	lea	dx,[bx+4]
	call	cd_hextodecimal
	out	dx,al
	sub	dx,2
	mov	al,ah
	call	cd_hextodecimal
	out	dx,al
	sub	dx,2
	mov	al,cl
	call	cd_hextodecimal
	out	dx,al
	pop	dx
	pop	cx
	pop	bx
	ret

;----------

cd_init_dma:
	; ���Z�b�g
	mov	al,3
	out	0a0h,al

	; �`���l���� CD �ɃZ�b�g
	mov	al,3
	out	0a1h,al

	; DMA����֎~
	mov	al,24h
	out	0a8h,al

	; �f�o�C�X�R���g���[��
	xor	al,al
	out	0a9h,al

	; ���[�h�R���g���[��
	mov	al,54h
	out	0aah,al
	ret

;----------

cd_dma_setlength:
	out	0a2h,ax
	ret

;----------

cd_dma_setaddress:
	out	0a4h,ax
	mov	ax,dx
	out	0a6h,ax
	ret

;----------

cd_dma_getaddress:
	in	ax,0a6h
	mov	dx,ax
	in	ax,0a4h
	ret

;----------

; DMA�]�����s��
cd_dma_transfer:
	push	dx
	; DMA���싖��
	mov	al,20h
	out	0a8h,al

	; �]���J�n
	mov	dx,4c6h
	mov	al,10h
	out	dx,al

	; �]�����I���܂ő҂�
	mov	dx,4c0h
.loop:
	in	al,dx
	test	al,10h
	jnz	.loop

	; DMA����֎~
	mov	al,24h
	out	0a8h,al
	pop	dx
	ret
