/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/

/*
 * file will be included in all cpu variants
 * put non i86 instructions in own files (i286, i386, nec)
 * function renaming will be added when necessary
 * timing value should move to separate array
 */

/*
    PHS - 2010-12-29

    Moved several instruction stubs so that they are compiled separately for
    the 8086 and 80186. The instructions affected are :

    _pop_ss, _es, _cs, _ss, _ds, _mov_sregw and _sti

    This is because they call the next instruction directly as it cannot be
    interrupted. If they are not compiled separately when executing on an
    80186, the wrong set of instructions are used (the 8086 set). This has
    the serious effect of ignoring the next instruction, as invalid, *IF*
    it is an 80186 specific instruction.

*/

#undef ICOUNT

#define ICOUNT cpustate->icount


//#if !defined(I80186)
void IX86_OPS_BASE::PREFIX86(_interrupt)(unsigned int_num)
{
	unsigned dest_seg, dest_off;
	WORD ip = cpustate->pc - cpustate->base[CS];

	if (int_num == -1)
		int_num = cpustate->pic->get_intr_ack();

		dest_off = ReadWord(int_num*4);
		dest_seg = ReadWord(int_num*4+2);

		PREFIX(_pushf());
		cpustate->TF = cpustate->IF = 0;
		PUSH(cpustate->sregs[CS]);
		PUSH(ip);
		cpustate->sregs[CS] = (WORD)dest_seg;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (cpustate->base[CS] + dest_off) & AMASK;
		CHANGE_PC(cpustate->pc);
	cpustate->extra_cycles += timing.exception;
}

void IX86_OPS_BASE::PREFIX86(_trap)()
{
	PREFIX(_instruction)[FETCHOP]();
	PREFIX(_interrupt)(1);
}
//#endif

//#ifndef I80186
void IX86_OPS_BASE::PREFIX86(_rotate_shift_Byte)(, unsigned ModRM, unsigned count, unsigned src)
{
}

void IX86_OPS_BASE::PREFIX86(_rotate_shift_Word)(, unsigned ModRM, unsigned count, unsigned src)
{
}
//#endif

void IX86_OPS_BASE::PREFIX(rep)(int flagval)
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
		PREFIX(rep)(flagval);
		break;
	case 0x2e:  /* CS: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = CS;
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.override;
		PREFIX(rep)(flagval);
		break;
	case 0x36:  /* SS: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = SS;
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.override;
		PREFIX(rep)(flagval);
		break;
	case 0x3e:  /* DS: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = DS;
		if (!cpustate->rep_in_progress)
			ICOUNT -= timing.override;
		PREFIX(rep)(flagval);
		break;
		// !8086 was deleted.
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
		PREFIX(_instruction)[next]();
	}
}

//#ifndef I80186
void IX86_OPS_BASE::PREFIX86(_add_br8)()    /* Opcode 0x00 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	ADDB(dst,src);
	PutbackRMByte(ModRM,dst);
}

void IX86_OPS_BASE::PREFIX86(_add_wr16)()    /* Opcode 0x01 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	ADDW(dst,src);
	PutbackRMWord(ModRM,dst);
}

void IX86_OPS_BASE::PREFIX86(_add_r8b)()    /* Opcode 0x02 */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ADDB(dst,src);
	RegByte(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_add_r16w)()    /* Opcode 0x03 */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ADDW(dst,src);
	RegWord(ModRM)=dst;
}


void IX86_OPS_BASE::PREFIX86(_add_ald8)()    /* Opcode 0x04 */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	ADDB(dst,src);
	cpustate->regs.b[AL]=dst;
}


void IX86_OPS_BASE::PREFIX86(_add_axd16)()    /* Opcode 0x05 */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	ADDW(dst,src);
	cpustate->regs.w[AX]=dst;
}


void IX86_OPS_BASE::PREFIX86(_push_es)()    /* Opcode 0x06 */
{
	ICOUNT -= timing.push_seg;
	PUSH(cpustate->sregs[ES]);
}


void IX86_OPS_BASE::PREFIX86(_pop_es)()    /* Opcode 0x07 */
{
	POP(cpustate->sregs[ES]);
	cpustate->base[ES] = SegBase(ES);

	ICOUNT -= timing.pop_seg;
}

void IX86_OPS_BASE::PREFIX86(_or_br8)()    /* Opcode 0x08 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	ORB(dst,src);
	PutbackRMByte(ModRM,dst);
}

void IX86_OPS_BASE::PREFIX86(_or_wr16)()    /* Opcode 0x09 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	ORW(dst,src);
	PutbackRMWord(ModRM,dst);
}

void IX86_OPS_BASE::PREFIX86(_or_r8b)()    /* Opcode 0x0a */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ORB(dst,src);
	RegByte(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_or_r16w)()    /* Opcode 0x0b */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ORW(dst,src);
	RegWord(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_or_ald8)()    /* Opcode 0x0c */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	ORB(dst,src);
	cpustate->regs.b[AL]=dst;
}

void IX86_OPS_BASE::PREFIX86(_or_axd16)()    /* Opcode 0x0d */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	ORW(dst,src);
	cpustate->regs.w[AX]=dst;
}

void IX86_OPS_BASE::PREFIX86(_push_cs)()    /* Opcode 0x0e */
{
	ICOUNT -= timing.push_seg;
	PUSH(cpustate->sregs[CS]);
}

//#ifndef I80286
void IX86_OPS_BASE::PREFIX86(_pop_cs)()    /* Opcode 0x0f */
{
	int ip = cpustate->pc - cpustate->base[CS];
	ICOUNT -= timing.push_seg;
	POP(cpustate->sregs[CS]);
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (ip + cpustate->base[CS]) & AMASK;
	CHANGE_PC(cpustate->pc);
}
//#endif

void IX86_OPS_BASE::PREFIX86(_adc_br8)()    /* Opcode 0x10 */
{
	int tmpcf;
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	src+=CF;
	ADCB(dst,src,tmpcf);
	PutbackRMByte(ModRM,dst);
	cpustate->CarryVal = tmpcf;
}

void IX86_OPS_BASE::PREFIX86(_adc_wr16)()    /* Opcode 0x11 */
{
	int tmpcf;
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	src+=CF;
	ADCW(dst,src,tmpcf);
	PutbackRMWord(ModRM,dst);
	cpustate->CarryVal = tmpcf;
}

void IX86_OPS_BASE::PREFIX86(_adc_r8b)()    /* Opcode 0x12 */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	src+=CF;
	ADDB(dst,src);
	RegByte(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_adc_r16w)()    /* Opcode 0x13 */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	src+=CF;
	ADDW(dst,src);
	RegWord(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_adc_ald8)()    /* Opcode 0x14 */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	src+=CF;
	ADDB(dst,src);
	cpustate->regs.b[AL] = dst;
}

void IX86_OPS_BASE::PREFIX86(_adc_axd16)()    /* Opcode 0x15 */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	src+=CF;
	ADDW(dst,src);
	cpustate->regs.w[AX]=dst;
}

void IX86_OPS_BASE::PREFIX86(_push_ss)()    /* Opcode 0x16 */
{
	PUSH(cpustate->sregs[SS]);
	ICOUNT -= timing.push_seg;
}

void IX86_OPS_BASE::PREFIX86(_sbb_br8)()    /* Opcode 0x18 */
{
	int tmpcf;
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	src+=CF;
	SBBB(dst,src,tmpcf);
	PutbackRMByte(ModRM,dst);
	cpustate->CarryVal = tmpcf;
}

void IX86_OPS_BASE::PREFIX86(_sbb_wr16)()    /* Opcode 0x19 */
{
	int tmpcf;
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	src+=CF;
	SBBW(dst,src,tmpcf);
	PutbackRMWord(ModRM,dst);
	cpustate->CarryVal = tmpcf;
}

void IX86_OPS_BASE::PREFIX86(_sbb_r8b)()    /* Opcode 0x1a */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	src+=CF;
	SUBB(dst,src);
	RegByte(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_sbb_r16w)()    /* Opcode 0x1b */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	src+=CF;
	SUBW(dst,src);
	RegWord(ModRM)= dst;
}

void IX86_OPS_BASE::PREFIX86(_sbb_ald8)()    /* Opcode 0x1c */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	src+=CF;
	SUBB(dst,src);
	cpustate->regs.b[AL] = dst;
}

void IX86_OPS_BASE::PREFIX86(_sbb_axd16)()    /* Opcode 0x1d */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	src+=CF;
	SUBW(dst,src);
	cpustate->regs.w[AX]=dst;
}

void IX86_OPS_BASE::PREFIX86(_push_ds)()    /* Opcode 0x1e */
{
	PUSH(cpustate->sregs[DS]);
	ICOUNT -= timing.push_seg;
}

void IX86_OPS_BASE::PREFIX86(_pop_ds)()    /* Opcode 0x1f */
{
	POP(cpustate->sregs[DS]);
	cpustate->base[DS] = SegBase(DS);

	ICOUNT -= timing.push_seg;
}

void IX86_OPS_BASE::PREFIX86(_and_br8)()    /* Opcode 0x20 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	ANDB(dst,src);
	PutbackRMByte(ModRM,dst);
}

void IX86_OPS_BASE::PREFIX86(_and_wr16)()    /* Opcode 0x21 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	ANDW(dst,src);
	PutbackRMWord(ModRM,dst);
}

void IX86_OPS_BASE::PREFIX86(_and_r8b)()    /* Opcode 0x22 */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ANDB(dst,src);
	RegByte(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_and_r16w)()    /* Opcode 0x23 */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ANDW(dst,src);
	RegWord(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_and_ald8)()    /* Opcode 0x24 */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	ANDB(dst,src);
	cpustate->regs.b[AL] = dst;
}

void IX86_OPS_BASE::PREFIX86(_and_axd16)()    /* Opcode 0x25 */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	ANDW(dst,src);
	cpustate->regs.w[AX]=dst;
}

void IX86_OPS_BASE::PREFIX86(_daa)()    /* Opcode 0x27 */
{
	if (AF || ((cpustate->regs.b[AL] & 0xf) > 9))
	{
		int tmp;
		cpustate->regs.b[AL] = tmp = cpustate->regs.b[AL] + 6;
		cpustate->AuxVal = 1;
		cpustate->CarryVal |= tmp & 0x100;
	}

	if (CF || (cpustate->regs.b[AL] > 0x9f))
	{
		cpustate->regs.b[AL] += 0x60;
		cpustate->CarryVal = 1;
	}

	SetSZPF_Byte(cpustate->regs.b[AL]);
	ICOUNT -= timing.daa;
}

void IX86_OPS_BASE::PREFIX86(_sub_br8)()    /* Opcode 0x28 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	SUBB(dst,src);
	PutbackRMByte(ModRM,dst);
}

void IX86_OPS_BASE::PREFIX86(_sub_wr16)()    /* Opcode 0x29 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	SUBW(dst,src);
	PutbackRMWord(ModRM,dst);
}

void IX86_OPS_BASE::PREFIX86(_sub_r8b)()    /* Opcode 0x2a */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	SUBB(dst,src);
	RegByte(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_sub_r16w)()    /* Opcode 0x2b */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	SUBW(dst,src);
	RegWord(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_sub_ald8)()    /* Opcode 0x2c */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	SUBB(dst,src);
	cpustate->regs.b[AL] = dst;
}

void IX86_OPS_BASE::PREFIX86(_sub_axd16)()    /* Opcode 0x2d */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	SUBW(dst,src);
	cpustate->regs.w[AX]=dst;
}

void IX86_OPS_BASE::PREFIX86(_das)()    /* Opcode 0x2f */
{
	UINT8 tmpAL=cpustate->regs.b[AL];
	if (AF || ((cpustate->regs.b[AL] & 0xf) > 9))
	{
		int tmp;
		cpustate->regs.b[AL] = tmp = cpustate->regs.b[AL] - 6;
		cpustate->AuxVal = 1;
		cpustate->CarryVal |= tmp & 0x100;
	}

	if (CF || (tmpAL > 0x9f))
	{
		cpustate->regs.b[AL] -= 0x60;
		cpustate->CarryVal = 1;
	}

	SetSZPF_Byte(cpustate->regs.b[AL]);
	ICOUNT -= timing.das;
}

void IX86_OPS_BASE::PREFIX86(_xor_br8)()    /* Opcode 0x30 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	XORB(dst,src);
	PutbackRMByte(ModRM,dst);
}

void IX86_OPS_BASE::PREFIX86(_xor_wr16)()    /* Opcode 0x31 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	XORW(dst,src);
	PutbackRMWord(ModRM,dst);
}

void IX86_OPS_BASE::PREFIX86(_xor_r8b)()    /* Opcode 0x32 */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	XORB(dst,src);
	RegByte(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_xor_r16w)()    /* Opcode 0x33 */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	XORW(dst,src);
	RegWord(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_xor_ald8)()    /* Opcode 0x34 */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	XORB(dst,src);
	cpustate->regs.b[AL] = dst;
}

void IX86_OPS_BASE::PREFIX86(_xor_axd16)()    /* Opcode 0x35 */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	XORW(dst,src);
	cpustate->regs.w[AX]=dst;
}

void IX86_OPS_BASE::PREFIX86(_aaa)()    /* Opcode 0x37 */
{
	UINT8 ALcarry=1;
	if (cpustate->regs.b[AL]>0xf9) ALcarry=2;

	if (AF || ((cpustate->regs.b[AL] & 0xf) > 9))
	{
		cpustate->regs.b[AL] += 6;
		cpustate->regs.b[AH] += ALcarry;
		cpustate->AuxVal = 1;
		cpustate->CarryVal = 1;
	}
	else
	{
		cpustate->AuxVal = 0;
		cpustate->CarryVal = 0;
	}
	cpustate->regs.b[AL] &= 0x0F;
	ICOUNT -= timing.aaa;
}

void IX86_OPS_BASE::PREFIX86(_cmp_br8)()    /* Opcode 0x38 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	SUBB(dst,src);
}

void IX86_OPS_BASE::PREFIX86(_cmp_wr16)()    /* Opcode 0x39 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	SUBW(dst,src);
}

void IX86_OPS_BASE::PREFIX86(_cmp_r8b)()    /* Opcode 0x3a */
{
	DEF_r8b(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	SUBB(dst,src);
}

void IX86_OPS_BASE::PREFIX86(_cmp_r16w)()    /* Opcode 0x3b */
{
	DEF_r16w(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	SUBW(dst,src);
}

void IX86_OPS_BASE::PREFIX86(_cmp_ald8)()    /* Opcode 0x3c */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	SUBB(dst,src);
}

void IX86_OPS_BASE::PREFIX86(_cmp_axd16)()    /* Opcode 0x3d */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	SUBW(dst,src);
}

void IX86_OPS_BASE::PREFIX86(_aas)()    /* Opcode 0x3f */
{
//  UINT8 ALcarry=1;
//  if (cpustate->regs.b[AL]>0xf9) ALcarry=2;

	if (AF || ((cpustate->regs.b[AL] & 0xf) > 9))
	{
		cpustate->regs.b[AL] -= 6;
		cpustate->regs.b[AH] -= 1;
		cpustate->AuxVal = 1;
		cpustate->CarryVal = 1;
	}
	else
	{
		cpustate->AuxVal = 0;
		cpustate->CarryVal = 0;
	}
	cpustate->regs.b[AL] &= 0x0F;
	ICOUNT -= timing.aas;
}

#define IncWordReg(Reg)                     \
{                                           \
	unsigned tmp = (unsigned)cpustate->regs.w[Reg]; \
	unsigned tmp1 = tmp+1;                  \
	SetOFW_Add(tmp1,tmp,1);                 \
	SetAF(tmp1,tmp,1);                      \
	SetSZPF_Word(tmp1);                     \
	cpustate->regs.w[Reg]=tmp1;                     \
	ICOUNT -= timing.incdec_r16;            \
}

void IX86_OPS_BASE::PREFIX86(_inc_ax)()    /* Opcode 0x40 */
{
	IncWordReg(AX);
}

void IX86_OPS_BASE::PREFIX86(_inc_cx)()    /* Opcode 0x41 */
{
	IncWordReg(CX);
}

void IX86_OPS_BASE::PREFIX86(_inc_dx)()    /* Opcode 0x42 */
{
	IncWordReg(DX);
}

void IX86_OPS_BASE::PREFIX(_inc_bx)()    /* Opcode 0x43 */
{
	IncWordReg(BX);
}

void IX86_OPS_BASE::PREFIX86(_inc_sp)()    /* Opcode 0x44 */
{
	IncWordReg(SP);
}

void IX86_OPS_BASE::PREFIX86(_inc_bp)()    /* Opcode 0x45 */
{
	IncWordReg(BP);
}

void IX86_OPS_BASE::PREFIX86(_inc_si)()    /* Opcode 0x46 */
{
	IncWordReg(SI);
}

void IX86_OPS_BASE::PREFIX86(_inc_di)()    /* Opcode 0x47 */
{
	IncWordReg(DI);
}

#define DecWordReg(Reg)                     \
{                                           \
	unsigned tmp = (unsigned)cpustate->regs.w[Reg]; \
	unsigned tmp1 = tmp-1;                  \
	SetOFW_Sub(tmp1,1,tmp);                 \
	SetAF(tmp1,tmp,1);                      \
	SetSZPF_Word(tmp1);                     \
	cpustate->regs.w[Reg]=tmp1;                     \
	ICOUNT -= timing.incdec_r16;            \
}

void IX86_OPS_BASE::PREFIX86(_dec_ax)()    /* Opcode 0x48 */
{
	DecWordReg(AX);
}

void IX86_OPS_BASE::PREFIX86(_dec_cx)()    /* Opcode 0x49 */
{
	DecWordReg(CX);
}

void IX86_OPS_BASE::PREFIX86(_dec_dx)()    /* Opcode 0x4a */
{
	DecWordReg(DX);
}

void IX86_OPS_BASE::PREFIX86(_dec_bx)()    /* Opcode 0x4b */
{
	DecWordReg(BX);
}

void IX86_OPS_BASE::PREFIX86(_dec_sp)()    /* Opcode 0x4c */
{
	DecWordReg(SP);
}

void IX86_OPS_BASE::PREFIX86(_dec_bp)()    /* Opcode 0x4d */
{
	DecWordReg(BP);
}

void IX86_OPS_BASE::PREFIX86(_dec_si)()    /* Opcode 0x4e */
{
	DecWordReg(SI);
}

void IX86_OPS_BASE::PREFIX86(_dec_di)()    /* Opcode 0x4f */
{
	DecWordReg(DI);
}

void IX86_OPS_BASE::PREFIX86(_push_ax)()    /* Opcode 0x50 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[AX]);
}

void IX86_OPS_BASE::PREFIX86(_push_cx)()    /* Opcode 0x51 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[CX]);
}

void IX86_OPS_BASE::PREFIX86(_push_dx)()    /* Opcode 0x52 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[DX]);
}

void IX86_OPS_BASE::PREFIX86(_push_bx)()    /* Opcode 0x53 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[BX]);
}

void IX86_OPS_BASE::PREFIX86(_push_sp)()    /* Opcode 0x54 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[SP]);
}

void IX86_OPS_BASE::PREFIX86(_push_bp)()    /* Opcode 0x55 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[BP]);
}


void IX86_OPS_BASE::PREFIX86(_push_si)()    /* Opcode 0x56 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[SI]);
}

void IX86_OPS_BASE::PREFIX86(_push_di)()    /* Opcode 0x57 */
{
	ICOUNT -= timing.push_r16;
	PUSH(cpustate->regs.w[DI]);
}

void IX86_OPS_BASE::PREFIX86(_pop_ax)()    /* Opcode 0x58 */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[AX]);
}

void IX86_OPS_BASE::PREFIX86(_pop_cx)()    /* Opcode 0x59 */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[CX]);
}

void IX86_OPS_BASE::PREFIX86(_pop_dx)()    /* Opcode 0x5a */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[DX]);
}

void IX86_OPS_BASE::PREFIX86(_pop_bx)()    /* Opcode 0x5b */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[BX]);
}

void IX86_OPS_BASE::PREFIX86(_pop_sp)()    /* Opcode 0x5c */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[SP]);
}

void IX86_OPS_BASE::PREFIX86(_pop_bp)()    /* Opcode 0x5d */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[BP]);
}

void IX86_OPS_BASE::PREFIX86(_pop_si)()    /* Opcode 0x5e */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[SI]);
}

void IX86_OPS_BASE::PREFIX86(_pop_di)()    /* Opcode 0x5f */
{
	ICOUNT -= timing.pop_r16;
	POP(cpustate->regs.w[DI]);
}

void IX86_OPS_BASE::PREFIX86(_jo)()    /* Opcode 0x70 */
{
	int tmp = (int)((INT8)FETCH);
	if (OF)
	{
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jno)()    /* Opcode 0x71 */
{
	int tmp = (int)((INT8)FETCH);
	if (!OF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jb)()    /* Opcode 0x72 */
{
	int tmp = (int)((INT8)FETCH);
	if (CF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jnb)()    /* Opcode 0x73 */
{
	int tmp = (int)((INT8)FETCH);
	if (!CF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jz)()    /* Opcode 0x74 */
{
	int tmp = (int)((INT8)FETCH);
	if (ZF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jnz)()    /* Opcode 0x75 */
{
	int tmp = (int)((INT8)FETCH);
	if (!ZF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jbe)()    /* Opcode 0x76 */
{
	int tmp = (int)((INT8)FETCH);
	if (CF || ZF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jnbe)()    /* Opcode 0x77 */
{
	int tmp = (int)((INT8)FETCH);
	if (!(CF || ZF)) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_js)()    /* Opcode 0x78 */
{
	int tmp = (int)((INT8)FETCH);
	if (SF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jns)()    /* Opcode 0x79 */
{
	int tmp = (int)((INT8)FETCH);
	if (!SF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jp)()    /* Opcode 0x7a */
{
	int tmp = (int)((INT8)FETCH);
	if (PF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jnp)()    /* Opcode 0x7b */
{
	int tmp = (int)((INT8)FETCH);
	if (!PF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jl)()    /* Opcode 0x7c */
{
	int tmp = (int)((INT8)FETCH);
	if ((SF!=OF)&&!ZF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jnl)()    /* Opcode 0x7d */
{
	int tmp = (int)((INT8)FETCH);
	if (ZF||(SF==OF)) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jle)()    /* Opcode 0x7e */
{
	int tmp = (int)((INT8)FETCH);
	if (ZF||(SF!=OF)) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_jnle)()    /* Opcode 0x7f */
{
	int tmp = (int)((INT8)FETCH);
	if ((SF==OF)&&!ZF) {
		cpustate->pc += tmp;
		ICOUNT -= timing.jcc_t;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.jcc_nt;
}

void IX86_OPS_BASE::PREFIX86(_80pre)()    /* Opcode 0x80 */
{
	unsigned ModRM = FETCHOP;
	unsigned dst = GetRMByte(ModRM);
	unsigned src = FETCH;
	int tmpcf;

	switch (ModRM & 0x38)
	{
	case 0x00:  /* ADD eb,d8 */
		ADDB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x08:  /* OR eb,d8 */
		ORB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x10:  /* ADC eb,d8 */
		src+=CF;
		ADCB(dst,src,tmpcf);
		PutbackRMByte(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x18:  /* SBB eb,b8 */
		src+=CF;
		SBBB(dst,src,tmpcf);
		PutbackRMByte(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x20:  /* AND eb,d8 */
		ANDB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x28:  /* SUB eb,d8 */
		SUBB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x30:  /* XOR eb,d8 */
		XORB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x38:  /* CMP eb,d8 */
		SUBB(dst,src);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8_ro;
		break;
	}
}


void IX86_OPS_BASE::PREFIX86(_81pre)()    /* Opcode 0x81 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMWord(ModRM);
	unsigned src = FETCH;
	int tmpcf;
	src+= (FETCH << 8);

	switch (ModRM & 0x38)
	{
	case 0x00:  /* ADD ew,d16 */
		ADDW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x08:  /* OR ew,d16 */
		ORW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x10:  /* ADC ew,d16 */
		src+=CF;
		ADCW(dst,src,tmpcf);
		PutbackRMWord(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x18:  /* SBB ew,d16 */
		src+=CF;
		SBBW(dst,src,tmpcf);
		PutbackRMWord(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x20:  /* AND ew,d16 */
		ANDW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x28:  /* SUB ew,d16 */
		SUBW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x30:  /* XOR ew,d16 */
		XORW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 0x38:  /* CMP ew,d16 */
		SUBW(dst,src);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16_ro;
		break;
	}
}

void IX86_OPS_BASE::PREFIX86(_82pre)()  /* Opcode 0x82 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMByte(ModRM);
	unsigned src = FETCH;
	int tmpcf;

	switch (ModRM & 0x38)
	{
	case 0x00:  /* ADD eb,d8 */
		ADDB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x08:  /* OR eb,d8 */
		ORB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x10:  /* ADC eb,d8 */
		src+=CF;
		ADCB(dst,src,tmpcf);
		PutbackRMByte(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x18:  /* SBB eb,d8 */
		src+=CF;
		SBBB(dst,src,tmpcf);
		PutbackRMByte(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x20:  /* AND eb,d8 */
		ANDB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x28:  /* SUB eb,d8 */
		SUBB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x30:  /* XOR eb,d8 */
		XORB(dst,src);
		PutbackRMByte(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 0x38:  /* CMP eb,d8 */
		SUBB(dst,src);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8_ro;
		break;
	}
}

void IX86_OPS_BASE::PREFIX86(_83pre)()    /* Opcode 0x83 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMWord(ModRM);
	unsigned src = (WORD)((INT16)((INT8)FETCH));
	int tmpcf;

	switch (ModRM & 0x38)
	{
	case 0x00:  /* ADD ew,d16 */
		ADDW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x08:  /* OR ew,d16 */
		ORW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x10:  /* ADC ew,d16 */
		src+=CF;
		ADCW(dst,src,tmpcf);
		PutbackRMWord(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x18:  /* SBB ew,d16 */
		src+=CF;
		SBBW(dst,src,tmpcf);
		PutbackRMWord(ModRM,dst);
		cpustate->CarryVal = tmpcf;
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x20:  /* AND ew,d16 */
		ANDW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x28:  /* SUB ew,d16 */
		SUBW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x30:  /* XOR ew,d16 */
		XORW(dst,src);
		PutbackRMWord(ModRM,dst);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 0x38:  /* CMP ew,d16 */
		SUBW(dst,src);
		ICOUNT -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8_ro;
		break;
	}
}

void IX86_OPS_BASE::PREFIX86(_test_br8)()    /* Opcode 0x84 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ANDB(dst,src);
}

void IX86_OPS_BASE::PREFIX86(_test_wr16)()    /* Opcode 0x85 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ANDW(dst,src);
}

void IX86_OPS_BASE::PREFIX86(_xchg_br8)()    /* Opcode 0x86 */
{
	DEF_br8(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.xchg_rr8 : timing.xchg_rm8;
	PutbackRMByte(ModRM,src);
	RegByte(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_xchg_wr16)()    /* Opcode 0x87 */
{
	DEF_wr16(dst,src);
	ICOUNT -= (ModRM >= 0xc0) ? timing.xchg_rr16 : timing.xchg_rm16;
	PutbackRMWord(ModRM,src);
	RegWord(ModRM)=dst;
}

void IX86_OPS_BASE::PREFIX86(_mov_br8)()    /* Opcode 0x88 */
{
	unsigned ModRM = FETCH;
	BYTE src = RegByte(ModRM);
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_rr8 : timing.mov_mr8;
	PutRMByte(ModRM,src);
}

void IX86_OPS_BASE::PREFIX86(_mov_wr16)()    /* Opcode 0x89 */
{
	unsigned ModRM = FETCH;
	WORD src = RegWord(ModRM);
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_rr16 : timing.mov_mr16;
	PutRMWord(ModRM,src);
}

void IX86_OPS_BASE::PREFIX86(_mov_r8b)()    /* Opcode 0x8a */
{
	unsigned ModRM = FETCH;
	BYTE src = GetRMByte(ModRM);
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_rr8 : timing.mov_rm8;
	RegByte(ModRM)=src;
}

void IX86_OPS_BASE::PREFIX86(_mov_r16w)()    /* Opcode 0x8b */
{
	unsigned ModRM = FETCH;
	WORD src = GetRMWord(ModRM);
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_rr8 : timing.mov_rm16;
	RegWord(ModRM)=src;
}

void IX86_OPS_BASE::PREFIX86(_mov_wsreg)()    /* Opcode 0x8c */
{
	unsigned ModRM = FETCH;
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_rs : timing.mov_ms;
	if (ModRM & 0x20) { /* HJB 12/13/98 1xx is invalid */
		cpustate->pc = cpustate->prevpc;
		return PREFIX86(_invalid)(cpustate);
	}

	PutRMWord(ModRM,cpustate->sregs[(ModRM & 0x38) >> 3]);
}

void IX86_OPS_BASE::PREFIX86(_lea)()    /* Opcode 0x8d */
{
	unsigned ModRM = FETCH;
	ICOUNT -= timing.lea;
	(void)(*GetEA[ModRM])(cpustate);
	RegWord(ModRM)=cpustate->eo;    /* HJB 12/13/98 effective offset (no segment part) */
}

void IX86_OPS_BASE::PREFIX86(_popw)()    /* Opcode 0x8f */
{
	unsigned ModRM = FETCH;
		WORD tmp;
	tmp = ReadWord(cpustate->base[SS] + cpustate->regs.w[SP]);
	ICOUNT -= (ModRM >= 0xc0) ? timing.pop_r16 : timing.pop_m16;
	PutRMWord(ModRM,tmp);
	cpustate->regs.w[SP] += 2;
}


#define XchgAXReg(Reg)              \
{                                   \
	WORD tmp;                       \
	tmp = cpustate->regs.w[Reg];            \
	cpustate->regs.w[Reg] = cpustate->regs.w[AX];   \
	cpustate->regs.w[AX] = tmp;             \
	ICOUNT -= timing.xchg_ar16;     \
}


void IX86_OPS_BASE::PREFIX86(_nop)()    /* Opcode 0x90 */
{
	/* this is XchgAXReg(AX); */
	ICOUNT -= timing.nop;
}

void IX86_OPS_BASE::PREFIX86(_xchg_axcx)()    /* Opcode 0x91 */
{
	XchgAXReg(CX);
}

void IX86_OPS_BASE::PREFIX86(_xchg_axdx)()    /* Opcode 0x92 */
{
	XchgAXReg(DX);
}

void IX86_OPS_BASE::PREFIX86(_xchg_axbx)()    /* Opcode 0x93 */
{
	XchgAXReg(BX);
}

void IX86_OPS_BASE::PREFIX86(_xchg_axsp)()    /* Opcode 0x94 */
{
	XchgAXReg(SP);
}

void IX86_OPS_BASE::PREFIX86(_xchg_axbp)()    /* Opcode 0x95 */
{
	XchgAXReg(BP);
}

void IX86_OPS_BASE::PREFIX86(_xchg_axsi)()    /* Opcode 0x96 */
{
	XchgAXReg(SI);
}

void IX86_OPS_BASE::PREFIX86(_xchg_axdi)()    /* Opcode 0x97 */
{
	XchgAXReg(DI);
}

void IX86_OPS_BASE::PREFIX86(_cbw)()    /* Opcode 0x98 */
{
	ICOUNT -= timing.cbw;
	cpustate->regs.b[AH] = (cpustate->regs.b[AL] & 0x80) ? 0xff : 0;
}

void IX86_OPS_BASE::PREFIX86(_cwd)()    /* Opcode 0x99 */
{
	ICOUNT -= timing.cwd;
	cpustate->regs.w[DX] = (cpustate->regs.b[AH] & 0x80) ? 0xffff : 0;
}

void IX86_OPS_BASE::PREFIX86(_call_far)()
{
	unsigned int tmp, tmp2;
	WORD cs, ip;

	tmp = FETCH;
	tmp += FETCH << 8;

	tmp2 = FETCH;
	tmp2 += FETCH << 8;

//#ifdef I86_PSEUDO_BIOS
//	if(cpustate->bios != NULL && cpustate->bios->bios_call_i86(((tmp2 << 4) + tmp) & AMASK, cpustate->regs.w, cpustate->sregs, &cpustate->ZeroVal, &cpustate->CarryVal)) {
//		ICOUNT -= timing.call_far;
//		return;
//	}
//#endif

	ip = cpustate->pc - cpustate->base[CS];
	cs = cpustate->sregs[CS];

//#ifdef I80286
//	i80286_code_descriptor(cpustate, tmp2, tmp, 2);
//#else
	cpustate->sregs[CS] = (WORD)tmp2;
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->base[CS] + (WORD)tmp) & AMASK;
//#endif
	PUSH(cs);
	PUSH(ip);
	ICOUNT -= timing.call_far;
	CHANGE_PC(cpustate->pc);
}

void IX86_OPS_BASE::PREFIX86(_wait)()    /* Opcode 0x9b */
{
	if (cpustate->test_state)
	{
		ICOUNT = 0;
		cpustate->pc--;
	}
	else
		ICOUNT -= timing.wait;
}

void IX86_OPS_BASE::PREFIX86(_pushf)()    /* Opcode 0x9c */
{
	unsigned tmp;
	ICOUNT -= timing.pushf;

	tmp = CompressFlags();
	PUSH( tmp );
}

//#ifndef I80286
void IX86_OPS_BASE::PREFIX86(_popf)()    /* Opcode 0x9d */
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
//#endif

void IX86_OPS_BASE::PREFIX86(_sahf)()    /* Opcode 0x9e */
{
	unsigned tmp = (CompressFlags() & 0xff00) | (cpustate->regs.b[AH] & 0xd5);
	ICOUNT -= timing.sahf;
	ExpandFlags(tmp);
}

void IX86_OPS_BASE::PREFIX86(_lahf)()    /* Opcode 0x9f */
{
	cpustate->regs.b[AH] = CompressFlags() & 0xff;
	ICOUNT -= timing.lahf;
}


void IX86_OPS_BASE::PREFIX86(_mov_aldisp)()    /* Opcode 0xa0 */
{
	unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

	ICOUNT -= timing.mov_am8;
	cpustate->regs.b[AL] = GetMemB(DS, addr);
}

void IX86_OPS_BASE::PREFIX86(_mov_axdisp)()    /* Opcode 0xa1 */
{
	unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

	ICOUNT -= timing.mov_am16;
	cpustate->regs.w[AX] = GetMemW(DS, addr);
}

void IX86_OPS_BASE::PREFIX86(_mov_dispal)()    /* Opcode 0xa2 */
{
	unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

	ICOUNT -= timing.mov_ma8;
	PutMemB(DS, addr, cpustate->regs.b[AL]);
}

void IX86_OPS_BASE::PREFIX86(_mov_dispax)()    /* Opcode 0xa3 */
{
	unsigned addr;

	addr = FETCH;
	addr += FETCH << 8;

	ICOUNT -= timing.mov_ma16;
	PutMemW(DS, addr, cpustate->regs.w[AX]);
}

void IX86_OPS_BASE::PREFIX86(_movsb)()    /* Opcode 0xa4 */
{
	BYTE tmp = GetMemB(DS,cpustate->regs.w[SI]);
	PutMemB(ES,cpustate->regs.w[DI], tmp);
	cpustate->regs.w[DI] += cpustate->DirVal;
	cpustate->regs.w[SI] += cpustate->DirVal;
	ICOUNT -= timing.movs8;
}

void IX86_OPS_BASE::PREFIX86(_movsw)()    /* Opcode 0xa5 */
{
	WORD tmp = GetMemW(DS,cpustate->regs.w[SI]);
	PutMemW(ES,cpustate->regs.w[DI], tmp);
	cpustate->regs.w[DI] += 2 * cpustate->DirVal;
	cpustate->regs.w[SI] += 2 * cpustate->DirVal;
	ICOUNT -= timing.movs16;
}

void IX86_OPS_BASE::PREFIX86(_cmpsb)()    /* Opcode 0xa6 */
{
	unsigned dst = GetMemB(ES, cpustate->regs.w[DI]);
	unsigned src = GetMemB(DS, cpustate->regs.w[SI]);
	SUBB(src,dst); /* opposite of the usual convention */
	cpustate->regs.w[DI] += cpustate->DirVal;
	cpustate->regs.w[SI] += cpustate->DirVal;
	ICOUNT -= timing.cmps8;
}

void IX86_OPS_BASE::PREFIX86(_cmpsw)()    /* Opcode 0xa7 */
{
	unsigned dst = GetMemW(ES, cpustate->regs.w[DI]);
	unsigned src = GetMemW(DS, cpustate->regs.w[SI]);
	SUBW(src,dst); /* opposite of the usual convention */
	cpustate->regs.w[DI] += 2 * cpustate->DirVal;
	cpustate->regs.w[SI] += 2 * cpustate->DirVal;
	ICOUNT -= timing.cmps16;
}

void IX86_OPS_BASE::PREFIX86(_test_ald8)()    /* Opcode 0xa8 */
{
	DEF_ald8(dst,src);
	ICOUNT -= timing.alu_ri8;
	ANDB(dst,src);
}

void IX86_OPS_BASE::PREFIX86(_test_axd16)()    /* Opcode 0xa9 */
{
	DEF_axd16(dst,src);
	ICOUNT -= timing.alu_ri16;
	ANDW(dst,src);
}

void IX86_OPS_BASE::PREFIX86(_stosb)()    /* Opcode 0xaa */
{
	PutMemB(ES,cpustate->regs.w[DI],cpustate->regs.b[AL]);
	cpustate->regs.w[DI] += cpustate->DirVal;
	ICOUNT -= timing.stos8;
}

void IX86_OPS_BASE::PREFIX86(_stosw)()    /* Opcode 0xab */
{
	PutMemW(ES,cpustate->regs.w[DI],cpustate->regs.w[AX]);
	cpustate->regs.w[DI] += 2 * cpustate->DirVal;
	ICOUNT -= timing.stos16;
}

void IX86_OPS_BASE::PREFIX86(_lodsb)()    /* Opcode 0xac */
{
	cpustate->regs.b[AL] = GetMemB(DS,cpustate->regs.w[SI]);
	cpustate->regs.w[SI] += cpustate->DirVal;
	ICOUNT -= timing.lods8;
}

void IX86_OPS_BASE::PREFIX86(_lodsw)()    /* Opcode 0xad */
{
	cpustate->regs.w[AX] = GetMemW(DS,cpustate->regs.w[SI]);
	cpustate->regs.w[SI] += 2 * cpustate->DirVal;
	ICOUNT -= timing.lods16;
}

void IX86_OPS_BASE::PREFIX86(_scasb)()    /* Opcode 0xae */
{
	unsigned src = GetMemB(ES, cpustate->regs.w[DI]);
	unsigned dst = cpustate->regs.b[AL];
	SUBB(dst,src);
	cpustate->regs.w[DI] += cpustate->DirVal;
	ICOUNT -= timing.scas8;
}

void IX86_OPS_BASE::PREFIX86(_scasw)()    /* Opcode 0xaf */
{
	unsigned src = GetMemW(ES, cpustate->regs.w[DI]);
	unsigned dst = cpustate->regs.w[AX];
	SUBW(dst,src);
	cpustate->regs.w[DI] += 2 * cpustate->DirVal;
	ICOUNT -= timing.scas16;
}

void IX86_OPS_BASE::PREFIX86(_mov_ald8)()    /* Opcode 0xb0 */
{
	cpustate->regs.b[AL] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

void IX86_OPS_BASE::PREFIX86(_mov_cld8)()    /* Opcode 0xb1 */
{
	cpustate->regs.b[CL] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

void IX86_OPS_BASE::PREFIX86(_mov_dld8)()    /* Opcode 0xb2 */
{
	cpustate->regs.b[DL] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

void IX86_OPS_BASE::PREFIX86(_mov_bld8)()    /* Opcode 0xb3 */
{
	cpustate->regs.b[BL] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

void IX86_OPS_BASE::PREFIX86(_mov_ahd8)()    /* Opcode 0xb4 */
{
	cpustate->regs.b[AH] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

void IX86_OPS_BASE::PREFIX86(_mov_chd8)()    /* Opcode 0xb5 */
{
	cpustate->regs.b[CH] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

void IX86_OPS_BASE::PREFIX86(_mov_dhd8)()    /* Opcode 0xb6 */
{
	cpustate->regs.b[DH] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

void IX86_OPS_BASE::PREFIX86(_mov_bhd8)()    /* Opcode 0xb7 */
{
	cpustate->regs.b[BH] = FETCH;
	ICOUNT -= timing.mov_ri8;
}

void IX86_OPS_BASE::PREFIX86(_mov_axd16)()    /* Opcode 0xb8 */
{
	cpustate->regs.b[AL] = FETCH;
	cpustate->regs.b[AH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

void IX86_OPS_BASE::PREFIX86(_mov_cxd16)()    /* Opcode 0xb9 */
{
	cpustate->regs.b[CL] = FETCH;
	cpustate->regs.b[CH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

void IX86_OPS_BASE::PREFIX86(_mov_dxd16)()    /* Opcode 0xba */
{
	cpustate->regs.b[DL] = FETCH;
	cpustate->regs.b[DH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

void IX86_OPS_BASE::PREFIX86(_mov_bxd16)()    /* Opcode 0xbb */
{
	cpustate->regs.b[BL] = FETCH;
	cpustate->regs.b[BH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

void IX86_OPS_BASE::PREFIX86(_mov_spd16)()    /* Opcode 0xbc */
{
	cpustate->regs.b[SPL] = FETCH;
	cpustate->regs.b[SPH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

void IX86_OPS_BASE::PREFIX86(_mov_bpd16)()    /* Opcode 0xbd */
{
	cpustate->regs.b[BPL] = FETCH;
	cpustate->regs.b[BPH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

void IX86_OPS_BASE::PREFIX86(_mov_sid16)()    /* Opcode 0xbe */
{
	cpustate->regs.b[SIL] = FETCH;
	cpustate->regs.b[SIH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

void IX86_OPS_BASE::PREFIX86(_mov_did16)()    /* Opcode 0xbf */
{
	cpustate->regs.b[DIL] = FETCH;
	cpustate->regs.b[DIH] = FETCH;
	ICOUNT -= timing.mov_ri16;
}

void IX86_OPS_BASE::PREFIX86(_ret_d16)()    /* Opcode 0xc2 */
{
	unsigned count = FETCH;
	count += FETCH << 8;
	POP(cpustate->pc);
	cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
	cpustate->regs.w[SP]+=count;
	ICOUNT -= timing.ret_near_imm;
	CHANGE_PC(cpustate->pc);
}

void IX86_OPS_BASE::PREFIX86(_ret)()    /* Opcode 0xc3 */
{
	POP(cpustate->pc);
	cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
	ICOUNT -= timing.ret_near;
	CHANGE_PC(cpustate->pc);
}

void IX86_OPS_BASE::PREFIX86(_les_dw)()    /* Opcode 0xc4 */
{
	unsigned ModRM = FETCH;
	WORD tmp = GetRMWord(ModRM);

	cpustate->sregs[ES] = GetnextRMWord;
	cpustate->base[ES] = SegBase(ES);
	RegWord(ModRM)= tmp;
	ICOUNT -= timing.load_ptr;
}

void IX86_OPS_BASE::PREFIX86(_lds_dw)()    /* Opcode 0xc5 */
{
	unsigned ModRM = FETCH;
	WORD tmp = GetRMWord(ModRM);

	cpustate->sregs[DS] = GetnextRMWord;
	cpustate->base[DS] = SegBase(DS);
	RegWord(ModRM)=tmp;
	ICOUNT -= timing.load_ptr;
}

void IX86_OPS_BASE::PREFIX86(_mov_bd8)()    /* Opcode 0xc6 */
{
	unsigned ModRM = FETCH;
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_ri8 : timing.mov_mi8;
	PutImmRMByte(ModRM);
}

void IX86_OPS_BASE::PREFIX86(_mov_wd16)()    /* Opcode 0xc7 */
{
	unsigned ModRM = FETCH;
	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_ri16 : timing.mov_mi16;
	PutImmRMWord(ModRM);
}

//#ifndef I80286
void IX86_OPS_BASE::PREFIX86(_retf_d16)()    /* Opcode 0xca */
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

void IX86_OPS_BASE::PREFIX86(_retf)()    /* Opcode 0xcb */
{
	POP(cpustate->pc);
	POP(cpustate->sregs[CS]);
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;
	ICOUNT -= timing.ret_far;
	CHANGE_PC(cpustate->pc);
}
//#endif

void IX86_OPS_BASE::PREFIX86(_int3)()    /* Opcode 0xcc */
{
	ICOUNT -= timing.int3;
	PREFIX(_interrupt)(cpustate, 3);
}

void IX86_OPS_BASE::PREFIX86(_int)()    /* Opcode 0xcd */
{
	unsigned int_num = FETCH;
	ICOUNT -= timing.int_imm;
//#ifdef I86_PSEUDO_BIOS
//	if(cpustate->bios != NULL && cpustate->bios->bios_int_i86(int_num, cpustate->regs.w, cpustate->sregs, &cpustate->ZeroVal, &cpustate->CarryVal)) {
//		return;
//	}
//#endif
	PREFIX(_interrupt)(cpustate, int_num);
}

void IX86_OPS_BASE::PREFIX86(_into)()    /* Opcode 0xce */
{
	if (OF) {
		ICOUNT -= timing.into_t;
		PREFIX(_interrupt)(cpustate, 4);
	} else ICOUNT -= timing.into_nt;
}


void IX86_OPS_BASE::PREFIX86(_iret)()    /* Opcode 0xcf */
{
//#ifndef I80286
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
//#endif
}

void IX86_OPS_BASE::PREFIX86(_rotshft_b)()    /* Opcode 0xd0 */
{
	unsigned ModRM = FETCHOP;
	PREFIX(_rotate_shift_Byte)(cpustate,ModRM,1,GetRMByte(ModRM));
}


void IX86_OPS_BASE::PREFIX86(_rotshft_w)()    /* Opcode 0xd1 */
{
	unsigned ModRM = FETCHOP;
	PREFIX(_rotate_shift_Word)(cpustate,ModRM,1,GetRMWord(ModRM));
}


void IX86_OPS_BASE::PREFIX86(_rotshft_bcl)()    /* Opcode 0xd2 */
{
	unsigned ModRM = FETCHOP;
	PREFIX(_rotate_shift_Byte)(cpustate,ModRM,cpustate->regs.b[CL],GetRMByte(ModRM));
}

void IX86_OPS_BASE::PREFIX86(_rotshft_wcl)()    /* Opcode 0xd3 */
{
	unsigned ModRM = FETCHOP;
	PREFIX(_rotate_shift_Word)(cpustate,ModRM,cpustate->regs.b[CL],GetRMWord(ModRM));
}

/* OB: Opcode works on NEC V-Series but not the Variants              */
/*     one could specify any byte value as operand but the NECs */
/*     always substitute 0x0a.              */
void IX86_OPS_BASE::PREFIX86(_aam)()    /* Opcode 0xd4 */
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

void IX86_OPS_BASE::PREFIX86(_aad)()    /* Opcode 0xd5 */
{
	unsigned mult = FETCH;

	ICOUNT -= timing.aad;

	cpustate->regs.b[AL] = cpustate->regs.b[AH] * mult + cpustate->regs.b[AL];
	cpustate->regs.b[AH] = 0;

	SetZF(cpustate->regs.b[AL]);
	SetPF(cpustate->regs.b[AL]);
	cpustate->SignVal = 0;
}


void IX86_OPS_BASE::PREFIX86(_xlat)()    /* Opcode 0xd7 */
{
	unsigned dest = cpustate->regs.w[BX]+cpustate->regs.b[AL];

	ICOUNT -= timing.xlat;
	cpustate->regs.b[AL] = GetMemB(DS, dest);
}

//#ifndef I80286
void IX86_OPS_BASE::PREFIX86(_escape)()    /* Opcodes 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde and 0xdf */
{
	unsigned ModRM = FETCH;
	ICOUNT -= timing.nop;
	GetRMByte(ModRM);
}
//#endif

void IX86_OPS_BASE::PREFIX86(_loopne)()    /* Opcode 0xe0 */
{
	int disp = (int)((INT8)FETCH);
	unsigned tmp = cpustate->regs.w[CX]-1;

	cpustate->regs.w[CX]=tmp;

	if (!ZF && tmp) {
		ICOUNT -= timing.loop_t;
		cpustate->pc += disp;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.loop_nt;
}

void IX86_OPS_BASE::PREFIX86(_loope)()    /* Opcode 0xe1 */
{
	int disp = (int)((INT8)FETCH);
	unsigned tmp = cpustate->regs.w[CX]-1;

	cpustate->regs.w[CX]=tmp;

	if (ZF && tmp) {
		ICOUNT -= timing.loope_t;
			cpustate->pc += disp;
/* ASG - can probably assume this is safe
         CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.loope_nt;
}

void IX86_OPS_BASE::PREFIX86(_loop)()    /* Opcode 0xe2 */
{
	int disp = (int)((INT8)FETCH);
	unsigned tmp = cpustate->regs.w[CX]-1;

	cpustate->regs.w[CX]=tmp;

	if (tmp) {
		ICOUNT -= timing.loop_t;
		cpustate->pc += disp;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else ICOUNT -= timing.loop_nt;
}

void IX86_OPS_BASE::PREFIX86(_jcxz)()    /* Opcode 0xe3 */
{
	int disp = (int)((INT8)FETCH);

	if (cpustate->regs.w[CX] == 0) {
		ICOUNT -= timing.jcxz_t;
		cpustate->pc += disp;
/* ASG - can probably assume this is safe
        CHANGE_PC(cpustate->pc);*/
	} else
		ICOUNT -= timing.jcxz_nt;
}

void IX86_OPS_BASE::PREFIX86(_inal)()    /* Opcode 0xe4 */
{
	unsigned port;
	port = FETCH;

	ICOUNT -= timing.in_imm8;
	cpustate->regs.b[AL] = read_port_byte(port);
}

void IX86_OPS_BASE::PREFIX86(_inax)()    /* Opcode 0xe5 */
{
	unsigned port;
	port = FETCH;

	ICOUNT -= timing.in_imm16;
	cpustate->regs.w[AX] = read_port_word(port);
}

void IX86_OPS_BASE::PREFIX86(_outal)()    /* Opcode 0xe6 */
{
	unsigned port;
	port = FETCH;

	ICOUNT -= timing.out_imm8;
	write_port_byte(port, cpustate->regs.b[AL]);
}

void IX86_OPS_BASE::PREFIX86(_outax)()    /* Opcode 0xe7 */
{
	unsigned port;
	port = FETCH;

	ICOUNT -= timing.out_imm16;
	write_port_word(port, cpustate->regs.w[AX]);
}

void IX86_OPS_BASE::PREFIX86(_call_d16)()    /* Opcode 0xe8 */
{
	WORD ip, tmp;

	FETCHWORD(tmp);
	ip = cpustate->pc - cpustate->base[CS];
	PUSH(ip);
	ip += tmp;
	cpustate->pc = (ip + cpustate->base[CS]) & AMASK;
	ICOUNT -= timing.call_near;
	CHANGE_PC(cpustate->pc);
}

void IX86_OPS_BASE::PREFIX86(_jmp_d16)()    /* Opcode 0xe9 */
{
	WORD ip, tmp;

	FETCHWORD(tmp);
	ip = cpustate->pc - cpustate->base[CS] + tmp;
	cpustate->pc = (ip + cpustate->base[CS]) & AMASK;
	ICOUNT -= timing.jmp_near;
	CHANGE_PC(cpustate->pc);
}

void IX86_OPS_BASE::PREFIX86(_jmp_far)()    /* Opcode 0xea */
{
	unsigned tmp,tmp1;

	tmp = FETCH;
	tmp += FETCH << 8;

	tmp1 = FETCH;
	tmp1 += FETCH << 8;

	cpustate->sregs[CS] = (WORD)tmp1;
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = (cpustate->base[CS] + tmp) & AMASK;
	ICOUNT -= timing.jmp_far;
	CHANGE_PC(cpustate->pc);
}

void IX86_OPS_BASE::PREFIX86(_jmp_d8)()    /* Opcode 0xeb */
{
	int tmp = (int)((INT8)FETCH);
	cpustate->pc += tmp;
/* ASG - can probably assume this is safe
    CHANGE_PC(cpustate->pc);*/
	ICOUNT -= timing.jmp_short;
}

void IX86_OPS_BASE::PREFIX86(_inaldx)()    /* Opcode 0xec */
{
	ICOUNT -= timing.in_dx8;
	cpustate->regs.b[AL] = read_port_byte(cpustate->regs.w[DX]);
}

void IX86_OPS_BASE::PREFIX86(_inaxdx)()    /* Opcode 0xed */
{
	unsigned port = cpustate->regs.w[DX];
	ICOUNT -= timing.in_dx16;
	cpustate->regs.w[AX] = read_port_word(port);
}

void IX86_OPS_BASE::PREFIX86(_outdxal)()    /* Opcode 0xee */
{
	ICOUNT -= timing.out_dx8;
	write_port_byte(cpustate->regs.w[DX], cpustate->regs.b[AL]);
}

void IX86_OPS_BASE::PREFIX86(_outdxax)()    /* Opcode 0xef */
{
	unsigned port = cpustate->regs.w[DX];
	ICOUNT -= timing.out_dx16;
	write_port_word(port, cpustate->regs.w[AX]);
}

/* I think thats not a V20 instruction...*/
void IX86_OPS_BASE::PREFIX86(_lock)()    /* Opcode 0xf0 */
{
	ICOUNT -= timing.nop;
	PREFIX(_instruction)[FETCHOP](cpustate);  /* un-interruptible */
}
#endif

void IX86_OPS_BASE::PREFIX(_pop_ss)()    /* Opcode 0x17 */
{
	POP(cpustate->sregs[SS]);
	cpustate->base[SS] = SegBase(SS);
	ICOUNT -= timing.pop_seg;
	PREFIX(_instruction)[FETCHOP](cpustate); /* no interrupt before next instruction */
}

void IX86_OPS_BASE::PREFIX(_es)()    /* Opcode 0x26 */
{
	cpustate->seg_prefix = TRUE;
	cpustate->prefix_seg = ES;
	ICOUNT -= timing.override;
	PREFIX(_instruction)[FETCHOP](cpustate);
}

void IX86_OPS_BASE::PREFIX(_cs)()    /* Opcode 0x2e */
{
	cpustate->seg_prefix = TRUE;
	cpustate->prefix_seg = CS;
	ICOUNT -= timing.override;
	PREFIX(_instruction)[FETCHOP](cpustate);
}

void IX86_OPS_BASE::PREFIX(_ss)()    /* Opcode 0x36 */
{
	cpustate->seg_prefix = TRUE;
	cpustate->prefix_seg = SS;
	ICOUNT -= timing.override;
	PREFIX(_instruction)[FETCHOP](cpustate);
}

void IX86_OPS_BASE::PREFIX(_ds)()    /* Opcode 0x3e */
{
	cpustate->seg_prefix = TRUE;
	cpustate->prefix_seg = DS;
	ICOUNT -= timing.override;
	PREFIX(_instruction)[FETCHOP](cpustate);
}

void IX86_OPS_BASE::PREFIX(_mov_sregw)()    /* Opcode 0x8e */
{
	unsigned ModRM = FETCH;
	WORD src = GetRMWord(ModRM);

	ICOUNT -= (ModRM >= 0xc0) ? timing.mov_sr : timing.mov_sm;
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
//#ifndef I80186
		int ip = cpustate->pc - cpustate->base[CS];
		cpustate->sregs[CS] = src;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (ip + cpustate->base[CS]) & AMASK;
		CHANGE_PC(cpustate->pc);
//#endif
		break;
	}
}

void IX86_OPS_BASE::PREFIX(_repne)()    /* Opcode 0xf2 */
{
		PREFIX(rep)(cpustate, 0);
}

void IX86_OPS_BASE::PREFIX(_repe)()    /* Opcode 0xf3 */
{
	PREFIX(rep)(cpustate, 1);
}

void IX86_OPS_BASE::PREFIX(_sti)()    /* Opcode 0xfb */
{
	ICOUNT -= timing.flag_ops;
	SetIF(1);
	PREFIX(_instruction)[FETCHOP](cpustate); /* no interrupt before next instruction */

	/* if an interrupt is pending, signal an interrupt */
	if (cpustate->irq_state) {
		PREFIX86(_interrupt)(cpustate, (UINT32)-1);
		cpustate->irq_state = 0;
	}
}

//#ifndef I80186
void IX86_OPS_BASE::PREFIX86(_hlt)()    /* Opcode 0xf4 */
{
	cpustate->halted=1;
	ICOUNT = 0;
}

void IX86_OPS_BASE::PREFIX86(_cmc)()    /* Opcode 0xf5 */
{
	ICOUNT -= timing.flag_ops;
	cpustate->CarryVal = !CF;
}

void IX86_OPS_BASE::PREFIX86(_f6pre)()
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


void IX86_OPS_BASE::PREFIX86(_f7pre)()
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


void IX86_OPS_BASE::PREFIX86(_clc)()    /* Opcode 0xf8 */
{
	ICOUNT -= timing.flag_ops;
	cpustate->CarryVal = 0;
}

void IX86_OPS_BASE::PREFIX86(_stc)()    /* Opcode 0xf9 */
{
	ICOUNT -= timing.flag_ops;
	cpustate->CarryVal = 1;
}

void IX86_OPS_BASE::PREFIX86(_cli)()    /* Opcode 0xfa */
{
	ICOUNT -= timing.flag_ops;
	SetIF(0);
}

void IX86_OPS_BASE::PREFIX86(_cld)()    /* Opcode 0xfc */
{
	ICOUNT -= timing.flag_ops;
	SetDF(0);
}

void IX86_OPS_BASE::PREFIX86(_std)()    /* Opcode 0xfd */
{
	ICOUNT -= timing.flag_ops;
	SetDF(1);
}

void IX86_OPS_BASE::PREFIX86(_fepre)()    /* Opcode 0xfe */
{
	unsigned ModRM = FETCH;
	unsigned tmp = GetRMByte(ModRM);
	unsigned tmp1;

	ICOUNT -= (ModRM >= 0xc0) ? timing.incdec_r8 : timing.incdec_m8;
	if ((ModRM & 0x38) == 0)  /* INC eb */
	{
		tmp1 = tmp+1;
		SetOFB_Add(tmp1,tmp,1);
	}
	else  /* DEC eb */
	{
		tmp1 = tmp-1;
		SetOFB_Sub(tmp1,1,tmp);
	}

	SetAF(tmp1,tmp,1);
	SetSZPF_Byte(tmp1);

	PutbackRMByte(ModRM,(BYTE)tmp1);
}


void IX86_OPS_BASE::PREFIX86(_ffpre)()    /* Opcode 0xff */
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
		ip = cpustate->pc - cpustate->base[CS];
		cpustate->sregs[CS] = tmp2;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (cpustate->base[CS] + tmp1) & AMASK;

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

		cpustate->pc = GetRMWord(ModRM);
		cpustate->sregs[CS] = GetnextRMWord;
		cpustate->base[CS] = SegBase(CS);
		cpustate->pc = (cpustate->pc + cpustate->base[CS]) & AMASK;

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


void IX86_OPS_BASE::PREFIX86(_invalid)()
{
	logerror("illegal instruction %.2x at %.5x\n",PEEKBYTE(cpustate->pc-1), cpustate->pc);
	/* i8086/i8088 ignore an invalid opcode. */
	/* i80186/i80188 probably also ignore an invalid opcode. */
	ICOUNT -= 10;
}

//#ifndef I80286
void IX86_OPS_BASE::PREFIX86(_invalid_2b)()
{
	unsigned ModRM = FETCH;
	GetRMByte(ModRM);
	logerror("illegal 2 byte instruction %.2x at %.5x\n",PEEKBYTE(cpustate->pc-2), cpustate->pc-2);
	ICOUNT -= 10;
}
//#endif
#endif
