#ifndef __LIBNEWDEV_I86_INLINEOPS_H__
#define __LIBNEWDEV_I86_INLINEOPS_H__
#include "./i86_macros.h"

inline void I86_BASE::_add_br8()    /* Opcode 0x00 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	ADDB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86_BASE::_add_wr16()    /* Opcode 0x01 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	ADDW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86_BASE::_add_r8b()    /* Opcode 0x02 */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ADDB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86_BASE::_add_r16w()    /* Opcode 0x03 */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ADDW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86_BASE::_add_ald8()    /* Opcode 0x04 */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	ADDB(dst, src);
	regs.b[AL] = dst;
}

inline void I86_BASE::_add_axd16()    /* Opcode 0x05 */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	ADDW(dst, src);
	regs.w[AX] = dst;
}

inline void I86_BASE::_push_es()    /* Opcode 0x06 */
{
	icount -= timing.push_seg;
	PUSH(sregs[ES]);
}

inline void I86_BASE::_pop_es()    /* Opcode 0x07 */
{
	POP(sregs[ES]);
	base[ES] = SegBase(ES);
	icount -= timing.pop_seg;
}

inline void I86_BASE::_or_br8()    /* Opcode 0x08 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	ORB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86_BASE::_or_wr16()    /* Opcode 0x09 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	ORW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86_BASE::_or_r8b()    /* Opcode 0x0a */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ORB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86_BASE::_or_r16w()    /* Opcode 0x0b */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ORW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86_BASE::_or_ald8()    /* Opcode 0x0c */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	ORB(dst, src);
	regs.b[AL] = dst;
}

inline void I86_BASE::_or_axd16()    /* Opcode 0x0d */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	ORW(dst, src);
	regs.w[AX] = dst;
}

inline void I86_BASE::_push_cs()    /* Opcode 0x0e */
{
	icount -= timing.push_seg;
	PUSH(sregs[CS]);
}


inline void I86_BASE::_adc_br8()    /* Opcode 0x10 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	src += CF;
	ADDB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86_BASE::_adc_wr16()    /* Opcode 0x11 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	src += CF;
	ADDW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86_BASE::_adc_r8b()    /* Opcode 0x12 */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	src += CF;
	ADDB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86_BASE::_adc_r16w()    /* Opcode 0x13 */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	src += CF;
	ADDW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86_BASE::_adc_ald8()    /* Opcode 0x14 */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	src += CF;
	ADDB(dst, src);
	regs.b[AL] = dst;
}

inline void I86_BASE::_adc_axd16()    /* Opcode 0x15 */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	src += CF;
	ADDW(dst, src);
	regs.w[AX] = dst;
}

inline void I86_BASE::_push_ss()    /* Opcode 0x16 */
{
	PUSH(sregs[SS]);
	icount -= timing.push_seg;
}

inline void I86_BASE::_pop_ss()    /* Opcode 0x17 */
{
	POP(sregs[SS]);
	base[SS] = SegBase(SS);
	icount -= timing.pop_seg;
	instruction(FETCHOP); /* no interrupt before next instruction */
}

inline void I86_BASE::_sbb_br8()    /* Opcode 0x18 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	src += CF;
	SUBB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86_BASE::_sbb_wr16()    /* Opcode 0x19 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	src += CF;
	SUBW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86_BASE::_sbb_r8b()    /* Opcode 0x1a */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	src += CF;
	SUBB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86_BASE::_sbb_r16w()    /* Opcode 0x1b */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	src += CF;
	SUBW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86_BASE::_sbb_ald8()    /* Opcode 0x1c */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	src += CF;
	SUBB(dst, src);
	regs.b[AL] = dst;
}

inline void I86_BASE::_sbb_axd16()    /* Opcode 0x1d */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	src += CF;
	SUBW(dst, src);
	regs.w[AX] = dst;
}

inline void I86_BASE::_push_ds()    /* Opcode 0x1e */
{
	PUSH(sregs[DS]);
	icount -= timing.push_seg;
}

inline void I86_BASE::_pop_ds()    /* Opcode 0x1f */
{
	POP(sregs[DS]);
	base[DS] = SegBase(DS);
	icount -= timing.push_seg;
}

inline void I86_BASE::_and_br8()    /* Opcode 0x20 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	ANDB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86_BASE::_and_wr16()    /* Opcode 0x21 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	ANDW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86_BASE::_and_r8b()    /* Opcode 0x22 */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ANDB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86_BASE::_and_r16w()    /* Opcode 0x23 */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ANDW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86_BASE::_and_ald8()    /* Opcode 0x24 */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	ANDB(dst, src);
	regs.b[AL] = dst;
}

inline void I86_BASE::_and_axd16()    /* Opcode 0x25 */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	ANDW(dst, src);
	regs.w[AX] = dst;
}

inline void I86_BASE::_es()    /* Opcode 0x26 */
{
	seg_prefix = true;
	prefix_seg = ES;
	icount -= timing.override;
	instruction(FETCHOP);
}

inline void I86_BASE::_daa()    /* Opcode 0x27 */
{
	if(AF || ((regs.b[AL] & 0xf) > 9)) {
		int tmp;
		regs.b[AL] = tmp = regs.b[AL] + 6;
		AuxVal = 1;
		CarryVal |= tmp & 0x100;
	}
	
	if(CF || (regs.b[AL] > 0x9f)) {
		regs.b[AL] += 0x60;
		CarryVal = 1;
	}
	
	SetSZPF_Byte(regs.b[AL]);
	icount -= timing.daa;
}

inline void I86_BASE::_sub_br8()    /* Opcode 0x28 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	SUBB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86_BASE::_sub_wr16()    /* Opcode 0x29 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	SUBW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86_BASE::_sub_r8b()    /* Opcode 0x2a */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	SUBB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86_BASE::_sub_r16w()    /* Opcode 0x2b */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	SUBW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86_BASE::_sub_ald8()    /* Opcode 0x2c */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	SUBB(dst, src);
	regs.b[AL] = dst;
}

inline void I86_BASE::_sub_axd16()    /* Opcode 0x2d */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	SUBW(dst, src);
	regs.w[AX] = dst;
}

inline void I86_BASE::_cs()    /* Opcode 0x2e */
{
	seg_prefix = true;
	prefix_seg = CS;
	icount -= timing.override;
	instruction(FETCHOP);
}

inline void I86_BASE::_das()    /* Opcode 0x2f */
{
	uint8_t tmpAL = regs.b[AL];
	if(AF || ((regs.b[AL] & 0xf) > 9)) {
		int tmp;
		regs.b[AL] = tmp = regs.b[AL] - 6;
		AuxVal = 1;
		CarryVal |= tmp & 0x100;
	}
	
	if(CF || (tmpAL > 0x9f)) {
		regs.b[AL] -= 0x60;
		CarryVal = 1;
	}
	
	SetSZPF_Byte(regs.b[AL]);
	icount -= timing.das;
}

inline void I86_BASE::_xor_br8()    /* Opcode 0x30 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	XORB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86_BASE::_xor_wr16()    /* Opcode 0x31 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	XORW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86_BASE::_xor_r8b()    /* Opcode 0x32 */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	XORB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86_BASE::_xor_r16w()    /* Opcode 0x33 */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	XORW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86_BASE::_xor_ald8()    /* Opcode 0x34 */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	XORB(dst, src);
	regs.b[AL] = dst;
}

inline void I86_BASE::_xor_axd16()    /* Opcode 0x35 */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	XORW(dst, src);
	regs.w[AX] = dst;
}

inline void I86_BASE::_ss()    /* Opcode 0x36 */
{
	seg_prefix = true;
	prefix_seg = SS;
	icount -= timing.override;
	instruction(FETCHOP);
}

inline void I86_BASE::_aaa()    /* Opcode 0x37 */
{
	uint8_t ALcarry = 1;
	if(regs.b[AL]>0xf9) {
		ALcarry = 2;
	}
	if(AF || ((regs.b[AL] & 0xf) > 9)) {
		regs.b[AL] += 6;
		regs.b[AH] += ALcarry;
		AuxVal = 1;
		CarryVal = 1;
	} else {
		AuxVal = 0;
		CarryVal = 0;
	}
	regs.b[AL] &= 0x0F;
	icount -= timing.aaa;
}

inline void I86_BASE::_cmp_br8()    /* Opcode 0x38 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	SUBB(dst, src);
}

inline void I86_BASE::_cmp_wr16()    /* Opcode 0x39 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	SUBW(dst, src);
}

inline void I86_BASE::_cmp_r8b()    /* Opcode 0x3a */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	SUBB(dst, src);
}

inline void I86_BASE::_cmp_r16w()    /* Opcode 0x3b */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	SUBW(dst, src);
}

inline void I86_BASE::_cmp_ald8()    /* Opcode 0x3c */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	SUBB(dst, src);
}

inline void I86_BASE::_cmp_axd16()    /* Opcode 0x3d */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	SUBW(dst, src);
}

inline void I86_BASE::_ds()    /* Opcode 0x3e */
{
	seg_prefix = true;
	prefix_seg = DS;
	icount -= timing.override;
	instruction(FETCHOP);
}

inline void I86_BASE::_aas()    /* Opcode 0x3f */
{
	uint8_t ALcarry = 1;
	if(regs.b[AL] > 0xf9) {
		ALcarry = 2;
	}
	if(AF || ((regs.b[AL] & 0xf) > 9)) {
		regs.b[AL] -= 6;
		regs.b[AH] -= 1;
		AuxVal = 1;
		CarryVal = 1;
	} else {
		AuxVal = 0;
		CarryVal = 0;
	}
	regs.b[AL] &= 0x0F;
	icount -= timing.aas;
}

#define IncWordReg(Reg) { \
	unsigned tmp = (unsigned)regs.w[Reg]; \
	unsigned tmp1 = tmp + 1; \
	SetOFW_Add(tmp1, tmp, 1); \
	SetAF(tmp1, tmp, 1); \
	SetSZPF_Word(tmp1); \
	regs.w[Reg] = tmp1; \
	icount -= timing.incdec_r16; \
}

inline void I86_BASE::_inc_ax()    /* Opcode 0x40 */
{
	IncWordReg(AX);
}

inline void I86_BASE::_inc_cx()    /* Opcode 0x41 */
{
	IncWordReg(CX);
}

inline void I86_BASE::_inc_dx()    /* Opcode 0x42 */
{
	IncWordReg(DX);
}

inline void I86_BASE::_inc_bx()    /* Opcode 0x43 */
{
	IncWordReg(BX);
}

inline void I86_BASE::_inc_sp()    /* Opcode 0x44 */
{
	IncWordReg(SP);
}

inline void I86_BASE::_inc_bp()    /* Opcode 0x45 */
{
	IncWordReg(BP);
}

inline void I86_BASE::_inc_si()    /* Opcode 0x46 */
{
	IncWordReg(SI);
}

inline void I86_BASE::_inc_di()    /* Opcode 0x47 */
{
	IncWordReg(DI);
}

#define DecWordReg(Reg) { \
	unsigned tmp = (unsigned)regs.w[Reg]; \
	unsigned tmp1 = tmp - 1; \
	SetOFW_Sub(tmp1, 1, tmp); \
	SetAF(tmp1, tmp, 1); \
	SetSZPF_Word(tmp1); \
	regs.w[Reg] = tmp1; \
	icount -= timing.incdec_r16; \
}

inline void I86_BASE::_dec_ax()    /* Opcode 0x48 */
{
	DecWordReg(AX);
}

inline void I86_BASE::_dec_cx()    /* Opcode 0x49 */
{
	DecWordReg(CX);
}

inline void I86_BASE::_dec_dx()    /* Opcode 0x4a */
{
	DecWordReg(DX);
}

inline void I86_BASE::_dec_bx()    /* Opcode 0x4b */
{
	DecWordReg(BX);
}

inline void I86_BASE::_dec_sp()    /* Opcode 0x4c */
{
	DecWordReg(SP);
}

inline void I86_BASE::_dec_bp()    /* Opcode 0x4d */
{
	DecWordReg(BP);
}

inline void I86_BASE::_dec_si()    /* Opcode 0x4e */
{
	DecWordReg(SI);
}

inline void I86_BASE::_dec_di()    /* Opcode 0x4f */
{
	DecWordReg(DI);
}

inline void I86_BASE::_push_ax()    /* Opcode 0x50 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[AX]);
}

inline void I86_BASE::_push_cx()    /* Opcode 0x51 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[CX]);
}

inline void I86_BASE::_push_dx()    /* Opcode 0x52 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[DX]);
}

inline void I86_BASE::_push_bx()    /* Opcode 0x53 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[BX]);
}

inline void I86_BASE::_push_sp()    /* Opcode 0x54 */
{
	unsigned tmp = regs.w[SP];
	
	icount -= timing.push_r16;
	PUSH(tmp - 2);
}

inline void I86_BASE::_push_bp()    /* Opcode 0x55 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[BP]);
}

inline void I86_BASE::_push_si()    /* Opcode 0x56 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[SI]);
}

inline void I86_BASE::_push_di()    /* Opcode 0x57 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[DI]);
}

inline void I86_BASE::_pop_ax()    /* Opcode 0x58 */
{
	icount -= timing.pop_r16;
	POP(regs.w[AX]);
}

inline void I86_BASE::_pop_cx()    /* Opcode 0x59 */
{
	icount -= timing.pop_r16;
	POP(regs.w[CX]);
}

inline void I86_BASE::_pop_dx()    /* Opcode 0x5a */
{
	icount -= timing.pop_r16;
	POP(regs.w[DX]);
}

inline void I86_BASE::_pop_bx()    /* Opcode 0x5b */
{
	icount -= timing.pop_r16;
	POP(regs.w[BX]);
}

inline void I86_BASE::_pop_sp()    /* Opcode 0x5c */
{
	unsigned tmp;
	
	icount -= timing.pop_r16;
	POP(tmp);
	regs.w[SP] = tmp;
}

inline void I86_BASE::_pop_bp()    /* Opcode 0x5d */
{
	icount -= timing.pop_r16;
	POP(regs.w[BP]);
}

inline void I86_BASE::_pop_si()    /* Opcode 0x5e */
{
	icount -= timing.pop_r16;
	POP(regs.w[SI]);
}

inline void I86_BASE::_pop_di()    /* Opcode 0x5f */
{
	icount -= timing.pop_r16;
	POP(regs.w[DI]);
}

inline void I86_BASE::_pusha()    /* Opcode 0x60 */
{
	unsigned tmp = regs.w[SP];
	
	icount -= timing.pusha;
	PUSH(regs.w[AX]);
	PUSH(regs.w[CX]);
	PUSH(regs.w[DX]);
	PUSH(regs.w[BX]);
	PUSH(tmp);
	PUSH(regs.w[BP]);
	PUSH(regs.w[SI]);
	PUSH(regs.w[DI]);
}

inline void I86_BASE::_popa()    /* Opcode 0x61 */
{
	unsigned tmp;
	
	icount -= timing.popa;
	POP(regs.w[DI]);
	POP(regs.w[SI]);
	POP(regs.w[BP]);
	POP(tmp);
	POP(regs.w[BX]);
	POP(regs.w[DX]);
	POP(regs.w[CX]);
	POP(regs.w[AX]);
}

inline void I86_BASE::_bound()    /* Opcode 0x62 */
{
	unsigned ModRM = FETCHOP;
	int low = (int16_t)GetRMWord(ModRM);
	int high = (int16_t)GetNextRMWord;
	int tmp = (int16_t)RegWord(ModRM);
	if(tmp < low || tmp>high) {
		pc -= (seg_prefix ? 3 : 2);
		interrupt(BOUNDS_CHECK_FAULT);
	}
	icount -= timing.bound;
}

inline void I86_BASE::_push_d16()    /* Opcode 0x68 */
{
	unsigned tmp = FETCH;
	icount -= timing.push_imm;
	tmp += FETCH << 8;
	PUSH(tmp);
}

inline void I86_BASE::_imul_d16()    /* Opcode 0x69 */
{
	DEF_r16w(dst, src);
	unsigned src2 = FETCH;
	src += (FETCH << 8);
	icount -= (ModRM >= 0xc0) ? timing.imul_rri16 : timing.imul_rmi16;
	dst = (int32_t)((int16_t)src) * (int32_t)((int16_t)src2);
	CarryVal = OverVal = (((int32_t)dst) >> 15 != 0) && (((int32_t)dst) >> 15 != -1);
	RegWord(ModRM) = (uint16_t)dst;
}

inline void I86_BASE::_push_d8()    /* Opcode 0x6a */
{
	unsigned tmp = (uint16_t)((int16_t)((int8_t)FETCH));
	icount -= timing.push_imm;
	PUSH(tmp);
}

inline void I86_BASE::_imul_d8()    /* Opcode 0x6b */
{
	DEF_r16w(dst, src);
	unsigned src2 = (uint16_t)((int16_t)((int8_t)FETCH));
	icount -= (ModRM >= 0xc0) ? timing.imul_rri8 : timing.imul_rmi8;
	dst = (int32_t)((int16_t)src) * (int32_t)((int16_t)src2);
	CarryVal = OverVal = (((int32_t)dst) >> 15 != 0) && (((int32_t)dst) >> 15 != -1);
	RegWord(ModRM) = (uint16_t)dst;
}

inline void I86_BASE::_insb()    /* Opcode 0x6c */
{
	icount -= timing.ins8;
	PutMemB(ES, regs.w[DI], read_port_byte(regs.w[DX]));
	regs.w[DI] += DirVal;
}

inline void I86_BASE::_insw()    /* Opcode 0x6d */
{
	icount -= timing.ins16;
	PutMemW(ES, regs.w[DI], read_port_word(regs.w[DX]));
	regs.w[DI] += 2 * DirVal;
}

inline void I86_BASE::_outsb()    /* Opcode 0x6e */
{
	icount -= timing.outs8;
	write_port_byte(regs.w[DX], GetMemB(DS, regs.w[SI]));
	regs.w[SI] += DirVal; /* GOL 11/27/01 */
}

inline void I86_BASE::_outsw()    /* Opcode 0x6f */
{
	icount -= timing.outs16;
	write_port_word(regs.w[DX], GetMemW(DS, regs.w[SI]));
	regs.w[SI] += 2 * DirVal; /* GOL 11/27/01 */
}

inline void I86_BASE::_jo()    /* Opcode 0x70 */
{
	int tmp = (int)((int8_t)FETCH);
	if(OF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jno()    /* Opcode 0x71 */
{
	int tmp = (int)((int8_t)FETCH);
	if(!OF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jb()    /* Opcode 0x72 */
{
	int tmp = (int)((int8_t)FETCH);
	if(CF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jnb()    /* Opcode 0x73 */
{
	int tmp = (int)((int8_t)FETCH);
	if(!CF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jz()    /* Opcode 0x74 */
{
	int tmp = (int)((int8_t)FETCH);
	if(ZF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jnz()    /* Opcode 0x75 */
{
	int tmp = (int)((int8_t)FETCH);
	if(!ZF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jbe()    /* Opcode 0x76 */
{
	int tmp = (int)((int8_t)FETCH);
	if(CF || ZF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jnbe()    /* Opcode 0x77 */
{
	int tmp = (int)((int8_t)FETCH);
	if(!(CF || ZF)) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_js()    /* Opcode 0x78 */
{
	int tmp = (int)((int8_t)FETCH);
	if(SF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jns()    /* Opcode 0x79 */
{
	int tmp = (int)((int8_t)FETCH);
	if(!SF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jp()    /* Opcode 0x7a */
{
	int tmp = (int)((int8_t)FETCH);
	if(PF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jnp()    /* Opcode 0x7b */
{
	int tmp = (int)((int8_t)FETCH);
	if(!PF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jl()    /* Opcode 0x7c */
{
	int tmp = (int)((int8_t)FETCH);
	if((SF!= OF) && !ZF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jnl()    /* Opcode 0x7d */
{
	int tmp = (int)((int8_t)FETCH);
	if(ZF || (SF == OF)) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jle()    /* Opcode 0x7e */
{
	int tmp = (int)((int8_t)FETCH);
	if(ZF || (SF!= OF)) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_jnle()    /* Opcode 0x7f */
{
	int tmp = (int)((int8_t)FETCH);
	if((SF == OF) && !ZF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86_BASE::_80pre()    /* Opcode 0x80 */
{
	unsigned ModRM = FETCHOP;
	unsigned dst = GetRMByte(ModRM);
	unsigned src = FETCH;
	
	switch((ModRM >> 3) & 7) {
	case 0:	/* ADD eb, d8 */
		ADDB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 1:	/* OR eb, d8 */
		ORB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 2:	/* ADC eb, d8 */
		src += CF;
		ADDB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 3:	/* SBB eb, b8 */
		src += CF;
		SUBB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 4:	/* AND eb, d8 */
		ANDB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 5:	/* SUB eb, d8 */
		SUBB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 6:	/* XOR eb, d8 */
		XORB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 7:	/* CMP eb, d8 */
		SUBB(dst, src);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8_ro;
		break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default:
		__assume(0);
#endif
	}
}

inline void I86_BASE::_81pre()    /* Opcode 0x81 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMWord(ModRM);
	unsigned src = FETCH;
	src += (FETCH << 8);
	
	switch((ModRM >> 3) & 7) {
	case 0:	/* ADD ew, d16 */
		ADDW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 1:	/* OR ew, d16 */
		ORW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 2:	/* ADC ew, d16 */
		src += CF;
		ADDW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 3:	/* SBB ew, d16 */
		src += CF;
		SUBW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 4:	/* AND ew, d16 */
		ANDW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 5:	/* SUB ew, d16 */
		SUBW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 6:	/* XOR ew, d16 */
		XORW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 7:	/* CMP ew, d16 */
		SUBW(dst, src);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16_ro;
		break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default:
		__assume(0);
#endif
	}
}

inline void I86_BASE::_82pre()    /* Opcode 0x82 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMByte(ModRM);
	unsigned src = FETCH;
	
	switch((ModRM >> 3) & 7) {
	case 0:	/* ADD eb, d8 */
		ADDB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 1:	/* OR eb, d8 */
		ORB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 2:	/* ADC eb, d8 */
		src += CF;
		ADDB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 3:	/* SBB eb, d8 */
		src += CF;
		SUBB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 4:	/* AND eb, d8 */
		ANDB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 5:	/* SUB eb, d8 */
		SUBB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 6:	/* XOR eb, d8 */
		XORB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 7:	/* CMP eb, d8 */
		SUBB(dst, src);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8_ro;
		break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default:
		__assume(0);
#endif
	}
}

inline void I86_BASE::_83pre()    /* Opcode 0x83 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMWord(ModRM);
	unsigned src = (uint16_t)((int16_t)((int8_t)FETCH));
	
	switch((ModRM >> 3) & 7) {
	case 0:	/* ADD ew, d16 */
		ADDW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 1:	/* OR ew, d16 */
		ORW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 2:	/* ADC ew, d16 */
		src += CF;
		ADDW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 3:	/* SBB ew, d16 */
		src += CF;
		SUBW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 4:	/* AND ew, d16 */
		ANDW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 5:	/* SUB ew, d16 */
		SUBW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 6:	/* XOR ew, d16 */
		XORW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 7:	/* CMP ew, d16 */
		SUBW(dst, src);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8_ro;
		break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default:
		__assume(0);
#endif
	}
}

inline void I86_BASE::_test_br8()    /* Opcode 0x84 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ANDB(dst, src);
}

inline void I86_BASE::_test_wr16()    /* Opcode 0x85 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ANDW(dst, src);
}

inline void I86_BASE::_xchg_br8()    /* Opcode 0x86 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.xchg_rr8 : timing.xchg_rm8;
	RegByte(ModRM) = dst;
	PutbackRMByte(ModRM, src);
}

inline void I86_BASE::_xchg_wr16()    /* Opcode 0x87 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.xchg_rr16 : timing.xchg_rm16;
	RegWord(ModRM) = dst;
	PutbackRMWord(ModRM, src);
}

inline void I86_BASE::_mov_br8()    /* Opcode 0x88 */
{
	unsigned ModRM = FETCH;
	uint8_t src = RegByte(ModRM);
	icount -= (ModRM >= 0xc0) ? timing.mov_rr8 : timing.mov_mr8;
	PutRMByte(ModRM, src);
}

inline void I86_BASE::_mov_wr16()    /* Opcode 0x89 */
{
	unsigned ModRM = FETCH;
	uint16_t src = RegWord(ModRM);
	icount -= (ModRM >= 0xc0) ? timing.mov_rr16 : timing.mov_mr16;
	PutRMWord(ModRM, src);
}

inline void I86_BASE::_mov_r8b()    /* Opcode 0x8a */
{
	unsigned ModRM = FETCH;
	uint8_t src = GetRMByte(ModRM);
	icount -= (ModRM >= 0xc0) ? timing.mov_rr8 : timing.mov_rm8;
	RegByte(ModRM) = src;
}

inline void I86_BASE::_mov_r16w()    /* Opcode 0x8b */
{
	unsigned ModRM = FETCH;
	uint16_t src = GetRMWord(ModRM);
	icount -= (ModRM >= 0xc0) ? timing.mov_rr8 : timing.mov_rm16;
	RegWord(ModRM) = src;
}

inline void I86_BASE::_mov_wsreg()    /* Opcode 0x8c */
{
	unsigned ModRM = FETCH;
	icount -= (ModRM >= 0xc0) ? timing.mov_rs : timing.mov_ms;
	if(ModRM & 0x20) {
		return;	/* 1xx is invalid */
	}
	PutRMWord(ModRM, sregs[(ModRM & 0x38) >> 3]);
}

inline void I86_BASE::_lea()    /* Opcode 0x8d */
{
	unsigned ModRM = FETCH;
	icount -= timing.lea;
	GetEA(ModRM);
	RegWord(ModRM) = eo;	/* effective offset (no segment part) */
}

inline void I86_BASE::_mov_sregw()    /* Opcode 0x8e */
{
	unsigned ModRM = FETCH;
	uint16_t src = GetRMWord(ModRM);
	
	icount -= (ModRM >= 0xc0) ? timing.mov_sr : timing.mov_sm;
	switch((ModRM >> 3) & 7) {
	case 0:  /* mov es, ew */
		sregs[ES] = src;
		base[ES] = SegBase(ES);
		break;
	case 1:  /* mov cs, ew */
		break;  /* doesn't do a jump far */
	case 2:  /* mov ss, ew */
		sregs[SS] = src;
		base[SS] = SegBase(SS); /* no interrupt allowed before next instr */
		instruction(FETCHOP);
		break;
	case 3:  /* mov ds, ew */
		sregs[DS] = src;
		base[DS] = SegBase(DS);
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default:
		__assume(0);
#endif
	}
}

inline void I86_BASE::_popw()    /* Opcode 0x8f */
{
	unsigned ModRM = FETCH;
	uint16_t tmp;
	POP(tmp);
	icount -= (ModRM >= 0xc0) ? timing.pop_r16 : timing.pop_m16;
	PutRMWord(ModRM, tmp);
}

#define XchgAXReg(Reg) { \
	uint16_t tmp; \
	tmp = regs.w[Reg]; \
	regs.w[Reg] = regs.w[AX]; \
	regs.w[AX] = tmp; \
	icount -= timing.xchg_ar16; \
}

inline void I86_BASE::_nop()    /* Opcode 0x90 */
{
	/* this is XchgAXReg(AX); */
	icount -= timing.nop;
}

inline void I86_BASE::_xchg_axcx()    /* Opcode 0x91 */
{
	XchgAXReg(CX);
}

inline void I86_BASE::_xchg_axdx()    /* Opcode 0x92 */
{
	XchgAXReg(DX);
}

inline void I86_BASE::_xchg_axbx()    /* Opcode 0x93 */
{
	XchgAXReg(BX);
}

inline void I86_BASE::_xchg_axsp()    /* Opcode 0x94 */
{
	XchgAXReg(SP);
}

inline void I86_BASE::_xchg_axbp()    /* Opcode 0x95 */
{
	XchgAXReg(BP);
}

inline void I86_BASE::_xchg_axsi()    /* Opcode 0x96 */
{
	XchgAXReg(SI);
}

inline void I86_BASE::_xchg_axdi()    /* Opcode 0x97 */
{
	XchgAXReg(DI);
}

inline void I86_BASE::_cbw()    /* Opcode 0x98 */
{
	icount -= timing.cbw;
	regs.b[AH] = (regs.b[AL] & 0x80) ? 0xff : 0;
}

inline void I86_BASE::_cwd()    /* Opcode 0x99 */
{
	icount -= timing.cwd;
	regs.w[DX] = (regs.b[AH] & 0x80) ? 0xffff : 0;
}


inline void I86_BASE::_wait()    /* Opcode 0x9b */
{
	if(test_state) {
		pc--;
	}
	icount -= timing.wait;
}

inline void I86_BASE::_pushf()    /* Opcode 0x9c */
{
	unsigned tmp;
	icount -= timing.pushf;
	
	tmp = CompressFlags();
	PUSH(tmp | 0xf000);
}

inline void I86_BASE::_popf()    /* Opcode 0x9d */
{
	unsigned tmp;
	POP(tmp);
	icount -= timing.popf;
	ExpandFlags(tmp);
	
	if(TF) {
		trap();
	}
	
	/* if the IF is set, and an interrupt is pending, signal an interrupt */
	if(IF && (int_state & INT_REQ_BIT)) {
		interrupt(-1);
	}
}

inline void I86_BASE::_sahf()    /* Opcode 0x9e */
{
	unsigned tmp = (CompressFlags() & 0xff00) | (regs.b[AH] & 0xd5);
	icount -= timing.sahf;
	ExpandFlags(tmp);
}

inline void I86_BASE::_lahf()    /* Opcode 0x9f */
{
	regs.b[AH] = CompressFlags() & 0xff;
	icount -= timing.lahf;
}

inline void I86_BASE::_mov_aldisp()    /* Opcode 0xa0 */
{
	unsigned addr;
	
	addr = FETCH;
	addr += FETCH << 8;
	
	icount -= timing.mov_am8;
	regs.b[AL] = GetMemB(DS, addr);
}

inline void I86_BASE::_mov_axdisp()    /* Opcode 0xa1 */
{
	unsigned addr;
	
	addr = FETCH;
	addr += FETCH << 8;
	
	icount -= timing.mov_am16;
	regs.w[AX] = GetMemW(DS, addr);
}

inline void I86_BASE::_mov_dispal()    /* Opcode 0xa2 */
{
	unsigned addr;
	
	addr = FETCH;
	addr += FETCH << 8;
	
	icount -= timing.mov_ma8;
	PutMemB(DS, addr, regs.b[AL]);
}

inline void I86_BASE::_mov_dispax()    /* Opcode 0xa3 */
{
	unsigned addr;
	
	addr = FETCH;
	addr += FETCH << 8;
	
	icount -= timing.mov_ma16;
	PutMemW(DS, addr, regs.w[AX]);
}

inline void I86_BASE::_movsb()    /* Opcode 0xa4 */
{
	uint8_t tmp = GetMemB(DS, regs.w[SI]);
	PutMemB(ES, regs.w[DI], tmp);
	regs.w[DI] += DirVal;
	regs.w[SI] += DirVal;
	icount -= timing.movs8;
}

inline void I86_BASE::_movsw()    /* Opcode 0xa5 */
{
	uint16_t tmp = GetMemW(DS, regs.w[SI]);
	PutMemW(ES, regs.w[DI], tmp);
	regs.w[DI] += 2 * DirVal;
	regs.w[SI] += 2 * DirVal;
	icount -= timing.movs16;
}

inline void I86_BASE::_cmpsb()    /* Opcode 0xa6 */
{
	unsigned dst = GetMemB(ES, regs.w[DI]);
	unsigned src = GetMemB(DS, regs.w[SI]);
	SUBB(src, dst); /* opposite of the usual convention */
	regs.w[DI] += DirVal;
	regs.w[SI] += DirVal;
	icount -= timing.cmps8;
}

inline void I86_BASE::_cmpsw()    /* Opcode 0xa7 */
{
	unsigned dst = GetMemW(ES, regs.w[DI]);
	unsigned src = GetMemW(DS, regs.w[SI]);
	SUBW(src, dst); /* opposite of the usual convention */
	regs.w[DI] += 2 * DirVal;
	regs.w[SI] += 2 * DirVal;
	icount -= timing.cmps16;
}

inline void I86_BASE::_test_ald8()    /* Opcode 0xa8 */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	ANDB(dst, src);
}

inline void I86_BASE::_test_axd16()    /* Opcode 0xa9 */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	ANDW(dst, src);
}

inline void I86_BASE::_stosb()    /* Opcode 0xaa */
{
	PutMemB(ES, regs.w[DI], regs.b[AL]);
	regs.w[DI] += DirVal;
	icount -= timing.stos8;
}

inline void I86_BASE::_stosw()    /* Opcode 0xab */
{
	PutMemW(ES, regs.w[DI], regs.w[AX]);
	regs.w[DI] += 2 * DirVal;
	icount -= timing.stos16;
}

inline void I86_BASE::_lodsb()    /* Opcode 0xac */
{
	regs.b[AL] = GetMemB(DS, regs.w[SI]);
	regs.w[SI] += DirVal;
	icount -= timing.lods8;
}

inline void I86_BASE::_lodsw()    /* Opcode 0xad */
{
	regs.w[AX] = GetMemW(DS, regs.w[SI]);
	regs.w[SI] += 2 * DirVal;
	icount -= timing.lods16;
}

inline void I86_BASE::_scasb()    /* Opcode 0xae */
{
	unsigned src = GetMemB(ES, regs.w[DI]);
	unsigned dst = regs.b[AL];
	SUBB(dst, src);
	regs.w[DI] += DirVal;
	icount -= timing.scas8;
}

inline void I86_BASE::_scasw()    /* Opcode 0xaf */
{
	unsigned src = GetMemW(ES, regs.w[DI]);
	unsigned dst = regs.w[AX];
	SUBW(dst, src);
	regs.w[DI] += 2 * DirVal;
	icount -= timing.scas16;
}

inline void I86_BASE::_mov_ald8()    /* Opcode 0xb0 */
{
	regs.b[AL] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86_BASE::_mov_cld8()    /* Opcode 0xb1 */
{
	regs.b[CL] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86_BASE::_mov_dld8()    /* Opcode 0xb2 */
{
	regs.b[DL] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86_BASE::_mov_bld8()    /* Opcode 0xb3 */
{
	regs.b[BL] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86_BASE::_mov_ahd8()    /* Opcode 0xb4 */
{
	regs.b[AH] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86_BASE::_mov_chd8()    /* Opcode 0xb5 */
{
	regs.b[CH] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86_BASE::_mov_dhd8()    /* Opcode 0xb6 */
{
	regs.b[DH] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86_BASE::_mov_bhd8()    /* Opcode 0xb7 */
{
	regs.b[BH] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86_BASE::_mov_axd16()    /* Opcode 0xb8 */
{
	regs.b[AL] = FETCH;
	regs.b[AH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86_BASE::_mov_cxd16()    /* Opcode 0xb9 */
{
	regs.b[CL] = FETCH;
	regs.b[CH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86_BASE::_mov_dxd16()    /* Opcode 0xba */
{
	regs.b[DL] = FETCH;
	regs.b[DH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86_BASE::_mov_bxd16()    /* Opcode 0xbb */
{
	regs.b[BL] = FETCH;
	regs.b[BH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86_BASE::_mov_spd16()    /* Opcode 0xbc */
{
	regs.b[SPL] = FETCH;
	regs.b[SPH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86_BASE::_mov_bpd16()    /* Opcode 0xbd */
{
	regs.b[BPL] = FETCH;
	regs.b[BPH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86_BASE::_mov_sid16()    /* Opcode 0xbe */
{
	regs.b[SIL] = FETCH;
	regs.b[SIH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86_BASE::_mov_did16()    /* Opcode 0xbf */
{
	regs.b[DIL] = FETCH;
	regs.b[DIH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86_BASE::_rotshft_bd8()    /* Opcode 0xc0 */
{
	unsigned ModRM = FETCH;
	unsigned count = FETCH;
	
	rotate_shift_byte(ModRM, count);
}

inline void I86_BASE::_rotshft_wd8()    /* Opcode 0xc1 */
{
	unsigned ModRM = FETCH;
	unsigned count = FETCH;
	
	rotate_shift_word(ModRM, count);
}

inline void I86_BASE::_ret_d16()    /* Opcode 0xc2 */
{
	unsigned count = FETCH;
	count += FETCH << 8;
	POP(pc);
	pc = (pc + base[CS]) & AMASK;
	regs.w[SP] += count;
	icount -= timing.ret_near_imm;
}

inline void I86_BASE::_ret()    /* Opcode 0xc3 */
{
	POP(pc);
	pc = (pc + base[CS]) & AMASK;
	icount -= timing.ret_near;
}

inline void I86_BASE::_les_dw()    /* Opcode 0xc4 */
{
	unsigned ModRM = FETCH;
	uint16_t tmp = GetRMWord(ModRM);
	RegWord(ModRM) = tmp;
	sregs[ES] = GetNextRMWord;
	base[ES] = SegBase(ES);
	icount -= timing.load_ptr;
}

inline void I86_BASE::_lds_dw()    /* Opcode 0xc5 */
{
	unsigned ModRM = FETCH;
	uint16_t tmp = GetRMWord(ModRM);
	RegWord(ModRM) = tmp;
	sregs[DS] = GetNextRMWord;
	base[DS] = SegBase(DS);
	icount -= timing.load_ptr;
}

inline void I86_BASE::_mov_bd8()    /* Opcode 0xc6 */
{
	unsigned ModRM = FETCH;
	icount -= (ModRM >= 0xc0) ? timing.mov_ri8 : timing.mov_mi8;
	PutImmRMByte(ModRM);
}

inline void I86_BASE::_mov_wd16()    /* Opcode 0xc7 */
{
	unsigned ModRM = FETCH;
	icount -= (ModRM >= 0xc0) ? timing.mov_ri16 : timing.mov_mi16;
	PutImmRMWord(ModRM);
}

inline void I86_BASE::_enter()    /* Opcode 0xc8 */
{
	unsigned nb = FETCH;
	unsigned i, level;
	
	nb += FETCH << 8;
	level = FETCH;
	icount -= (level == 0) ? timing.enter0 : (level == 1) ? timing.enter1 : timing.enter_base + level * timing.enter_count;
	PUSH(regs.w[BP]);
	regs.w[BP] = regs.w[SP];
	regs.w[SP] -= nb;
	for(i = 1; i < level; i++) {
		PUSH(GetMemW(SS, regs.w[BP] - i * 2));
	}
	if(level) {
		PUSH(regs.w[BP]);
	}
}

inline void I86_BASE::_leav()    /* Opcode 0xc9 */
{
	icount -= timing.leave;
	regs.w[SP] = regs.w[BP];
	POP(regs.w[BP]);
}

inline void I86_BASE::_retf_d16()    /* Opcode 0xca */
{
	unsigned count = FETCH;
	count += FETCH << 8;
	POP(pc);
	POP(sregs[CS]);
	base[CS] = SegBase(CS);
	pc = (pc + base[CS]) & AMASK;
	regs.w[SP] += count;
	icount -= timing.ret_far_imm;
}

inline void I86_BASE::_retf()    /* Opcode 0xcb */
{
	POP(pc);
	POP(sregs[CS]);
	base[CS] = SegBase(CS);
	pc = (pc + base[CS]) & AMASK;
	icount -= timing.ret_far;
}

inline void I86_BASE::_int3()    /* Opcode 0xcc */
{
	icount -= timing.int3;
	interrupt(3);
}

inline void I86_BASE::_into()    /* Opcode 0xce */
{
	if(OF) {
		icount -= timing.into_t;
		interrupt(OVERFLOW_TRAP);
	} else {
		icount -= timing.into_nt;
	}
}

inline void I86_BASE::_iret()    /* Opcode 0xcf */
{
	icount -= timing.iret;
	POP(pc);
	POP(sregs[CS]);
	base[CS] = SegBase(CS);
	pc = (pc + base[CS]) & AMASK;
	_popf();
	
	/* if the IF is set, and an interrupt is pending, signal an interrupt */
	if(IF && (int_state & INT_REQ_BIT)) {
		interrupt(-1);
	}
}

inline void I86_BASE::_rotshft_b()    /* Opcode 0xd0 */
{
	rotate_shift_byte(FETCHOP, 1);
}

inline void I86_BASE::_rotshft_w()    /* Opcode 0xd1 */
{
	rotate_shift_word(FETCHOP, 1);
}

inline void I86_BASE::_rotshft_bcl()    /* Opcode 0xd2 */
{
	rotate_shift_byte(FETCHOP, regs.b[CL]);
}

inline void I86_BASE::_rotshft_wcl()    /* Opcode 0xd3 */
{
	rotate_shift_word(FETCHOP, regs.b[CL]);
}

/* OB: Opcode works on NEC V-Series but not the Variants        */
/*     one could specify any byte value as operand but the NECs */
/*     always substitute 0x0a.                                  */
inline void I86_BASE::_aam()    /* Opcode 0xd4 */
{
	unsigned mult = FETCH;
	icount -= timing.aam;
	if(mult == 0) {
		interrupt(DIVIDE_FAULT);
	} else {
		regs.b[AH] = regs.b[AL] / mult;
		regs.b[AL] %= mult;
		SetSZPF_Word(regs.w[AX]);
	}
}


inline void I86_BASE::_setalc()    /* Opcode 0xd6 */
{
	regs.b[AL] = (CF) ? 0xff : 0x00;
	icount -= 3;
}

inline void I86_BASE::_xlat()    /* Opcode 0xd7 */
{
	unsigned dest = regs.w[BX] + regs.b[AL];
	icount -= timing.xlat;
	regs.b[AL] = GetMemB(DS, dest);
}

inline void I86_BASE::_escape()    /* Opcodes 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde and 0xdf */
{
	unsigned ModRM = FETCH;
	icount -= timing.nop;
	GetRMByte(ModRM);
}

inline void I86_BASE::_loopne()    /* Opcode 0xe0 */
{
	int disp = (int)((int8_t)FETCH);
	unsigned tmp = regs.w[CX] - 1;
	regs.w[CX] = tmp;
	if(!ZF && tmp) {
		icount -= timing.loop_t;
		pc += disp;
	} else {
		icount -= timing.loop_nt;
	}
}

inline void I86_BASE::_loope()    /* Opcode 0xe1 */
{
	int disp = (int)((int8_t)FETCH);
	unsigned tmp = regs.w[CX] - 1;
	regs.w[CX] = tmp;
	if(ZF && tmp) {
		icount -= timing.loope_t;
		pc += disp;
	} else {
		icount -= timing.loope_nt;
	}
}

inline void I86_BASE::_loop()    /* Opcode 0xe2 */
{
	int disp = (int)((int8_t)FETCH);
	unsigned tmp = regs.w[CX] - 1;
	regs.w[CX] = tmp;
	if(tmp) {
		icount -= timing.loop_t;
		pc += disp;
	} else {
		icount -= timing.loop_nt;
	}
}

inline void I86_BASE::_jcxz()    /* Opcode 0xe3 */
{
	int disp = (int)((int8_t)FETCH);
	if(regs.w[CX] == 0) {
		icount -= timing.jcxz_t;
		pc += disp;
	} else {
		icount -= timing.jcxz_nt;
	}
}

inline void I86_BASE::_inal()    /* Opcode 0xe4 */
{
	unsigned port = FETCH;
	icount -= timing.in_imm8;
	regs.b[AL] = read_port_byte(port);
}

inline void I86_BASE::_inax()    /* Opcode 0xe5 */
{
	unsigned port = FETCH;
	icount -= timing.in_imm16;
	regs.w[AX] = read_port_word(port);
}

inline void I86_BASE::_outal()    /* Opcode 0xe6 */
{
	unsigned port = FETCH;
	icount -= timing.out_imm8;
	write_port_byte(port, regs.b[AL]);
}

inline void I86_BASE::_outax()    /* Opcode 0xe7 */
{
	unsigned port = FETCH;
	icount -= timing.out_imm16;
	write_port_word(port, regs.w[AX]);
}


inline void I86_BASE::_jmp_d16()    /* Opcode 0xe9 */
{
	uint16_t ip, tmp;
	
	FETCHWORD(tmp);
	ip = pc - base[CS] + tmp;
	pc = (ip + base[CS]) & AMASK;
	icount -= timing.jmp_near;
}

inline void I86_BASE::_jmp_far()    /* Opcode 0xea */
{
	unsigned tmp, tmp1;
	
	tmp = FETCH;
	tmp += FETCH << 8;
	
	tmp1 = FETCH;
	tmp1 += FETCH << 8;
	
	sregs[CS] = (uint16_t)tmp1;
	base[CS] = SegBase(CS);
	pc = (base[CS] + tmp) & AMASK;
	icount -= timing.jmp_far;
}

inline void I86_BASE::_jmp_d8()    /* Opcode 0xeb */
{
	int tmp = (int)((int8_t)FETCH);
	pc += tmp;
	icount -= timing.jmp_short;
}

inline void I86_BASE::_inaldx()    /* Opcode 0xec */
{
	icount -= timing.in_dx8;
	regs.b[AL] = read_port_byte(regs.w[DX]);
}

inline void I86_BASE::_inaxdx()    /* Opcode 0xed */
{
	unsigned port = regs.w[DX];
	icount -= timing.in_dx16;
	regs.w[AX] = read_port_word(port);
}

inline void I86_BASE::_outdxal()    /* Opcode 0xee */
{
	icount -= timing.out_dx8;
	write_port_byte(regs.w[DX], regs.b[AL]);
}

inline void I86_BASE::_outdxax()    /* Opcode 0xef */
{
	unsigned port = regs.w[DX];
	icount -= timing.out_dx16;
	write_port_word(port, regs.w[AX]);
}

/* I think thats not a V20 instruction...*/
inline void I86_BASE::_lock()    /* Opcode 0xf0 */
{
	icount -= timing.nop;
	instruction(FETCHOP);  /* un-interruptible */
}


inline void I86_BASE::_repne()    /* Opcode 0xf2 */
{
	_rep(0);
}

inline void I86_BASE::_repe()    /* Opcode 0xf3 */
{
	_rep(1);
}

inline void I86_BASE::_hlt()    /* Opcode 0xf4 */
{
	pc--;
	halted = true;
	icount -= 2;
}

inline void I86_BASE::_cmc()    /* Opcode 0xf5 */
{
	icount -= timing.flag_ops;
	CarryVal = !CF;
}

inline void I86_BASE::_f6pre()    /* Opcode 0xf6 */
{
	unsigned ModRM = FETCH;
	unsigned tmp = (unsigned)GetRMByte(ModRM);
	unsigned tmp2;
	
	switch((ModRM >> 3) & 7) {
	case 0:  /* TEST Eb, data8 */
	case 1:  /* ??? */
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8_ro;
		tmp &= FETCH;
		
		CarryVal = OverVal = AuxVal = 0;
		SetSZPF_Byte(tmp);
		break;
		
	case 2:  /* NOT Eb */
		icount -= (ModRM >= 0xc0) ? timing.negnot_r8 : timing.negnot_m8;
		PutbackRMByte(ModRM, ~tmp);
		break;
		
	case 3:  /* NEG Eb */
		icount -= (ModRM >= 0xc0) ? timing.negnot_r8 : timing.negnot_m8;
		tmp2 = 0;
		SUBB(tmp2, tmp);
		PutbackRMByte(ModRM, tmp2);
		break;
		
	case 4:  /* MUL AL, Eb */
		icount -= (ModRM >= 0xc0) ? timing.mul_r8 : timing.mul_m8;
		{
			uint16_t result;
			
			tmp2 = regs.b[AL];
			
			SetSF((int8_t)tmp2);
			SetPF(tmp2);
			
			result = (uint16_t)tmp2 * tmp;
			regs.w[AX] = (uint16_t)result;
			
			SetZF(regs.w[AX]);
			CarryVal = OverVal = (regs.b[AH] != 0);
		}
		break;
		
	case 5:  /* IMUL AL, Eb */
		icount -= (ModRM >= 0xc0) ? timing.imul_r8 : timing.imul_m8;
		{
			int16_t result;
			
			tmp2 = (unsigned)regs.b[AL];
			
			SetSF((int8_t)tmp2);
			SetPF(tmp2);
			
			result = (int16_t)((int8_t)tmp2) * (int16_t)((int8_t)tmp);
			regs.w[AX] = (uint16_t)result;
			
			SetZF(regs.w[AX]);
			CarryVal = OverVal = (result >> 7 != 0) && (result >> 7 != -1);
		}
		break;
		
	case 6:  /* DIV AL, Ew */
		icount -= (ModRM >= 0xc0) ? timing.div_r8 : timing.div_m8;
		{
			uint16_t result;
			
			result = regs.w[AX];
			
			if(tmp) {
				if((result / tmp) > 0xff) {
					interrupt(DIVIDE_FAULT);
					break;
				} else {
					regs.b[AH] = result % tmp;
					regs.b[AL] = result / tmp;
				}
			} else {
				interrupt(DIVIDE_FAULT);
				break;
			}
		}
		break;
		
	case 7:  /* IDIV AL, Ew */
		icount -= (ModRM >= 0xc0) ? timing.idiv_r8 : timing.idiv_m8;
		{
			int16_t result;
			
			result = regs.w[AX];
			
			if(tmp) {
				tmp2 = result % (int16_t)((int8_t)tmp);
				
				if((result /= (int16_t)((int8_t)tmp)) > 0xff) {
					interrupt(DIVIDE_FAULT);
					break;
				} else {
					regs.b[AL] = (uint8_t)result;
					regs.b[AH] = tmp2;
				}
			} else {
				interrupt(DIVIDE_FAULT);
				break;
			}
		}
		break;
		
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default:
		__assume(0);
#endif
	}
}

inline void I86_BASE::_f7pre()    /* Opcode 0xf7 */
{
	unsigned ModRM = FETCH;
	unsigned tmp = GetRMWord(ModRM);
	unsigned tmp2;
	
	switch((ModRM >> 3) & 7) {
	case 0:  /* TEST Ew, data16 */
	case 1:  /* ??? */
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16_ro;
		tmp2 = FETCH;
		tmp2 += FETCH << 8;
		
		tmp &= tmp2;
		
		CarryVal = OverVal = AuxVal = 0;
		SetSZPF_Word(tmp);
		break;
		
	case 2:  /* NOT Ew */
		icount -= (ModRM >= 0xc0) ? timing.negnot_r16 : timing.negnot_m16;
		tmp = ~tmp;
		PutbackRMWord(ModRM, tmp);
		break;
		
	case 3:  /* NEG Ew */
		icount -= (ModRM >= 0xc0) ? timing.negnot_r16 : timing.negnot_m16;
		tmp2 = 0;
		SUBW(tmp2, tmp);
		PutbackRMWord(ModRM, tmp2);
		break;
		
	case 4:  /* MUL AX, Ew */
		icount -= (ModRM >= 0xc0) ? timing.mul_r16 : timing.mul_m16;
		{
			uint32_t result;
			tmp2 = regs.w[AX];
			
			SetSF((int16_t)tmp2);
			SetPF(tmp2);
			
			result = (uint32_t)tmp2 * tmp;
			regs.w[AX] = (uint16_t)result;
			result >>= 16;
			regs.w[DX] = result;
			
			SetZF(regs.w[AX] | regs.w[DX]);
			CarryVal = OverVal = (regs.w[DX] != 0);
		}
		break;
		
	case 5:  /* IMUL AX, Ew */
		icount -= (ModRM >= 0xc0) ? timing.imul_r16 : timing.imul_m16;
		{
			int32_t result;
			
			tmp2 = regs.w[AX];
			
			SetSF((int16_t)tmp2);
			SetPF(tmp2);
			
			result = (int32_t)((int16_t)tmp2) * (int32_t)((int16_t)tmp);
			CarryVal = OverVal = (result >> 15 != 0) && (result >> 15 != -1);
			
			regs.w[AX] = (uint16_t)result;
			result = (uint16_t)(result >> 16);
			regs.w[DX] = result;
			
			SetZF(regs.w[AX] | regs.w[DX]);
		}
		break;
		
	case 6:  /* DIV AX, Ew */
		icount -= (ModRM >= 0xc0) ? timing.div_r16 : timing.div_m16;
		{
			uint32_t result;
			
			result = (regs.w[DX] << 16) + regs.w[AX];
			
			if(tmp) {
				tmp2 = result % tmp;
				if((result / tmp) > 0xffff) {
					interrupt(DIVIDE_FAULT);
					break;
				} else {
					regs.w[DX] = tmp2;
					result /= tmp;
					regs.w[AX] = result;
				}
			} else {
				interrupt(DIVIDE_FAULT);
				break;
			}
		}
		break;
		
	case 7:  /* IDIV AX, Ew */
		icount -= (ModRM >= 0xc0) ? timing.idiv_r16 : timing.idiv_m16;
		{
			int32_t result;
			
			result = (regs.w[DX] << 16) + regs.w[AX];
			
			if(tmp) {
				tmp2 = result % (int32_t)((int16_t)tmp);
				if((result /= (int32_t)((int16_t)tmp)) > 0xffff) {
					interrupt(DIVIDE_FAULT);
					break;
				} else {
					regs.w[AX] = result;
					regs.w[DX] = tmp2;
				}
			} else {
				interrupt(DIVIDE_FAULT);
				break;
			}
		}
		break;
		
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default:
		__assume(0);
#endif
	}
}

inline void I86_BASE::_clc()    /* Opcode 0xf8 */
{
	icount -= timing.flag_ops;
	CarryVal = 0;
}

inline void I86_BASE::_stc()    /* Opcode 0xf9 */
{
	icount -= timing.flag_ops;
	CarryVal = 1;
}

inline void I86_BASE::_cli()    /* Opcode 0xfa */
{
	icount -= timing.flag_ops;
	SetIF(0);
}

inline void I86_BASE::_sti()    /* Opcode 0xfb */
{
	icount -= timing.flag_ops;
	SetIF(1);
	instruction(FETCHOP); /* no interrupt before next instruction */

	/* if an interrupt is pending, signal an interrupt */
	if(IF && (int_state & INT_REQ_BIT)) {
		interrupt(-1);
	}
}

inline void I86_BASE::_cld()    /* Opcode 0xfc */
{
	icount -= timing.flag_ops;
	SetDF(0);
}

inline void I86_BASE::_std()    /* Opcode 0xfd */
{
	icount -= timing.flag_ops;
	SetDF(1);
}

inline void I86_BASE::_fepre()    /* Opcode 0xfe */
{
	unsigned ModRM = FETCH;
	unsigned tmp = GetRMByte(ModRM);
	unsigned tmp1;
	
	icount -= (ModRM >= 0xc0) ? timing.incdec_r8 : timing.incdec_m8;
	if((ModRM & 0x38) == 0) {
		/* INC eb */
		tmp1 = tmp + 1;
		SetOFB_Add(tmp1, tmp, 1);
	} else {
		/* DEC eb */
		tmp1 = tmp - 1;
		SetOFB_Sub(tmp1, 1, tmp);
	}
	SetAF(tmp1, tmp, 1);
	SetSZPF_Byte(tmp1);
	PutbackRMByte(ModRM, (uint8_t)tmp1);
}

inline void I86_BASE::_invalid()
{
	/* i8086/i8088 ignore an invalid opcode. */
	/* i80186/i80188 probably also ignore an invalid opcode. */
	icount -= 10;
}

// Below are workaround VMs using another CPU.
#undef SetTF
#undef SetIF
#undef SetDF
#undef SetMD

#undef SetOFW_Add
#undef SetOFB_Add
#undef SetOFW_Sub
#undef SetOFB_Sub

#undef SetCFB
#undef SetCFW
#undef SetAF
#undef SetSF
#undef SetZF
#undef SetPF

#undef SetSZPF_Byte
#undef SetSZPF_Word

#undef ADDB
#undef ADDW

#undef SUBB
#undef SUBW

#undef ORB
#undef ORW

#undef ANDB
#undef ANDW

#undef XORB
#undef XORW

#undef CF
#undef SF
#undef ZF
#undef PF
#undef AF
#undef OF
#undef DF
#undef MD

/************************************************************************/

#undef AMASK

#undef read_mem_byte
#undef read_mem_word
#undef write_mem_byte
#undef write_mem_word

#undef read_port_byte
#undef read_port_word
#undef write_port_byte
#undef write_port_word

/************************************************************************/

#undef SegBase

#undef DefaultSeg
#undef DefaultBase

#undef GetMemB
#undef GetMemW
#undef PutMemB
#undef PutMemW

#undef ReadByte
#undef ReadWord
#undef WriteByte
#undef WriteWord
#undef FETCH
#undef FETCHOP
#undef FETCHWORD
#undef PUSH
#undef POP

/************************************************************************/

#undef CompressFlags
#undef ExpandFlags
#undef RegWord
#undef RegByte
#undef GetRMWord
#undef PutbackRMWord
#undef GetNextRMWord
#undef GetRMWordOffset
#undef GetRMByteOffset
#undef PutRMWord
#undef PutRMWordOffset
#undef PutRMByteOffset
#undef PutImmRMWord
#undef GetRMByte
#undef PutRMByte
#undef PutImmRMByte
#undef PutbackRMByte
#undef DEF_br8
#undef DEF_wr16
#undef DEF_r8b
#undef DEF_r16w
#undef DEF_ald8
#undef DEF_axd16

#endif
