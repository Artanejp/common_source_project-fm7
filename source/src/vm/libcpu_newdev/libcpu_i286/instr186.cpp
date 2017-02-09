/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/

// file will be included in all cpu variants
// function renaming will be added when necessary
// timing value should move to separate array

#include "./ix86_opdef.h"

#undef ICOUNT

#define ICOUNT cpustate->icount
#define PREFIX(XXX) PREFIX86(XXXX)

void IX86_OPS_BASE::PREFIX186(_interrupt)(unsigned int_num)
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

void IX86_OPS_BASE::PREFIX186(_pusha)()    /* Opcode 0x60 */
{
	unsigned tmp=cpustate->regs.w[SP];

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

static unsigned i186_popa_tmp;  // hack around GCC 4.6 error because we need the side effects of POP
void IX86_OPS_BASE::PREFIX186(_popa)()    /* Opcode 0x61 */
{

	ICOUNT -= timing.popa;
	POP(cpustate->regs.w[DI]);
	POP(cpustate->regs.w[SI]);
	POP(cpustate->regs.w[BP]);
	POP(i186_popa_tmp);
	POP(cpustate->regs.w[BX]);
	POP(cpustate->regs.w[DX]);
	POP(cpustate->regs.w[CX]);
	POP(cpustate->regs.w[AX]);
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

void IX86_OPS_BASE::PREFIX186(_push_d16)()    /* Opcode 0x68 */
{
	unsigned tmp = FETCH;

	ICOUNT -= timing.push_imm;
	tmp += FETCH << 8;
	PUSH(tmp);
}

void IX86_OPS_BASE::PREFIX186(_imul_d16)()    /* Opcode 0x69 */
{
	DEF_r16w(dst,src);
	unsigned src2=FETCH;
	src2+=(FETCH<<8);

	ICOUNT -= (ModRM >= 0xc0) ? timing.imul_rri16 : timing.imul_rmi16;

	dst = (INT32)((INT16)src)*(INT32)((INT16)src2);
	cpustate->CarryVal = cpustate->OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1);
	RegWord(ModRM)=(WORD)dst;
}


void IX86_OPS_BASE::PREFIX186(_push_d8)()    /* Opcode 0x6a */
{
	unsigned tmp = (WORD)((INT16)((INT8)FETCH));

	ICOUNT -= timing.push_imm;
	PUSH(tmp);
}

void IX86_OPS_BASE::PREFIX186(_imul_d8)()    /* Opcode 0x6b */
{
	DEF_r16w(dst,src);
	unsigned src2= (WORD)((INT16)((INT8)FETCH));

	ICOUNT -= (ModRM >= 0xc0) ? timing.imul_rri8 : timing.imul_rmi8;

	dst = (INT32)((INT16)src)*(INT32)((INT16)src2);
	cpustate->CarryVal = cpustate->OverVal = (((INT32)dst) >> 15 != 0) && (((INT32)dst) >> 15 != -1);
	RegWord(ModRM)=(WORD)dst;
}

void IX86_OPS_BASE::PREFIX186(_insb)()    /* Opcode 0x6c */
{
	ICOUNT -= timing.ins8;
	PutMemB(ES,cpustate->regs.w[DI],read_port_byte(cpustate->regs.w[DX]));
	cpustate->regs.w[DI] += cpustate->DirVal;
}

void IX86_OPS_BASE::PREFIX186(_insw)()    /* Opcode 0x6d */
{
	ICOUNT -= timing.ins16;
	PutMemW(ES,cpustate->regs.w[DI],read_port_word(cpustate->regs.w[DX]));
	cpustate->regs.w[DI] += 2 * cpustate->DirVal;
}

void IX86_OPS_BASE::PREFIX186(_outsb)()    /* Opcode 0x6e */
{
	ICOUNT -= timing.outs8;
	write_port_byte(cpustate->regs.w[DX],GetMemB(DS,cpustate->regs.w[SI]));
	cpustate->regs.w[SI] += cpustate->DirVal; /* GOL 11/27/01 */
}

void IX86_OPS_BASE::PREFIX186(_outsw)()    /* Opcode 0x6f */
{
	ICOUNT -= timing.outs16;
	write_port_word(cpustate->regs.w[DX],GetMemW(DS,cpustate->regs.w[SI]));
	cpustate->regs.w[SI] += 2 * cpustate->DirVal; /* GOL 11/27/01 */
}

void IX86_OPS_BASE::PREFIX186(_rotshft_bd8)()    /* Opcode 0xc0 */
{
	unsigned ModRM = FETCH;
	unsigned src = GetRMByte(ModRM);
	unsigned count = FETCH;

	PREFIX86(_rotate_shift_Byte)(cpustate,ModRM,count & 0x1f,src);
}

void IX86_OPS_BASE::PREFIX186(_rotshft_wd8)()    /* Opcode 0xc1 */
{
	unsigned ModRM = FETCH;
	unsigned src = GetRMWord(ModRM);
	unsigned count = FETCH;

	PREFIX86(_rotate_shift_Word)(cpustate,ModRM,count & 0x1f,src);
}

void IX86_OPS_BASE::PREFIX186(_enter)()    /* Opcode 0xc8 */
{
	unsigned nb = FETCH;
	unsigned i,level;
	UINT16 fp;

	nb += FETCH << 8;
	level = FETCH;
	ICOUNT -= (level == 0) ? timing.enter0 : (level == 1) ? timing.enter1 : timing.enter_base + level * timing.enter_count;
	PUSH(cpustate->regs.w[BP]);
	fp = cpustate->regs.w[SP];
	for (i=1;i<level;i++)
		PUSH(GetMemW(SS,cpustate->regs.w[BP]-i*2));
	if (level) PUSH(fp);
	cpustate->regs.w[BP] = fp;
	cpustate->regs.w[SP] -= nb;
}

void IX86_OPS_BASE::PREFIX186(_leave)()    /* Opcode 0xc9 */
{
	ICOUNT -= timing.leave;
	cpustate->regs.w[SP]=cpustate->regs.w[BP];
	POP(cpustate->regs.w[BP]);
}

void IX86_OPS_BASE::PREFIX186(_rotshft_bcl)()    /* Opcode 0xd2 */
{
	unsigned ModRM = FETCHOP;
	PREFIX86(_rotate_shift_Byte)(cpustate,ModRM,cpustate->regs.b[CL] & 0x1f,GetRMByte(ModRM));
}

void IX86_OPS_BASE::PREFIX186(_rotshft_wcl)()    /* Opcode 0xd3 */
{
	unsigned ModRM = FETCHOP;
	PREFIX86(_rotate_shift_Word)(cpustate,ModRM,cpustate->regs.b[CL] & 0x1f,GetRMWord(ModRM));
}

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
		PREFIX(_instruction)[FETCHOP]();
		break;
	case 0x08:  /* mov cs,ew */
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
