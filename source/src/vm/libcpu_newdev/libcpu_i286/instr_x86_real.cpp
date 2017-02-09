#include "./i86priv.h"

#ifdef I80286
#undef GetMemB
#undef GetMemW
#undef PutMemB
#undef PutMemW
#undef PUSH
#undef POP
#undef IOPL
#undef NT
#undef xF
#undef CompressFlags

#define GetMemB(Seg,Off)        (read_mem_byte(GetMemAddr(cpustate,Seg,Off,1,I80286_READ)))
#define GetMemW(Seg,Off)        (read_mem_word(GetMemAddr(cpustate,Seg,Off,2,I80286_READ)))
#define PutMemB(Seg,Off,x)      write_mem_byte(GetMemAddr(cpustate,Seg,Off,1,I80286_WRITE), (x))
#define PutMemW(Seg,Off,x)      write_mem_word(GetMemAddr(cpustate,Seg,Off,2,I80286_WRITE), (x))

#define PUSH(val)               { if(PM) i80286_check_permission(cpustate, SS, cpustate->regs.w[SP]-2, I80286_WORD, I80286_WRITE); cpustate->regs.w[SP] -= 2; WriteWord(((cpustate->base[SS] + cpustate->regs.w[SP]) & AMASK), val); }
#define POP(var)                { if(PM) i80286_check_permission(cpustate, SS, cpustate->regs.w[SP], I80286_WORD, I80286_READ); cpustate->regs.w[SP] += 2; var = ReadWord(((cpustate->base[SS] + ((cpustate->regs.w[SP]-2) & 0xffff)) & AMASK)); }

#define IOPL ((cpustate->flags&0x3000)>>12)
#define NT ((cpustate->flags&0x4000)>>14)
#define xF (0)

#define CompressFlags() (WORD)(CF | 2 |(PF << 2) | (AF << 4) | (ZF << 6) \
				| (SF << 7) | (cpustate->TF << 8) | (cpustate->IF << 9) | (cpustate->MF << 15) \
				| (DF << 10) | (OF << 11) | (IOPL << 12) | (NT << 14) | (xF << 15))

#endif
#include "./ix86_opdef_real.h"
#ifdef I80286
#include "./modrm286.h"
#else
#include "./modrm.h"
#endif

// Interrupt

// 80186
void IX86_OPS::PREFIX186(_pusha)()    /* Opcode 0x60 */
{
	unsigned tmp=cpustate->regs.w[SP];

#ifdef I80286
	if(PM) i80286_check_permission(SS, cpustate->regs.w[SP]-16, 16, I80286_WRITE);
#endif
	ICOUNT -= timing.pusha;
	PUSH(cpustate->regs.w[AX]);
	PUSH(cpustate->regs.w[CX]);
	PUSH(cpustate->regs.w[DX]);
	PUSH(cpustate->regs.w[BX]);
	PUSH(tmp);
	PUSH(cpustate->regs.w[BP]);
	PUSH(cpustate->regs.w[SI]);
	PUSH(cpustate->regs.w[DI]);
}

static unsigned i186_popa_tmp_r;  // hack around GCC 4.6 error because we need the side effects of POP
void IX86_OPS::PREFIX186(_popa)()    /* Opcode 0x61 */
{

#ifdef I80286
	if(PM) i80286_check_permission(SS, cpustate->regs.w[SP], 16, I80286_READ);
#endif
	ICOUNT -= timing.popa;
	POP(cpustate->regs.w[DI]);
	POP(cpustate->regs.w[SI]);
	POP(cpustate->regs.w[BP]);
	POP(i186_popa_tmp_r);
	POP(cpustate->regs.w[BX]);
	POP(cpustate->regs.w[DX]);
	POP(cpustate->regs.w[CX]);
	POP(cpustate->regs.w[AX]);
}

void IX86_OPS::PREFIX186(_insb)()    /* Opcode 0x6c */
{
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.ins8;
	PutMemB(ES,cpustate->regs.w[DI],read_port_byte(cpustate->regs.w[DX]));
	cpustate->regs.w[DI] += cpustate->DirVal;
}

void IX86_OPS::PREFIX186(_insw)()    /* Opcode 0x6d */
{
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.ins16;
	PutMemW(ES,cpustate->regs.w[DI],read_port_word(cpustate->regs.w[DX]));
	cpustate->regs.w[DI] += 2 * cpustate->DirVal;
}

void IX86_OPS::PREFIX186(_outsb)()    /* Opcode 0x6e */
{
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.outs8;
	write_port_byte(cpustate->regs.w[DX],GetMemB(DS,cpustate->regs.w[SI]));
	cpustate->regs.w[SI] += cpustate->DirVal; /* GOL 11/27/01 */
}

void IX86_OPS::PREFIX186(_outsw)()    /* Opcode 0x6f */
{
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.outs16;
	write_port_word(cpustate->regs.w[DX],GetMemW(DS,cpustate->regs.w[SI]));
	cpustate->regs.w[SI] += 2 * cpustate->DirVal; /* GOL 11/27/01 */
}

void IX86_OPS::PREFIX186(_enter)()    /* Opcode 0xc8 */
{
	unsigned nb = FETCH;
	unsigned i,level;
	UINT16 fp;

	nb += FETCH << 8;
#ifdef I80286
	level = FETCH & 0x1f;
	if(PM) i80286_check_permission(SS, cpustate->regs.w[SP]-2-(level*2), 2+(level*2), I80286_WRITE);
#else
	level = FETCH;
#endif
	ICOUNT -= (level == 0) ? timing.enter0 : (level == 1) ? timing.enter1 : timing.enter_base + level * timing.enter_count;
	PUSH(cpustate->regs.w[BP]);
	fp = cpustate->regs.w[SP];
	for (i=1;i<level;i++)
		PUSH(GetMemW(SS,cpustate->regs.w[BP]-i*2));
	if (level) PUSH(fp);
	cpustate->regs.w[BP] = fp;
	cpustate->regs.w[SP] -= nb;
}

// 98086
void IX86_OPS::PREFIX86(_trap)()
{
	PREFIX(_instruction)[FETCHOP]();
	PREFIX(_interrupt)(1);
}


void IX86_OPS::PREFIX86(_rotate_shift_Byte)(unsigned ModRM, unsigned count, unsigned src)
{
//  unsigned src = (unsigned)GetRMByte(ModRM);
	unsigned dst=src;

	if (count==0)
	{
		ICOUNT -= (ModRM >= 0xc0) ? timing.rot_reg_base : timing.rot_m8_base;
	}
	else if (count==1)
	{
		ICOUNT -= (ModRM >= 0xc0) ? timing.rot_reg_1 : timing.rot_m8_1;

		switch (ModRM & 0x38)
		{
		case 0x00:  /* ROL eb,1 */
			cpustate->CarryVal = src & 0x80;
			dst=(src<<1)+CF;
			PutbackRMByte(ModRM,dst);
			cpustate->OverVal = (src^dst)&0x80;
			break;
		case 0x08:  /* ROR eb,1 */
			cpustate->CarryVal = src & 0x01;
			dst = ((CF<<8)+src) >> 1;
			PutbackRMByte(ModRM,dst);
			cpustate->OverVal = (src^dst)&0x80;
			break;
		case 0x10:  /* RCL eb,1 */
			dst=(src<<1)+CF;
			PutbackRMByte(ModRM,dst);
			SetCFB(dst);
			cpustate->OverVal = (src^dst)&0x80;
			break;
		case 0x18:  /* RCR eb,1 */
			dst = ((CF<<8)+src) >> 1;
			PutbackRMByte(ModRM,dst);
			cpustate->CarryVal = src & 0x01;
			cpustate->OverVal = (src^dst)&0x80;
			break;
		case 0x20:  /* SHL eb,1 */
		case 0x30:
			dst = src << 1;
			PutbackRMByte(ModRM,dst);
			SetCFB(dst);
			cpustate->OverVal = (src^dst)&0x80;
			cpustate->AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		case 0x28:  /* SHR eb,1 */
			dst = src >> 1;
			PutbackRMByte(ModRM,dst);
			cpustate->CarryVal = src & 0x01;
			cpustate->OverVal = src & 0x80;
			cpustate->AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		case 0x38:  /* SAR eb,1 */
			dst = ((INT8)src) >> 1;
			PutbackRMByte(ModRM,dst);
			cpustate->CarryVal = src & 0x01;
			cpustate->OverVal = 0;
			cpustate->AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		}
	}
	else
	{
		int tmpcf = CF;
		ICOUNT -= (ModRM >= 0xc0) ? timing.rot_reg_base + (timing.rot_reg_bit * count) : timing.rot_m8_base + (timing.rot_m8_bit * count);

		switch (ModRM & 0x38)
		{
		case 0x00:  /* ROL eb,count */
			for (; count > 0; count--)
			{
				cpustate->CarryVal = dst & 0x80;
				dst = (dst << 1) + CF;
			}
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x08:  /* ROR eb,count */
			for (; count > 0; count--)
			{
				cpustate->CarryVal = dst & 0x01;
				dst = (dst >> 1) + (CF << 7);
			}
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x10:  /* RCL eb,count */
			for (; count > 0; count--)
			{
				dst = (dst << 1) + tmpcf;
				tmpcf = (int)((dst & 0x100) != 0);
			}
			PutbackRMByte(ModRM,(BYTE)dst);
			cpustate->CarryVal = tmpcf;
			break;
		case 0x18:  /* RCR eb,count */
			for (; count > 0; count--)
			{
				dst = (tmpcf<<8)+dst;
				tmpcf = dst & 0x01;
				dst >>= 1;
			}
			PutbackRMByte(ModRM,(BYTE)dst);
			cpustate->CarryVal = tmpcf;
			break;
		case 0x20:
		case 0x30:  /* SHL eb,count */
			for(int i=0;i<count;i++) dst<<= 1;
			SetCFB(dst);
			cpustate->AuxVal = 1;
			SetSZPF_Byte(dst);
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x28:  /* SHR eb,count */
			for(int i=0;i<count-1;i++) dst>>= 1;
			cpustate->CarryVal = dst & 0x1;
			dst >>= 1;
			SetSZPF_Byte(dst);
			cpustate->AuxVal = 1;
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		case 0x38:  /* SAR eb,count */
			for(int i=0;i<count-1;i++) dst = ((INT8)dst) >> 1;
			cpustate->CarryVal = dst & 0x1;
			dst = ((INT8)((BYTE)dst)) >> 1;
			SetSZPF_Byte(dst);
			cpustate->AuxVal = 1;
			PutbackRMByte(ModRM,(BYTE)dst);
			break;
		}
	}
}

void IX86_OPS::PREFIX86(_rotate_shift_Word)(unsigned ModRM, unsigned count, unsigned src)
{
//  unsigned src = GetRMWord(ModRM);
	unsigned dst=src;

	if (count==0)
	{
		ICOUNT -= (ModRM >= 0xc0) ? timing.rot_reg_base : timing.rot_m16_base;
	}
	else if (count==1)
	{
		ICOUNT -= (ModRM >= 0xc0) ? timing.rot_reg_1 : timing.rot_m16_1;

		switch (ModRM & 0x38)
		{
#if 0
		case 0x00:  /* ROL ew,1 */
			tmp2 = (tmp << 1) + CF;
			SetCFW(tmp2);
			cpustate->OverVal = !(!(tmp & 0x4000)) != CF;
			PutbackRMWord(ModRM,tmp2);
			break;
		case 0x08:  /* ROR ew,1 */
			cpustate->CarryVal = tmp & 0x01;
			tmp2 = (tmp >> 1) + ((unsigned)CF << 15);
			cpustate->OverVal = !(!(tmp & 0x8000)) != CF;
			PutbackRMWord(ModRM,tmp2);
			break;
		case 0x10:  /* RCL ew,1 */
			tmp2 = (tmp << 1) + CF;
			SetCFW(tmp2);
			cpustate->OverVal = (tmp ^ (tmp << 1)) & 0x8000;
			PutbackRMWord(ModRM,tmp2);
			break;
		case 0x18:  /* RCR ew,1 */
			tmp2 = (tmp >> 1) + ((unsigned)CF << 15);
			cpustate->OverVal = !(!(tmp & 0x8000)) != CF;
			cpustate->CarryVal = tmp & 0x01;
			PutbackRMWord(ModRM,tmp2);
			break;
		case 0x20:  /* SHL ew,1 */
		case 0x30:
			tmp <<= 1;

			SetCFW(tmp);
			SetOFW_Add(tmp,tmp2,tmp2);
			cpustate->AuxVal = 1;
			SetSZPF_Word(tmp);

			PutbackRMWord(ModRM,tmp);
			break;
		case 0x28:  /* SHR ew,1 */
			cpustate->CarryVal = tmp & 0x01;
			cpustate->OverVal = tmp & 0x8000;

			tmp2 = tmp >> 1;

			SetSZPF_Word(tmp2);
			cpustate->AuxVal = 1;
			PutbackRMWord(ModRM,tmp2);
			break;
			case 0x38:  /* SAR ew,1 */
			cpustate->CarryVal = tmp & 0x01;
			cpustate->OverVal = 0;

			tmp2 = (tmp >> 1) | (tmp & 0x8000);

			SetSZPF_Word(tmp2);
			cpustate->AuxVal = 1;
			PutbackRMWord(ModRM,tmp2);
			break;
#else
		case 0x00:  /* ROL ew,1 */
			cpustate->CarryVal = src & 0x8000;
			dst=(src<<1)+CF;
			PutbackRMWord(ModRM,dst);
			cpustate->OverVal = (src^dst)&0x8000;
			break;
		case 0x08:  /* ROR ew,1 */
			cpustate->CarryVal = src & 0x01;
			dst = ((CF<<16)+src) >> 1;
			PutbackRMWord(ModRM,dst);
			cpustate->OverVal = (src^dst)&0x8000;
			break;
		case 0x10:  /* RCL ew,1 */
			dst=(src<<1)+CF;
			PutbackRMWord(ModRM,dst);
			SetCFW(dst);
			cpustate->OverVal = (src^dst)&0x8000;
			break;
		case 0x18:  /* RCR ew,1 */
			dst = ((CF<<16)+src) >> 1;
			PutbackRMWord(ModRM,dst);
			cpustate->CarryVal = src & 0x01;
			cpustate->OverVal = (src^dst)&0x8000;
			break;
		case 0x20:  /* SHL ew,1 */
		case 0x30:
			dst = src << 1;
			PutbackRMWord(ModRM,dst);
			SetCFW(dst);
			cpustate->OverVal = (src^dst)&0x8000;
			cpustate->AuxVal = 1;
			SetSZPF_Word(dst);
			break;
		case 0x28:  /* SHR ew,1 */
			dst = src >> 1;
			PutbackRMWord(ModRM,dst);
			cpustate->CarryVal = src & 0x01;
			cpustate->OverVal = src & 0x8000;
			cpustate->AuxVal = 1;
			SetSZPF_Word(dst);
			break;
		case 0x38:  /* SAR ew,1 */
			dst = ((INT16)src) >> 1;
			PutbackRMWord(ModRM,dst);
			cpustate->CarryVal = src & 0x01;
			cpustate->OverVal = 0;
			cpustate->AuxVal = 1;
			SetSZPF_Word(dst);
			break;
#endif
		}
	}
	else
	{
		int tmpcf = CF;
		ICOUNT -= (ModRM >= 0xc0) ? timing.rot_reg_base + (timing.rot_reg_bit * count) : timing.rot_m8_base + (timing.rot_m16_bit * count);

		switch (ModRM & 0x38)
		{
		case 0x00:  /* ROL ew,count */
			for (; count > 0; count--)
			{
				cpustate->CarryVal = dst & 0x8000;
				dst = (dst << 1) + CF;
			}
			PutbackRMWord(ModRM,dst);
			break;
		case 0x08:  /* ROR ew,count */
			for (; count > 0; count--)
			{
				cpustate->CarryVal = dst & 0x01;
				dst = (dst >> 1) + (CF << 15);
			}
			PutbackRMWord(ModRM,dst);
			break;
		case 0x10:  /* RCL ew,count */
			for (; count > 0; count--)
			{
				dst = (dst << 1) + tmpcf;
				tmpcf = (int)((dst & 0x10000) != 0);
			}
			PutbackRMWord(ModRM,dst);
			cpustate->CarryVal = tmpcf;
			break;
		case 0x18:  /* RCR ew,count */
			for (; count > 0; count--)
			{
				dst = dst + (tmpcf << 16);
				tmpcf = dst & 0x01;
				dst >>= 1;
			}
			PutbackRMWord(ModRM,dst);
			cpustate->CarryVal = tmpcf;
			break;
		case 0x20:
		case 0x30:  /* SHL ew,count */
			for(int i=0;i<count;i++) dst<<= 1;
			SetCFW(dst);
			cpustate->AuxVal = 1;
			SetSZPF_Word(dst);
			PutbackRMWord(ModRM,dst);
			break;
		case 0x28:  /* SHR ew,count */
			for(int i=0;i<count-1;i++) dst>>= 1;
			cpustate->CarryVal = dst & 0x1;
			dst >>= 1;
			SetSZPF_Word(dst);
			cpustate->AuxVal = 1;
			PutbackRMWord(ModRM,dst);
			break;
		case 0x38:  /* SAR ew,count */
			for(int i=0;i<count-1;i++) dst = ((INT16)dst) >> 1;
			cpustate->CarryVal = dst & 0x01;
			dst = ((INT16)((WORD)dst)) >> 1;
			SetSZPF_Word(dst);
			cpustate->AuxVal = 1;
			PutbackRMWord(ModRM,dst);
			break;
		}
	}
}

void IX86_OPS::PREFIX86(_pop_ds)(i8086_state *cpustate)    /* Opcode 0x1f */
{
#ifdef I80286
	i80286_pop_seg(cpustate,DS);
#else
	POP(cpustate->sregs[DS]);
	cpustate->base[DS] = SegBase(DS);
#endif
	ICOUNT -= timing.push_seg;
}


void IX86_OPS::PREFIX(rep)(int flagval)
{
	/* Handles rep- and repnz- prefixes. flagval is the value of ZF for the
	     loop  to continue for CMPS and SCAS instructions. */

	unsigned next = FETCHOP;

	switch(next)
	{
	case 0x26:  /* ES: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = ES;
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.override;
		PREFIX(rep)(cpustate, flagval);
		break;
	case 0x2e:  /* CS: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = CS;
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.override;
		PREFIX(rep)(cpustate, flagval);
		break;
	case 0x36:  /* SS: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = SS;
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.override;
		PREFIX(rep)(cpustate, flagval);
		break;
	case 0x3e:  /* DS: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = DS;
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.override;
		PREFIX(rep)(cpustate, flagval);
		break;
#ifndef I8086
	case 0x6c:  /* REP INSB */
#ifdef I80286
		if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_ins8_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			PutMemB(ES,cpustate->regs.w[DI],read_port_byte(cpustate->regs.w[DX]));
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += cpustate->DirVal;
			ICOUNT -= timing.rep_ins8_count;
		}
		break;
	case 0x6d:  /* REP INSW */
#ifdef I80286
		if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_ins16_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			PutMemW(ES,cpustate->regs.w[DI],read_port_word(cpustate->regs.w[DX]));
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += 2 * cpustate->DirVal;
			ICOUNT -= timing.rep_ins16_count;
		}
		break;
	case 0x6e:  /* REP OUTSB */
#ifdef I80286
		if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_outs8_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			write_port_byte(cpustate->regs.w[DX],GetMemB(DS,cpustate->regs.w[SI]));
			cpustate->regs.w[CX]--;
			cpustate->regs.w[SI] += cpustate->DirVal; /* GOL 11/27/01 */
			ICOUNT -= timing.rep_outs8_count;
		}
		break;
	case 0x6f:  /* REP OUTSW */
#ifdef I80286
		if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_outs16_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			write_port_word(cpustate->regs.w[DX],GetMemW(DS,cpustate->regs.w[SI]));
			cpustate->regs.w[CX]--;
			cpustate->regs.w[SI] += 2 * cpustate->DirVal; /* GOL 11/27/01 */
			ICOUNT -= timing.rep_outs16_count;
		}
		break;
#endif
	case 0xa4:  /* REP MOVSB */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_movs8_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
			BYTE tmp;

//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			tmp = GetMemB(DS,cpustate->regs.w[SI]);
			PutMemB(ES,cpustate->regs.w[DI], tmp);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += cpustate->DirVal;
			cpustate->regs.w[SI] += cpustate->DirVal;
			ICOUNT -= timing.rep_movs8_count;
		}
		break;
	case 0xa5:  /* REP MOVSW */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_movs16_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
			WORD tmp;

//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			tmp = GetMemW(DS,cpustate->regs.w[SI]);
			PutMemW(ES,cpustate->regs.w[DI], tmp);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += 2 * cpustate->DirVal;
			cpustate->regs.w[SI] += 2 * cpustate->DirVal;
			ICOUNT -= timing.rep_movs16_count;
		}
		break;
	case 0xa6:  /* REP(N)E CMPSB */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_cmps8_base;
		cpustate->rep_in_progress = FALSE;
		cpustate->ZeroVal = !flagval;
		while(cpustate->regs.w[CX] && (ZF == flagval))
		{
			unsigned dst, src;

//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			dst = GetMemB(ES, cpustate->regs.w[DI]);
			src = GetMemB(DS, cpustate->regs.w[SI]);
			SUBB(src,dst); /* opposite of the usual convention */
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += cpustate->DirVal;
			cpustate->regs.w[SI] += cpustate->DirVal;
			ICOUNT -= timing.rep_cmps8_count;
		}
		break;
	case 0xa7:  /* REP(N)E CMPSW */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_cmps16_base;
		cpustate->rep_in_progress = FALSE;
		cpustate->ZeroVal = !flagval;
		while(cpustate->regs.w[CX] && (ZF == flagval))
		{
			unsigned dst, src;

//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			dst = GetMemW(ES, cpustate->regs.w[DI]);
			src = GetMemW(DS, cpustate->regs.w[SI]);
			SUBW(src,dst); /* opposite of the usual convention */
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += 2 * cpustate->DirVal;
			cpustate->regs.w[SI] += 2 * cpustate->DirVal;
			ICOUNT -= timing.rep_cmps16_count;
		}
		break;
	case 0xaa:  /* REP STOSB */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_stos8_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			PutMemB(ES,cpustate->regs.w[DI],cpustate->regs.b[AL]);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += cpustate->DirVal;
			ICOUNT -= timing.rep_stos8_count;
		}
		break;
	case 0xab:  /* REP STOSW */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_stos16_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			PutMemW(ES,cpustate->regs.w[DI],cpustate->regs.w[AX]);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += 2 * cpustate->DirVal;
			ICOUNT -= timing.rep_stos16_count;
		}
		break;
	case 0xac:  /* REP LODSB */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_lods8_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			cpustate->regs.b[AL] = GetMemB(DS,cpustate->regs.w[SI]);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[SI] += cpustate->DirVal;
			ICOUNT -= timing.rep_lods8_count;
		}
		break;
	case 0xad:  /* REP LODSW */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_lods16_base;
		cpustate->rep_in_progress = FALSE;
		while(cpustate->regs.w[CX])
		{
//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			cpustate->regs.w[AX] = GetMemW(DS,cpustate->regs.w[SI]);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[SI] += 2 * cpustate->DirVal;
			ICOUNT -= timing.rep_lods16_count;
		}
		break;
	case 0xae:  /* REP(N)E SCASB */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_scas8_base;
		cpustate->rep_in_progress = FALSE;
		cpustate->ZeroVal = !flagval;
		while(cpustate->regs.w[CX] && (ZF == flagval))
		{
			unsigned src, dst;

//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			src = GetMemB(ES, cpustate->regs.w[DI]);
			dst = cpustate->regs.b[AL];
			SUBB(dst,src);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += cpustate->DirVal;
			ICOUNT -= timing.rep_scas8_count;
		}
		break;
	case 0xaf:  /* REP(N)E SCASW */
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.rep_scas16_base;
		cpustate->rep_in_progress = FALSE;
		cpustate->ZeroVal = !flagval;
		while(cpustate->regs.w[CX] && (ZF == flagval))
		{
			unsigned src, dst;

//			if (ICOUNT <= 0) { cpustate->pc = cpustate->prevpc; cpustate->rep_in_progress = TRUE; break; }
			src = GetMemW(ES, cpustate->regs.w[DI]);
			dst = cpustate->regs.w[AX];
			SUBW(dst,src);
			cpustate->regs.w[CX]--;
			cpustate->regs.w[DI] += 2 * cpustate->DirVal;
			ICOUNT -= timing.rep_scas16_count;
		}
		break;
	default:
		PREFIX(_instruction)[next](cpustate);
	}
}


void IX86_OPS::PREFIX86(_pop_es)()    /* Opcode 0x07 */
{
#ifdef I80286
	i80286_pop_seg(cpustate,ES);
#else
	POP(cpustate->sregs[ES]);
	cpustate->base[ES] = SegBase(ES);
#endif
	ICOUNT -= timing.pop_seg;
}


void IX86_OPS::PREFIX86(_pop_cs)()    /* Opcode 0x0f */
{
#ifndef I80286
	int ip = cpustate->pc - cpustate->base[CS];
	ICOUNT -= timing.push_seg;
	POP(cpustate->sregs[CS]);
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (ip + cpustate->base[CS]) & AMASK;
	CHANGE_PC(cpustate->pc);
#endif
}

void IX86_OPS::PREFIX86(_push_sp)()    /* Opcode 0x54 */
{
	ICOUNT -= timing.push_r16;
#ifdef I80286
	PUSH(cpustate->regs.w[SP]+2);
#else
	PUSH(cpustate->regs.w[SP]);
#endif
}
void IX86_OPS_BASE::PREFIX186(_bound)()    /* Opcode 0x62 */
{
	unsigned ModRM = FETCHOP;
	int low = (INT16)GetRMWord(ModRM);
	int high= (INT16)GetnextRMWord;
	int tmp= (INT16)RegWord(ModRM);
	if (tmp<low || tmp>high) {
		cpustate->pc-= (cpustate->seg_prefix ? 3 : 2 );
		PREFIX(_interrupt)(5);
	}
	ICOUNT -= timing.bound;
}

void IX86_OPS::PREFIX86(_call_far)()
{
	unsigned int tmp, tmp2;
	WORD cs, ip;

	tmp = FETCH;
	tmp += FETCH << 8;

	tmp2 = FETCH;
	tmp2 += FETCH << 8;

#ifdef I86_PSEUDO_BIOS
	if(cpustate->bios != NULL && cpustate->bios->bios_call_i86(((tmp2 << 4) + tmp) & AMASK, cpustate->regs.w, cpustate->sregs, &cpustate->ZeroVal, &cpustate->CarryVal)) {
		ICOUNT -= timing.call_far;
		return;
	}
#endif

	ip = cpustate->pc - cpustate->base[CS];
	cs = cpustate->sregs[CS];

#ifdef I80286
	i80286_code_descriptor(cpustate, tmp2, tmp, 2);
#else
	cpustate->sregs[CS] = (WORD)tmp2;
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->base[CS] + (WORD)tmp) & AMASK;
#endif
	PUSH(cs);
	PUSH(ip);
	ICOUNT -= timing.call_far;
	CHANGE_PC(cpustate->pc);
}

void IX86_OPS::PREFIX86(_wait)()    /* Opcode 0x9b */
{
#ifdef I80286
	if ((cpustate->msw&0x0a) == 0x0a) throw TRAP(FPU_UNAVAILABLE,-1);
#endif
	if (cpustate->test_state)
	{
		ICOUNT = 0;
		cpustate->pc--;
	}
	else
		ICOUNT -= timing.wait;
}

void IX86_OPS::PREFIX86(_pushf)()    /* Opcode 0x9c */
{
	unsigned tmp;
	ICOUNT -= timing.pushf;

	tmp = CompressFlags();
#ifdef I80286
	if(!PM) ( tmp &= ~0xf000 );
#endif
	PUSH( tmp );
}

#ifndef I80286
void IX86_OPS::PREFIX86(_popf)()    /* Opcode 0x9d */
{
	unsigned tmp;
	POP(tmp);
	ICOUNT -= timing.popf;

	ExpandFlags(tmp);
	cpustate->flags = tmp;
	cpustate->flags = CompressFlags();

	if (cpustate->TF) PREFIX(_trap)(cpustate);

	/* if the IF is set, and an interrupt is pending, signal an interrupt */
	if (cpustate->IF && cpustate->irq_state) {
		PREFIX(_interrupt)(cpustate, (UINT32)-1);
		cpustate->irq_state = 0;
	}
}
#endif

void IX86_OPS::PREFIX86(_les_dw)()    /* Opcode 0xc4 */
{
	unsigned ModRM = FETCH;
	WORD tmp = GetRMWord(ModRM);

#ifdef I80286
	i80286_data_descriptor(cpustate,ES,GetnextRMWord);
#else
	cpustate->sregs[ES] = GetnextRMWord;
	cpustate->base[ES] = SegBase(ES);
#endif
	RegWord(ModRM)= tmp;
	ICOUNT -= timing.load_ptr;
}

void IX86_OPS::PREFIX86(_lds_dw)()    /* Opcode 0xc5 */
{
	unsigned ModRM = FETCH;
	WORD tmp = GetRMWord(ModRM);

#ifdef I80286
	i80286_data_descriptor(cpustate,DS,GetnextRMWord);
#else
	cpustate->sregs[DS] = GetnextRMWord;
	cpustate->base[DS] = SegBase(DS);
#endif
	RegWord(ModRM)=tmp;
	ICOUNT -= timing.load_ptr;
}

#ifndef I80286
void IX86_OPS::PREFIX86(_retf_d16)()    /* Opcode 0xca */
{
	unsigned count = FETCH;
	count += FETCH << 8;

	POP(cpustate->pc);
	POP(cpustate->sregs[CS]);
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
	cpustate->regs.w[SP]+=count;
	ICOUNT -= timing.ret_far_imm;
	CHANGE_PC(cpustate->pc);
}

void IX86_OPS::PREFIX86(_retf)()    /* Opcode 0xcb */
{
	POP(cpustate->pc);
	POP(cpustate->sregs[CS]);
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
	ICOUNT -= timing.ret_far;
	CHANGE_PC(cpustate->pc);
}
#endif

void IX86_OPS::PREFIX86(_int)()    /* Opcode 0xcd */
{
	unsigned int_num = FETCH;
	ICOUNT -= timing.int_imm;
#ifdef I86_PSEUDO_BIOS
	if(cpustate->bios != NULL && cpustate->bios->bios_int_i86(int_num, cpustate->regs.w, cpustate->sregs, &cpustate->ZeroVal, &cpustate->CarryVal)) {
		return;
	}
#endif
	PREFIX(_interrupt)(cpustate, int_num);
}

#ifndef I80286
void IX86_OPS::PREFIX86(_iret)(i8086_state *cpustate)    /* Opcode 0xcf */
{
	ICOUNT -= timing.iret;
	POP(cpustate->pc);
	POP(cpustate->sregs[CS]);
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
		PREFIX(_popf)(cpustate);
	CHANGE_PC(cpustate->pc);

	/* if the IF is set, and an interrupt is pending, signal an interrupt */
	if (cpustate->IF && cpustate->irq_state) {
		PREFIX(_interrupt)(cpustate, (UINT32)-1);
		cpustate->irq_state = 0;
	}
}
#endif

void IX86_BASE::PREFIX86(_rotshft_b)()    /* Opcode 0xd0 */
{
	unsigned ModRM = FETCHOP;
	PREFIX(_rotate_shift_Byte)(cpustate,ModRM,1,GetRMByte(ModRM));
}

void IX86_BASE::PREFIX86(_rotshft_w)()    /* Opcode 0xd1 */
{
	unsigned ModRM = FETCHOP;
	PREFIX(_rotate_shift_Word)(cpustate,ModRM,1,GetRMWord(ModRM));
}

#ifdef I8086
void IX86_OPS::PREFIX86(_rotshft_bcl)()    /* Opcode 0xd2 */
{
	unsigned ModRM = FETCHOP;
	PREFIX(_rotate_shift_Byte)(cpustate,ModRM,cpustate->regs.b[CL],GetRMByte(ModRM));
}

void IX86_OPS::PREFIX86(_rotshft_wcl)()    /* Opcode 0xd3 */
{
	unsigned ModRM = FETCHOP;
	PREFIX(_rotate_shift_Word)(cpustate,ModRM,cpustate->regs.b[CL],GetRMWord(ModRM));
}
#endif
/* OB: Opcode works on NEC V-Series but not the Variants              */
/*     one could specify any byte value as operand but the NECs */
/*     always substitute 0x0a.              */
void IX86_BASE::PREFIX86(_aam)()    /* Opcode 0xd4 */
{
	unsigned mult = FETCH;

	ICOUNT -= timing.aam;
	if (mult == 0)
		PREFIX(_interrupt)(cpustate, 0);
	else
	{
		cpustate->regs.b[AH] = cpustate->regs.b[AL] / mult;
		cpustate->regs.b[AL] %= mult;

		SetSZPF_Word(cpustate->regs.w[AX]);
	}
}

#ifndef I80286
void IX86_OPS::PREFIX86(_escape)()    /* Opcodes 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde and 0xdf */
{
	unsigned ModRM = FETCH;
	ICOUNT -= timing.nop;
	GetRMByte(ModRM);
}
#endif

void IX86_OPS::PREFIX86(_inal)()    /* Opcode 0xe4 */
{
	unsigned port;
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	port = FETCH;

	ICOUNT -= timing.in_imm8;
	cpustate->regs.b[AL] = read_port_byte(port);
}

void IX86_OPS::PREFIX86(_inax)()    /* Opcode 0xe5 */
{
	unsigned port;
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	port = FETCH;

	ICOUNT -= timing.in_imm16;
	cpustate->regs.w[AX] = read_port_word(port);
}

void IX86_OPS::PREFIX86(_outal)()    /* Opcode 0xe6 */
{
	unsigned port;
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	port = FETCH;

	ICOUNT -= timing.out_imm8;
	write_port_byte(port, cpustate->regs.b[AL]);
}

void IX86_OPS::PREFIX86(_outax)()    /* Opcode 0xe7 */
{
	unsigned port;
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	port = FETCH;

	ICOUNT -= timing.out_imm16;
	write_port_word(port, cpustate->regs.w[AX]);
}

void IX86_OPS::PREFIX86(_call_d16)()    /* Opcode 0xe8 */
{
	WORD ip, tmp;

	FETCHWORD(tmp);
#ifdef I86_PSEUDO_BIOS
	if(cpustate->bios != NULL && cpustate->bios->bios_call_i86((cpustate->pc + tmp) & AMASK, cpustate->regs.w, cpustate->sregs, &cpustate->ZeroVal, &cpustate->CarryVal)) {
		ICOUNT -= timing.call_near;
		return;
	}
#endif
	ip = cpustate->pc - cpustate->base[CS];
	PUSH(ip);
	ip += tmp;
	cpustate->pc = (ip + cpustate->base[CS]) & AMASK;
	ICOUNT -= timing.call_near;
	CHANGE_PC(cpustate->pc);
}

void IX86_OPS::PREFIX86(_jmp_d16)()    /* Opcode 0xe9 */
{
	WORD ip, tmp;

	FETCHWORD(tmp);
	ip = cpustate->pc - cpustate->base[CS] + tmp;
	cpustate->pc = (ip + cpustate->base[CS]) & AMASK;
	ICOUNT -= timing.jmp_near;
	CHANGE_PC(cpustate->pc);
}

void IX86_OPS::PREFIX86(_jmp_far)()    /* Opcode 0xea */
{
	unsigned tmp,tmp1;

	tmp = FETCH;
	tmp += FETCH << 8;

	tmp1 = FETCH;
	tmp1 += FETCH << 8;

#ifdef I80286
	i80286_code_descriptor(cpustate, tmp1,tmp, 1);
#else
	cpustate->sregs[CS] = (WORD)tmp1;
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->base[CS] + tmp) & AMASK;
#endif
	ICOUNT -= timing.jmp_far;
	CHANGE_PC(cpustate->pc);
}

void IX86_OPS::PREFIX86(_jmp_d8)()    /* Opcode 0xeb */
{
	int tmp = (int)((INT8)FETCH);
	cpustate->pc += tmp;
/* ASG - can probably assume this is safe
    CHANGE_PC(cpustate->pc);*/
	ICOUNT -= timing.jmp_short;
}
void IX86_OPS::PREFIX86(_inaldx)()    /* Opcode 0xec */
{
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.in_dx8;
	cpustate->regs.b[AL] = read_port_byte(cpustate->regs.w[DX]);
}

void IX86_OPS::PREFIX86(_inaxdx)()    /* Opcode 0xed */
{
	unsigned port = cpustate->regs.w[DX];
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.in_dx16;
	cpustate->regs.w[AX] = read_port_word(port);
}

void IX86_OPS::PREFIX86(_outdxal)()    /* Opcode 0xee */
{
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.out_dx8;
	write_port_byte(cpustate->regs.w[DX], cpustate->regs.b[AL]);
}

void IX86_OPS::PREFIX86(_outdxax)()    /* Opcode 0xef */
{
	unsigned port = cpustate->regs.w[DX];
#ifdef I80286
	if (PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT, 0);
#endif
	ICOUNT -= timing.out_dx16;
	write_port_word(port, cpustate->regs.w[AX]);
}


/* I think thats not a V20 instruction...*/
void IX86_OPS::PREFIX86(_lock)()    /* Opcode 0xf0 */
{
#ifdef I80286
	if(PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
#endif
	ICOUNT -= timing.nop;
	PREFIX(_instruction)[FETCHOP]();  /* un-interruptible */
}

void IX86_OPS::PREFIX(_pop_ss)()    /* Opcode 0x17 */
{
#ifdef I80286
	i80286_pop_seg(cpustate, SS);
#else
	POP(cpustate->sregs[SS]);
	cpustate->base[SS] = SegBase(SS);
#endif
	ICOUNT -= timing.pop_seg;
	PREFIX(_instruction)[FETCHOP](cpustate); /* no interrupt before next instruction */
}

void IX86_OPS::PREFIX(_mov_sregw)()    /* Opcode 0x8e */
{
	unsigned ModRM = FETCH;
	WORD src = GetRMWord(ModRM);

	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_sr : timing.mov_sm;
#ifdef I80286
	switch (ModRM & 0x38)
	{
	case 0x00:  /* mov es,ew */
		i80286_data_descriptor(cpustate,ES,src);
		break;
	case 0x18:  /* mov ds,ew */
		i80286_data_descriptor(cpustate,DS,src);
		break;
	case 0x10:  /* mov ss,ew */
		i80286_data_descriptor(cpustate,SS,src);
		cpustate->seg_prefix = FALSE;
		PREFIX(_instruction)[FETCHOP](cpustate);
		break;
	case 0x08:  /* mov cs,ew */
		PREFIX(_invalid)(cpustate);
		break;  /* doesn't do a jump far */
	}
#else
	switch (ModRM & 0x38)
	{
	case 0x00:  /* mov es,ew */
		cpustate->sregs[ES] = src;
		cpustate->base[ES] = SegBase(ES);
		break;
	case 0x18:  /* mov ds,ew */
		cpustate->sregs[DS] = src;
		cpustate->base[DS] = SegBase(DS);
		break;
	case 0x10:  /* mov ss,ew */
		cpustate->sregs[SS] = src;
		cpustate->base[SS] = SegBase(SS); /* no interrupt allowed before next instr */
		cpustate->seg_prefix = FALSE;
		PREFIX(_instruction)[FETCHOP](cpustate);
		break;
	case 0x08:  /* mov cs,ew */
#ifndef I80186
		int ip = cpustate->pc - cpustate->base[CS];
		cpustate->sregs[CS] = src;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (ip + cpustate->base[CS]) & AMASK;
		CHANGE_PC(cpustate->pc);
#endif
		break;
	}
#endif
}
void IX86_OPS::PREFIX(_sti)()    /* Opcode 0xfb */
{
#ifdef I80286
	if(PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
#endif
	ICOUNT -= timing.flag_ops;
	SetIF(1);
	PREFIX(_instruction)[FETCHOP](cpustate); /* no interrupt before next instruction */

	/* if an interrupt is pending, signal an interrupt */
	if (cpustate->irq_state) {
#ifdef I80286
		i80286_interrupt_descriptor(cpustate, cpustate->pic->get_intr_ack(), 2, -1);
#else
		PREFIX86(_interrupt)(cpustate, (UINT32)-1);
#endif
		cpustate->irq_state = 0;
	}
}

//#ifndef I80186
void IX86_OPS::PREFIX86(_hlt)()    /* Opcode 0xf4 */
{
#ifdef I80286
	if(PM && (CPL!=0)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
#endif
	cpustate->halted=1;
	ICOUNT = 0;
}

void IX86_OPS::PREFIX86(_cli)()    /* Opcode 0xfa */
{
#ifdef I80286
	if(PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
#endif
	ICOUNT -= timing.flag_ops;
	SetIF(0);
}

void IX86_OPS::PREFIX86(_ffpre)()    /* Opcode 0xff */
{
	unsigned ModRM = FETCHOP;
	unsigned tmp;
	unsigned tmp1, tmp2;
	WORD ip;

	switch(ModRM & 0x38)
	{
	case 0x00:  /* INC ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.incdec_r16 : timing.incdec_m16;
		tmp = GetRMWord(ModRM);
		tmp1 = tmp+1;

		SetOFW_Add(tmp1,tmp,1);
		SetAF(tmp1,tmp,1);
		SetSZPF_Word(tmp1);

		PutbackRMWord(ModRM,(WORD)tmp1);
		break;

	case 0x08:  /* DEC ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.incdec_r16 : timing.incdec_m16;
		tmp = GetRMWord(ModRM);
		tmp1 = tmp-1;

		SetOFW_Sub(tmp1,1,tmp);
		SetAF(tmp1,tmp,1);
		SetSZPF_Word(tmp1);

		PutbackRMWord(ModRM,(WORD)tmp1);
		break;

	case 0x10:  /* CALL ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.call_r16 : timing.call_m16;
		tmp = GetRMWord(ModRM);
#ifdef I86_PSEUDO_BIOS
		if(cpustate->bios != NULL && cpustate->bios->bios_call_i86((cpustate->base[CS] + (WORD)tmp) & AMASK, cpustate->regs.w, cpustate->sregs, &cpustate->ZeroVal, &cpustate->CarryVal)) {
			ICOUNT -= timing.call_far;
			return;
		}
#endif
		ip = cpustate->pc - cpustate->base[CS];
		PUSH(ip);
		cpustate->pc = (cpustate->base[CS] + (WORD)tmp) & AMASK;
		CHANGE_PC(cpustate->pc);
		break;

	case 0x18:  /* CALL FAR ea */
		ICOUNT -= timing.call_m32;
		tmp = cpustate->sregs[CS];  /* HJB 12/13/98 need to skip displacements of cpustate->ea */
		tmp1 = GetRMWord(ModRM);
		tmp2 = GetnextRMWord;
#ifdef I86_PSEUDO_BIOS
		if(cpustate->bios != NULL && cpustate->bios->bios_call_i86(((tmp2 << 4) + tmp1) & AMASK, cpustate->regs.w, cpustate->sregs, &cpustate->ZeroVal, &cpustate->CarryVal)) {
			return;
		}
#endif
		ip = cpustate->pc - cpustate->base[CS];
#ifdef I80286
		i80286_code_descriptor(cpustate, tmp2, tmp1, 2);
#else
		cpustate->sregs[CS] = tmp2;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (cpustate->base[CS] + tmp1) & AMASK;
#endif
		PUSH(tmp);
		PUSH(ip);
		CHANGE_PC(cpustate->pc);
		break;

	case 0x20:  /* JMP ea */
		ICOUNT -= (ModRM >= 0xc0) ? timing.jmp_r16 : timing.jmp_m16;
		ip = GetRMWord(ModRM);
		cpustate->pc = (cpustate->base[CS] + ip) & AMASK;
		CHANGE_PC(cpustate->pc);
		break;

	case 0x28:  /* JMP FAR ea */
		ICOUNT -= timing.jmp_m32;

#ifdef I80286
		tmp = GetRMWord(ModRM);
		i80286_code_descriptor(cpustate, GetnextRMWord, tmp, 1);
#else
		cpustate->pc = GetRMWord(ModRM);
		cpustate->sregs[CS] = GetnextRMWord;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
#endif
		CHANGE_PC(cpustate->pc);
		break;

	case 0x30:  /* PUSH ea */
		ICOUNT -= (ModRM >= 0xc0) ? timing.push_r16 : timing.push_m16;
		tmp = GetRMWord(ModRM);
		PUSH(tmp);
		break;
	default:
		tmp = GetRMWord(ModRM);  // 286 doesn't matter but 8086?
		return PREFIX(_invalid)(cpustate);
	}
}


void IX86_OPS::PREFIX86(_invalid)()
{
	logerror("illegal instruction %.2x at %.5x\n",PEEKBYTE(cpustate->pc-1), cpustate->pc);
#ifdef I80286
	throw TRAP(ILLEGAL_INSTRUCTION,-1);
#else
	/* i8086/i8088 ignore an invalid opcode. */
	/* i80186/i80188 probably also ignore an invalid opcode. */
	ICOUNT -= 10;
#endif
}

#ifndef I80286
void IX86_OPS::PREFIX86(_invalid_2b)()
{
	unsigned ModRM = FETCH;
	GetRMByte(ModRM);
	logerror("illegal 2 byte instruction %.2x at %.5x\n",PEEKBYTE(cpustate->pc-2), cpustate->pc-2);
	ICOUNT -= 10;
}
#endif

void IX86_BASE::PREFIX(_inc_bx)()    /* Opcode 0x43 */
{
	IncWordReg(BX);
}

void IX86_BASE::PREFIX86(_int3)()    /* Opcode 0xcc */
{
	ICOUNT -= timing.int3;
	PREFIX(_interrupt)(3);
}

void IX86_BASE::PREFIX86(_int)()    /* Opcode 0xcd */
{
	unsigned int_num = FETCH;
	ICOUNT -= timing.int_imm;
#ifdef I86_PSEUDO_BIOS
	if(cpustate->bios != NULL && cpustate->bios->bios_int_i86(int_num, cpustate->regs.w, cpustate->sregs, &cpustate->ZeroVal, &cpustate->CarryVal)) {
		return;
	}
#endif
	PREFIX(_interrupt)(int_num);
}

void IX86_OPS::PREFIX86(_into)(i8086_state *cpustate)    /* Opcode 0xce */
{
	if (OF) {
		ICOUNT -= timing.into_t;
		PREFIX(_interrupt)(cpustate, 4);
	} else ICOUNT -= timing.into_nt;
}



#ifndef I80286
void IX86_BASE::PREFIX86(_iret)()    /* Opcode 0xcf */
{
	ICOUNT -= timing.iret;
	POP(cpustate->pc);
	POP(cpustate->sregs[CS]);
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
		PREFIX(_popf)(cpustate);
	CHANGE_PC(cpustate->pc);

	/* if the IF is set, and an interrupt is pending, signal an interrupt */
	if (cpustate->IF && cpustate->irq_state) {
		PREFIX(_interrupt)(cpustate, (UINT32)-1);
		cpustate->irq_state = 0;
	}
}
#endif



/* I think thats not a V20 instruction...*/
void IX86_BASE::PREFIX86(_lock)()    /* Opcode 0xf0 */
{
#ifdef I80286
	if(PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
#endif
	ICOUNT -= timing.nop;
	PREFIX(_instruction)[FETCHOP](cpustate);  /* un-interruptible */
}
#endif
void IX86_BASE::PREFIX(_pop_ss)()    /* Opcode 0x17 */
{
#ifdef I80286
	i80286_pop_seg(cpustate, SS);
#else
	POP(cpustate->sregs[SS]);
	cpustate->base[SS] = SegBase(SS);
#endif
	ICOUNT -= timing.pop_seg;
	PREFIX(_instruction)[FETCHOP](cpustate); /* no interrupt before next instruction */
}

void IX86_BASE::PREFIX(_es)()    /* Opcode 0x26 */
{
	cpustate->seg_prefix = TRUE;
	cpustate->prefix_seg = ES;
	ICOUNT -= timing.override;
	PREFIX(_instruction)[FETCHOP](cpustate);
}

void IX86_BASE::PREFIX(_cs)()    /* Opcode 0x2e */
{
	cpustate->seg_prefix = TRUE;
	cpustate->prefix_seg = CS;
	ICOUNT -= timing.override;
	PREFIX(_instruction)[FETCHOP](cpustate);
}

void IX86_BASE::PREFIX(_ss)()    /* Opcode 0x36 */
{
	cpustate->seg_prefix = TRUE;
	cpustate->prefix_seg = SS;
	ICOUNT -= timing.override;
	PREFIX(_instruction)[FETCHOP](cpustate);
}

void IX86_BASE::PREFIX(_ds)()    /* Opcode 0x3e */
{
	cpustate->seg_prefix = TRUE;
	cpustate->prefix_seg = DS;
	ICOUNT -= timing.override;
	PREFIX(_instruction)[FETCHOP](cpustate);
}

void IX86_BASE::PREFIX(_mov_sregw)()    /* Opcode 0x8e */
{
	unsigned ModRM = FETCH;
	WORD src = GetRMWord(ModRM);

	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_sr : timing.mov_sm;
#ifdef I80286
	switch (ModRM & 0x38)
	{
	case 0x00:  /* mov es,ew */
		i80286_data_descriptor(cpustate,ES,src);
		break;
	case 0x18:  /* mov ds,ew */
		i80286_data_descriptor(cpustate,DS,src);
		break;
	case 0x10:  /* mov ss,ew */
		i80286_data_descriptor(cpustate,SS,src);
		cpustate->seg_prefix = FALSE;
		PREFIX(_instruction)[FETCHOP](cpustate);
		break;
	case 0x08:  /* mov cs,ew */
		PREFIX(_invalid)(cpustate);
		break;  /* doesn't do a jump far */
	}
#else
	switch (ModRM & 0x38)
	{
	case 0x00:  /* mov es,ew */
		cpustate->sregs[ES] = src;
		cpustate->base[ES] = SegBase(ES);
		break;
	case 0x18:  /* mov ds,ew */
		cpustate->sregs[DS] = src;
		cpustate->base[DS] = SegBase(DS);
		break;
	case 0x10:  /* mov ss,ew */
		cpustate->sregs[SS] = src;
		cpustate->base[SS] = SegBase(SS); /* no interrupt allowed before next instr */
		cpustate->seg_prefix = FALSE;
		PREFIX(_instruction)[FETCHOP](cpustate);
		break;
	case 0x08:  /* mov cs,ew */
#ifndef I80186
		int ip = cpustate->pc - cpustate->base[CS];
		cpustate->sregs[CS] = src;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (ip + cpustate->base[CS]) & AMASK;
		CHANGE_PC(cpustate->pc);
#endif
		break;
	}
#endif
}

void IX86_BASE::PREFIX(_repne)()    /* Opcode 0xf2 */
{
	PREFIX(rep)(0);
}

void IX86_BASE::PREFIX(_repe)()    /* Opcode 0xf3 */
{
	PREFIX(rep)(1);
}

void IX86_BASE::PREFIX(_sti)()    /* Opcode 0xfb */
{
#ifdef I80286
	if(PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
#endif
	ICOUNT -= timing.flag_ops;
	SetIF(1);
	PREFIX(_instruction)[FETCHOP](cpustate); /* no interrupt before next instruction */

	/* if an interrupt is pending, signal an interrupt */
	if (cpustate->irq_state) {
#ifdef I80286
		i80286_interrupt_descriptor(cpustate, cpustate->pic->get_intr_ack(), 2, -1);
#else
		PREFIX86(_interrupt)(cpustate, (UINT32)-1);
#endif
		cpustate->irq_state = 0;
	}
}
void IX86_BASE::PREFIX86(_hlt)()    /* Opcode 0xf4 */
{
#ifdef I80286
	if(PM && (CPL!=0)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
#endif
	cpustate->halted=1;
	ICOUNT = 0;
}

void IX86_BASE::PREFIX86(_f6pre)()
{
	/* Opcode 0xf6 */
	unsigned ModRM = FETCH;
	unsigned tmp = (unsigned)GetRMByte(ModRM);
	unsigned tmp2;


	switch (ModRM & 0x38)
	{
	case 0x00:  /* TEST Eb, data8 */
	case 0x08:  /* ??? */
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8_ro;
		tmp &= FETCH;

		cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0;
		SetSZPF_Byte(tmp);
		break;

	case 0x10:  /* NOT Eb */
		ICOUNT -= (ModRM >= 0xc0) ? timing.negnot_r8 : timing.negnot_m8;
		PutbackRMByte(ModRM,~tmp);
		break;

		case 0x18:  /* NEG Eb */
		ICOUNT -= (ModRM >= 0xc0) ? timing.negnot_r8 : timing.negnot_m8;
		tmp2=0;
		SUBB(tmp2,tmp);
		PutbackRMByte(ModRM,tmp2);
		break;
	case 0x20:  /* MUL AL, Eb */
		ICOUNT -= (ModRM >= 0xc0) ? timing.mul_r8 : timing.mul_m8;
		{
			UINT16 result;
			tmp2 = cpustate->regs.b[AL];

			SetSF((INT8)tmp2);
			SetPF(tmp2);

			result = (UINT16)tmp2*tmp;
			cpustate->regs.w[AX]=(WORD)result;

			SetZF(cpustate->regs.w[AX]);
			cpustate->CarryVal = cpustate->OverVal = (cpustate->regs.b[AH] != 0);
		}
		break;
		case 0x28:  /* IMUL AL, Eb */
		ICOUNT -= (ModRM >= 0xc0) ? timing.imul_r8 : timing.imul_m8;
		{
			INT16 result;

			tmp2 = (unsigned)cpustate->regs.b[AL];

			SetSF((INT8)tmp2);
			SetPF(tmp2);

			result = (INT16)((INT8)tmp2)*(INT16)((INT8)tmp);
			cpustate->regs.w[AX]=(WORD)result;

			SetZF(cpustate->regs.w[AX]);

			cpustate->CarryVal = cpustate->OverVal = (result >> 7 != 0) && (result >> 7 != -1);
		}
		break;
	case 0x30:  /* DIV AL, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.div_r8 : timing.div_m8;
		{
			UINT16 result;

			result = cpustate->regs.w[AX];

			if (tmp)
			{
				if ((result / tmp) > 0xff)
				{
					PREFIX(_interrupt)(cpustate, 0);
					break;
				}
				else
				{
					cpustate->regs.b[AH] = result % tmp;
					cpustate->regs.b[AL] = result / tmp;
				}
			}
			else
			{
				PREFIX(_interrupt)(cpustate, 0);
				break;
			}
		}
		break;
	case 0x38:  /* IDIV AL, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.idiv_r8 : timing.idiv_m8;
		{

			INT16 result;

			result = cpustate->regs.w[AX];

			if (tmp)
			{
				tmp2 = result % (INT16)((INT8)tmp);

				if ((result /= (INT16)((INT8)tmp)) > 0xff)
				{
					PREFIX(_interrupt)(cpustate, 0);
					break;
				}
				else
				{
					cpustate->regs.b[AL] = result;
					cpustate->regs.b[AH] = tmp2;
				}
			}
			else
			{
				PREFIX(_interrupt)(cpustate, 0);
				break;
			}
		}
		break;
	}
}


void IX86_BASE::PREFIX86(_f7pre)()
{
	/* Opcode 0xf7 */
	unsigned ModRM = FETCH;
		unsigned tmp = GetRMWord(ModRM);
	unsigned tmp2;


	switch (ModRM & 0x38)
	{
	case 0x00:  /* TEST Ew, data16 */
	case 0x08:  /* ??? */
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16_ro;
		tmp2 = FETCH;
		tmp2 += FETCH << 8;

		tmp &= tmp2;

		cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0;
		SetSZPF_Word(tmp);
		break;

	case 0x10:  /* NOT Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.negnot_r16 : timing.negnot_m16;
		tmp = ~tmp;
		PutbackRMWord(ModRM,tmp);
		break;

	case 0x18:  /* NEG Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.negnot_r16 : timing.negnot_m16;
		tmp2 = 0;
		SUBW(tmp2,tmp);
		PutbackRMWord(ModRM,tmp2);
		break;
	case 0x20:  /* MUL AX, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.mul_r16 : timing.mul_m16;
		{
			UINT32 result;
			tmp2 = cpustate->regs.w[AX];

			SetSF((INT16)tmp2);
			SetPF(tmp2);

			result = (UINT32)tmp2*tmp;
			cpustate->regs.w[AX]=(WORD)result;
			result >>= 16;
			cpustate->regs.w[DX]=result;

			SetZF(cpustate->regs.w[AX] | cpustate->regs.w[DX]);
			cpustate->CarryVal = cpustate->OverVal = (cpustate->regs.w[DX] != 0);
		}
		break;

	case 0x28:  /* IMUL AX, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.imul_r16 : timing.imul_m16;
		{
			INT32 result;

			tmp2 = cpustate->regs.w[AX];

			SetSF((INT16)tmp2);
			SetPF(tmp2);

			result = (INT32)((INT16)tmp2)*(INT32)((INT16)tmp);
			cpustate->CarryVal = cpustate->OverVal = (result >> 15 != 0) && (result >> 15 != -1);

			cpustate->regs.w[AX]=(WORD)result;
			result = (WORD)(result >> 16);
			cpustate->regs.w[DX]=result;

			SetZF(cpustate->regs.w[AX] | cpustate->regs.w[DX]);
		}
		break;
		case 0x30:  /* DIV AX, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.div_r16 : timing.div_m16;
		{
			UINT32 result;

			result = (cpustate->regs.w[DX] << 16) + cpustate->regs.w[AX];

			if (tmp)
			{
				tmp2 = result % tmp;
				if ((result / tmp) > 0xffff)
				{
					PREFIX(_interrupt)(cpustate, 0);
					break;
				}
				else
				{
					cpustate->regs.w[DX]=tmp2;
					result /= tmp;
					cpustate->regs.w[AX]=result;
				}
			}
			else
			{
				PREFIX(_interrupt)(cpustate, 0);
				break;
			}
		}
		break;
	case 0x38:  /* IDIV AX, Ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.idiv_r16 : timing.idiv_m16;
		{
			INT32 result;

			result = (cpustate->regs.w[DX] << 16) + cpustate->regs.w[AX];

			if (tmp)
			{
				tmp2 = result % (INT32)((INT16)tmp);
				if ((result /= (INT32)((INT16)tmp)) > 0xffff)
				{
					PREFIX(_interrupt)(cpustate, 0);
					break;
				}
				else
				{
					cpustate->regs.w[AX]=result;
					cpustate->regs.w[DX]=tmp2;
				}
			}
			else
			{
				PREFIX(_interrupt)(cpustate, 0);
				break;
			}
		}
		break;
	}
}

void IX86_BASE::PREFIX86(_cli)()    /* Opcode 0xfa */
{
#ifdef I80286
	if(PM && (CPL>IOPL)) throw TRAP(GENERAL_PROTECTION_FAULT,0);
#endif
	ICOUNT -= timing.flag_ops;
	SetIF(0);
}

void IX86_BASE::PREFIX86(_ffpre)()    /* Opcode 0xff */
{
	unsigned ModRM = FETCHOP;
	unsigned tmp;
	unsigned tmp1, tmp2;
	WORD ip;

	switch(ModRM & 0x38)
	{
	case 0x00:  /* INC ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.incdec_r16 : timing.incdec_m16;
		tmp = GetRMWord(ModRM);
		tmp1 = tmp+1;

		SetOFW_Add(tmp1,tmp,1);
		SetAF(tmp1,tmp,1);
		SetSZPF_Word(tmp1);

		PutbackRMWord(ModRM,(WORD)tmp1);
		break;

	case 0x08:  /* DEC ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.incdec_r16 : timing.incdec_m16;
		tmp = GetRMWord(ModRM);
		tmp1 = tmp-1;

		SetOFW_Sub(tmp1,1,tmp);
		SetAF(tmp1,tmp,1);
		SetSZPF_Word(tmp1);

		PutbackRMWord(ModRM,(WORD)tmp1);
		break;

	case 0x10:  /* CALL ew */
		ICOUNT -= (ModRM >= 0xc0) ? timing.call_r16 : timing.call_m16;
		tmp = GetRMWord(ModRM);
#ifdef I86_PSEUDO_BIOS
		if(cpustate->bios != NULL && cpustate->bios->bios_call_i86((cpustate->base[CS] + (WORD)tmp) & AMASK, cpustate->regs.w, cpustate->sregs, &cpustate->ZeroVal, &cpustate->CarryVal)) {
			ICOUNT -= timing.call_far;
			return;
		}
#endif
		ip = cpustate->pc - cpustate->base[CS];
		PUSH(ip);
		cpustate->pc = (cpustate->base[CS] + (WORD)tmp) & AMASK;
		CHANGE_PC(cpustate->pc);
		break;

	case 0x18:  /* CALL FAR ea */
		ICOUNT -= timing.call_m32;
		tmp = cpustate->sregs[CS];  /* HJB 12/13/98 need to skip displacements of cpustate->ea */
		tmp1 = GetRMWord(ModRM);
		tmp2 = GetnextRMWord;
#ifdef I86_PSEUDO_BIOS
		if(cpustate->bios != NULL && cpustate->bios->bios_call_i86(((tmp2 << 4) + tmp1) & AMASK, cpustate->regs.w, cpustate->sregs, &cpustate->ZeroVal, &cpustate->CarryVal)) {
			return;
		}
#endif
		ip = cpustate->pc - cpustate->base[CS];
#ifdef I80286
		i80286_code_descriptor(cpustate, tmp2, tmp1, 2);
#else
		cpustate->sregs[CS] = tmp2;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (cpustate->base[CS] + tmp1) & AMASK;
#endif
		PUSH(tmp);
		PUSH(ip);
		CHANGE_PC(cpustate->pc);
		break;

	case 0x20:  /* JMP ea */
		ICOUNT -= (ModRM >= 0xc0) ? timing.jmp_r16 : timing.jmp_m16;
		ip = GetRMWord(ModRM);
		cpustate->pc = (cpustate->base[CS] + ip) & AMASK;
		CHANGE_PC(cpustate->pc);
		break;

	case 0x28:  /* JMP FAR ea */
		ICOUNT -= timing.jmp_m32;

#ifdef I80286
		tmp = GetRMWord(ModRM);
		i80286_code_descriptor(cpustate, GetnextRMWord, tmp, 1);
#else
		cpustate->pc = GetRMWord(ModRM);
		cpustate->sregs[CS] = GetnextRMWord;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
#endif
		CHANGE_PC(cpustate->pc);
		break;

	case 0x30:  /* PUSH ea */
		ICOUNT -= (ModRM >= 0xc0) ? timing.push_r16 : timing.push_m16;
		tmp = GetRMWord(ModRM);
		PUSH(tmp);
		break;
	default:
		tmp = GetRMWord(ModRM);  // 286 doesn't matter but 8086?
		return PREFIX(_invalid)(cpustate);
	}
}

void IX86_BASE::PREFIX86(_invalid)()
{
	logerror("illegal instruction %.2x at %.5x\n",PEEKBYTE(cpustate->pc-1), cpustate->pc);
#ifdef I80286
	throw TRAP(ILLEGAL_INSTRUCTION,-1);
#else
	/* i8086/i8088 ignore an invalid opcode. */
	/* i80186/i80188 probably also ignore an invalid opcode. */
	ICOUNT -= 10;
#endif
}

#ifndef I80286
void IX86_BASE::PREFIX86(_invalid_2b)()
{
	unsigned ModRM = FETCH;
	GetRMByte(ModRM);
	logerror("illegal 2 byte instruction %.2x at %.5x\n",PEEKBYTE(cpustate->pc-2), cpustate->pc-2);
	ICOUNT -= 10;
}
#endif
//#endif
