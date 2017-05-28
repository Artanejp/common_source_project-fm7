// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philp Bennett
#include "./i386_opdef.h"

#define FAULT(fault,error) {cpustate->ext = 1; i386_trap_with_error(fault,0,0,error); return;}
#define FAULT_EXP(fault,error) {cpustate->ext = 1; i386_trap_with_error(fault,0,trap_level+1,error); return;}

UINT8 I386_OPS_BASE::I386OP(shift_rotate8)( UINT8 modrm, UINT32 value, UINT8 shift)
{
	UINT32 src = value & 0xff;
	UINT8 dst = value;

	if( shift == 0 ) {
		CYCLES_RM(modrm, 3, 7);
	} else if( shift == 1 ) {
		switch( (modrm >> 3) & 0x7 )
		{
			case 0:         /* ROL rm8, 1 */
				cpustate->CF = (src & 0x80) ? 1 : 0;
				dst = (src << 1) + cpustate->CF;
				cpustate->OF = ((src ^ dst) & 0x80) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:         /* ROR rm8, 1 */
				cpustate->CF = (src & 0x1) ? 1 : 0;
				dst = (cpustate->CF << 7) | (src >> 1);
				cpustate->OF = ((src ^ dst) & 0x80) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:         /* RCL rm8, 1 */
				dst = (src << 1) + cpustate->CF;
				cpustate->CF = (src & 0x80) ? 1 : 0;
				cpustate->OF = ((src ^ dst) & 0x80) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:         /* RCR rm8, 1 */
				dst = (cpustate->CF << 7) | (src >> 1);
				cpustate->CF = src & 0x1;
				cpustate->OF = ((src ^ dst) & 0x80) ? 1 : 0;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:         /* SHL/SAL rm8, 1 */
			case 6:
				dst = src << 1;
				cpustate->CF = (src & 0x80) ? 1 : 0;
				cpustate->OF = (((cpustate->CF << 7) ^ dst) & 0x80) ? 1 : 0;
				SetSZPF8(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:         /* SHR rm8, 1 */
				dst = src >> 1;
				cpustate->CF = src & 0x1;
				cpustate->OF = (dst & 0x80) ? 1 : 0;
				SetSZPF8(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:         /* SAR rm8, 1 */
				dst = (INT8)(src) >> 1;
				cpustate->CF = src & 0x1;
				cpustate->OF = 0;
				SetSZPF8(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	} else {
		switch( (modrm >> 3) & 0x7 )
		{
			case 0:         /* ROL rm8, i8 */
				if(!(shift & 7))
				{
					if(shift & 0x18)
					{
						cpustate->CF = src & 1;
						cpustate->OF = (src & 1) ^ ((src >> 7) & 1);
					}
					break;
				}
				shift &= 7;
				dst = ((src & ((UINT8)0xff >> shift)) << shift) |
						((src & ((UINT8)0xff << (8-shift))) >> (8-shift));
				cpustate->CF = dst & 0x1;
				cpustate->OF = (dst & 1) ^ (dst >> 7);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:         /* ROR rm8, i8 */
				if(!(shift & 7))
				{
					if(shift & 0x18)
					{
						cpustate->CF = (src >> 7) & 1;
						cpustate->OF = ((src >> 7) & 1) ^ ((src >> 6) & 1);
					}
					break;
				}
				shift &= 7;
				dst = ((src & ((UINT8)0xff << shift)) >> shift) |
						((src & ((UINT8)0xff >> (8-shift))) << (8-shift));
				cpustate->CF = (dst >> 7) & 1;
				cpustate->OF = ((dst >> 7) ^ (dst >> 6)) & 1;
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:         /* RCL rm8, i8 */
				shift %= 9;
				dst = ((src & ((UINT8)0xff >> shift)) << shift) |
						((src & ((UINT8)0xff << (9-shift))) >> (9-shift)) |
						(cpustate->CF << (shift-1));
				if(shift) cpustate->CF = (src >> (8-shift)) & 0x1;
				cpustate->OF = cpustate->CF ^ ((dst >> 7) & 1);
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:         /* RCR rm8, i8 */
				shift %= 9;
				dst = ((src & ((UINT8)0xff << shift)) >> shift) |
						((src & ((UINT8)0xff >> (8-shift))) << (9-shift)) |
						(cpustate->CF << (8-shift));
				if(shift) cpustate->CF = (src >> (shift-1)) & 0x1;
				cpustate->OF = ((dst >> 7) ^ (dst >> 6)) & 1;
				CYCLES_RM(modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:         /* SHL/SAL rm8, i8 */
			case 6:
				shift &= 31;
				dst = src << shift;
				cpustate->CF = (shift <= 8) && ((src >> (8 - shift)) & 1);
				SetSZPF8(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:         /* SHR rm8, i8 */
				shift &= 31;
				dst = src >> shift;
				cpustate->CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF8(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:         /* SAR rm8, i8 */
				shift &= 31;
				dst = (INT8)src >> shift;
				cpustate->CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF8(dst);
				CYCLES_RM(modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}
	}

	return dst;
}



void I386_OPS_BASE::I386OP(adc_rm8_r8)()        // Opcode 0x10
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = ADC8( dst, src, cpustate->CF);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = ADC8( dst, src, cpustate->CF);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void I386_OPS_BASE::I386OP(adc_r8_rm8)()        // Opcode 0x12
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = ADC8( dst, src, cpustate->CF);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		dst = ADC8( dst, src, cpustate->CF);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void I386_OPS_BASE::I386OP(adc_al_i8)()     // Opcode 0x14
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(AL);
	dst = ADC8( dst, src, cpustate->CF);
	REG8(AL) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void I386_OPS_BASE::I386OP(add_rm8_r8)()        // Opcode 0x00
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = ADD8(dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = ADD8(dst, src);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void I386_OPS_BASE::I386OP(add_r8_rm8)()        // Opcode 0x02
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = ADD8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		dst = ADD8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void I386_OPS_BASE::I386OP(add_al_i8)()     // Opcode 0x04
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(AL);
	dst = ADD8(dst, src);
	REG8(AL) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void I386_OPS_BASE::I386OP(and_rm8_r8)()        // Opcode 0x20
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = AND8(dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = AND8(dst, src);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void I386_OPS_BASE::I386OP(and_r8_rm8)()        // Opcode 0x22
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = AND8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		dst = AND8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void I386_OPS_BASE::I386OP(and_al_i8)()         // Opcode 0x24
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(AL);
	dst = AND8(dst, src);
	REG8(AL) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void I386_OPS_BASE::I386OP(clc)()               // Opcode 0xf8
{
	cpustate->CF = 0;
	CYCLES(CYCLES_CLC);
}

void I386_OPS_BASE::I386OP(cld)()               // Opcode 0xfc
{
	cpustate->DF = 0;
	CYCLES(CYCLES_CLD);
}

void I386_OPS_BASE::I386OP(cli)()               // Opcode 0xfa
{
	if(PROTECTED_MODE)
	{
		UINT8 IOPL = cpustate->IOP1 | (cpustate->IOP2 << 1);
		if(cpustate->CPL > IOPL)
			FAULT(FAULT_GP,0);
	}
	cpustate->IF = 0;
	CYCLES(CYCLES_CLI);
}

void I386_OPS_BASE::I386OP(cmc)()               // Opcode 0xf5
{
	cpustate->CF ^= 1;
	CYCLES(CYCLES_CMC);
}

void I386_OPS_BASE::I386OP(cmp_rm8_r8)()        // Opcode 0x38
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		SUB8(dst, src);
		CYCLES(CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		SUB8(dst, src);
		CYCLES(CYCLES_CMP_REG_MEM);
	}
}

void I386_OPS_BASE::I386OP(cmp_r8_rm8)()        // Opcode 0x3a
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		SUB8(dst, src);
		CYCLES(CYCLES_CMP_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		SUB8(dst, src);
		CYCLES(CYCLES_CMP_MEM_REG);
	}
}

void I386_OPS_BASE::I386OP(cmp_al_i8)()         // Opcode 0x3c
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(AL);
	SUB8(dst, src);
	CYCLES(CYCLES_CMP_IMM_ACC);
}

void I386_OPS_BASE::I386OP(cmpsb)()             // Opcode 0xa6
{
	UINT32 eas, ead;
	UINT8 src, dst;
	if( cpustate->segment_prefix ) {
		eas = i386_translate( cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate( DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	ead = i386_translate( ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 0 );
	src = READ8(eas);
	dst = READ8(ead);
	SUB8(src, dst);
	BUMP_SI(1);
	BUMP_DI(1);
	CYCLES(CYCLES_CMPS);
}

void I386_OPS_BASE::I386OP(in_al_i8)()          // Opcode 0xe4
{
	UINT16 port = FETCH();
	UINT8 data = READPORT8( port);
	REG8(AL) = data;
	CYCLES(CYCLES_IN_VAR);
}

void I386_OPS_BASE::I386OP(in_al_dx)()          // Opcode 0xec
{
	UINT16 port = REG16(DX);
	UINT8 data = READPORT8( port);
	REG8(AL) = data;
	CYCLES(CYCLES_IN);
}

void I386_OPS_BASE::I386OP(ja_rel8)()           // Opcode 0x77
{
	INT8 disp = FETCH();
	if( cpustate->CF == 0 && cpustate->ZF == 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jbe_rel8)()          // Opcode 0x76
{
	INT8 disp = FETCH();
	if( cpustate->CF != 0 || cpustate->ZF != 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jc_rel8)()           // Opcode 0x72
{
	INT8 disp = FETCH();
	if( cpustate->CF != 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jg_rel8)()           // Opcode 0x7f
{
	INT8 disp = FETCH();
	if( cpustate->ZF == 0 && (cpustate->SF == cpustate->OF) ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jge_rel8)()          // Opcode 0x7d
{
	INT8 disp = FETCH();
	if(cpustate->SF == cpustate->OF) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jl_rel8)()           // Opcode 0x7c
{
	INT8 disp = FETCH();
	if( (cpustate->SF != cpustate->OF) ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jle_rel8)()      // Opcode 0x7e
{
	INT8 disp = FETCH();
	if( cpustate->ZF != 0 || (cpustate->SF != cpustate->OF) ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jnc_rel8)()          // Opcode 0x73
{
	INT8 disp = FETCH();
	if( cpustate->CF == 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jno_rel8)()          // Opcode 0x71
{
	INT8 disp = FETCH();
	if( cpustate->OF == 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jnp_rel8)()          // Opcode 0x7b
{
	INT8 disp = FETCH();
	if( cpustate->PF == 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jns_rel8)()          // Opcode 0x79
{
	INT8 disp = FETCH();
	if( cpustate->SF == 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jnz_rel8)()          // Opcode 0x75
{
	INT8 disp = FETCH();
	if( cpustate->ZF == 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jo_rel8)()           // Opcode 0x70
{
	INT8 disp = FETCH();
	if( cpustate->OF != 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jp_rel8)()           // Opcode 0x7a
{
	INT8 disp = FETCH();
	if( cpustate->PF != 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(js_rel8)()           // Opcode 0x78
{
	INT8 disp = FETCH();
	if( cpustate->SF != 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jz_rel8)()           // Opcode 0x74
{
	INT8 disp = FETCH();
	if( cpustate->ZF != 0 ) {
		NEAR_BRANCH(disp);
		CYCLES(CYCLES_JCC_DISP8);      /* TODO: Timing = 7 + m */
	} else {
		CYCLES(CYCLES_JCC_DISP8_NOBRANCH);
	}
}

void I386_OPS_BASE::I386OP(jmp_rel8)()          // Opcode 0xeb
{
	INT8 disp = FETCH();
	NEAR_BRANCH(disp);
	CYCLES(CYCLES_JMP_SHORT);      /* TODO: Timing = 7 + m */
}

void I386_OPS_BASE::I386OP(lahf)()              // Opcode 0x9f
{
	REG8(AH) = get_flags() & 0xd7;
	CYCLES(CYCLES_LAHF);
}

void I386_OPS_BASE::I386OP(lodsb)()             // Opcode 0xac
{
	UINT32 eas;
	if( cpustate->segment_prefix ) {
		eas = i386_translate( cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate( DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	REG8(AL) = READ8(eas);
	BUMP_SI(1);
	CYCLES(CYCLES_LODS);
}

void I386_OPS_BASE::I386OP(mov_rm8_r8)()        // Opcode 0x88
{
	UINT8 src;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		STORE_RM8(modrm, src);
		CYCLES(CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG8(modrm);
		WRITE8(ea, src);
		CYCLES(CYCLES_MOV_REG_MEM);
	}
}

void I386_OPS_BASE::I386OP(mov_r8_rm8)()        // Opcode 0x8a
{
	UINT8 src;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		STORE_REG8(modrm, src);
		CYCLES(CYCLES_MOV_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ8(ea);
		STORE_REG8(modrm, src);
		CYCLES(CYCLES_MOV_MEM_REG);
	}
}

void I386_OPS_BASE::I386OP(mov_rm8_i8)()        // Opcode 0xc6
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 value = FETCH();
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_MOV_IMM_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		UINT8 value = FETCH();
		WRITE8(ea, value);
		CYCLES(CYCLES_MOV_IMM_MEM);
	}
}

void I386_OPS_BASE::I386OP(mov_r32_cr)()        // Opcode 0x0f 20
{
	if(PROTECTED_MODE && cpustate->CPL)
		FAULT(FAULT_GP, 0);
	UINT8 modrm = FETCH();
	UINT8 cr = (modrm >> 3) & 0x7;

	STORE_RM32(modrm, cpustate->cr[cr]);
	CYCLES(CYCLES_MOV_CR_REG);
}

void I386_OPS_BASE::I386OP(mov_r32_dr)()        // Opcode 0x0f 21
{
	if(PROTECTED_MODE && cpustate->CPL)
		FAULT(FAULT_GP, 0);
	UINT8 modrm = FETCH();
	UINT8 dr = (modrm >> 3) & 0x7;

	STORE_RM32(modrm, cpustate->dr[dr]);
	switch(dr)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			CYCLES(CYCLES_MOV_REG_DR0_3);
			break;
		case 6:
		case 7:
			CYCLES(CYCLES_MOV_REG_DR6_7);
			break;
	}
}

void I386_OPS_BASE::I386OP(mov_cr_r32)()        // Opcode 0x0f 22
{
	if(PROTECTED_MODE && cpustate->CPL)
		FAULT(FAULT_GP, 0);
	UINT8 modrm = FETCH();
	UINT8 cr = (modrm >> 3) & 0x7;
	UINT32 data = LOAD_RM32(modrm);
	switch(cr)
	{
		case 0:
			data &= 0xfffeffff; // wp not supported on 386
			CYCLES(CYCLES_MOV_REG_CR0);
			break;
		case 2: CYCLES(CYCLES_MOV_REG_CR2); break;
		case 3:
			CYCLES(CYCLES_MOV_REG_CR3);
			vtlb_flush_dynamic(cpustate->vtlb);
			break;
		case 4: CYCLES(1); break; // TODO
		default:
			logerror("i386: mov_cr_r32 CR%d!\n", cr);
			return;
	}
	cpustate->cr[cr] = data;
}

void I386_OPS_BASE::I386OP(mov_dr_r32)()        // Opcode 0x0f 23
{
	if(PROTECTED_MODE && cpustate->CPL)
		FAULT(FAULT_GP, 0);
	UINT8 modrm = FETCH();
	UINT8 dr = (modrm >> 3) & 0x7;

	cpustate->dr[dr] = LOAD_RM32(modrm);
	switch(dr)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			CYCLES(CYCLES_MOV_DR0_3_REG);
			break;
		case 6:
		case 7:
			CYCLES(CYCLES_MOV_DR6_7_REG);
			break;
		default:
			logerror("i386: mov_dr_r32 DR%d!\n", dr);
			return;
	}
}

void I386_OPS_BASE::I386OP(mov_al_m8)()         // Opcode 0xa0
{
	UINT32 offset, ea;
	if( cpustate->address_size ) {
		offset = FETCH32();
	} else {
		offset = FETCH16();
	}
	/* TODO: Not sure if this is correct... */
	if( cpustate->segment_prefix ) {
		ea = i386_translate( cpustate->segment_override, offset, 0 );
	} else {
		ea = i386_translate( DS, offset, 0 );
	}
	REG8(AL) = READ8(ea);
	CYCLES(CYCLES_MOV_IMM_MEM);
}

void I386_OPS_BASE::I386OP(mov_m8_al)()         // Opcode 0xa2
{
	UINT32 offset, ea;
	if( cpustate->address_size ) {
		offset = FETCH32();
	} else {
		offset = FETCH16();
	}
	/* TODO: Not sure if this is correct... */
	if( cpustate->segment_prefix ) {
		ea = i386_translate( cpustate->segment_override, offset, 1 );
	} else {
		ea = i386_translate( DS, offset, 1 );
	}
	WRITE8( ea, REG8(AL) );
	CYCLES(CYCLES_MOV_MEM_ACC);
}

void I386_OPS_BASE::I386OP(mov_rm16_sreg)()     // Opcode 0x8c
{
	UINT8 modrm = FETCH();
	int s = (modrm >> 3) & 0x7;

	if( modrm >= 0xc0 ) {
		if(cpustate->operand_size)
			STORE_RM32(modrm, cpustate->sreg[s].selector);
		else
			STORE_RM16(modrm, cpustate->sreg[s].selector);
		CYCLES(CYCLES_MOV_SREG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE16(ea, cpustate->sreg[s].selector);
		CYCLES(CYCLES_MOV_SREG_MEM);
	}
}

void I386_OPS_BASE::I386OP(mov_sreg_rm16)()     // Opcode 0x8e
{
	UINT16 selector;
	UINT8 modrm = FETCH();
	bool fault;
	int s = (modrm >> 3) & 0x7;

	if( modrm >= 0xc0 ) {
		selector = LOAD_RM16(modrm);
		CYCLES(CYCLES_MOV_REG_SREG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		selector = READ16(ea);
		CYCLES(CYCLES_MOV_MEM_SREG);
	}

	i386_sreg_load(selector,s,&fault);
	if((s == SS) && !fault)
	{
		if(cpustate->IF != 0) // if external interrupts are enabled
		{
			cpustate->IF = 0;  // reset IF for the next instruction
			cpustate->delayed_interrupt_enable = 1;
		}
	}
}

void I386_OPS_BASE::I386OP(mov_al_i8)()         // Opcode 0xb0
{
	REG8(AL) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void I386_OPS_BASE::I386OP(mov_cl_i8)()         // Opcode 0xb1
{
	REG8(CL) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void I386_OPS_BASE::I386OP(mov_dl_i8)()         // Opcode 0xb2
{
	REG8(DL) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void I386_OPS_BASE::I386OP(mov_bl_i8)()         // Opcode 0xb3
{
	REG8(BL) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void I386_OPS_BASE::I386OP(mov_ah_i8)()         // Opcode 0xb4
{
	REG8(AH) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void I386_OPS_BASE::I386OP(mov_ch_i8)()         // Opcode 0xb5
{
	REG8(CH) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void I386_OPS_BASE::I386OP(mov_dh_i8)()         // Opcode 0xb6
{
	REG8(DH) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void I386_OPS_BASE::I386OP(mov_bh_i8)()         // Opcode 0xb7
{
	REG8(BH) = FETCH();
	CYCLES(CYCLES_MOV_IMM_REG);
}

void I386_OPS_BASE::I386OP(movsb)()             // Opcode 0xa4
{
	UINT32 eas, ead;
	UINT8 v;
	if( cpustate->segment_prefix ) {
		eas = i386_translate( cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate( DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}
	ead = i386_translate( ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 1 );
	v = READ8(eas);
	WRITE8(ead, v);
	BUMP_SI(1);
	BUMP_DI(1);
	CYCLES(CYCLES_MOVS);
}

void I386_OPS_BASE::I386OP(or_rm8_r8)()         // Opcode 0x08
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = OR8(dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = OR8(dst, src);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void I386_OPS_BASE::I386OP(or_r8_rm8)()         // Opcode 0x0a
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = OR8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		dst = OR8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void I386_OPS_BASE::I386OP(or_al_i8)()          // Opcode 0x0c
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(AL);
	dst = OR8(dst, src);
	REG8(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void I386_OPS_BASE::I386OP(out_al_i8)()         // Opcode 0xe6
{
	UINT16 port = FETCH();
	UINT8 data = REG8(AL);
	WRITEPORT8( port, data);
	CYCLES(CYCLES_OUT_VAR);
}

void I386_OPS_BASE::I386OP(out_al_dx)()         // Opcode 0xee
{
	UINT16 port = REG16(DX);
	UINT8 data = REG8(AL);
	WRITEPORT8( port, data);
	CYCLES(CYCLES_OUT);
}


void I386_OPS_BASE::I386OP(arpl)()           // Opcode 0x63
{
	UINT16 src, dst;
	UINT8 modrm = FETCH();
	UINT8 flag = 0;

	if(PROTECTED_MODE && !V8086_MODE)
	{
			if( modrm >= 0xc0 ) {
			src = LOAD_REG16(modrm);
			dst = LOAD_RM16(modrm);
			if( (dst&0x3) < (src&0x3) ) {
				dst = (dst&0xfffc) | (src&0x3);
				flag = 1;
				STORE_RM16(modrm, dst);
			}
		} else {
			UINT32 ea = GetEA( modrm,1);
			src = LOAD_REG16(modrm);
			dst = READ16( ea);
			if( (dst&0x3) < (src&0x3) ) {
				dst = (dst&0xfffc) | (src&0x3);
				flag = 1;
				WRITE16( ea, dst);
			}
		}
		SetZF(flag);
	}
	else
		i386_trap( 6, 0, 0);  // invalid opcode in real mode or v8086 mode
}

void I386_OPS_BASE::I386OP(push_i8)()           // Opcode 0x6a
{
	UINT8 value = FETCH();
	PUSH8(value);
	CYCLES(CYCLES_PUSH_IMM);
}

void I386_OPS_BASE::I386OP(ins_generic)( int size)
{
	UINT32 ead;
	UINT8 vb;
	UINT16 vw;
	UINT32 vd;

	ead = i386_translate( ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 1 );

	switch(size) {
	case 1:
		vb = READPORT8( REG16(DX));
		WRITE8(ead, vb);
		break;
	case 2:
		vw = READPORT16( REG16(DX));
		WRITE16(ead, vw);
		break;
	case 4:
		vd = READPORT32( REG16(DX));
		WRITE32(ead, vd);
		break;
	}

	if(cpustate->address_size)
		REG32(EDI) += ((cpustate->DF) ? -1 : 1) * size;
	else
		REG16(DI) += ((cpustate->DF) ? -1 : 1) * size;
	CYCLES(CYCLES_INS);    // TODO: Confirm this value
}

void I386_OPS_BASE::I386OP(insb)()              // Opcode 0x6c
{
	I386_OPS_BASE::I386OP(ins_generic)( 1);
}

void I386_OPS_BASE::I386OP(insw)()              // Opcode 0x6d
{
	I386_OPS_BASE::I386OP(ins_generic)( 2);
}

void I386_OPS_BASE::I386OP(insd)()              // Opcode 0x6d
{
	I386_OPS_BASE::I386OP(ins_generic)( 4);
}

void I386_OPS_BASE::I386OP(outs_generic)( int size)
{
	UINT32 eas;
	UINT8 vb;
	UINT16 vw;
	UINT32 vd;

	if( cpustate->segment_prefix ) {
		eas = i386_translate( cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	} else {
		eas = i386_translate( DS, cpustate->address_size ? REG32(ESI) : REG16(SI), 0 );
	}

	switch(size) {
	case 1:
		vb = READ8(eas);
		WRITEPORT8( REG16(DX), vb);
		break;
	case 2:
		vw = READ16(eas);
		WRITEPORT16( REG16(DX), vw);
		break;
	case 4:
		vd = READ32(eas);
		WRITEPORT32( REG16(DX), vd);
		break;
	}

	if(cpustate->address_size)
		REG32(ESI) += ((cpustate->DF) ? -1 : 1) * size;
	else
		REG16(SI) += ((cpustate->DF) ? -1 : 1) * size;
	CYCLES(CYCLES_OUTS);   // TODO: Confirm this value
}

void I386_OPS_BASE::I386OP(outsb)()             // Opcode 0x6e
{
	I386_OPS_BASE::I386OP(outs_generic)( 1);
}

void I386_OPS_BASE::I386OP(outsw)()             // Opcode 0x6f
{
	I386_OPS_BASE::I386OP(outs_generic)( 2);
}

void I386_OPS_BASE::I386OP(outsd)()             // Opcode 0x6f
{
	I386_OPS_BASE::I386OP(outs_generic)( 4);
}

void I386_OPS_BASE::I386OP(repeat)( int invert_flag)
{
	UINT32 repeated_eip = cpustate->eip;
	UINT32 repeated_pc = cpustate->pc;
	UINT8 opcode; // = FETCH();
//  UINT32 eas, ead;
	UINT32 count;
	INT32 cycle_base = 0, cycle_adjustment = 0;
	UINT8 prefix_flag=1;
	UINT8 *flag = NULL;


	do {
	repeated_eip = cpustate->eip;
	repeated_pc = cpustate->pc;
	opcode = FETCH();
	switch(opcode) {
		case 0x0f:
		if (invert_flag == 0)
			I386_OPS_BASE::I386OP(decode_three_bytef3)(); // sse f3 0f
		else
			I386_OPS_BASE::I386OP(decode_three_bytef2)(); // sse f2 0f
		return;
		case 0x26:
		cpustate->segment_override=ES;
		cpustate->segment_prefix=1;
		break;
		case 0x2e:
		cpustate->segment_override=CS;
		cpustate->segment_prefix=1;
		break;
		case 0x36:
		cpustate->segment_override=SS;
		cpustate->segment_prefix=1;
		break;
		case 0x3e:
		cpustate->segment_override=DS;
		cpustate->segment_prefix=1;
		break;
		case 0x64:
		cpustate->segment_override=FS;
		cpustate->segment_prefix=1;
		break;
		case 0x65:
		cpustate->segment_override=GS;
		cpustate->segment_prefix=1;
		break;
		case 0x66:
		cpustate->operand_size ^= 1;
		cpustate->xmm_operand_size ^= 1;
		break;
		case 0x67:
		cpustate->address_size ^= 1;
		break;
		default:
		prefix_flag=0;
	}
	} while (prefix_flag);


	if( cpustate->segment_prefix ) {
		// FIXME: the following does not work if both address override and segment override are used
		i386_translate( cpustate->segment_override, cpustate->sreg[cpustate->segment_prefix].d ? REG32(ESI) : REG16(SI), -1 );
	} else {
		//eas =
		i386_translate( DS, cpustate->address_size ? REG32(ESI) : REG16(SI), -1 );
	}
	i386_translate( ES, cpustate->address_size ? REG32(EDI) : REG16(DI), -1 );

	switch(opcode)
	{
		case 0x6c:
		case 0x6d:
			/* INSB, INSW, INSD */
			// TODO: cycle count
			cycle_base = 8;
			cycle_adjustment = -4;
			flag = NULL;
			break;

		case 0x6e:
		case 0x6f:
			/* OUTSB, OUTSW, OUTSD */
			// TODO: cycle count
			cycle_base = 8;
			cycle_adjustment = -4;
			flag = NULL;
			break;

		case 0xa4:
		case 0xa5:
			/* MOVSB, MOVSW, MOVSD */
			cycle_base = 8;
			cycle_adjustment = -4;
			flag = NULL;
			break;

		case 0xa6:
		case 0xa7:
			/* CMPSB, CMPSW, CMPSD */
			cycle_base = 5;
			cycle_adjustment = -1;
			flag = &cpustate->ZF;
			break;

		case 0xac:
		case 0xad:
			/* LODSB, LODSW, LODSD */
			cycle_base = 5;
			cycle_adjustment = 1;
			flag = NULL;
			break;

		case 0xaa:
		case 0xab:
			/* STOSB, STOSW, STOSD */
			cycle_base = 5;
			cycle_adjustment = 0;
			flag = NULL;
			break;

		case 0xae:
		case 0xaf:
			/* SCASB, SCASW, SCASD */
			cycle_base = 5;
			cycle_adjustment = 0;
			flag = &cpustate->ZF;
			break;

		case 0x90:
			CYCLES(CYCLES_NOP);
			return;

		case 0xc2: // sigh
		case 0xc3:
			cpustate->pc--;
			return;

		default:
			logerror("i386: Invalid REP/opcode %02X combination at %08x\n",opcode, cpustate->pc - 2);
			cpustate->pc--;
			break;
	}

	if( cpustate->address_size ) {
		if( REG32(ECX) == 0 )
			return;
	} else {
		if( REG16(CX) == 0 )
			return;
	}

	/* now actually perform the repeat */
	CYCLES_NUM(cycle_base);
	do
	{
		cpustate->eip = repeated_eip;
		cpustate->pc = repeated_pc;
		try
		{
			I386_OPS_BASE::I386OP(decode_opcode)();
		}
		catch (UINT64 e)
		{
			cpustate->eip = cpustate->prev_eip;
			throw e;
		}

		CYCLES_NUM(cycle_adjustment);

		if (cpustate->address_size)
			count = --REG32(ECX);
		else
			count = --REG16(CX);
		if (cpustate->cycles <= 0)
			goto outofcycles;
	}
	while( count && (!flag || (invert_flag ? !*flag : *flag)) );
	return;

outofcycles:
	/* if we run out of cycles to execute, and we are still in the repeat, we need
	 * to exit this instruction in such a way to go right back into it when we have
	 * time to execute cycles */
	if(flag && (invert_flag ? *flag : !*flag))
		return;
	cpustate->eip = cpustate->prev_eip;
	CHANGE_PC(cpustate->eip);
	CYCLES_NUM(-cycle_base);
}

void I386_OPS_BASE::I386OP(rep)()               // Opcode 0xf3
{
	I386_OPS_BASE::I386OP(repeat)( 0);
}

void I386_OPS_BASE::I386OP(repne)()             // Opcode 0xf2
{
	I386_OPS_BASE::I386OP(repeat)( 1);
}

void I386_OPS_BASE::I386OP(sahf)()              // Opcode 0x9e
{
	set_flags( (get_flags() & 0xffffff00) | (REG8(AH) & 0xd7) );
	CYCLES(CYCLES_SAHF);
}

void I386_OPS_BASE::I386OP(sbb_rm8_r8)()        // Opcode 0x18
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = SBB8( dst, src, cpustate->CF);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = SBB8( dst, src, cpustate->CF);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void I386_OPS_BASE::I386OP(sbb_r8_rm8)()        // Opcode 0x1a
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = SBB8( dst, src, cpustate->CF);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		dst = SBB8( dst, src, cpustate->CF);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void I386_OPS_BASE::I386OP(sbb_al_i8)()         // Opcode 0x1c
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(AL);
	dst = SBB8( dst, src, cpustate->CF);
	REG8(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void I386_OPS_BASE::I386OP(scasb)()             // Opcode 0xae
{
	UINT32 eas;
	UINT8 src, dst;
	eas = i386_translate( ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 0 );
	src = READ8(eas);
	dst = REG8(AL);
	SUB8(dst, src);
	BUMP_DI(1);
	CYCLES(CYCLES_SCAS);
}

void I386_OPS_BASE::I386OP(setalc)()            // Opcode 0xd6 (undocumented)
{
	if( cpustate->CF ) {
		REG8(AL) = 0xff;
	} else {
		REG8(AL) = 0;
	}
	CYCLES(3);
}

void I386_OPS_BASE::I386OP(seta_rm8)()          // Opcode 0x0f 97
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->CF == 0 && cpustate->ZF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(setbe_rm8)()         // Opcode 0x0f 96
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->CF != 0 || cpustate->ZF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(setc_rm8)()          // Opcode 0x0f 92
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->CF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(setg_rm8)()          // Opcode 0x0f 9f
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->ZF == 0 && (cpustate->SF == cpustate->OF) ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(setge_rm8)()         // Opcode 0x0f 9d
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if(cpustate->SF == cpustate->OF) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(setl_rm8)()          // Opcode 0x0f 9c
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->SF != cpustate->OF ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(setle_rm8)()         // Opcode 0x0f 9e
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->ZF != 0 || (cpustate->SF != cpustate->OF) ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(setnc_rm8)()         // Opcode 0x0f 93
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->CF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(setno_rm8)()         // Opcode 0x0f 91
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->OF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(setnp_rm8)()         // Opcode 0x0f 9b
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->PF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(setns_rm8)()         // Opcode 0x0f 99
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->SF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(setnz_rm8)()         // Opcode 0x0f 95
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->ZF == 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(seto_rm8)()          // Opcode 0x0f 90
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->OF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(setp_rm8)()          // Opcode 0x0f 9a
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->PF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(sets_rm8)()          // Opcode 0x0f 98
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->SF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(setz_rm8)()          // Opcode 0x0f 94
{
	UINT8 modrm = FETCH();
	UINT8 value = 0;
	if( cpustate->ZF != 0 ) {
		value = 1;
	}
	if( modrm >= 0xc0 ) {
		STORE_RM8(modrm, value);
		CYCLES(CYCLES_SETCC_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		WRITE8(ea, value);
		CYCLES(CYCLES_SETCC_MEM);
	}
}

void I386_OPS_BASE::I386OP(stc)()               // Opcode 0xf9
{
	cpustate->CF = 1;
	CYCLES(CYCLES_STC);
}

void I386_OPS_BASE::I386OP(std)()               // Opcode 0xfd
{
	cpustate->DF = 1;
	CYCLES(CYCLES_STD);
}

void I386_OPS_BASE::I386OP(sti)()               // Opcode 0xfb
{
	if(PROTECTED_MODE)
	{
		UINT8 IOPL = cpustate->IOP1 | (cpustate->IOP2 << 1);
		if(cpustate->CPL > IOPL)
			FAULT(FAULT_GP,0);
	}
	cpustate->delayed_interrupt_enable = 1;  // IF is set after the next instruction.
	CYCLES(CYCLES_STI);
}

void I386_OPS_BASE::I386OP(stosb)()             // Opcode 0xaa
{
	UINT32 ead;
	ead = i386_translate( ES, cpustate->address_size ? REG32(EDI) : REG16(DI), 1 );
	WRITE8(ead, REG8(AL));
	BUMP_DI(1);
	CYCLES(CYCLES_STOS);
}

void I386_OPS_BASE::I386OP(sub_rm8_r8)()        // Opcode 0x28
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = SUB8(dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = SUB8(dst, src);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void I386_OPS_BASE::I386OP(sub_r8_rm8)()        // Opcode 0x2a
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = SUB8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		dst = SUB8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void I386_OPS_BASE::I386OP(sub_al_i8)()         // Opcode 0x2c
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(EAX);
	dst = SUB8(dst, src);
	REG8(EAX) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void I386_OPS_BASE::I386OP(test_al_i8)()        // Opcode 0xa8
{
	UINT8 src = FETCH();
	UINT8 dst = REG8(AL);
	dst = src & dst;
	SetSZPF8(dst);
	cpustate->CF = 0;
	cpustate->OF = 0;
	CYCLES(CYCLES_ALU_IMM_ACC);
}

void I386_OPS_BASE::I386OP(test_rm8_r8)()       // Opcode 0x84
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = src & dst;
		SetSZPF8(dst);
		cpustate->CF = 0;
		cpustate->OF = 0;
		CYCLES(CYCLES_TEST_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = src & dst;
		SetSZPF8(dst);
		cpustate->CF = 0;
		cpustate->OF = 0;
		CYCLES(CYCLES_TEST_REG_MEM);
	}
}

void I386_OPS_BASE::I386OP(xchg_r8_rm8)()       // Opcode 0x86
{
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		UINT8 src = LOAD_RM8(modrm);
		UINT8 dst = LOAD_REG8(modrm);
		STORE_REG8(modrm, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_XCHG_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		UINT8 src = READ8(ea);
		UINT8 dst = LOAD_REG8(modrm);
		WRITE8(ea, dst);
		STORE_REG8(modrm, src);
		CYCLES(CYCLES_XCHG_REG_MEM);
	}
}

void I386_OPS_BASE::I386OP(xor_rm8_r8)()        // Opcode 0x30
{
	UINT8 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_REG8(modrm);
		dst = LOAD_RM8(modrm);
		dst = XOR8(dst, src);
		STORE_RM8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,1);
		src = LOAD_REG8(modrm);
		dst = READ8(ea);
		dst = XOR8(dst, src);
		WRITE8(ea, dst);
		CYCLES(CYCLES_ALU_REG_MEM);
	}
}

void I386_OPS_BASE::I386OP(xor_r8_rm8)()        // Opcode 0x32
{
	UINT32 src, dst;
	UINT8 modrm = FETCH();
	if( modrm >= 0xc0 ) {
		src = LOAD_RM8(modrm);
		dst = LOAD_REG8(modrm);
		dst = XOR8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_REG_REG);
	} else {
		UINT32 ea = GetEA(modrm,0);
		src = READ8(ea);
		dst = LOAD_REG8(modrm);
		dst = XOR8(dst, src);
		STORE_REG8(modrm, dst);
		CYCLES(CYCLES_ALU_MEM_REG);
	}
}

void I386_OPS_BASE::I386OP(xor_al_i8)()         // Opcode 0x34
{
	UINT8 src, dst;
	src = FETCH();
	dst = REG8(AL);
	dst = XOR8(dst, src);
	REG8(AL) = dst;
	CYCLES(CYCLES_ALU_IMM_ACC);
}



void I386_OPS_BASE::I386OP(group80_8)()         // Opcode 0x80
{
	UINT32 ea;
	UINT8 src, dst;
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:     // ADD Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				dst = ADD8(dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,0);
				dst = READ8(ea);
				src = FETCH();
				dst = ADD8(dst, src);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:     // OR Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				dst = OR8(dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ8(ea);
				src = FETCH();
				dst = OR8(dst, src);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:     // ADC Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				dst = ADC8( dst, src, cpustate->CF);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ8(ea);
				src = FETCH();
				dst = ADC8( dst, src, cpustate->CF);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:     // SBB Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				dst = SBB8( dst, src, cpustate->CF);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ8(ea);
				src = FETCH();
				dst = SBB8( dst, src, cpustate->CF);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:     // AND Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				dst = AND8(dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ8(ea);
				src = FETCH();
				dst = AND8(dst, src);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:     // SUB Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				dst = SUB8(dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ8(ea);
				src = FETCH();
				dst = SUB8(dst, src);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:     // XOR Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				dst = XOR8(dst, src);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(modrm,1);
				dst = READ8(ea);
				src = FETCH();
				dst = XOR8(dst, src);
				WRITE8(ea, dst);
				CYCLES(CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:     // CMP Rm8, i8
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM8(modrm);
				src = FETCH();
				SUB8(dst, src);
				CYCLES(CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(modrm,0);
				dst = READ8(ea);
				src = FETCH();
				SUB8(dst, src);
				CYCLES(CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

void I386_OPS_BASE::I386OP(groupC0_8)()         // Opcode 0xc0
{
	UINT8 dst;
	UINT8 modrm = FETCH();
	UINT8 shift;

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM8(modrm);
		shift = FETCH() & 0x1f;
		dst = i386_shift_rotate8( modrm, dst, shift);
		STORE_RM8(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm,1);
		dst = READ8(ea);
		shift = FETCH() & 0x1f;
		dst = i386_shift_rotate8( modrm, dst, shift);
		WRITE8(ea, dst);
	}
}

void I386_OPS_BASE::I386OP(groupD0_8)()         // Opcode 0xd0
{
	UINT8 dst;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM8(modrm);
		dst = i386_shift_rotate8( modrm, dst, 1);
		STORE_RM8(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm,1);
		dst = READ8(ea);
		dst = i386_shift_rotate8( modrm, dst, 1);
		WRITE8(ea, dst);
	}
}

void I386_OPS_BASE::I386OP(groupD2_8)()         // Opcode 0xd2
{
	UINT8 dst;
	UINT8 modrm = FETCH();

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM8(modrm);
		dst = i386_shift_rotate8( modrm, dst, REG8(CL));
		STORE_RM8(modrm, dst);
	} else {
		UINT32 ea = GetEA(modrm,1);
		dst = READ8(ea);
		dst = i386_shift_rotate8( modrm, dst, REG8(CL));
		WRITE8(ea, dst);
	}
}

void I386_OPS_BASE::I386OP(groupF6_8)()         // Opcode 0xf6
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* TEST Rm8, i8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				UINT8 src = FETCH();
				dst &= src;
				cpustate->CF = cpustate->OF = cpustate->AF = 0;
				SetSZPF8(dst);
				CYCLES(CYCLES_TEST_IMM_REG);
			} else {
				UINT32 ea = GetEA(modrm,0);
				UINT8 dst = READ8(ea);
				UINT8 src = FETCH();
				dst &= src;
				cpustate->CF = cpustate->OF = cpustate->AF = 0;
				SetSZPF8(dst);
				CYCLES(CYCLES_TEST_IMM_MEM);
			}
			break;
		case 2:         /* NOT Rm8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				dst = ~dst;
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_NOT_REG);
			} else {
				UINT32 ea = GetEA(modrm,1);
				UINT8 dst = READ8(ea);
				dst = ~dst;
				WRITE8(ea, dst);
				CYCLES(CYCLES_NOT_MEM);
			}
			break;
		case 3:         /* NEG Rm8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				dst = SUB8( 0, dst );
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_NEG_REG);
			} else {
				UINT32 ea = GetEA(modrm,1);
				UINT8 dst = READ8(ea);
				dst = SUB8( 0, dst );
				WRITE8(ea, dst);
				CYCLES(CYCLES_NEG_MEM);
			}
			break;
		case 4:         /* MUL AL, Rm8 */
			{
				UINT16 result;
				UINT8 src, dst;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM8(modrm);
					CYCLES(CYCLES_MUL8_ACC_REG);       /* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(modrm,0);
					src = READ8(ea);
					CYCLES(CYCLES_MUL8_ACC_MEM);       /* TODO: Correct multiply timing */
				}

				dst = REG8(AL);
				result = (UINT16)src * (UINT16)dst;
				REG16(AX) = (UINT16)result;

				cpustate->CF = cpustate->OF = (REG16(AX) > 0xff);
			}
			break;
		case 5:         /* IMUL AL, Rm8 */
			{
				INT16 result;
				INT16 src, dst;
				if( modrm >= 0xc0 ) {
					src = (INT16)(INT8)LOAD_RM8(modrm);
					CYCLES(CYCLES_IMUL8_ACC_REG);      /* TODO: Correct multiply timing */
				} else {
					UINT32 ea = GetEA(modrm,0);
					src = (INT16)(INT8)READ8(ea);
					CYCLES(CYCLES_IMUL8_ACC_MEM);      /* TODO: Correct multiply timing */
				}

				dst = (INT16)(INT8)REG8(AL);
				result = src * dst;

				REG16(AX) = (UINT16)result;

				cpustate->CF = cpustate->OF = !(result == (INT16)(INT8)result);
			}
			break;
		case 6:         /* DIV AL, Rm8 */
			{
				UINT16 quotient, remainder, result;
				UINT8 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM8(modrm);
					CYCLES(CYCLES_DIV8_ACC_REG);
				} else {
					UINT32 ea = GetEA(modrm,0);
					src = READ8(ea);
					CYCLES(CYCLES_DIV8_ACC_MEM);
				}

				quotient = (UINT16)REG16(AX);
				if( src ) {
					remainder = quotient % (UINT16)src;
					result = quotient / (UINT16)src;
					if( result > 0xff ) {
						/* TODO: Divide error */
					} else {
						REG8(AH) = (UINT8)remainder & 0xff;
						REG8(AL) = (UINT8)result & 0xff;

						// this flag is actually undefined, enable on non-cyrix
						if (cpustate->cpuid_id0 != 0x69727943)
							cpustate->CF = 1;
					}
				} else {
					i386_trap( 0, 0, 0);
				}
			}
			break;
		case 7:         /* IDIV AL, Rm8 */
			{
				INT16 quotient, remainder, result;
				UINT8 src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM8(modrm);
					CYCLES(CYCLES_IDIV8_ACC_REG);
				} else {
					UINT32 ea = GetEA(modrm,0);
					src = READ8(ea);
					CYCLES(CYCLES_IDIV8_ACC_MEM);
				}

				quotient = (INT16)REG16(AX);
				if( src ) {
					remainder = quotient % (INT16)(INT8)src;
					result = quotient / (INT16)(INT8)src;
					if( result > 0xff ) {
						/* TODO: Divide error */
					} else {
						REG8(AH) = (UINT8)remainder & 0xff;
						REG8(AL) = (UINT8)result & 0xff;

						// this flag is actually undefined, enable on non-cyrix
						if (cpustate->cpuid_id0 != 0x69727943)
							cpustate->CF = 1;
					}
				} else {
					i386_trap( 0, 0, 0);
				}
			}
			break;
	}
}

void I386_OPS_BASE::I386OP(groupFE_8)()         // Opcode 0xfe
{
	UINT8 modrm = FETCH();

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:         /* INC Rm8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				dst = INC8(dst);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_INC_REG);
			} else {
				UINT32 ea = GetEA(modrm,1);
				UINT8 dst = READ8(ea);
				dst = INC8(dst);
				WRITE8(ea, dst);
				CYCLES(CYCLES_INC_MEM);
			}
			break;
		case 1:         /* DEC Rm8 */
			if( modrm >= 0xc0 ) {
				UINT8 dst = LOAD_RM8(modrm);
				dst = DEC8(dst);
				STORE_RM8(modrm, dst);
				CYCLES(CYCLES_DEC_REG);
			} else {
				UINT32 ea = GetEA(modrm,1);
				UINT8 dst = READ8(ea);
				dst = DEC8(dst);
				WRITE8(ea, dst);
				CYCLES(CYCLES_DEC_MEM);
			}
			break;
		case 6:         /* PUSH Rm8*/
			{
				UINT8 value;
				if( modrm >= 0xc0 ) {
					value = LOAD_RM8(modrm);
				} else {
					UINT32 ea = GetEA(modrm,0);
					value = READ8(ea);
				}
				if( cpustate->operand_size ) {
					PUSH32(value);
				} else {
					PUSH16(value);
				}
				CYCLES(CYCLES_PUSH_RM);
			}
			break;
		default:
			report_invalid_modrm( "groupFE_8", modrm);
			break;
	}
}



void I386_OPS_BASE::I386OP(segment_CS)()        // Opcode 0x2e
{
	cpustate->segment_prefix = 1;
	cpustate->segment_override = CS;

	I386_OPS_BASE::I386OP(decode_opcode)();
}

void I386_OPS_BASE::I386OP(segment_DS)()        // Opcode 0x3e
{
	cpustate->segment_prefix = 1;
	cpustate->segment_override = DS;
	CYCLES(0); // TODO: Specify cycle count
	I386_OPS_BASE::I386OP(decode_opcode)();
}

void I386_OPS_BASE::I386OP(segment_ES)()        // Opcode 0x26
{
	cpustate->segment_prefix = 1;
	cpustate->segment_override = ES;
	CYCLES(0); // TODO: Specify cycle count
	I386_OPS_BASE::I386OP(decode_opcode)();
}

void I386_OPS_BASE::I386OP(segment_FS)()        // Opcode 0x64
{
	cpustate->segment_prefix = 1;
	cpustate->segment_override = FS;
	CYCLES(1); // TODO: Specify cycle count
	I386_OPS_BASE::I386OP(decode_opcode)();
}

void I386_OPS_BASE::I386OP(segment_GS)()        // Opcode 0x65
{
	cpustate->segment_prefix = 1;
	cpustate->segment_override = GS;
	CYCLES(1); // TODO: Specify cycle count
	I386_OPS_BASE::I386OP(decode_opcode)();
}

void I386_OPS_BASE::I386OP(segment_SS)()        // Opcode 0x36
{
	cpustate->segment_prefix = 1;
	cpustate->segment_override = SS;
	CYCLES(0); // TODO: Specify cycle count
	I386_OPS_BASE::I386OP(decode_opcode)();
}

void I386_OPS_BASE::I386OP(operand_size)()      // Opcode prefix 0x66
{
	if(cpustate->operand_prefix == 0)
	{
		cpustate->operand_size ^= 1;
		cpustate->xmm_operand_size ^= 1;
		cpustate->operand_prefix = 1;
	}
	cpustate->opcode = FETCH();
	if (cpustate->opcode == 0x0f)
		I386_OPS_BASE::I386OP(decode_three_byte66)();
	else
	{
		if( cpustate->operand_size )
			(this->*cpustate->opcode_table1_32[cpustate->opcode])();
		else
			(this->*cpustate->opcode_table1_16[cpustate->opcode])();
	}
}

void I386_OPS_BASE::I386OP(address_size)()      // Opcode 0x67
{
	if(cpustate->address_prefix == 0)
	{
		cpustate->address_size ^= 1;
		cpustate->address_prefix = 1;
	}
	I386_OPS_BASE::I386OP(decode_opcode)();
}

void I386_OPS_BASE::I386OP(nop)()               // Opcode 0x90
{
	CYCLES(CYCLES_NOP);
}

void I386_OPS_BASE::I386OP(int3)()              // Opcode 0xcc
{
	CYCLES(CYCLES_INT3);
	cpustate->ext = 0; // not an external interrupt
	i386_trap(3, 1, 0);
	cpustate->ext = 1;
}

void I386_OPS_BASE::I386_OPS_BASE::I386OP(int)()               // Opcode 0xcd
{
	int interrupt = FETCH();
	CYCLES(CYCLES_INT);
	// Without pseudo BIOS.
	cpustate->ext = 0; // not an external interrupt
	i386_trap(interrupt, 1, 0);
	cpustate->ext = 1;
}

void I386_OPS_BASE::I386OP(into)()              // Opcode 0xce
{
	if( cpustate->OF ) {
		cpustate->ext = 0;
		i386_trap(4, 1, 0);
		cpustate->ext = 1;
		CYCLES(CYCLES_INTO_OF1);
	}
	else
	{
		CYCLES(CYCLES_INTO_OF0);
	}
}

void I386_OPS_BASE::I386OP(escape)()            // Opcodes 0xd8 - 0xdf
{
	UINT8 modrm = FETCH();
	if(modrm < 0xc0)
	{
		i386_escape_ea = GetEA(modrm,0);
	}
	CYCLES(3); // TODO: confirm this
	(void) LOAD_RM8(modrm);
}

void I386_OPS_BASE::I386OP(hlt)()               // Opcode 0xf4
{
	if(PROTECTED_MODE && cpustate->CPL != 0)
		FAULT(FAULT_GP,0);
	cpustate->halted = 1;
	CYCLES(CYCLES_HLT);
	if (cpustate->cycles > 0)
		cpustate->cycles = 0;
}

void I386_OPS_BASE::I386OP(decimal_adjust)(int direction)
{
	UINT8 tmpAL = REG8(AL);
	UINT8 tmpCF = cpustate->CF;

	if (cpustate->AF || ((REG8(AL) & 0xf) > 9))
	{
		UINT16 t= (UINT16)REG8(AL) + (direction * 0x06);
		REG8(AL) = (UINT8)t&0xff;
		cpustate->AF = 1;
		if (t & 0x100)
			cpustate->CF = 1;
		if (direction > 0)
			tmpAL = REG8(AL);
	}

	if (tmpCF || (tmpAL > 0x99))
	{
		REG8(AL) += (direction * 0x60);
		cpustate->CF = 1;
	}

	SetSZPF8(REG8(AL));
}

void I386_OPS_BASE::I386OP(daa)()               // Opcode 0x27
{
	I386_OPS_BASE::I386OP(decimal_adjust)( +1);
	CYCLES(CYCLES_DAA);
}

void I386_OPS_BASE::I386OP(das)()               // Opcode 0x2f
{
	I386_OPS_BASE::I386OP(decimal_adjust)( -1);
	CYCLES(CYCLES_DAS);
}

void I386_OPS_BASE::I386OP(aaa)()               // Opcode 0x37
{
	if( ( (REG8(AL) & 0x0f) > 9) || (cpustate->AF != 0) ) {
		REG16(AX) = REG16(AX) + 6;
		REG8(AH) = REG8(AH) + 1;
		cpustate->AF = 1;
		cpustate->CF = 1;
	} else {
		cpustate->AF = 0;
		cpustate->CF = 0;
	}
	REG8(AL) = REG8(AL) & 0x0f;
	CYCLES(CYCLES_AAA);
}

void I386_OPS_BASE::I386OP(aas)()               // Opcode 0x3f
{
	if (cpustate->AF || ((REG8(AL) & 0xf) > 9))
	{
		REG16(AX) -= 6;
		REG8(AH) -= 1;
		cpustate->AF = 1;
		cpustate->CF = 1;
	}
	else
	{
		cpustate->AF = 0;
		cpustate->CF = 0;
	}
	REG8(AL) &= 0x0f;
	CYCLES(CYCLES_AAS);
}

void I386_OPS_BASE::I386OP(aad)()               // Opcode 0xd5
{
	UINT8 tempAL = REG8(AL);
	UINT8 tempAH = REG8(AH);
	UINT8 i = FETCH();

	REG8(AL) = (tempAL + (tempAH * i)) & 0xff;
	REG8(AH) = 0;
	SetSZPF8( REG8(AL) );
	CYCLES(CYCLES_AAD);
}

void I386_OPS_BASE::I386OP(aam)()               // Opcode 0xd4
{
	UINT8 tempAL = REG8(AL);
	UINT8 i = FETCH();

	if(!i)
	{
		i386_trap( 0, 0, 0);
		return;
	}
	REG8(AH) = tempAL / i;
	REG8(AL) = tempAL % i;
	SetSZPF8( REG8(AL) );
	CYCLES(CYCLES_AAM);
}

void I386_OPS_BASE::I386OP(clts)()              // Opcode 0x0f 0x06
{
	// Privileged instruction, CPL must be zero.  Can be used in real or v86 mode.
	if(PROTECTED_MODE && cpustate->CPL != 0)
		FAULT(FAULT_GP,0)
	cpustate->cr[0] &= ~0x08;   /* clear TS bit */
	CYCLES(CYCLES_CLTS);
}

void I386_OPS_BASE::I386OP(wait)()              // Opcode 0x9B
{
	// TODO
}

void I386_OPS_BASE::I386OP(lock)()              // Opcode 0xf0
{
	// lock doesn't depend on iopl on 386
	cpustate->lock = true;
	CYCLES(CYCLES_LOCK);       // TODO: Determine correct cycle count
	I386_OPS_BASE::I386OP(decode_opcode)();
}

void I386_OPS_BASE::I386OP(mov_r32_tr)()        // Opcode 0x0f 24
{
	FETCH();
	CYCLES(1);     // TODO: correct cycle count
}

void I386_OPS_BASE::I386OP(mov_tr_r32)()        // Opcode 0x0f 26
{
	FETCH();
	CYCLES(1);     // TODO: correct cycle count
}

void I386_OPS_BASE::I386OP(loadall)()       // Opcode 0x0f 0x07 (0x0f 0x05 on 80286), undocumented
{
	if(PROTECTED_MODE && (cpustate->CPL != 0))
		FAULT(FAULT_GP,0)
	UINT32 ea = i386_translate(ES, REG32(EDI), 0);
	cpustate->cr[0] = READ32(ea) & 0xfffeffff; // wp not supported on 386
	set_flags(READ32(ea + 0x04));
	cpustate->eip = READ32( ea + 0x08);
	REG32(EDI) = READ32( ea + 0x0c);
	REG32(ESI) = READ32( ea + 0x10);
	REG32(EBP) = READ32( ea + 0x14);
	REG32(ESP) = READ32( ea + 0x18);
	REG32(EBX) = READ32( ea + 0x1c);
	REG32(EDX) = READ32( ea + 0x20);
	REG32(ECX) = READ32( ea + 0x24);
	REG32(EAX) = READ32( ea + 0x28);
	cpustate->dr[6] = READ32( ea + 0x2c);
	cpustate->dr[7] = READ32( ea + 0x30);
	cpustate->task.segment = READ16(ea + 0x34);
	cpustate->ldtr.segment = READ16(ea + 0x38);
	cpustate->sreg[GS].selector = READ16(ea + 0x3c);
	cpustate->sreg[FS].selector = READ16(ea + 0x40);
	cpustate->sreg[DS].selector = READ16(ea + 0x44);
	cpustate->sreg[SS].selector = READ16(ea + 0x48);
	cpustate->sreg[CS].selector = READ16(ea + 0x4c);
	cpustate->sreg[ES].selector = READ16(ea + 0x50);
	cpustate->task.flags = READ32( ea + 0x54) >> 8;
	cpustate->task.base = READ32( ea + 0x58);
	cpustate->task.limit = READ32( ea + 0x5c);
	cpustate->idtr.base = READ32( ea + 0x64);
	cpustate->idtr.limit = READ32( ea + 0x68);
	cpustate->gdtr.base = READ32( ea + 0x70);
	cpustate->gdtr.limit = READ32( ea + 0x74);
	cpustate->ldtr.flags = READ32( ea + 0x78) >> 8;
	cpustate->ldtr.base = READ32( ea + 0x7c);
	cpustate->ldtr.limit = READ32( ea + 0x80);
	cpustate->sreg[GS].flags = READ32( ea + 0x84) >> 8;
	cpustate->sreg[GS].base = READ32( ea + 0x88);
	cpustate->sreg[GS].limit = READ32( ea + 0x8c);
	cpustate->sreg[FS].flags = READ32( ea + 0x90) >> 8;
	cpustate->sreg[FS].base = READ32( ea + 0x94);
	cpustate->sreg[FS].limit = READ32( ea + 0x98);
	cpustate->sreg[DS].flags = READ32( ea + 0x9c) >> 8;
	cpustate->sreg[DS].base = READ32( ea + 0xa0);
	cpustate->sreg[DS].limit = READ32( ea + 0xa4);
	cpustate->sreg[SS].flags = READ32( ea + 0xa8) >> 8;
	cpustate->sreg[SS].base = READ32( ea + 0xac);
	cpustate->sreg[SS].limit = READ32( ea + 0xb0);
	cpustate->sreg[CS].flags = READ32( ea + 0xb4) >> 8;
	cpustate->sreg[CS].base = READ32( ea + 0xb8);
	cpustate->sreg[CS].limit = READ32( ea + 0xbc);
	cpustate->sreg[ES].flags = READ32( ea + 0xc0) >> 8;
	cpustate->sreg[ES].base = READ32( ea + 0xc4);
	cpustate->sreg[ES].limit = READ32( ea + 0xc8);
	cpustate->CPL = (cpustate->sreg[SS].flags >> 5) & 3; // cpl == dpl of ss

	for(int i = 0; i <= GS; i++)
	{
		cpustate->sreg[i].valid = (cpustate->sreg[i].flags & 0x80) ? true : false;
		cpustate->sreg[i].d = (cpustate->sreg[i].flags & 0x4000) ? 1 : 0;
	}
	CHANGE_PC(cpustate->eip);
}

void I386_OPS_BASE::I386OP(invalid)()
{
	report_invalid_opcode();
	i386_trap( 6, 0, 0);
}

void I386_OPS_BASE::I386OP(xlat)()          // Opcode 0xd7
{
	UINT32 ea;
	if( cpustate->segment_prefix ) {
		if(!cpustate->address_size)
		{
			ea = i386_translate( cpustate->segment_override, REG16(BX) + REG8(AL), 0 );
		}
		else
		{
			ea = i386_translate( cpustate->segment_override, REG32(EBX) + REG8(AL), 0 );
		}
	} else {
		if(!cpustate->address_size)
		{
			ea = i386_translate( DS, REG16(BX) + REG8(AL), 0 );
		}
		else
		{
			ea = i386_translate( DS, REG32(EBX) + REG8(AL), 0 );
		}
	}
	REG8(AL) = READ8(ea);
	CYCLES(CYCLES_XLAT);
}
