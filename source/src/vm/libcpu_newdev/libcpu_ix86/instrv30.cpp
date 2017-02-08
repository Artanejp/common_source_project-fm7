#include "./ix86_opdef.h"

#define PREFIX(XXX) PREFIXV30(XXXX)

void IX86_OPS_BASE::PREFIX(_interrupt)(unsigned int_num)
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
// ??
void IX86_OPS_BASE::PREFIX(_trap)()
{
	PREFIX(_instruction)[FETCHOP]();
	PREFIX(_interrupt)(1);
}

void IX86_OPS_BASE::PREFIX(_pushf)()    /* Opcode 0x9c */
{
	unsigned tmp;
	ICOUNT -= timing.pushf;

	tmp = CompressFlags();
	PUSH( tmp );
}

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
	case 0x6c:  /* REP INSB */
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

void IX86_OPS_BASE::PREFIXV30(_0fpre)()	/* Opcode 0x0f */
{
	static const unsigned bytes[] = {
		1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768
	};

	unsigned Opcode = FETCH;
	unsigned ModRM;
	unsigned tmp;
	unsigned tmp2;

	switch (Opcode)
	{
	case 0x10:							/* 0F 10 47 30 - TEST1 [bx+30h],cl */
		ModRM = FETCH;
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.b[Mod_RM.RM.b[ModRM]];
			ICOUNT -= 3;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadByte(cpustate->ea);
			ICOUNT = old - 12;		/* my source says 14 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = cpustate->regs.b[CL] & 0x7;
		cpustate->ZeroVal = tmp & bytes[tmp2] ? 1 : 0;
/*      SetZF(tmp & (1<<tmp2)); */
		break;

	case 0x11:							/* 0F 11 47 30 - TEST1 [bx+30h],cl */
		ModRM = FETCH;
		/* tmp = GetRMWord(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.w[Mod_RM.RM.w[ModRM]];
			ICOUNT -= 3;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadWord(cpustate->ea);
			ICOUNT = old - 12;		/* my source says 14 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = cpustate->regs.b[CL] & 0xF;
		cpustate->ZeroVal = tmp & bytes[tmp2] ? 1 : 0;
/*      SetZF(tmp & (1<<tmp2)); */
		break;

	case 0x12:							/* 0F 12 [mod:000:r/m] - CLR1 reg/m8,cl */
		ModRM = FETCH;
		/* need the long if due to correct cycles OB[19.07.99] */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.b[Mod_RM.RM.b[ModRM]];
			ICOUNT -= 5;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadByte(cpustate->ea);
			ICOUNT = old - 14;		/* my source says 14 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = cpustate->regs.b[CL] & 0x7;		/* hey its a Byte so &07 NOT &0f */
		tmp &= ~(bytes[tmp2]);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x13:							/* 0F 13 [mod:000:r/m] - CLR1 reg/m16,cl */
		ModRM = FETCH;
		/* tmp = GetRMWord(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.w[Mod_RM.RM.w[ModRM]];
			ICOUNT -= 5;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadWord(cpustate->ea);
			ICOUNT = old - 14;		/* my source says 14 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = cpustate->regs.b[CL] & 0xF;		/* this time its a word */
		tmp &= ~(bytes[tmp2]);
		PutbackRMWord(ModRM, tmp);
		break;

	case 0x14:							/* 0F 14 47 30 - SET1 [bx+30h],cl */
		ModRM = FETCH;
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.b[Mod_RM.RM.b[ModRM]];
			ICOUNT -= 4;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadByte(cpustate->ea);
			ICOUNT = old - 13;
		}
		tmp2 = cpustate->regs.b[CL] & 0x7;
		tmp |= (bytes[tmp2]);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x15:							/* 0F 15 C6 - SET1 si,cl */
		ModRM = FETCH;
		/* tmp = GetRMWord(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.w[Mod_RM.RM.w[ModRM]];
			ICOUNT -= 4;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadWord(cpustate->ea);
			ICOUNT = old - 13;
		}
		tmp2 = cpustate->regs.b[CL] & 0xF;
		tmp |= (bytes[tmp2]);
		PutbackRMWord(ModRM, tmp);
		break;

	case 0x16:							/* 0F 16 C6 - NOT1 si,cl */
		ModRM = FETCH;
		/* need the long if due to correct cycles OB[19.07.99] */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.b[Mod_RM.RM.b[ModRM]];
			ICOUNT -= 4;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadByte(cpustate->ea);
			ICOUNT = old - 18;		/* my source says 18 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = cpustate->regs.b[CL] & 0x7;		/* hey its a Byte so &07 NOT &0f */
		if (tmp & bytes[tmp2])
			tmp &= ~(bytes[tmp2]);
		else
			tmp |= (bytes[tmp2]);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x17:							/* 0F 17 C6 - NOT1 si,cl */
		ModRM = FETCH;
		/* tmp = GetRMWord(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.w[Mod_RM.RM.w[ModRM]];
			ICOUNT -= 4;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadWord(cpustate->ea);
			ICOUNT = old - 18;		/* my source says 14 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = cpustate->regs.b[CL] & 0xF;		/* this time its a word */
		if (tmp & bytes[tmp2])
			tmp &= ~(bytes[tmp2]);
		else
			tmp |= (bytes[tmp2]);
		PutbackRMWord(ModRM, tmp);
		break;

	case 0x18:							/* 0F 18 XX - TEST1 [bx+30h],07 */
		ModRM = FETCH;
		/* tmp = GetRMByte(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.b[Mod_RM.RM.b[ModRM]];
			ICOUNT -= 4;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadByte(cpustate->ea);
			ICOUNT = old - 13;		/* my source says 15 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = FETCH;
		tmp2 &= 0xF;
		cpustate->ZeroVal = tmp & (bytes[tmp2]) ? 1 : 0;
/*      SetZF(tmp & (1<<tmp2)); */
		break;

	case 0x19:							/* 0F 19 XX - TEST1 [bx+30h],07 */
		ModRM = FETCH;
		/* tmp = GetRMWord(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.w[Mod_RM.RM.w[ModRM]];
			ICOUNT -= 4;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadWord(cpustate->ea);
			ICOUNT = old - 13;		/* my source says 14 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = FETCH;
		tmp2 &= 0xf;
		cpustate->ZeroVal = tmp & (bytes[tmp2]) ? 1 : 0;
/*      SetZF(tmp & (1<<tmp2)); */
		break;

	case 0x1a:							/* 0F 1A 06 - CLR1 si,cl */
		ModRM = FETCH;
		/* tmp = GetRMByte(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.b[Mod_RM.RM.b[ModRM]];
			ICOUNT -= 6;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadByte(cpustate->ea);
			ICOUNT = old - 15;		/* my source says 15 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = FETCH;
		tmp2 &= 0x7;
		tmp &= ~(bytes[tmp2]);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x1B:							/* 0F 1B 06 - CLR1 si,cl */
		ModRM = FETCH;
		/* tmp = GetRMWord(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.w[Mod_RM.RM.w[ModRM]];
			ICOUNT -= 6;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadWord(cpustate->ea);
			ICOUNT = old - 15;		/* my source says 15 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = FETCH;
		tmp2 &= 0xF;
		tmp &= ~(bytes[tmp2]);
		PutbackRMWord(ModRM, tmp);
		break;

	case 0x1C:							/* 0F 1C 47 30 - SET1 [bx+30h],cl */
		ModRM = FETCH;
		/* tmp = GetRMByte(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.b[Mod_RM.RM.b[ModRM]];
			ICOUNT -= 5;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadByte(cpustate->ea);
			ICOUNT = old - 14;		/* my source says 15 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = FETCH;
		tmp2 &= 0x7;
		tmp |= (bytes[tmp2]);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x1D:							/* 0F 1D C6 - SET1 si,cl */
		/* logerror("PC=%06x : Set1 ",activecpu_get_pc()-2); */
		ModRM = FETCH;
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.w[Mod_RM.RM.w[ModRM]];
			ICOUNT -= 5;
			/* logerror("reg=%04x ->",tmp); */
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);			/* calculate EA */
			tmp = ReadWord(cpustate->ea);			/* read from EA */
			ICOUNT = old - 14;
			/* logerror("[%04x]=%04x ->",EA,tmp); */
		}
		tmp2 = FETCH;
		tmp2 &= 0xF;
		tmp |= (bytes[tmp2]);
		/* logerror("%04x",tmp); */
		PutbackRMWord(ModRM, tmp);
		break;

	case 0x1e:							/* 0F 1e C6 - NOT1 si,07 */
		ModRM = FETCH;
		/* tmp = GetRMByte(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.b[Mod_RM.RM.b[ModRM]];
			ICOUNT -= 5;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadByte(cpustate->ea);
			ICOUNT = old - 19;
		}
		tmp2 = FETCH;
		tmp2 &= 0x7;
		if (tmp & bytes[tmp2])
			tmp &= ~(bytes[tmp2]);
		else
			tmp |= (bytes[tmp2]);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x1f:							/* 0F 1f C6 - NOT1 si,07 */
		ModRM = FETCH;
		//tmp = GetRMWord(ModRM);
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.w[Mod_RM.RM.w[ModRM]];
			ICOUNT -= 5;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadWord(cpustate->ea);
			ICOUNT = old - 19;		/* my source says 15 cycles everytime and not
                                         * ModRM-dependent like GetEA[] does..hmmm */
		}
		tmp2 = FETCH;
		tmp2 &= 0xF;
		if (tmp & bytes[tmp2])
			tmp &= ~(bytes[tmp2]);
		else
			tmp |= (bytes[tmp2]);
		PutbackRMWord(ModRM, tmp);
		break;

	case 0x20:							/* 0F 20 59 - add4s */
		{
			/* length in words ! */
			int count = (cpustate->regs.b[CL] + 1) / 2;
			int i;
			unsigned di = cpustate->regs.w[DI];
			unsigned si = cpustate->regs.w[SI];

			cpustate->ZeroVal = 1;
			cpustate->CarryVal = 0;				/* NOT ADC */
			for (i = 0; i < count; i++)
			{
				int v1, v2;
				int result;

				tmp = GetMemB(DS, si);
				tmp2 = GetMemB(ES, di);

				v1 = (tmp >> 4) * 10 + (tmp & 0xf);
				v2 = (tmp2 >> 4) * 10 + (tmp2 & 0xf);
				result = v1 + v2 + cpustate->CarryVal;
				cpustate->CarryVal = result > 99 ? 1 : 0;
				result = result % 100;
				v1 = ((result / 10) << 4) | (result % 10);
				PutMemB(ES, di, v1);
				if (v1)
					cpustate->ZeroVal = 0;
				si++;
				di++;
			}
			cpustate->OverVal = cpustate->CarryVal;
			ICOUNT -= 7 + 19 * count;	/* 7+19n, n #operand words */
		}
		break;

	case 0x22:							/* 0F 22 59 - sub4s */
		{
			int count = (cpustate->regs.b[CL] + 1) / 2;
			int i;
			unsigned di = cpustate->regs.w[DI];
			unsigned si = cpustate->regs.w[SI];

			cpustate->ZeroVal = 1;
			cpustate->CarryVal = 0;				/* NOT ADC */
			for (i = 0; i < count; i++)
			{
				int v1, v2;
				int result;

				tmp = GetMemB(ES, di);
				tmp2 = GetMemB(DS, si);

				v1 = (tmp >> 4) * 10 + (tmp & 0xf);
				v2 = (tmp2 >> 4) * 10 + (tmp2 & 0xf);
				if (v1 < (v2 + cpustate->CarryVal))
				{
					v1 += 100;
					result = v1 - (v2 + cpustate->CarryVal);
					cpustate->CarryVal = 1;
				}
				else
				{
					result = v1 - (v2 + cpustate->CarryVal);
					cpustate->CarryVal = 0;
				}
				v1 = ((result / 10) << 4) | (result % 10);
				PutMemB(ES, di, v1);
				if (v1)
					cpustate->ZeroVal = 0;
				si++;
				di++;
			}
			cpustate->OverVal = cpustate->CarryVal;
			ICOUNT -= 7 + 19 * count;
		}
		break;

	case 0x25:
		/*
         * ----------O-MOVSPA---------------------------------
         * OPCODE MOVSPA     -  Move Stack Pointer After Bank Switched
         *
         * CPU:  NEC V25,V35,V25 Plus,V35 Plus,V25 Software Guard
         * Type of Instruction: System
         *
         * Instruction:  MOVSPA
         *
         * Description:  This instruction transfer   both SS and SP  of the old register
         * bank to new register bank after the bank has been switched by
         * interrupt or BRKCS instruction.
         *
         * Flags Affected:   None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   MOVSPA
         * COP (Code of Operation)   : 0Fh 25h
         *
         * Clocks:   16
         */
		logerror("PC=%06x : MOVSPA\n", activecpu_get_pc() - 2);
		ICOUNT -= 16;
		break;
	case 0x26:							/* 0F 22 59 - cmp4s */
		{
			int count = (cpustate->regs.b[CL] + 1) / 2;
			int i;
			unsigned di = cpustate->regs.w[DI];
			unsigned si = cpustate->regs.w[SI];

			cpustate->ZeroVal = 1;
			cpustate->CarryVal = 0;				/* NOT ADC */
			for (i = 0; i < count; i++)
			{
				int v1, v2;
				int result;

				tmp = GetMemB(ES, di);
				tmp2 = GetMemB(DS, si);

				v1 = (tmp >> 4) * 10 + (tmp & 0xf);
				v2 = (tmp2 >> 4) * 10 + (tmp2 & 0xf);
				if (v1 < (v2 + cpustate->CarryVal))
				{
					v1 += 100;
					result = v1 - (v2 + cpustate->CarryVal);
					cpustate->CarryVal = 1;
				}
				else
				{
					result = v1 - (v2 + cpustate->CarryVal);
					cpustate->CarryVal = 0;
				}
				v1 = ((result / 10) << 4) | (result % 10);
/*              PutMemB(ES, di,v1); */	/* no store, only compare */
				if (v1)
					cpustate->ZeroVal = 0;
				si++;
				di++;
			}
			cpustate->OverVal = cpustate->CarryVal;
			ICOUNT -= 7 + 19 * (cpustate->regs.b[CL] + 1);	// 7+19n, n #operand bytes
		}
		break;
	case 0x28:							/* 0F 28 C7 - ROL4 bh */
		ModRM = FETCH;
		/* tmp = GetRMByte(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.b[Mod_RM.RM.b[ModRM]];
			ICOUNT -= 25;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadByte(cpustate->ea);
			ICOUNT = old - 28;
		}
		tmp <<= 4;
		tmp |= cpustate->regs.b[AL] & 0xF;
		cpustate->regs.b[AL] = (cpustate->regs.b[AL] & 0xF0) | ((tmp >> 8) & 0xF);
		tmp &= 0xff;
		PutbackRMByte(ModRM, tmp);
		break;

        /* Is this a REAL instruction?? */
	case 0x29:							/* 0F 29 C7 - ROL4 bx */

		ModRM = FETCH;
		/*
         * if (ModRM >= 0xc0)
         * {
         *     tmp=cpustate->regs.w[Mod_RM.RM.w[ModRM]];
         *     ICOUNT-=29;
         * }
         * else
         * {
         *     int old=ICOUNT;
         *     (*GetEA[ModRM])();
         *     tmp=ReadWord(cpustate->ea);
         *     ICOUNT=old-33;
         * }
         * tmp <<= 4;
         * tmp |= cpustate->regs.b[AL] & 0xF;
         * cpustate->regs.b[AL] = (cpustate->regs.b[AL] & 0xF0) | ((tmp>>8)&0xF);
         * tmp &= 0xffff;
         * PutbackRMWord(ModRM,tmp);
         */
		logerror("PC=%06x : ROL4 %02x\n", activecpu_get_pc() - 3, ModRM);
		break;

	case 0x2A:							/* 0F 2a c2 - ROR4 bh */
		ModRM = FETCH;
		/* tmp = GetRMByte(ModRM); */
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.b[Mod_RM.RM.b[ModRM]];
			ICOUNT -= 29;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadByte(cpustate->ea);
			ICOUNT = old - 33;
		}
		tmp2 = (cpustate->regs.b[AL] & 0xF) << 4;
		cpustate->regs.b[AL] = (cpustate->regs.b[AL] & 0xF0) | (tmp & 0xF);
		tmp = tmp2 | (tmp >> 4);
		PutbackRMByte(ModRM, tmp);
		break;

	case 0x2B:							// 0F 2b c2 - ROR4 bx
		ModRM = FETCH;
		/*
         * /* tmp = GetRMWord(ModRM);
         * if (ModRM >= 0xc0)
         * {
         *     tmp=cpustate->regs.w[Mod_RM.RM.w[ModRM]];
         *     ICOUNT-=29;
         * }
         * else {
         *     int old=ICOUNT;
         *     (*GetEA[ModRM])();
         *     tmp=ReadWord(cpustate->ea);
         *     ICOUNT=old-33;
         * }
         * tmp2 = (cpustate->regs.b[AL] & 0xF)<<4;
         * cpustate->regs.b[AL] = (cpustate->regs.b[AL] & 0xF0) | (tmp&0xF);
         * tmp = tmp2 | (tmp>>4);
         * PutbackRMWord(ModRM,tmp);
         */
		logerror("PC=%06x : ROR4 %02x\n", activecpu_get_pc() - 3, ModRM);
		break;

	case 0x2D:							/* 0Fh 2Dh <1111 1RRR> */
		/* OPCODE BRKCS  -   Break with Contex Switch
         * CPU:  NEC V25,V35,V25 Plus,V35 Plus,V25 Software Guard
         * Description:
         *
         * Perform a High-Speed Software Interrupt with contex-switch to
         * register bank indicated by the lower 3-bits of 'bank'.
         *
         * Info:    NEC V25/V35/V25 Plus/V35 Plus Bank System
         *
         * This Chips have   8 32bytes register banks, which placed in
         * Internal chip RAM by addresses:
         * xxE00h..xxE1Fh Bank 0
         * xxE20h..xxE3Fh Bank 1
         * .........
         * xxEC0h..xxEDFh Bank 6
         * xxEE0h..xxEFFh Bank 7
         * xxF00h..xxFFFh Special Functions Register
         * Where xx is Value of IDB register.
         * IBD is Byte Register contained Internal data area base
         * IBD addresses is FFFFFh and xxFFFh where xx is data in IBD.
         *
         * Format of Bank:
         * +0   Reserved
         * +2   Vector PC
         * +4   Save   PSW
         * +6   Save   PC
         * +8   DS0     ;DS
         * +A   SS      ;SS
         * +C   PS      ;CS
         * +E   DS1     ;ES
         * +10  IY      ;DI
         * +11  IX      ;SI
         * +14  BP      ;BP
         * +16  SP      ;SP
         * +18  BW      ;BX
         * +1A  DW      ;DX
         * +1C  CW      ;CX
         * +1E  AW      ;AX
         *
         * Format of V25 etc. PSW (FLAGS):
         * Bit  Description
         * 15   1
         * 14   RB2 \
         * 13   RB1  >  Current Bank Number
         * 12   RB0 /
         * 11   V   ;OF
         * 10   IYR ;DF
         * 9    IE  ;IF
         * 8    BRK ;TF
         * 7    S   ;SF
         * 6    Z   ;ZF
         * 5    F1  General Purpose user flag #1 (accessed by Flag Special Function Register)
         * 4    AC  ;AF
         * 3    F0  General purpose user flag #0 (accessed by Flag Special Function Register)
         * 2    P   ;PF
         * 1    BRKI    I/O Trap Enable Flag
         * 0    CY  ;CF
         *
         * Flags Affected:   None
         */
		ModRM = FETCH;
		logerror("PC=%06x : BRKCS %02x\n", activecpu_get_pc() - 3, ModRM);
		ICOUNT -= 15;				/* checked ! */
		break;

	case 0x31:							/* 0F 31 [mod:reg:r/m] - INS reg8,reg8 or INS reg8,imm4 */
		ModRM = FETCH;
		logerror("PC=%06x : INS ", activecpu_get_pc() - 2);
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.b[Mod_RM.RM.b[ModRM]];
			logerror("ModRM=%04x \n", ModRM);
			ICOUNT -= 29;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadByte(cpustate->ea);
			logerror("ModRM=%04x  Byte=%04x\n", EA, tmp);
			ICOUNT = old - 33;
		}

		/* more to come
         * bfl=tmp2 & 0xf;      /* bit field length
         * bfs=tmp & 0xf;       /* bit field start (bit offset in DS:SI)
         * cpustate->regs.b[AH] =0;     /* AH =0
         */

		/* 2do: the rest is silence....yet
         * ----------O-INS------------------------------------
         * OPCODE INS  -  Insert Bit String
         *
         * CPU: NEC/Sony  all V-series
         * Type of Instruction: User
         *
         * Instruction:  INS  start,len
         *
         * Description:
         *
         * BitField [        BASE =  ES:DI
         * START BIT OFFSET =  start
         * LENGTH =  len
         * ]   <-    AX [ bits= (len-1)..0]
         *
         * Note:    di and start automatically UPDATE
         * Note:    Alternative Name of this instruction is NECINS
         *
         * Flags Affected: None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form         : INS  reg8,reg8
         * COP (Code of Operation)   : 0FH 31H  PostByte
         */

		/* ICOUNT-=31; */			/* 31 -117 clocks ....*/
		break;

    case 0x33:                          /* 0F 33 [mod:reg:r/m] - EXT reg8,reg8 or EXT reg8,imm4 */
		ModRM = FETCH;
		logerror("PC=%06x : EXT ", activecpu_get_pc() - 2);
		if (ModRM >= 0xc0)
		{
			tmp = cpustate->regs.b[Mod_RM.RM.b[ModRM]];
			logerror("ModRM=%04x \n", ModRM);
			ICOUNT -= 29;
		}
		else
		{
			int old = ICOUNT;

			(void)(*GetEA[ModRM])(cpustate);
			tmp = ReadByte(cpustate->ea);
			logerror("ModRM=%04x  Byte=%04x\n", EA, tmp);
			ICOUNT = old - 33;
		}
		/* 2do: the rest is silence....yet
        /*
         * bfl=tmp2 & 0xf;      /* bit field length
         * bfs=tmp & 0xf;       /* bit field start (bit offset in DS:SI)
         * cpustate->regs.b[AH] =0;     /* AH =0
         */

		/*
         *
         * ----------O-EXT------------------------------------
         * OPCODE EXT  -  Extract Bit Field
         *
         * CPU: NEC/Sony all  V-series
         * Type of Instruction: User
         *
         * Instruction:  EXT  start,len
         *
         * Description:
         *
         * AX <- BitField [
         *     BASE =  DS:SI
         *     START BIT OFFSET =  start
         *     LENGTH =  len
         * ];
         *
         * Note:    si and start automatically UPDATE
         *
         * Flags Affected: None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form         : EXT  reg8,reg8
         * COP (Code of Operation)   : 0FH 33H  PostByte
         *
         * Clocks:      EXT  reg8,reg8
         * NEC V20: 26-55
         */

		/* NEC_ICount-=26; */			/* 26 -55 clocks ....*/
		break;

    case 0x91:
		/*
         * ----------O-RETRBI---------------------------------
         * OPCODE RETRBI     -  Return from Register Bank Context
         * Switch  Interrupt.
         *
         * CPU:  NEC V25,V35,V25 Plus,V35 Plus,V25 Software Guard
         * Type of Instruction: System
         *
         * Instruction:  RETRBI
         *
         * Description:
         *
         * PC  <- Save PC;
         * PSW <- Save PSW;
         *
         * Flags Affected:   All
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   RETRBI
         * COP (Code of Operation)   : 0Fh 91h
         *
         * Clocks:   12
         */
		logerror("PC=%06x : RETRBI\n", activecpu_get_pc() - 2);
		ICOUNT -= 12;
		break;

	case 0x94:
		/*
         * ----------O-TSKSW----------------------------------
         * OPCODE TSKSW  -    Task Switch
         *
         * CPU:  NEC V25,V35,V25 Plus,V35 Plus,V25 Software Guard
         * Type of Instruction: System
         *
         * Instruction:  TSKSW   reg16
         *
         * Description:  Perform a High-Speed task switch to the register bank indicated
         * by lower 3 bits of reg16. The PC and PSW are saved in the old
         * banks. PC and PSW save Registers and the new PC and PSW values
         * are retrived from the new register bank's save area.
         *
         * Note:         See BRKCS instruction for more Info about banks.
         *
         * Flags Affected:   All
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   TSCSW reg16
         * COP (Code of Operation)   : 0Fh 94h <1111 1RRR>
         *
         * Clocks:   11
         */
		ModRM = FETCH;

		logerror("PC=%06x : TSCSW %02x\n", activecpu_get_pc() - 3, ModRM);
		ICOUNT -= 11;
		break;

    case 0x95:
		/*
         * ----------O-MOVSPB---------------------------------
         * OPCODE MOVSPB     -  Move Stack Pointer Before Bamk Switching
         *
         * CPU:  NEC V25,V35,V25 Plus,V35 Plus,V25 Software Guard
         * Type of Instruction: System
         *
         * Instruction:  MOVSPB  Number_of_bank
         *
         * Description:  The MOVSPB instruction transfers the current SP and SS before
         * the bank switching to new register bank.
         *
         * Note:          New Register Bank Number indicated by lower 3bit of Number_of_
         * _bank.
         *
         * Note:          See BRKCS instruction for more info about banks.
         *
         * Flags Affected:   None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   MOVSPB    reg16
         * COP (Code of Operation)   : 0Fh 95h <1111 1RRR>
         *
         * Clocks:   11
         */
		ModRM = FETCH;
		logerror("PC=%06x : MOVSPB %02x\n", activecpu_get_pc() - 3, ModRM);
		ICOUNT -= 11;
		break;

    case 0xbe:
		/*
         * ----------O-STOP-----------------------------------
         * OPCODE STOP    -  Stop CPU
         *
         * CPU:  NEC V25,V35,V25 Plus,V35 Plus,V25 Software Guard
         * Type of Instruction: System
         *
         * Instruction:  STOP
         *
         * Description:
         * PowerDown instruction, Stop Oscillator,
         * Halt CPU.
         *
         * Flags Affected:   None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   STOP
         * COP (Code of Operation)   : 0Fh BEh
         *
         * Clocks:   N/A
         */
		logerror("PC=%06x : STOP\n", activecpu_get_pc() - 2);
		ICOUNT -= 2;				/* of course this is crap */
		break;

    case 0xe0:
		/*
         * ----------O-BRKXA----------------------------------
         * OPCODE BRKXA   -  Break to Expansion Address
         *
         * CPU:  NEC V33/V53  only
         * Type of Instruction: System
         *
         * Instruction:  BRKXA int_vector
         *
         * Description:
         * [sp-1,sp-2] <- PSW       ; PSW EQU FLAGS
         * [sp-3,sp-4] <- PS        ; PS  EQU CS
         * [sp-5,sp-6] <- PC        ; PC  EQU IP
         * SP    <-  SP -6
         * IE    <-  0
         * BRK <-  0
         * MD    <-  0
         * PC    <- [int_vector*4 +0,+1]
         * PS    <- [int_vector*4 +2,+3]
         * Enter Expansion Address Mode.
         *
         * Note:    In NEC V53 Memory Space dividing into 1024 16K pages.
         * The programming model is Same as in Normal mode.
         *
         * Mechanism is:
         * 20 bit Logical Address:   19..14 Page Num  13..0 Offset
         *
         * page Num convertin by internal table to 23..14 Page Base
         * tHE pHYIXCAL ADDRESS is both Base and Offset.
         *
         * Address Expansion Registers:
         * logical Address A19..A14 I/O Address
         * 0                FF00h
         * 1                FF02h
         * ...              ...
         * 63               FF7Eh
         *
         * Register XAM aliased with port # FF80h indicated current mode
         * of operation.
         * Format of XAM register (READ ONLY):
         * 15..1    reserved
         * 0    XA Flag, if=1 then in XA mode.
         *
         * Format   of  V53 PSW:
         * 15..12   1
         * 11   V
         * 10   IYR
         * 9    IE
         * 8    BRK
         * 7    S
         * 6    Z
         * 5    0
         * 4    AC
         * 3    0
         * 2    P
         * 1    1
         * 0    CY
         *
         * Flags Affected:   None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   BRKXA  imm8
         * COP (Code of Operation)   : 0Fh E0h imm8
         */

		ModRM = FETCH;
		logerror("PC=%06x : BRKXA %02x\n", activecpu_get_pc() - 3, ModRM);
		ICOUNT -= 12;
		break;

    case 0xf0:
		/*
         * ----------O-RETXA----------------------------------
         * OPCODE RETXA   -  Return from  Expansion Address
         *
         * CPU:  NEC V33/V53 only
         * Type of Instruction: System
         *
         * Instruction:  RETXA int_vector
         *
         * Description:
         * [sp-1,sp-2] <- PSW       ; PSW EQU FLAGS
         * [sp-3,sp-4] <- PS        ; PS  EQU CS
         * [sp-5,sp-6] <- PC        ; PC  EQU IP
         * SP    <-  SP -6
         * IE    <-  0
         * BRK <-  0
         * MD    <-  0
         * PC    <- [int_vector*4 +0,+1]
         * PS    <- [int_vector*4 +2,+3]
         * Disable EA mode.
         *
         * Flags Affected:   None
         *
         * CPU mode: RM
         *
         * +++++++++++++++++++++++
         * Physical Form:   RETXA  imm8
         * COP (Code of Operation)   : 0Fh F0h imm8
         *
         * Clocks:   12
         */
		ModRM = FETCH;
		logerror("PC=%06x : RETXA %02x\n", activecpu_get_pc() - 3, ModRM);
		ICOUNT -= 12;
		break;

    case 0xff:                          /* 0F ff imm8 - BRKEM */
		/*
         * OPCODE BRKEM  -   Break for Emulation
         *
         * CPU: NEC/Sony V20/V30/V40/V50
         * Description:
         *
         * PUSH FLAGS
         * PUSH CS
         * PUSH IP
         * MOV  CS,0:[intnum*4+2]
         * MOV  IP,0:[intnum*4]
         * MD <- 0; // Enable 8080 emulation
         *
         * Note:
         * BRKEM instruction do software interrupt and then New CS,IP loaded
         * it switch to 8080 mode i.e. CPU will execute 8080 code.
         * Mapping Table of Registers in 8080 Mode
         * 8080 Md.   A  B   C  D  E  H  L  SP PC  F
         * native.     AL CH CL DH DL BH BL BP IP  FLAGS(low)
         * For Return of 8080 mode use CALLN instruction.
         * Note:    I.e. 8080 addressing only 64KB then "Real Address" is CS*16+PC
         *
         * Flags Affected: MD
         */
		ModRM = FETCH;
		ICOUNT -= 38;
		logerror("PC=%06x : BRKEM %02x\n", activecpu_get_pc() - 3, ModRM);
		PREFIX86(_interrupt)(cpustate, ModRM);
		break;
	}
}

void IX86_OPS_BASE::PREFIXV30(_brkn)()		/* Opcode 0x63 BRKN -  Break to Native Mode */
{
	/*
     * CPU:  NEC (V25/V35) Software Guard only
     * Instruction:  BRKN int_vector
     *
     * Description:
     * [sp-1,sp-2] <- PSW       ; PSW EQU FLAGS
     * [sp-3,sp-4] <- PS        ; PS  EQU CS
     * [sp-5,sp-6] <- PC        ; PC  EQU IP
     * SP    <-  SP -6
     * IE    <-  0
     * BRK <-  0
     * MD    <-  1
     * PC    <- [int_vector*4 +0,+1]
     * PS    <- [int_vector*4 +2,+3]
     *
     * Note:    The BRKN instruction switches operations in Native Mode
     * from Security Mode via Interrupt call. In Normal Mode
     * Instruction executed as   mPD70320/70322 (V25) operation mode.
     *
     * Flags Affected:   None
     *
     * CPU mode: RM
     *
     * +++++++++++++++++++++++
     * Physical Form:   BRKN  imm8
     * COP (Code of Operation)   : 63h imm8
     *
     * Clocks:   56+10T [44+10T]
     */
	/* ICOUNT-=56; */
	unsigned int_vector;

	int_vector = FETCH;
	logerror("PC=%06x : BRKN %02x\n", activecpu_get_pc() - 2, int_vector);
}

void IX86_OPS_BASE::PREFIXV30(repc)(int flagval)
{
	/* Handles repc- and repnc- prefixes. flagval is the value of ZF
     * for the loop to continue for CMPS and SCAS instructions.
     */

	unsigned next = FETCHOP;
	unsigned count = cpustate->regs.w[CX];

	switch (next)
	{
	case 0x26:							/* ES: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = ES;
		ICOUNT -= 2;
		PREFIXV30(repc)(cpustate, flagval);
		break;
	case 0x2e:							/* CS: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = CS;
		ICOUNT -= 2;
		PREFIXV30(repc)(cpustate, flagval);
		break;
	case 0x36:							/* SS: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = SS;
		ICOUNT -= 2;
		PREFIXV30(repc)(cpustate, flagval);
		break;
	case 0x3e:							/* DS: */
		cpustate->seg_prefix = TRUE;
		cpustate->prefix_seg = DS;
		ICOUNT -= 2;
		PREFIXV30(repc)(cpustate, flagval);
		break;
	case 0x6c:							/* REP INSB */
		ICOUNT -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX186(_insb)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	case 0x6d:							/* REP INSW */
		ICOUNT -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX186(_insw)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	case 0x6e:							/* REP OUTSB */
		ICOUNT -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX186(_outsb)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	case 0x6f:							/* REP OUTSW */
		ICOUNT -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX186(_outsw)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	case 0xa4:							/* REP MOVSB */
		ICOUNT -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX86(_movsb)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	case 0xa5:							/* REP MOVSW */
		ICOUNT -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX86(_movsw)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	case 0xa6:							/* REP(N)E CMPSB */
		ICOUNT -= 9;
		for (cpustate->ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--)
			PREFIX86(_cmpsb)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	case 0xa7:							/* REP(N)E CMPSW */
		ICOUNT -= 9;
		for (cpustate->ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--)
			PREFIX86(_cmpsw)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	case 0xaa:							/* REP STOSB */
		ICOUNT -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX86(_stosb)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	case 0xab:							/* REP STOSW */
		ICOUNT -= 9 - count;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX86(_stosw)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	case 0xac:							/* REP LODSB */
		ICOUNT -= 9;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX86(_lodsb)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	case 0xad:							/* REP LODSW */
		ICOUNT -= 9;
		for (; (CF == flagval) && (count > 0); count--)
			PREFIX86(_lodsw)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	case 0xae:							/* REP(N)E SCASB */
		ICOUNT -= 9;
		for (cpustate->ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--)
			PREFIX86(_scasb)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	case 0xaf:							/* REP(N)E SCASW */
		ICOUNT -= 9;
		for (cpustate->ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--)
			PREFIX86(_scasw)(cpustate);
		cpustate->regs.w[CX] = count;
		break;
	default:
		PREFIXV30(_instruction)[next](cpustate);
	}
}

void IX86_OPS_BASE::PREFIXV30(_repnc)()	/* Opcode 0x64 */
{
	PREFIXV30(repc)(cpustate, 0);
}

void IX86_OPS_BASE::PREFIXV30(_repc)()		/* Opcode 0x65 */
{
	PREFIXV30(repc)(cpustate, 1);
}

void IX86_OPS_BASE::PREFIXV30(_aad)()    /* Opcode 0xd5 */
{
	unsigned mult = FETCH;

	ICOUNT -= timing.aad;

//	cpustate->regs.b[AL] = cpustate->regs.b[AH] * mult + cpustate->regs.b[AL];
	cpustate->regs.b[AL] = cpustate->regs.b[AH] * 10 + cpustate->regs.b[AL];
	cpustate->regs.b[AH] = 0;

	SetZF(cpustate->regs.b[AL]);
	SetPF(cpustate->regs.b[AL]);
	cpustate->SignVal = 0;
}

void IX86_OPS_BASE::PREFIXV30(_setalc)()	/* Opcode 0xd6 */
{
	/*
     * ----------O-SETALC---------------------------------
     * OPCODE SETALC  - Set AL to Carry Flag
     *
     * CPU:  Intel 8086 and all its clones and upward
     * compatibility chips.
     * Type of Instruction: User
     *
     * Instruction: SETALC
     *
     * Description:
     *
     * IF (CF=0) THEN AL:=0 ELSE AL:=FFH;
     *
     * Flags Affected: None
     *
     * CPU mode: RM,PM,VM,SMM
     *
     * Physical Form:        SETALC
     * COP (Code of Operation): D6H
     * Clocks:        80286    : n/a   [3]
     * 80386    : n/a   [3]
     * Cx486SLC  : n/a   [2]
     * i486     : n/a   [3]
     * Pentium  : n/a   [3]
     * Note: n/a is Time that Intel etc not say.
     * [3] is real time it executed.
     *
     */
	cpustate->regs.b[AL] = (CF) ? 0xff : 0x00;
	ICOUNT -= 3;					// V30
	logerror("PC=%06x : SETALC\n", activecpu_get_pc() - 1);
}

#if 0
void IX86_OPS_BASE::PREFIXV30(_brks)()		/* Opcode 0xf1 - Break to Security Mode */
{
	/*
     * CPU:  NEC (V25/V35) Software Guard  only
     * Instruction:  BRKS int_vector
     *
     * Description:
     * [sp-1,sp-2] <- PSW       ; PSW EQU FLAGS
     * [sp-3,sp-4] <- PS        ; PS  EQU CS
     * [sp-5,sp-6] <- PC        ; PC  EQU IP
     * SP    <-  SP -6
     * IE    <-  0
     * BRK <-  0
     * MD    <-  0
     * PC    <- [int_vector*4 +0,+1]
     * PS    <- [int_vector*4 +2,+3]
     *
     * Note:    The BRKS instruction switches operations in Security Mode
     * via Interrupt call. In Security Mode the fetched operation
     * code is executed after conversion in accordance with build-in
     * translation table
     *
     * Flags Affected:   None
     *
     * CPU mode: RM
     *
     * +++++++++++++++++++++++
     * Physical Form:   BRKS  imm8
     * Clocks:   56+10T [44+10T]
     */
	unsigned int_vector;

	int_vector = FETCH;
	logerror("PC=%06x : BRKS %02x\n", activecpu_get_pc() - 2, int_vector);
}
#endif

void IX86_OPS::PREFIX(_pop_ss)()    /* Opcode 0x17 */
{
	POP(cpustate->sregs[SS]);
	cpustate->base[SS] = SegBase(SS);

	ICOUNT -= timing.pop_seg;
	PREFIX(_instruction)[FETCHOP](cpustate); /* no interrupt before next instruction */
}

void IX86_OPS::PREFIX(_mov_sregw)()    /* Opcode 0x8e */
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

void IX86_OPS::PREFIX(_sti)()    /* Opcode 0xfb */
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

void IX86_OPS_BASE::PREFIX(_inc_bx)()    /* Opcode 0x43 */
{
	IncWordReg(BX);
}

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
	PREFIX(rep)(0);
}

void IX86_OPS_BASE::PREFIX(_repe)()    /* Opcode 0xf3 */
{
	PREFIX(rep)(1);
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
