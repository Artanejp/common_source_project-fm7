/*
	Skelton for retropc emulator

	Origin : MAME 0.145
	Author : Takeda.Toshiya
	Date   : 2012.02.15-

	[ Z80 ]
*/

#include "vm_template.h"
#include "../emu_template.h"
#include "z80.h"
//#ifdef USE_DEBUGGER
#include "debugger.h"
//#endif

//#ifndef CPU_START_ADDR
//#define CPU_START_ADDR	0
//#endif

#define NMI_REQ_BIT	0x80000000

#define CF	0x01
#define NF	0x02
#define PF	0x04
#define VF	PF
#define XF	0x08
#define HF	0x10
#define YF	0x20
#define ZF	0x40
#define SF	0x80

#define PCD	pc.d
#define PC	pc.w.l

#define SPD 	sp.d
#define SP	sp.w.l

#define AFD 	af.d
#define AF	af.w.l
#define A	af.b.h
#define F	af.b.l

#define BCD 	bc.d
#define BC	bc.w.l
#define B	bc.b.h
#define C	bc.b.l

#define DED 	de.d
#define DE	de.w.l
#define D	de.b.h
#define E	de.b.l

#define HLD 	hl.d
#define HL	hl.w.l
#define H	hl.b.h
#define L	hl.b.l

#define IXD 	ix.d
#define IX	ix.w.l
#define HX	ix.b.h
#define LX	ix.b.l

#define IYD 	iy.d
#define IY	iy.w.l
#define HY	iy.b.h
#define LY	iy.b.l

#define AF2	af2.w.l
#define A2	af2.b.h
#define F2	af2.b.l

#define BC2	bc2.w.l
#define B2	bc2.b.h
#define C2	bc2.b.l

#define DE2	de2.w.l
#define D2	de2.b.h
#define E2	de2.b.l

#define HL2	hl2.w.l
#define H2	hl2.b.h
#define L2	hl2.b.l

#define WZD	wz.d
#define WZ	wz.w.l
#define WZ_H	wz.b.h
#define WZ_L	wz.b.l

// opecode definitions

#define ENTER_HALT() do { \
	PC--; \
	after_halt = true; \
} while(0)

#define LEAVE_HALT() do { \
	if(after_halt) { \
		after_halt = false; \
		PC++; \
	} \
} while(0)

// opecode definitions

#define ENTER_HALT() do { \
	PC--; \
	after_halt = true; \
} while(0)

#define LEAVE_HALT() do { \
	if(after_halt) { \
		after_halt = false; \
		PC++; \
	} \
} while(0)

#define CLOCK_IN_OP(clock) do { \
	if(is_primary) { \
		if(wait || wait_icount > 0) { \
			wait_icount += (clock); \
		} \
		event_icount += (clock); \
	} \
} while(0)

#define UPDATE_EVENT_IN_OP(clock) do { \
	if(is_primary ) { \
		if(wait || wait_icount > 0) { \
			wait_icount += (clock); \
		} \
		event_icount += (clock); \
		if(!(wait || wait_icount > 0)) { \
			update_event_in_opecode(event_icount); \
			event_done_icount += event_icount; \
			event_icount = 0; \
		} \
	} \
} while(0)


Z80_INLINE uint8_t  Z80::RM8(uint32_t addr)
{
	UPDATE_EVENT_IN_OP(1);
	int wait_clock = 0;
	uint8_t val = d_mem->read_data8w(addr, &wait_clock);
	icount -= wait_clock;
	CLOCK_IN_OP(2 + wait_clock);
	return val;
}

Z80_INLINE void  Z80::WM8(uint32_t addr, uint8_t val)
{
	UPDATE_EVENT_IN_OP(1);
	int wait_clock = 0;
	d_mem->write_data8w(addr, val, &wait_clock);
	icount -= wait_clock;
	CLOCK_IN_OP(2 + wait_clock);
}

Z80_INLINE void  Z80::RM16(uint32_t addr, pair32_t *r)
{
	r->b.l = RM8(addr);
	r->b.h = RM8((addr + 1) & 0xffff);
}

Z80_INLINE void  Z80::WM16(uint32_t addr, pair32_t *r)
{
	WM8(addr, r->b.l);
	WM8((addr + 1) & 0xffff, r->b.h);
}

Z80_INLINE uint8_t  Z80::FETCHOP()
{
	unsigned pctmp = PCD;
	PC++;
	R++;

	// consider m1 cycle wait
	UPDATE_EVENT_IN_OP(1);
	int wait_clock = 0;
	uint8_t val = d_mem->fetch_op(pctmp, &wait_clock);
	icount -= wait_clock;
	CLOCK_IN_OP(3 + wait_clock);
	return val;
}

Z80_INLINE uint8_t  Z80::FETCH8()
{
	unsigned pctmp = PCD;
	PC++;
	return RM8(pctmp);
}

Z80_INLINE uint32_t  Z80::FETCH16()
{
	unsigned pctmp = PCD;
	PC += 2;
	return RM8(pctmp) | ((uint32_t)RM8((pctmp + 1) & 0xffff) << 8);
}

Z80_INLINE uint8_t  Z80::IN8(uint32_t addr)
{
	UPDATE_EVENT_IN_OP(2);
	int wait_clock = 0;
	uint8_t val = d_io->read_io8w(addr, &wait_clock);
	icount -= wait_clock;
	CLOCK_IN_OP(2 + wait_clock);
	return val;
}

Z80_INLINE void  Z80::OUT8(uint32_t addr, uint8_t val)
{
//#ifdef HAS_NSC800
	UPDATE_EVENT_IN_OP(2);
	if(has_nsc800) {
		if((addr & 0xff) == 0xbb) {
			icr = val;
			CLOCK_IN_OP(2);
			return;
		}
	}
//#endif
	int wait_clock = 0;
	d_io->write_io8w(addr, val, &wait_clock);
	icount -= wait_clock;
	CLOCK_IN_OP(2 + wait_clock);
}

#define EAX() do { \
	ea = (uint32_t)(uint16_t)(IX + (int8_t)FETCH8()); \
	WZ = ea; \
} while(0)

#define EAY() do { \
	ea = (uint32_t)(uint16_t)(IY + (int8_t)FETCH8()); \
	WZ = ea; \
} while(0)

#define POP(DR) do { \
	RM16(SPD, &DR); \
	SP += 2; \
} while(0)

#define PUSH(SR) do { \
	SP -= 2; \
	CLOCK_IN_OP(1); \
	WM16(SPD, &SR); \
} while(0)

#define JP() do { \
	PCD = FETCH16(); \
	WZ = PCD; \
} while(0)

#define JP_COND(cond) do { \
	if(cond) { \
		PCD = FETCH16(); \
		WZ = PCD; \
	} else { \
		WZ = FETCH16(); /* implicit do PC += 2 */ \
	} \
} while(0)

#define JR() do { \
	int8_t arg = (int8_t)FETCH8(); /* FETCH8() also increments PC */ \
	PC += arg; /* so don't do PC += FETCH8() */ \
	WZ = PC; \
} while(0)

#define JR_COND(cond, opcode) do { \
	if(cond) { \
		JR(); \
		icount -= cc_ex[opcode]; \
	} else FETCH8();			 \
} while(0)

#define CALL() do { \
	ea = FETCH16(); \
	WZ = ea; \
	PUSH(pc); \
	PCD = ea; \
} while(0)

#define CALL_COND(cond, opcode) do { \
	if(cond) { \
		ea = FETCH16(); \
		WZ = ea; \
		PUSH(pc); \
		PCD = ea; \
		icount -= cc_ex[opcode]; \
	} else { \
		WZ = FETCH16(); /* implicit call PC+=2; */ \
	} \
} while(0)

#define RET_COND(cond, opcode) do { \
	if(cond) { \
		POP(pc); \
		WZ = PC; \
		icount -= cc_ex[opcode]; \
	} \
} while(0)

#define RETN() do { \
	POP(pc); \
	WZ = PC; \
	iff1 = iff2; \
} while(0)

#define RETI() do { \
	POP(pc); \
	WZ = PC; \
	iff1 = iff2; \
	if(d_pic != NULL) d_pic->notify_intr_reti();	\
} while(0)

#define LD_R_A() do { \
	R = A; \
	R2 = A & 0x80; /* keep bit 7 of r */ \
} while(0)

#define LD_A_R() do { \
	A = (R & 0x7f) | R2; \
	F = (F & CF) | SZ[A] | (iff2 << 2); \
	after_ldair = true; \
} while(0)

#define LD_I_A() do { \
	I = A; \
} while(0)

#define LD_A_I() do { \
	A = I; \
	F = (F & CF) | SZ[A] | (iff2 << 2); \
	after_ldair = true; \
} while(0)

#define RST(addr) do { \
	PUSH(pc); \
	PCD = addr; \
	WZ = PC; \
} while(0)

Z80_INLINE uint8_t  Z80::INC(uint8_t value)
{
	uint8_t res = value + 1;
	F = (F & CF) | SZHV_inc[res];
	return (uint8_t)res;
}

Z80_INLINE uint8_t Z80::DEC(uint8_t value)
{
	uint8_t res = value - 1;
	F = (F & CF) | SZHV_dec[res];
	return res;
}

#define RLCA() do { \
	A = (A << 1) | (A >> 7); \
	F = (F & (SF | ZF | PF)) | (A & (YF | XF | CF)); \
} while(0)

#define RRCA() do { \
	F = (F & (SF | ZF | PF)) | (A & CF); \
	A = (A >> 1) | (A << 7); \
	F |= (A & (YF | XF)); \
} while(0)

#define RLA() do { \
	uint8_t res = (A << 1) | (F & CF); \
	uint8_t c = (A & 0x80) ? CF : 0; \
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF)); \
	A = res; \
} while(0)

#define RRA() do { \
	uint8_t res = (A >> 1) | (F << 7); \
	uint8_t c = (A & 0x01) ? CF : 0; \
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF)); \
	A = res; \
} while(0)

#define RRD() do { \
	uint8_t n = RM8(HL); \
	WZ = HL + 1; \
	CLOCK_IN_OP(4); \
	WM8(HL, (n >> 4) | (A << 4)); \
	A = (A & 0xf0) | (n & 0x0f); \
	F = (F & CF) | SZP[A]; \
} while(0)

#define RLD() do { \
	uint8_t n = RM8(HL); \
	WZ = HL + 1; \
	CLOCK_IN_OP(4); \
	WM8(HL, (n << 4) | (A & 0x0f)); \
	A = (A & 0xf0) | (n >> 4); \
	F = (F & CF) | SZP[A]; \
} while(0)

#define ADD(value) do { \
	uint32_t ah = AFD & 0xff00; \
	uint32_t res = (uint8_t)((ah >> 8) + value); \
	F = SZHVC_add[ah | res]; \
	A = res; \
} while(0)

#define ADC(value) do { \
	uint32_t ah = AFD & 0xff00, c = AFD & 1; \
	uint32_t res = (uint8_t)((ah >> 8) + value + c); \
	F = SZHVC_add[(c << 16) | ah | res]; \
	A = res; \
} while(0)

#define SUB(value) do { \
	uint32_t ah = AFD & 0xff00; \
	uint32_t res = (uint8_t)((ah >> 8) - value); \
	F = SZHVC_sub[ah | res]; \
	A = res; \
} while(0)

#define SBC(value) do { \
	uint32_t ah = AFD & 0xff00, c = AFD & 1; \
	uint32_t res = (uint8_t)((ah >> 8) - value - c); \
	F = SZHVC_sub[(c << 16) | ah | res]; \
	A = res; \
} while(0)

#define NEG() do { \
	uint8_t value = A; \
	A = 0; \
	SUB(value); \
} while(0)

#define DAA() do { \
	uint8_t a = A; \
	if(F & NF) { \
		if((F & HF) | ((A & 0xf) > 9)) a -= 6; \
		if((F & CF) | (A > 0x99)) a -= 0x60; \
	} else { \
		if((F & HF) | ((A & 0xf) > 9)) a += 6; \
		if((F & CF) | (A > 0x99)) a += 0x60; \
	} \
	F = (F & (CF | NF)) | (A > 0x99) | ((A ^ a) & HF) | SZP[a]; \
	A = a; \
} while(0)

#define AND(value) do { \
	A &= value; \
	F = SZP[A] | HF; \
} while(0)

#define OR(value) do { \
	A |= value; \
	F = SZP[A]; \
} while(0)

#define XOR(value) do { \
	A ^= value; \
	F = SZP[A]; \
} while(0)

#define CP(value) do { \
	unsigned val = value; \
	uint32_t ah = AFD & 0xff00; \
	uint32_t res = (uint8_t)((ah >> 8) - val); \
	F = (SZHVC_sub[ah | res] & ~(YF | XF)) | (val & (YF | XF)); \
} while(0)

#define EX_AF() do { \
	pair32_t tmp; \
	tmp = af; af = af2; af2 = tmp; \
} while(0)

#define EX_DE_HL() do { \
	pair32_t tmp; \
	tmp = de; de = hl; hl = tmp; \
} while(0)

#define EXX() do { \
	pair32_t tmp; \
	tmp = bc; bc = bc2; bc2 = tmp; \
	tmp = de; de = de2; de2 = tmp; \
	tmp = hl; hl = hl2; hl2 = tmp; \
} while(0)

#define EXSP(DR) do { \
	pair32_t tmp; \
	tmp.d = 0; \
	RM16(SPD, &tmp); \
	CLOCK_IN_OP(1); \
	WM16(SPD, &DR); \
	CLOCK_IN_OP(2); \
	DR = tmp; \
	WZ = DR.d; \
} while(0)

#define ADD16(DR, SR) do { \
	uint32_t res = DR.d + SR.d; \
	WZ = DR.d + 1; \
	F = (F & (SF | ZF | VF)) | (((DR.d ^ res ^ SR.d) >> 8) & HF) | ((res >> 16) & CF) | ((res >> 8) & (YF | XF)); \
	DR.w.l = (uint16_t)res; \
} while(0)

#define ADC16(Reg) do { \
	uint32_t res = HLD + Reg.d + (F & CF); \
	WZ = HL + 1; \
	F = (((HLD ^ res ^ Reg.d) >> 8) & HF) | ((res >> 16) & CF) | ((res >> 8) & (SF | YF | XF)) | ((res & 0xffff) ? 0 : ZF) | (((Reg.d ^ HLD ^ 0x8000) & (Reg.d ^ res) & 0x8000) >> 13); \
	HL = (uint16_t)res; \
} while(0)

#define SBC16(Reg) do { \
	uint32_t res = HLD - Reg.d - (F & CF); \
	WZ = HL + 1; \
	F = (((HLD ^ res ^ Reg.d) >> 8) & HF) | NF | ((res >> 16) & CF) | ((res >> 8) & (SF | YF | XF)) | ((res & 0xffff) ? 0 : ZF) | (((Reg.d ^ HLD) & (HLD ^ res) &0x8000) >> 13); \
	HL = (uint16_t)res; \
} while(0)

Z80_INLINE uint8_t  Z80::RLC(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (res >> 7)) & 0xff;
	F = SZP[res] | c;
	return res;
}

Z80_INLINE uint8_t  Z80::RRC(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res << 7)) & 0xff;
	F = SZP[res] | c;
	return res;
}

Z80_INLINE uint8_t  Z80::RL(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (F & CF)) & 0xff;
	F = SZP[res] | c;
	return res;
}

Z80_INLINE uint8_t  Z80::RR(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (F << 7)) & 0xff;
	F = SZP[res] | c;
	return res;
}

Z80_INLINE uint8_t  Z80::SLA(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = (res << 1) & 0xff;
	F = SZP[res] | c;
	return res;
}

Z80_INLINE uint8_t  Z80::SRA(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res & 0x80)) & 0xff;
	F = SZP[res] | c;
	return res;
}

Z80_INLINE uint8_t  Z80::SLL(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | 0x01) & 0xff;
	F = SZP[res] | c;
	return res;
}

Z80_INLINE uint8_t  Z80::SRL(uint8_t value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = (res >> 1) & 0xff;
	F = SZP[res] | c;
	return res;
}

#define BIT(bit, reg) do { \
	F = (F & CF) | HF | (SZ_BIT[reg & (1 << bit)] & ~(YF | XF)) | (reg & (YF | XF)); \
} while(0)

#define BIT_HL(bit, reg) do { \
	F = (F & CF) | HF | (SZ_BIT[reg & (1 << bit)] & ~(YF | XF)) | (WZ_H & (YF | XF)); \
} while(0)

#define BIT_XY(bit, reg) do { \
	F = (F & CF) | HF | (SZ_BIT[reg & (1 << bit)] & ~(YF | XF)) | ((ea >> 8) & (YF | XF)); \
} while(0)

Z80_INLINE uint8_t  Z80::RES(uint8_t bit, uint8_t value)
{
	return value & ~(1 << bit);
}

Z80_INLINE uint8_t Z80::SET(uint8_t bit, uint8_t value)
{
	return value | (1 << bit);
}

#define LDI() do { \
	uint8_t io = RM8(HL); \
	WM8(DE, io); \
	F &= SF | ZF | CF; \
	if((A + io) & 0x02) F |= YF; /* bit 1 -> flag 5 */ \
	if((A + io) & 0x08) F |= XF; /* bit 3 -> flag 3 */ \
	HL++; DE++; BC--; \
	if(BC) F |= VF; \
	CLOCK_IN_OP(2); \
} while(0)

#define CPI() do { \
	uint8_t val = RM8(HL); \
	uint8_t res = A - val; \
	WZ++; \
	HL++; BC--; \
	F = (F & CF) | (SZ[res] & ~(YF | XF)) | ((A ^ val ^ res) & HF) | NF; \
	if(F & HF) res -= 1; \
	if(res & 0x02) F |= YF; /* bit 1 -> flag 5 */ \
	if(res & 0x08) F |= XF; /* bit 3 -> flag 3 */ \
	if(BC) F |= VF; \
	CLOCK_IN_OP(5); \
} while(0)

#define INI() do { \
	CLOCK_IN_OP(1); \
	uint8_t io = IN8(BC); \
	WZ = BC + 1; \
	B--; \
	WM8(HL, io); \
	HL++; \
	F = SZ[B]; \
	unsigned t = (unsigned)((C + 1) & 0xff) + (unsigned)io; \
	if(io & SF) F |= NF; \
	if(t & 0x100) F |= HF | CF; \
	F |= SZP[(uint8_t)(t & 0x07) ^ B] & PF; \
} while(0)

#define OUTI() do { \
	CLOCK_IN_OP(1); \
	uint8_t io = RM8(HL); \
	B--; \
	WZ = BC + 1; \
	OUT8(BC, io); \
	HL++; \
	F = SZ[B]; \
	unsigned t = (unsigned)L + (unsigned)io;	\
	if(io & SF) F |= NF; \
	if(t & 0x100) F |= HF | CF; \
	F |= SZP[(uint8_t)(t & 0x07) ^ B] & PF; \
} while(0)

#define LDD() do { \
	uint8_t io = RM8(HL); \
	WM8(DE, io); \
	F &= SF | ZF | CF; \
	if((A + io) & 0x02) F |= YF; /* bit 1 -> flag 5 */ \
	if((A + io) & 0x08) F |= XF; /* bit 3 -> flag 3 */ \
	HL--; DE--; BC--; \
	if(BC) F |= VF; \
	CLOCK_IN_OP(2); \
} while(0)

#define CPD() do { \
	uint8_t val = RM8(HL); \
	uint8_t res = A - val; \
	WZ--; \
	HL--; BC--; \
	F = (F & CF) | (SZ[res] & ~(YF | XF)) | ((A ^ val ^ res) & HF) | NF; \
	if(F & HF) res -= 1; \
	if(res & 0x02) F |= YF; /* bit 1 -> flag 5 */ \
	if(res & 0x08) F |= XF; /* bit 3 -> flag 3 */ \
	if(BC) F |= VF; \
	CLOCK_IN_OP(5); \
} while(0)

#define IND() do { \
	CLOCK_IN_OP(1); \
	uint8_t io = IN8(BC); \
	WZ = BC - 1; \
	B--; \
	WM8(HL, io); \
	HL--; \
	F = SZ[B]; \
	unsigned t = ((unsigned)(C - 1) & 0xff) + (unsigned)io; \
	if(io & SF) F |= NF; \
	if(t & 0x100) F |= HF | CF; \
	F |= SZP[(uint8_t)(t & 0x07) ^ B] & PF; \
} while(0)

#define OUTD() do { \
	CLOCK_IN_OP(1); \
	uint8_t io = RM8(HL); \
	B--; \
	WZ = BC - 1; \
	OUT8(BC, io); \
	HL--; \
	F = SZ[B]; \
	unsigned t = (unsigned)L + (unsigned)io; \
	if(io & SF) F |= NF; \
	if(t & 0x100) F |= HF | CF; \
	F |= SZP[(uint8_t)(t & 0x07) ^ B] & PF; \
} while(0)

#define LDIR() do { \
	LDI(); \
	if(BC != 0) { \
		PC -= 2; \
		WZ = PC + 1; \
		icount -= cc_ex[0xb0]; \
	} \
} while(0)

#define CPIR() do { \
	CPI(); \
	if(BC != 0 && !(F & ZF)) { \
		PC -= 2; \
		WZ = PC + 1; \
		icount -= cc_ex[0xb1]; \
	} \
} while(0)

#define INIR() do { \
	INI(); \
	if(B != 0) { \
		PC -= 2; \
		icount -= cc_ex[0xb2]; \
	} \
} while(0)

#define OTIR() do { \
	OUTI(); \
	if(B != 0) { \
		PC -= 2; \
		icount -= cc_ex[0xb3]; \
	} \
} while(0)

#define LDDR() do { \
	LDD(); \
	if(BC != 0) { \
		PC -= 2; \
		WZ = PC + 1; \
		icount -= cc_ex[0xb8]; \
	} \
} while(0)

#define CPDR() do { \
	CPD(); \
	if(BC != 0 && !(F & ZF)) { \
		PC -= 2; \
		WZ = PC + 1; \
		icount -= cc_ex[0xb9]; \
	} \
} while(0)

#define INDR() do { \
	IND(); \
	if(B != 0) { \
		PC -= 2; \
		icount -= cc_ex[0xba]; \
	} \
} while(0)

#define OTDR() do { \
	OUTD(); \
	if(B != 0) { \
		PC -= 2; \
		icount -= cc_ex[0xbb]; \
	} \
} while(0)

#define EI() do { \
	iff1 = iff2 = 1; \
	after_ei = true; \
} while(0)

void Z80::OP_CB(uint8_t code)
{
	// Done: M1 + M1
	uint8_t v;

	icount -= cc_cb[code];

	switch(code) {
	case 0x00: B = RLC(B); break;						/* RLC  B           */
	case 0x01: C = RLC(C); break;						/* RLC  C           */
	case 0x02: D = RLC(D); break;						/* RLC  D           */
	case 0x03: E = RLC(E); break;						/* RLC  E           */
	case 0x04: H = RLC(H); break;						/* RLC  H           */
	case 0x05: L = RLC(L); break;						/* RLC  L           */
	case 0x06: v = RLC(RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;		/* RLC  (HL)        */
	case 0x07: A = RLC(A); break;						/* RLC  A           */
	case 0x08: B = RRC(B); break;						/* RRC  B           */
	case 0x09: C = RRC(C); break;						/* RRC  C           */
	case 0x0a: D = RRC(D); break;						/* RRC  D           */
	case 0x0b: E = RRC(E); break;						/* RRC  E           */
	case 0x0c: H = RRC(H); break;						/* RRC  H           */
	case 0x0d: L = RRC(L); break;						/* RRC  L           */
	case 0x0e: v = RRC(RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;		/* RRC  (HL)        */
	case 0x0f: A = RRC(A); break;						/* RRC  A           */
	case 0x10: B = RL(B); break;						/* RL   B           */
	case 0x11: C = RL(C); break;						/* RL   C           */
	case 0x12: D = RL(D); break;						/* RL   D           */
	case 0x13: E = RL(E); break;						/* RL   E           */
	case 0x14: H = RL(H); break;						/* RL   H           */
	case 0x15: L = RL(L); break;						/* RL   L           */
	case 0x16: v = RL(RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;		/* RL   (HL)        */
	case 0x17: A = RL(A); break;						/* RL   A           */
	case 0x18: B = RR(B); break;						/* RR   B           */
	case 0x19: C = RR(C); break;						/* RR   C           */
	case 0x1a: D = RR(D); break;						/* RR   D           */
	case 0x1b: E = RR(E); break;						/* RR   E           */
	case 0x1c: H = RR(H); break;						/* RR   H           */
	case 0x1d: L = RR(L); break;						/* RR   L           */
	case 0x1e: v = RR(RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;		/* RR   (HL)        */
	case 0x1f: A = RR(A); break;						/* RR   A           */
	case 0x20: B = SLA(B); break;						/* SLA  B           */
	case 0x21: C = SLA(C); break;						/* SLA  C           */
	case 0x22: D = SLA(D); break;						/* SLA  D           */
	case 0x23: E = SLA(E); break;						/* SLA  E           */
	case 0x24: H = SLA(H); break;						/* SLA  H           */
	case 0x25: L = SLA(L); break;						/* SLA  L           */
	case 0x26: v = SLA(RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;		/* SLA  (HL)        */
	case 0x27: A = SLA(A); break;						/* SLA  A           */
	case 0x28: B = SRA(B); break;						/* SRA  B           */
	case 0x29: C = SRA(C); break;						/* SRA  C           */
	case 0x2a: D = SRA(D); break;						/* SRA  D           */
	case 0x2b: E = SRA(E); break;						/* SRA  E           */
	case 0x2c: H = SRA(H); break;						/* SRA  H           */
	case 0x2d: L = SRA(L); break;						/* SRA  L           */
	case 0x2e: v = SRA(RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;		/* SRA  (HL)        */
	case 0x2f: A = SRA(A); break;						/* SRA  A           */
	case 0x30: B = SLL(B); break;						/* SLL  B           */
	case 0x31: C = SLL(C); break;						/* SLL  C           */
	case 0x32: D = SLL(D); break;						/* SLL  D           */
	case 0x33: E = SLL(E); break;						/* SLL  E           */
	case 0x34: H = SLL(H); break;						/* SLL  H           */
	case 0x35: L = SLL(L); break;						/* SLL  L           */
	case 0x36: v = SLL(RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;		/* SLL  (HL)        */
	case 0x37: A = SLL(A); break;						/* SLL  A           */
	case 0x38: B = SRL(B); break;						/* SRL  B           */
	case 0x39: C = SRL(C); break;						/* SRL  C           */
	case 0x3a: D = SRL(D); break;						/* SRL  D           */
	case 0x3b: E = SRL(E); break;						/* SRL  E           */
	case 0x3c: H = SRL(H); break;						/* SRL  H           */
	case 0x3d: L = SRL(L); break;						/* SRL  L           */
	case 0x3e: v = SRL(RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;		/* SRL  (HL)        */
	case 0x3f: A = SRL(A); break;						/* SRL  A           */
	case 0x40: BIT(0, B); break;						/* BIT  0,B         */
	case 0x41: BIT(0, C); break;						/* BIT  0,C         */
	case 0x42: BIT(0, D); break;						/* BIT  0,D         */
	case 0x43: BIT(0, E); break;						/* BIT  0,E         */
	case 0x44: BIT(0, H); break;						/* BIT  0,H         */
	case 0x45: BIT(0, L); break;						/* BIT  0,L         */
	case 0x46: v = RM8(HL); CLOCK_IN_OP(1); BIT_HL(0, v); break;		/* BIT  0,(HL)      */
	case 0x47: BIT(0, A); break;						/* BIT  0,A         */
	case 0x48: BIT(1, B); break;						/* BIT  1,B         */
	case 0x49: BIT(1, C); break;						/* BIT  1,C         */
	case 0x4a: BIT(1, D); break;						/* BIT  1,D         */
	case 0x4b: BIT(1, E); break;						/* BIT  1,E         */
	case 0x4c: BIT(1, H); break;						/* BIT  1,H         */
	case 0x4d: BIT(1, L); break;						/* BIT  1,L         */
	case 0x4e: v = RM8(HL); CLOCK_IN_OP(1); BIT_HL(1, v); break;		/* BIT  1,(HL)      */
	case 0x4f: BIT(1, A); break;						/* BIT  1,A         */
	case 0x50: BIT(2, B); break;						/* BIT  2,B         */
	case 0x51: BIT(2, C); break;						/* BIT  2,C         */
	case 0x52: BIT(2, D); break;						/* BIT  2,D         */
	case 0x53: BIT(2, E); break;						/* BIT  2,E         */
	case 0x54: BIT(2, H); break;						/* BIT  2,H         */
	case 0x55: BIT(2, L); break;						/* BIT  2,L         */
	case 0x56: v = RM8(HL); CLOCK_IN_OP(1); BIT_HL(2, v); break;		/* BIT  2,(HL)      */
	case 0x57: BIT(2, A); break;						/* BIT  2,A         */
	case 0x58: BIT(3, B); break;						/* BIT  3,B         */
	case 0x59: BIT(3, C); break;						/* BIT  3,C         */
	case 0x5a: BIT(3, D); break;						/* BIT  3,D         */
	case 0x5b: BIT(3, E); break;						/* BIT  3,E         */
	case 0x5c: BIT(3, H); break;						/* BIT  3,H         */
	case 0x5d: BIT(3, L); break;						/* BIT  3,L         */
	case 0x5e: v = RM8(HL); CLOCK_IN_OP(1); BIT_HL(3, v); break;		/* BIT  3,(HL)      */
	case 0x5f: BIT(3, A); break;						/* BIT  3,A         */
	case 0x60: BIT(4, B); break;						/* BIT  4,B         */
	case 0x61: BIT(4, C); break;						/* BIT  4,C         */
	case 0x62: BIT(4, D); break;						/* BIT  4,D         */
	case 0x63: BIT(4, E); break;						/* BIT  4,E         */
	case 0x64: BIT(4, H); break;						/* BIT  4,H         */
	case 0x65: BIT(4, L); break;						/* BIT  4,L         */
	case 0x66: v = RM8(HL); CLOCK_IN_OP(1); BIT_HL(4, v); break;		/* BIT  4,(HL)      */
	case 0x67: BIT(4, A); break;						/* BIT  4,A         */
	case 0x68: BIT(5, B); break;						/* BIT  5,B         */
	case 0x69: BIT(5, C); break;						/* BIT  5,C         */
	case 0x6a: BIT(5, D); break;						/* BIT  5,D         */
	case 0x6b: BIT(5, E); break;						/* BIT  5,E         */
	case 0x6c: BIT(5, H); break;						/* BIT  5,H         */
	case 0x6d: BIT(5, L); break;						/* BIT  5,L         */
	case 0x6e: v = RM8(HL); CLOCK_IN_OP(1); BIT_HL(5, v); break;		/* BIT  5,(HL)      */
	case 0x6f: BIT(5, A); break;						/* BIT  5,A         */
	case 0x70: BIT(6, B); break;						/* BIT  6,B         */
	case 0x71: BIT(6, C); break;						/* BIT  6,C         */
	case 0x72: BIT(6, D); break;						/* BIT  6,D         */
	case 0x73: BIT(6, E); break;						/* BIT  6,E         */
	case 0x74: BIT(6, H); break;						/* BIT  6,H         */
	case 0x75: BIT(6, L); break;						/* BIT  6,L         */
	case 0x76: v = RM8(HL); CLOCK_IN_OP(1); BIT_HL(6, v); break;		/* BIT  6,(HL)      */
	case 0x77: BIT(6, A); break;						/* BIT  6,A         */
	case 0x78: BIT(7, B); break;						/* BIT  7,B         */
	case 0x79: BIT(7, C); break;						/* BIT  7,C         */
	case 0x7a: BIT(7, D); break;						/* BIT  7,D         */
	case 0x7b: BIT(7, E); break;						/* BIT  7,E         */
	case 0x7c: BIT(7, H); break;						/* BIT  7,H         */
	case 0x7d: BIT(7, L); break;						/* BIT  7,L         */
	case 0x7e: v = RM8(HL); CLOCK_IN_OP(1); BIT_HL(7, v); break;		/* BIT  7,(HL)      */
	case 0x7f: BIT(7, A); break;						/* BIT  7,A         */
	case 0x80: B = RES(0, B); break;					/* RES  0,B         */
	case 0x81: C = RES(0, C); break;					/* RES  0,C         */
	case 0x82: D = RES(0, D); break;					/* RES  0,D         */
	case 0x83: E = RES(0, E); break;					/* RES  0,E         */
	case 0x84: H = RES(0, H); break;					/* RES  0,H         */
	case 0x85: L = RES(0, L); break;					/* RES  0,L         */
	case 0x86: v = RES(0, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* RES  0,(HL)      */
	case 0x87: A = RES(0, A); break;					/* RES  0,A         */
	case 0x88: B = RES(1, B); break;					/* RES  1,B         */
	case 0x89: C = RES(1, C); break;					/* RES  1,C         */
	case 0x8a: D = RES(1, D); break;					/* RES  1,D         */
	case 0x8b: E = RES(1, E); break;					/* RES  1,E         */
	case 0x8c: H = RES(1, H); break;					/* RES  1,H         */
	case 0x8d: L = RES(1, L); break;					/* RES  1,L         */
	case 0x8e: v = RES(1, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* RES  1,(HL)      */
	case 0x8f: A = RES(1, A); break;					/* RES  1,A         */
	case 0x90: B = RES(2, B); break;					/* RES  2,B         */
	case 0x91: C = RES(2, C); break;					/* RES  2,C         */
	case 0x92: D = RES(2, D); break;					/* RES  2,D         */
	case 0x93: E = RES(2, E); break;					/* RES  2,E         */
	case 0x94: H = RES(2, H); break;					/* RES  2,H         */
	case 0x95: L = RES(2, L); break;					/* RES  2,L         */
	case 0x96: v = RES(2, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* RES  2,(HL)      */
	case 0x97: A = RES(2, A); break;					/* RES  2,A         */
	case 0x98: B = RES(3, B); break;					/* RES  3,B         */
	case 0x99: C = RES(3, C); break;					/* RES  3,C         */
	case 0x9a: D = RES(3, D); break;					/* RES  3,D         */
	case 0x9b: E = RES(3, E); break;					/* RES  3,E         */
	case 0x9c: H = RES(3, H); break;					/* RES  3,H         */
	case 0x9d: L = RES(3, L); break;					/* RES  3,L         */
	case 0x9e: v = RES(3, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* RES  3,(HL)      */
	case 0x9f: A = RES(3, A); break;					/* RES  3,A         */
	case 0xa0: B = RES(4,	B); break;					/* RES  4,B         */
	case 0xa1: C = RES(4,	C); break;					/* RES  4,C         */
	case 0xa2: D = RES(4,	D); break;					/* RES  4,D         */
	case 0xa3: E = RES(4,	E); break;					/* RES  4,E         */
	case 0xa4: H = RES(4,	H); break;					/* RES  4,H         */
	case 0xa5: L = RES(4,	L); break;					/* RES  4,L         */
	case 0xa6: v = RES(4, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* RES  4,(HL)      */
	case 0xa7: A = RES(4,	A); break;					/* RES  4,A         */
	case 0xa8: B = RES(5, B); break;					/* RES  5,B         */
	case 0xa9: C = RES(5, C); break;					/* RES  5,C         */
	case 0xaa: D = RES(5, D); break;					/* RES  5,D         */
	case 0xab: E = RES(5, E); break;					/* RES  5,E         */
	case 0xac: H = RES(5, H); break;					/* RES  5,H         */
	case 0xad: L = RES(5, L); break;					/* RES  5,L         */
	case 0xae: v = RES(5, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* RES  5,(HL)      */
	case 0xaf: A = RES(5, A); break;					/* RES  5,A         */
	case 0xb0: B = RES(6, B); break;					/* RES  6,B         */
	case 0xb1: C = RES(6, C); break;					/* RES  6,C         */
	case 0xb2: D = RES(6, D); break;					/* RES  6,D         */
	case 0xb3: E = RES(6, E); break;					/* RES  6,E         */
	case 0xb4: H = RES(6, H); break;					/* RES  6,H         */
	case 0xb5: L = RES(6, L); break;					/* RES  6,L         */
	case 0xb6: v = RES(6, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* RES  6,(HL)      */
	case 0xb7: A = RES(6, A); break;					/* RES  6,A         */
	case 0xb8: B = RES(7, B); break;					/* RES  7,B         */
	case 0xb9: C = RES(7, C); break;					/* RES  7,C         */
	case 0xba: D = RES(7, D); break;					/* RES  7,D         */
	case 0xbb: E = RES(7, E); break;					/* RES  7,E         */
	case 0xbc: H = RES(7, H); break;					/* RES  7,H         */
	case 0xbd: L = RES(7, L); break;					/* RES  7,L         */
	case 0xbe: v = RES(7, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* RES  7,(HL)      */
	case 0xbf: A = RES(7, A); break;					/* RES  7,A         */
	case 0xc0: B = SET(0, B); break;					/* SET  0,B         */
	case 0xc1: C = SET(0, C); break;					/* SET  0,C         */
	case 0xc2: D = SET(0, D); break;					/* SET  0,D         */
	case 0xc3: E = SET(0, E); break;					/* SET  0,E         */
	case 0xc4: H = SET(0, H); break;					/* SET  0,H         */
	case 0xc5: L = SET(0, L); break;					/* SET  0,L         */
	case 0xc6: v = SET(0, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* SET  0,(HL)      */
	case 0xc7: A = SET(0, A); break;					/* SET  0,A         */
	case 0xc8: B = SET(1, B); break;					/* SET  1,B         */
	case 0xc9: C = SET(1, C); break;					/* SET  1,C         */
	case 0xca: D = SET(1, D); break;					/* SET  1,D         */
	case 0xcb: E = SET(1, E); break;					/* SET  1,E         */
	case 0xcc: H = SET(1, H); break;					/* SET  1,H         */
	case 0xcd: L = SET(1, L); break;					/* SET  1,L         */
	case 0xce: v = SET(1, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* SET  1,(HL)      */
	case 0xcf: A = SET(1, A); break;					/* SET  1,A         */
	case 0xd0: B = SET(2, B); break;					/* SET  2,B         */
	case 0xd1: C = SET(2, C); break;					/* SET  2,C         */
	case 0xd2: D = SET(2, D); break;					/* SET  2,D         */
	case 0xd3: E = SET(2, E); break;					/* SET  2,E         */
	case 0xd4: H = SET(2, H); break;					/* SET  2,H         */
	case 0xd5: L = SET(2, L); break;					/* SET  2,L         */
	case 0xd6: v = SET(2, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* SET  2,(HL)      */
	case 0xd7: A = SET(2, A); break;					/* SET  2,A         */
	case 0xd8: B = SET(3, B); break;					/* SET  3,B         */
	case 0xd9: C = SET(3, C); break;					/* SET  3,C         */
	case 0xda: D = SET(3, D); break;					/* SET  3,D         */
	case 0xdb: E = SET(3, E); break;					/* SET  3,E         */
	case 0xdc: H = SET(3, H); break;					/* SET  3,H         */
	case 0xdd: L = SET(3, L); break;					/* SET  3,L         */
	case 0xde: v = SET(3, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* SET  3,(HL)      */
	case 0xdf: A = SET(3, A); break;					/* SET  3,A         */
	case 0xe0: B = SET(4, B); break;					/* SET  4,B         */
	case 0xe1: C = SET(4, C); break;					/* SET  4,C         */
	case 0xe2: D = SET(4, D); break;					/* SET  4,D         */
	case 0xe3: E = SET(4, E); break;					/* SET  4,E         */
	case 0xe4: H = SET(4, H); break;					/* SET  4,H         */
	case 0xe5: L = SET(4, L); break;					/* SET  4,L         */
	case 0xe6: v = SET(4, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* SET  4,(HL)      */
	case 0xe7: A = SET(4, A); break;					/* SET  4,A         */
	case 0xe8: B = SET(5, B); break;					/* SET  5,B         */
	case 0xe9: C = SET(5, C); break;					/* SET  5,C         */
	case 0xea: D = SET(5, D); break;					/* SET  5,D         */
	case 0xeb: E = SET(5, E); break;					/* SET  5,E         */
	case 0xec: H = SET(5, H); break;					/* SET  5,H         */
	case 0xed: L = SET(5, L); break;					/* SET  5,L         */
	case 0xee: v = SET(5, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* SET  5,(HL)      */
	case 0xef: A = SET(5, A); break;					/* SET  5,A         */
	case 0xf0: B = SET(6, B); break;					/* SET  6,B         */
	case 0xf1: C = SET(6, C); break;					/* SET  6,C         */
	case 0xf2: D = SET(6, D); break;					/* SET  6,D         */
	case 0xf3: E = SET(6, E); break;					/* SET  6,E         */
	case 0xf4: H = SET(6, H); break;					/* SET  6,H         */
	case 0xf5: L = SET(6, L); break;					/* SET  6,L         */
	case 0xf6: v = SET(6, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* SET  6,(HL)      */
	case 0xf7: A = SET(6, A); break;					/* SET  6,A         */
	case 0xf8: B = SET(7, B); break;					/* SET  7,B         */
	case 0xf9: C = SET(7, C); break;					/* SET  7,C         */
	case 0xfa: D = SET(7, D); break;					/* SET  7,D         */
	case 0xfb: E = SET(7, E); break;					/* SET  7,E         */
	case 0xfc: H = SET(7, H); break;					/* SET  7,H         */
	case 0xfd: L = SET(7, L); break;					/* SET  7,L         */
	case 0xfe: v = SET(7, RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;	/* SET  7,(HL)      */
	case 0xff: A = SET(7, A); break;					/* SET  7,A         */
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}


void Z80::OP_XY(uint8_t code)
{
	// Done: M1 + M1 + R + R + 2
	uint8_t v;

	icount -= cc_xycb[code];

	switch(code) {
	case 0x00: B = RLC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;		/* RLC  B=(XY+o)    */
	case 0x01: C = RLC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;		/* RLC  C=(XY+o)    */
	case 0x02: D = RLC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;		/* RLC  D=(XY+o)    */
	case 0x03: E = RLC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;		/* RLC  E=(XY+o)    */
	case 0x04: H = RLC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;		/* RLC  H=(XY+o)    */
	case 0x05: L = RLC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;		/* RLC  L=(XY+o)    */
	case 0x06: v = RLC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;		/* RLC  (XY+o)      */
	case 0x07: A = RLC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;		/* RLC  A=(XY+o)    */
	case 0x08: B = RRC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;		/* RRC  B=(XY+o)    */
	case 0x09: C = RRC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;		/* RRC  C=(XY+o)    */
	case 0x0a: D = RRC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;		/* RRC  D=(XY+o)    */
	case 0x0b: E = RRC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;		/* RRC  E=(XY+o)    */
	case 0x0c: H = RRC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;		/* RRC  H=(XY+o)    */
	case 0x0d: L = RRC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;		/* RRC  L=(XY+o)    */
	case 0x0e: v = RRC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;		/* RRC  (XY+o)      */
	case 0x0f: A = RRC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;		/* RRC  A=(XY+o)    */
	case 0x10: B = RL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;		/* RL   B=(XY+o)    */
	case 0x11: C = RL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;		/* RL   C=(XY+o)    */
	case 0x12: D = RL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;		/* RL   D=(XY+o)    */
	case 0x13: E = RL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;		/* RL   E=(XY+o)    */
	case 0x14: H = RL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;		/* RL   H=(XY+o)    */
	case 0x15: L = RL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;		/* RL   L=(XY+o)    */
	case 0x16: v = RL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;		/* RL   (XY+o)      */
	case 0x17: A = RL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;		/* RL   A=(XY+o)    */
	case 0x18: B = RR(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;		/* RR   B=(XY+o)    */
	case 0x19: C = RR(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;		/* RR   C=(XY+o)    */
	case 0x1a: D = RR(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;		/* RR   D=(XY+o)    */
	case 0x1b: E = RR(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;		/* RR   E=(XY+o)    */
	case 0x1c: H = RR(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;		/* RR   H=(XY+o)    */
	case 0x1d: L = RR(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;		/* RR   L=(XY+o)    */
	case 0x1e: v = RR(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;		/* RR   (XY+o)      */
	case 0x1f: A = RR(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;		/* RR   A=(XY+o)    */
	case 0x20: B = SLA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;		/* SLA  B=(XY+o)    */
	case 0x21: C = SLA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;		/* SLA  C=(XY+o)    */
	case 0x22: D = SLA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;		/* SLA  D=(XY+o)    */
	case 0x23: E = SLA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;		/* SLA  E=(XY+o)    */
	case 0x24: H = SLA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;		/* SLA  H=(XY+o)    */
	case 0x25: L = SLA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;		/* SLA  L=(XY+o)    */
	case 0x26: v = SLA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;		/* SLA  (XY+o)      */
	case 0x27: A = SLA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;		/* SLA  A=(XY+o)    */
	case 0x28: B = SRA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;		/* SRA  B=(XY+o)    */
	case 0x29: C = SRA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;		/* SRA  C=(XY+o)    */
	case 0x2a: D = SRA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;		/* SRA  D=(XY+o)    */
	case 0x2b: E = SRA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;		/* SRA  E=(XY+o)    */
	case 0x2c: H = SRA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;		/* SRA  H=(XY+o)    */
	case 0x2d: L = SRA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;		/* SRA  L=(XY+o)    */
	case 0x2e: v = SRA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;		/* SRA  (XY+o)      */
	case 0x2f: A = SRA(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;		/* SRA  A=(XY+o)    */
	case 0x30: B = SLL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;		/* SLL  B=(XY+o)    */
	case 0x31: C = SLL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;		/* SLL  C=(XY+o)    */
	case 0x32: D = SLL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;		/* SLL  D=(XY+o)    */
	case 0x33: E = SLL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;		/* SLL  E=(XY+o)    */
	case 0x34: H = SLL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;		/* SLL  H=(XY+o)    */
	case 0x35: L = SLL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;		/* SLL  L=(XY+o)    */
	case 0x36: v = SLL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;		/* SLL  (XY+o)      */
	case 0x37: A = SLL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;		/* SLL  A=(XY+o)    */
	case 0x38: B = SRL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;		/* SRL  B=(XY+o)    */
	case 0x39: C = SRL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;		/* SRL  C=(XY+o)    */
	case 0x3a: D = SRL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;		/* SRL  D=(XY+o)    */
	case 0x3b: E = SRL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;		/* SRL  E=(XY+o)    */
	case 0x3c: H = SRL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;		/* SRL  H=(XY+o)    */
	case 0x3d: L = SRL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;		/* SRL  L=(XY+o)    */
	case 0x3e: v = SRL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;		/* SRL  (XY+o)      */
	case 0x3f: A = SRL(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;		/* SRL  A=(XY+o)    */
	case 0x40: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(0, v); break;		/* BIT  0,(XY+o)    */
	case 0x41: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(0, v); break;		/* BIT  0,(XY+o)    */
	case 0x42: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(0, v); break;		/* BIT  0,(XY+o)    */
	case 0x43: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(0, v); break;		/* BIT  0,(XY+o)    */
	case 0x44: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(0, v); break;		/* BIT  0,(XY+o)    */
	case 0x45: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(0, v); break;		/* BIT  0,(XY+o)    */
	case 0x46: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(0, v); break;		/* BIT  0,(XY+o)    */
	case 0x47: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(0, v); break;		/* BIT  0,(XY+o)    */
	case 0x48: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(1, v); break;		/* BIT  1,(XY+o)    */
	case 0x49: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(1, v); break;		/* BIT  1,(XY+o)    */
	case 0x4a: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(1, v); break;		/* BIT  1,(XY+o)    */
	case 0x4b: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(1, v); break;		/* BIT  1,(XY+o)    */
	case 0x4c: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(1, v); break;		/* BIT  1,(XY+o)    */
	case 0x4d: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(1, v); break;		/* BIT  1,(XY+o)    */
	case 0x4e: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(1, v); break;		/* BIT  1,(XY+o)    */
	case 0x4f: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(1, v); break;		/* BIT  1,(XY+o)    */
	case 0x50: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(2, v); break;		/* BIT  2,(XY+o)    */
	case 0x51: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(2, v); break;		/* BIT  2,(XY+o)    */
	case 0x52: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(2, v); break;		/* BIT  2,(XY+o)    */
	case 0x53: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(2, v); break;		/* BIT  2,(XY+o)    */
	case 0x54: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(2, v); break;		/* BIT  2,(XY+o)    */
	case 0x55: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(2, v); break;		/* BIT  2,(XY+o)    */
	case 0x56: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(2, v); break;		/* BIT  2,(XY+o)    */
	case 0x57: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(2, v); break;		/* BIT  2,(XY+o)    */
	case 0x58: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(3, v); break;		/* BIT  3,(XY+o)    */
	case 0x59: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(3, v); break;		/* BIT  3,(XY+o)    */
	case 0x5a: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(3, v); break;		/* BIT  3,(XY+o)    */
	case 0x5b: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(3, v); break;		/* BIT  3,(XY+o)    */
	case 0x5c: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(3, v); break;		/* BIT  3,(XY+o)    */
	case 0x5d: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(3, v); break;		/* BIT  3,(XY+o)    */
	case 0x5e: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(3, v); break;		/* BIT  3,(XY+o)    */
	case 0x5f: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(3, v); break;		/* BIT  3,(XY+o)    */
	case 0x60: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(4, v); break;		/* BIT  4,(XY+o)    */
	case 0x61: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(4, v); break;		/* BIT  4,(XY+o)    */
	case 0x62: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(4, v); break;		/* BIT  4,(XY+o)    */
	case 0x63: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(4, v); break;		/* BIT  4,(XY+o)    */
	case 0x64: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(4, v); break;		/* BIT  4,(XY+o)    */
	case 0x65: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(4, v); break;		/* BIT  4,(XY+o)    */
	case 0x66: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(4, v); break;		/* BIT  4,(XY+o)    */
	case 0x67: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(4, v); break;		/* BIT  4,(XY+o)    */
	case 0x68: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(5, v); break;		/* BIT  5,(XY+o)    */
	case 0x69: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(5, v); break;		/* BIT  5,(XY+o)    */
	case 0x6a: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(5, v); break;		/* BIT  5,(XY+o)    */
	case 0x6b: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(5, v); break;		/* BIT  5,(XY+o)    */
	case 0x6c: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(5, v); break;		/* BIT  5,(XY+o)    */
	case 0x6d: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(5, v); break;		/* BIT  5,(XY+o)    */
	case 0x6e: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(5, v); break;		/* BIT  5,(XY+o)    */
	case 0x6f: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(5, v); break;		/* BIT  5,(XY+o)    */
	case 0x70: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(6, v); break;		/* BIT  6,(XY+o)    */
	case 0x71: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(6, v); break;		/* BIT  6,(XY+o)    */
	case 0x72: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(6, v); break;		/* BIT  6,(XY+o)    */
	case 0x73: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(6, v); break;		/* BIT  6,(XY+o)    */
	case 0x74: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(6, v); break;		/* BIT  6,(XY+o)    */
	case 0x75: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(6, v); break;		/* BIT  6,(XY+o)    */
	case 0x76: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(6, v); break;		/* BIT  6,(XY+o)    */
	case 0x77: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(6, v); break;		/* BIT  6,(XY+o)    */
	case 0x78: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(7, v); break;		/* BIT  7,(XY+o)    */
	case 0x79: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(7, v); break;		/* BIT  7,(XY+o)    */
	case 0x7a: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(7, v); break;		/* BIT  7,(XY+o)    */
	case 0x7b: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(7, v); break;		/* BIT  7,(XY+o)    */
	case 0x7c: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(7, v); break;		/* BIT  7,(XY+o)    */
	case 0x7d: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(7, v); break;		/* BIT  7,(XY+o)    */
	case 0x7e: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(7, v); break;		/* BIT  7,(XY+o)    */
	case 0x7f: v = RM8(ea); CLOCK_IN_OP(1); BIT_XY(7, v); break;		/* BIT  7,(XY+o)    */
	case 0x80: B = RES(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* RES  0,B=(XY+o)  */
	case 0x81: C = RES(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* RES  0,C=(XY+o)  */
	case 0x82: D = RES(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* RES  0,D=(XY+o)  */
	case 0x83: E = RES(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* RES  0,E=(XY+o)  */
	case 0x84: H = RES(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* RES  0,H=(XY+o)  */
	case 0x85: L = RES(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* RES  0,L=(XY+o)  */
	case 0x86: v = RES(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* RES  0,(XY+o)    */
	case 0x87: A = RES(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* RES  0,A=(XY+o)  */
	case 0x88: B = RES(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* RES  1,B=(XY+o)  */
	case 0x89: C = RES(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* RES  1,C=(XY+o)  */
	case 0x8a: D = RES(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* RES  1,D=(XY+o)  */
	case 0x8b: E = RES(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* RES  1,E=(XY+o)  */
	case 0x8c: H = RES(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* RES  1,H=(XY+o)  */
	case 0x8d: L = RES(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* RES  1,L=(XY+o)  */
	case 0x8e: v = RES(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* RES  1,(XY+o)    */
	case 0x8f: A = RES(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* RES  1,A=(XY+o)  */
	case 0x90: B = RES(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* RES  2,B=(XY+o)  */
	case 0x91: C = RES(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* RES  2,C=(XY+o)  */
	case 0x92: D = RES(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* RES  2,D=(XY+o)  */
	case 0x93: E = RES(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* RES  2,E=(XY+o)  */
	case 0x94: H = RES(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* RES  2,H=(XY+o)  */
	case 0x95: L = RES(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* RES  2,L=(XY+o)  */
	case 0x96: v = RES(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* RES  2,(XY+o)    */
	case 0x97: A = RES(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* RES  2,A=(XY+o)  */
	case 0x98: B = RES(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* RES  3,B=(XY+o)  */
	case 0x99: C = RES(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* RES  3,C=(XY+o)  */
	case 0x9a: D = RES(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* RES  3,D=(XY+o)  */
	case 0x9b: E = RES(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* RES  3,E=(XY+o)  */
	case 0x9c: H = RES(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* RES  3,H=(XY+o)  */
	case 0x9d: L = RES(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* RES  3,L=(XY+o)  */
	case 0x9e: v = RES(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* RES  3,(XY+o)    */
	case 0x9f: A = RES(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* RES  3,A=(XY+o)  */
	case 0xa0: B = RES(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* RES  4,B=(XY+o)  */
	case 0xa1: C = RES(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* RES  4,C=(XY+o)  */
	case 0xa2: D = RES(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* RES  4,D=(XY+o)  */
	case 0xa3: E = RES(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* RES  4,E=(XY+o)  */
	case 0xa4: H = RES(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* RES  4,H=(XY+o)  */
	case 0xa5: L = RES(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* RES  4,L=(XY+o)  */
	case 0xa6: v = RES(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* RES  4,(XY+o)    */
	case 0xa7: A = RES(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* RES  4,A=(XY+o)  */
	case 0xa8: B = RES(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* RES  5,B=(XY+o)  */
	case 0xa9: C = RES(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* RES  5,C=(XY+o)  */
	case 0xaa: D = RES(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* RES  5,D=(XY+o)  */
	case 0xab: E = RES(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* RES  5,E=(XY+o)  */
	case 0xac: H = RES(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* RES  5,H=(XY+o)  */
	case 0xad: L = RES(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* RES  5,L=(XY+o)  */
	case 0xae: v = RES(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* RES  5,(XY+o)    */
	case 0xaf: A = RES(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* RES  5,A=(XY+o)  */
	case 0xb0: B = RES(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* RES  6,B=(XY+o)  */
	case 0xb1: C = RES(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* RES  6,C=(XY+o)  */
	case 0xb2: D = RES(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* RES  6,D=(XY+o)  */
	case 0xb3: E = RES(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* RES  6,E=(XY+o)  */
	case 0xb4: H = RES(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* RES  6,H=(XY+o)  */
	case 0xb5: L = RES(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* RES  6,L=(XY+o)  */
	case 0xb6: v = RES(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* RES  6,(XY+o)    */
	case 0xb7: A = RES(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* RES  6,A=(XY+o)  */
	case 0xb8: B = RES(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* RES  7,B=(XY+o)  */
	case 0xb9: C = RES(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* RES  7,C=(XY+o)  */
	case 0xba: D = RES(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* RES  7,D=(XY+o)  */
	case 0xbb: E = RES(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* RES  7,E=(XY+o)  */
	case 0xbc: H = RES(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* RES  7,H=(XY+o)  */
	case 0xbd: L = RES(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* RES  7,L=(XY+o)  */
	case 0xbe: v = RES(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* RES  7,(XY+o)    */
	case 0xbf: A = RES(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* RES  7,A=(XY+o)  */
	case 0xc0: B = SET(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* SET  0,B=(XY+o)  */
	case 0xc1: C = SET(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* SET  0,C=(XY+o)  */
	case 0xc2: D = SET(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* SET  0,D=(XY+o)  */
	case 0xc3: E = SET(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* SET  0,E=(XY+o)  */
	case 0xc4: H = SET(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* SET  0,H=(XY+o)  */
	case 0xc5: L = SET(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* SET  0,L=(XY+o)  */
	case 0xc6: v = SET(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* SET  0,(XY+o)    */
	case 0xc7: A = SET(0, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* SET  0,A=(XY+o)  */
	case 0xc8: B = SET(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* SET  1,B=(XY+o)  */
	case 0xc9: C = SET(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* SET  1,C=(XY+o)  */
	case 0xca: D = SET(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* SET  1,D=(XY+o)  */
	case 0xcb: E = SET(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* SET  1,E=(XY+o)  */
	case 0xcc: H = SET(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* SET  1,H=(XY+o)  */
	case 0xcd: L = SET(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* SET  1,L=(XY+o)  */
	case 0xce: v = SET(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* SET  1,(XY+o)    */
	case 0xcf: A = SET(1, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* SET  1,A=(XY+o)  */
	case 0xd0: B = SET(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* SET  2,B=(XY+o)  */
	case 0xd1: C = SET(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* SET  2,C=(XY+o)  */
	case 0xd2: D = SET(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* SET  2,D=(XY+o)  */
	case 0xd3: E = SET(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* SET  2,E=(XY+o)  */
	case 0xd4: H = SET(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* SET  2,H=(XY+o)  */
	case 0xd5: L = SET(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* SET  2,L=(XY+o)  */
	case 0xd6: v = SET(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* SET  2,(XY+o)    */
	case 0xd7: A = SET(2, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* SET  2,A=(XY+o)  */
	case 0xd8: B = SET(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* SET  3,B=(XY+o)  */
	case 0xd9: C = SET(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* SET  3,C=(XY+o)  */
	case 0xda: D = SET(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* SET  3,D=(XY+o)  */
	case 0xdb: E = SET(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* SET  3,E=(XY+o)  */
	case 0xdc: H = SET(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* SET  3,H=(XY+o)  */
	case 0xdd: L = SET(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* SET  3,L=(XY+o)  */
	case 0xde: v = SET(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* SET  3,(XY+o)    */
	case 0xdf: A = SET(3, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* SET  3,A=(XY+o)  */
	case 0xe0: B = SET(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* SET  4,B=(XY+o)  */
	case 0xe1: C = SET(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* SET  4,C=(XY+o)  */
	case 0xe2: D = SET(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* SET  4,D=(XY+o)  */
	case 0xe3: E = SET(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* SET  4,E=(XY+o)  */
	case 0xe4: H = SET(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* SET  4,H=(XY+o)  */
	case 0xe5: L = SET(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* SET  4,L=(XY+o)  */
	case 0xe6: v = SET(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* SET  4,(XY+o)    */
	case 0xe7: A = SET(4, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* SET  4,A=(XY+o)  */
	case 0xe8: B = SET(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* SET  5,B=(XY+o)  */
	case 0xe9: C = SET(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* SET  5,C=(XY+o)  */
	case 0xea: D = SET(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* SET  5,D=(XY+o)  */
	case 0xeb: E = SET(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* SET  5,E=(XY+o)  */
	case 0xec: H = SET(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* SET  5,H=(XY+o)  */
	case 0xed: L = SET(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* SET  5,L=(XY+o)  */
	case 0xee: v = SET(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* SET  5,(XY+o)    */
	case 0xef: A = SET(5, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* SET  5,A=(XY+o)  */
	case 0xf0: B = SET(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* SET  6,B=(XY+o)  */
	case 0xf1: C = SET(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* SET  6,C=(XY+o)  */
	case 0xf2: D = SET(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* SET  6,D=(XY+o)  */
	case 0xf3: E = SET(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* SET  6,E=(XY+o)  */
	case 0xf4: H = SET(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* SET  6,H=(XY+o)  */
	case 0xf5: L = SET(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* SET  6,L=(XY+o)  */
	case 0xf6: v = SET(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* SET  6,(XY+o)    */
	case 0xf7: A = SET(6, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* SET  6,A=(XY+o)  */
	case 0xf8: B = SET(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, B); break;	/* SET  7,B=(XY+o)  */
	case 0xf9: C = SET(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, C); break;	/* SET  7,C=(XY+o)  */
	case 0xfa: D = SET(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, D); break;	/* SET  7,D=(XY+o)  */
	case 0xfb: E = SET(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, E); break;	/* SET  7,E=(XY+o)  */
	case 0xfc: H = SET(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, H); break;	/* SET  7,H=(XY+o)  */
	case 0xfd: L = SET(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, L); break;	/* SET  7,L=(XY+o)  */
	case 0xfe: v = SET(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* SET  7,(XY+o)    */
	case 0xff: A = SET(7, RM8(ea)); CLOCK_IN_OP(1); WM8(ea, A); break;	/* SET  7,A=(XY+o)  */
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}

void Z80::OP_DD(uint8_t code)
{
	// Done: M1 + M1
	uint8_t v;

	icount -= cc_xy[code];

	switch(code) {
	case 0x09: ADD16(ix, bc); break;					/* ADD  IX,BC       */
	case 0x19: ADD16(ix, de); break;					/* ADD  IX,DE       */
	case 0x21: IX = FETCH16(); break;					/* LD   IX,w        */
	case 0x22: ea = FETCH16(); WM16(ea, &ix); WZ = ea + 1; break;		/* LD   (w),IX      */
	case 0x23: IX++; break;							/* INC  IX          */
	case 0x24: HX = INC(HX); break;						/* INC  HX          */
	case 0x25: HX = DEC(HX); break;						/* DEC  HX          */
	case 0x26: HX = FETCH8(); break;					/* LD   HX,n        */
	case 0x29: ADD16(ix, ix); break;					/* ADD  IX,IX       */
	case 0x2a: ea = FETCH16(); RM16(ea, &ix); WZ = ea + 1; break;		/* LD   IX,(w)      */
	case 0x2b: IX--; break;							/* DEC  IX          */
	case 0x2c: LX = INC(LX); break;						/* INC  LX          */
	case 0x2d: LX = DEC(LX); break;						/* DEC  LX          */
	case 0x2e: LX = FETCH8(); break;					/* LD   LX,n        */
	case 0x34: EAX(); CLOCK_IN_OP(5); v = INC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* INC  (IX+o)      */
	case 0x35: EAX(); CLOCK_IN_OP(5); v = DEC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* DEC  (IX+o)      */
	case 0x36: EAX(); v = FETCH8(); CLOCK_IN_OP(2); WM8(ea, v); break;	/* LD   (IX+o),n    */
	case 0x39: ADD16(ix, sp); break;					/* ADD  IX,SP       */
	case 0x44: B = HX; break;						/* LD   B,HX        */
	case 0x45: B = LX; break;						/* LD   B,LX        */
	case 0x46: EAX(); CLOCK_IN_OP(5); B = RM8(ea); break;			/* LD   B,(IX+o)    */
	case 0x4c: C = HX; break;						/* LD   C,HX        */
	case 0x4d: C = LX; break;						/* LD   C,LX        */
	case 0x4e: EAX(); CLOCK_IN_OP(5); C = RM8(ea); break;			/* LD   C,(IX+o)    */
	case 0x54: D = HX; break;						/* LD   D,HX        */
	case 0x55: D = LX; break;						/* LD   D,LX        */
	case 0x56: EAX(); CLOCK_IN_OP(5); D = RM8(ea); break;			/* LD   D,(IX+o)    */
	case 0x5c: E = HX; break;						/* LD   E,HX        */
	case 0x5d: E = LX; break;						/* LD   E,LX        */
	case 0x5e: EAX(); CLOCK_IN_OP(5); E = RM8(ea); break;			/* LD   E,(IX+o)    */
	case 0x60: HX = B; break;						/* LD   HX,B        */
	case 0x61: HX = C; break;						/* LD   HX,C        */
	case 0x62: HX = D; break;						/* LD   HX,D        */
	case 0x63: HX = E; break;						/* LD   HX,E        */
	case 0x64: break;							/* LD   HX,HX       */
	case 0x65: HX = LX; break;						/* LD   HX,LX       */
	case 0x66: EAX(); CLOCK_IN_OP(5); H = RM8(ea); break;			/* LD   H,(IX+o)    */
	case 0x67: HX = A; break;						/* LD   HX,A        */
	case 0x68: LX = B; break;						/* LD   LX,B        */
	case 0x69: LX = C; break;						/* LD   LX,C        */
	case 0x6a: LX = D; break;						/* LD   LX,D        */
	case 0x6b: LX = E; break;						/* LD   LX,E        */
	case 0x6c: LX = HX; break;						/* LD   LX,HX       */
	case 0x6d: break;							/* LD   LX,LX       */
	case 0x6e: EAX(); CLOCK_IN_OP(5); L = RM8(ea); break;			/* LD   L,(IX+o)    */
	case 0x6f: LX = A; break;						/* LD   LX,A        */
	case 0x70: EAX(); CLOCK_IN_OP(5); WM8(ea, B); break;			/* LD   (IX+o),B    */
	case 0x71: EAX(); CLOCK_IN_OP(5); WM8(ea, C); break;			/* LD   (IX+o),C    */
	case 0x72: EAX(); CLOCK_IN_OP(5); WM8(ea, D); break;			/* LD   (IX+o),D    */
	case 0x73: EAX(); CLOCK_IN_OP(5); WM8(ea, E); break;			/* LD   (IX+o),E    */
	case 0x74: EAX(); CLOCK_IN_OP(5); WM8(ea, H); break;			/* LD   (IX+o),H    */
	case 0x75: EAX(); CLOCK_IN_OP(5); WM8(ea, L); break;			/* LD   (IX+o),L    */
	case 0x77: EAX(); CLOCK_IN_OP(5); WM8(ea, A); break;			/* LD   (IX+o),A    */
	case 0x7c: A = HX; break;						/* LD   A,HX        */
	case 0x7d: A = LX; break;						/* LD   A,LX        */
	case 0x7e: EAX(); CLOCK_IN_OP(5); A = RM8(ea); break;			/* LD   A,(IX+o)    */
	case 0x84: ADD(HX); break;						/* ADD  A,HX        */
	case 0x85: ADD(LX); break;						/* ADD  A,LX        */
	case 0x86: EAX(); CLOCK_IN_OP(5); ADD(RM8(ea)); break;			/* ADD  A,(IX+o)    */
	case 0x8c: ADC(HX); break;						/* ADC  A,HX        */
	case 0x8d: ADC(LX); break;						/* ADC  A,LX        */
	case 0x8e: EAX(); CLOCK_IN_OP(5); ADC(RM8(ea)); break;			/* ADC  A,(IX+o)    */
	case 0x94: SUB(HX); break;						/* SUB  HX          */
	case 0x95: SUB(LX); break;						/* SUB  LX          */
	case 0x96: EAX(); CLOCK_IN_OP(5); SUB(RM8(ea)); break;			/* SUB  (IX+o)      */
	case 0x9c: SBC(HX); break;						/* SBC  A,HX        */
	case 0x9d: SBC(LX); break;						/* SBC  A,LX        */
	case 0x9e: EAX(); CLOCK_IN_OP(5); SBC(RM8(ea)); break;			/* SBC  A,(IX+o)    */
	case 0xa4: AND(HX); break;						/* AND  HX          */
	case 0xa5: AND(LX); break;						/* AND  LX          */
	case 0xa6: EAX(); CLOCK_IN_OP(5); AND(RM8(ea)); break;			/* AND  (IX+o)      */
	case 0xac: XOR(HX); break;						/* XOR  HX          */
	case 0xad: XOR(LX); break;						/* XOR  LX          */
	case 0xae: EAX(); CLOCK_IN_OP(5); XOR(RM8(ea)); break;			/* XOR  (IX+o)      */
	case 0xb4: OR(HX); break;						/* OR   HX          */
	case 0xb5: OR(LX); break;						/* OR   LX          */
	case 0xb6: EAX(); CLOCK_IN_OP(5); OR(RM8(ea)); break;			/* OR   (IX+o)      */
	case 0xbc: CP(HX); break;						/* CP   HX          */
	case 0xbd: CP(LX); break;						/* CP   LX          */
	case 0xbe: EAX(); CLOCK_IN_OP(5); CP(RM8(ea)); break;			/* CP   (IX+o)      */
	case 0xcb: EAX(); v = FETCH8(); CLOCK_IN_OP(2); OP_XY(v); break;	/* **   DD CB xx    */
	case 0xe1: POP(ix); break;						/* POP  IX          */
	case 0xe3: EXSP(ix); break;						/* EX   (SP),IX     */
	case 0xe5: PUSH(ix); break;						/* PUSH IX          */
	case 0xe9: PC = IX; break;						/* JP   (IX)        */
	case 0xf9: SP = IX; break;						/* LD   SP,IX       */
	default:   OP(code); break;
	}
}

void Z80::OP_FD(uint8_t code)
{
	// Done: M1 + M1
	uint8_t v;

	icount -= cc_xy[code];

	switch(code) {
	case 0x09: ADD16(iy, bc); break;					/* ADD  IY,BC       */
	case 0x19: ADD16(iy, de); break;					/* ADD  IY,DE       */
	case 0x21: IY = FETCH16(); break;					/* LD   IY,w        */
	case 0x22: ea = FETCH16(); WM16(ea, &iy); WZ = ea + 1; break;		/* LD   (w),IY      */
	case 0x23: IY++; break;							/* INC  IY          */
	case 0x24: HY = INC(HY); break;						/* INC  HY          */
	case 0x25: HY = DEC(HY); break;						/* DEC  HY          */
	case 0x26: HY = FETCH8(); break;					/* LD   HY,n        */
	case 0x29: ADD16(iy, iy); break;					/* ADD  IY,IY       */
	case 0x2a: ea = FETCH16(); RM16(ea, &iy); WZ = ea + 1; break;		/* LD   IY,(w)      */
	case 0x2b: IY--; break;							/* DEC  IY          */
	case 0x2c: LY = INC(LY); break;						/* INC  LY          */
	case 0x2d: LY = DEC(LY); break;						/* DEC  LY          */
	case 0x2e: LY = FETCH8(); break;					/* LD   LY,n        */
	case 0x34: EAY(); CLOCK_IN_OP(5); v = INC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* INC  (IY+o)      */
	case 0x35: EAY(); CLOCK_IN_OP(5); v = DEC(RM8(ea)); CLOCK_IN_OP(1); WM8(ea, v); break;	/* DEC  (IY+o)      */
	case 0x36: EAY(); v = FETCH8(); CLOCK_IN_OP(2); WM8(ea, v); break;	/* LD   (IY+o),n    */
	case 0x39: ADD16(iy, sp); break;					/* ADD  IY,SP       */
	case 0x44: B = HY; break;						/* LD   B,HY        */
	case 0x45: B = LY; break;						/* LD   B,LY        */
	case 0x46: EAY(); CLOCK_IN_OP(5); B = RM8(ea); break;			/* LD   B,(IY+o)    */
	case 0x4c: C = HY; break;						/* LD   C,HY        */
	case 0x4d: C = LY; break;						/* LD   C,LY        */
	case 0x4e: EAY(); CLOCK_IN_OP(5); C = RM8(ea); break;			/* LD   C,(IY+o)    */
	case 0x54: D = HY; break;						/* LD   D,HY        */
	case 0x55: D = LY; break;						/* LD   D,LY        */
	case 0x56: EAY(); CLOCK_IN_OP(5); D = RM8(ea); break;			/* LD   D,(IY+o)    */
	case 0x5c: E = HY; break;						/* LD   E,HY        */
	case 0x5d: E = LY; break;						/* LD   E,LY        */
	case 0x5e: EAY(); CLOCK_IN_OP(5); E = RM8(ea); break;			/* LD   E,(IY+o)    */
	case 0x60: HY = B; break;						/* LD   HY,B        */
	case 0x61: HY = C; break;						/* LD   HY,C        */
	case 0x62: HY = D; break;						/* LD   HY,D        */
	case 0x63: HY = E; break;						/* LD   HY,E        */
	case 0x64: break;							/* LD   HY,HY       */
	case 0x65: HY = LY; break;						/* LD   HY,LY       */
	case 0x66: EAY(); CLOCK_IN_OP(5); H = RM8(ea); break;			/* LD   H,(IY+o)    */
	case 0x67: HY = A; break;						/* LD   HY,A        */
	case 0x68: LY = B; break;						/* LD   LY,B        */
	case 0x69: LY = C; break;						/* LD   LY,C        */
	case 0x6a: LY = D; break;						/* LD   LY,D        */
	case 0x6b: LY = E; break;						/* LD   LY,E        */
	case 0x6c: LY = HY; break;						/* LD   LY,HY       */
	case 0x6d: break;							/* LD   LY,LY       */
	case 0x6e: EAY(); CLOCK_IN_OP(5); L = RM8(ea); break;			/* LD   L,(IY+o)    */
	case 0x6f: LY = A; break;						/* LD   LY,A        */
	case 0x70: EAY(); CLOCK_IN_OP(5); WM8(ea, B); break;			/* LD   (IY+o),B    */
	case 0x71: EAY(); CLOCK_IN_OP(5); WM8(ea, C); break;			/* LD   (IY+o),C    */
	case 0x72: EAY(); CLOCK_IN_OP(5); WM8(ea, D); break;			/* LD   (IY+o),D    */
	case 0x73: EAY(); CLOCK_IN_OP(5); WM8(ea, E); break;			/* LD   (IY+o),E    */
	case 0x74: EAY(); CLOCK_IN_OP(5); WM8(ea, H); break;			/* LD   (IY+o),H    */
	case 0x75: EAY(); CLOCK_IN_OP(5); WM8(ea, L); break;			/* LD   (IY+o),L    */
	case 0x77: EAY(); CLOCK_IN_OP(5); WM8(ea, A); break;			/* LD   (IY+o),A    */
	case 0x7c: A = HY; break;						/* LD   A,HY        */
	case 0x7d: A = LY; break;						/* LD   A,LY        */
	case 0x7e: EAY(); CLOCK_IN_OP(5); A = RM8(ea); break;			/* LD   A,(IY+o)    */
	case 0x84: ADD(HY); break;						/* ADD  A,HY        */
	case 0x85: ADD(LY); break;						/* ADD  A,LY        */
	case 0x86: EAY(); CLOCK_IN_OP(5); ADD(RM8(ea)); break;			/* ADD  A,(IY+o)    */
	case 0x8c: ADC(HY); break;						/* ADC  A,HY        */
	case 0x8d: ADC(LY); break;						/* ADC  A,LY        */
	case 0x8e: EAY(); CLOCK_IN_OP(5); ADC(RM8(ea)); break;			/* ADC  A,(IY+o)    */
	case 0x94: SUB(HY); break;						/* SUB  HY          */
	case 0x95: SUB(LY); break;						/* SUB  LY          */
	case 0x96: EAY(); CLOCK_IN_OP(5); SUB(RM8(ea)); break;			/* SUB  (IY+o)      */
	case 0x9c: SBC(HY); break;						/* SBC  A,HY        */
	case 0x9d: SBC(LY); break;						/* SBC  A,LY        */
	case 0x9e: EAY(); CLOCK_IN_OP(5); SBC(RM8(ea)); break;			/* SBC  A,(IY+o)    */
	case 0xa4: AND(HY); break;						/* AND  HY          */
	case 0xa5: AND(LY); break;						/* AND  LY          */
	case 0xa6: EAY(); CLOCK_IN_OP(5); AND(RM8(ea)); break;			/* AND  (IY+o)      */
	case 0xac: XOR(HY); break;						/* XOR  HY          */
	case 0xad: XOR(LY); break;						/* XOR  LY          */
	case 0xae: EAY(); CLOCK_IN_OP(5); XOR(RM8(ea)); break;			/* XOR  (IY+o)      */
	case 0xb4: OR(HY); break;						/* OR   HY          */
	case 0xb5: OR(LY); break;						/* OR   LY          */
	case 0xb6: EAY(); CLOCK_IN_OP(5); OR(RM8(ea)); break;			/* OR   (IY+o)      */
	case 0xbc: CP(HY); break;						/* CP   HY          */
	case 0xbd: CP(LY); break;						/* CP   LY          */
	case 0xbe: EAY(); CLOCK_IN_OP(5); CP(RM8(ea)); break;			/* CP   (IY+o)      */
	case 0xcb: EAY(); v = FETCH8(); CLOCK_IN_OP(2); OP_XY(v); break;	/* **   FD CB xx    */
	case 0xe1: POP(iy); break;						/* POP  IY          */
	case 0xe3: EXSP(iy); break;						/* EX   (SP),IY     */
	case 0xe5: PUSH(iy); break;						/* PUSH IY          */
	case 0xe9: PC = IY; break;						/* JP   (IY)        */
	case 0xf9: SP = IY; break;						/* LD   SP,IY       */
	default:   OP(code); break;
	}
}

void Z80::OP_ED(uint8_t code)
{
	// Done: M1 + M1
	icount -= cc_ed[code];

	switch(code) {
	case 0x40: B = IN8(BC); F = (F & CF) | SZP[B]; break;			/* IN   B,(C)       */
	case 0x41: OUT8(BC, B); break;						/* OUT  (C),B       */
	case 0x42: SBC16(bc); break;						/* SBC  HL,BC       */
	case 0x43: ea = FETCH16(); WM16(ea, &bc); WZ = ea + 1; break;		/* LD   (w),BC      */
	case 0x44: NEG(); break;						/* NEG              */
	case 0x45: RETN(); break;						/* RETN             */
	case 0x46: im = 0; break;						/* im   0           */
	case 0x47: LD_I_A(); break;						/* LD   i,A         */
	case 0x48: C = IN8(BC); F = (F & CF) | SZP[C]; break;			/* IN   C,(C)       */
	case 0x49: OUT8(BC, C); break;						/* OUT  (C),C       */
	case 0x4a: ADC16(bc); break;						/* ADC  HL,BC       */
	case 0x4b: ea = FETCH16(); RM16(ea, &bc); WZ = ea + 1; break;		/* LD   BC,(w)      */
	case 0x4c: NEG(); break;						/* NEG              */
	case 0x4d: RETI(); break;						/* RETI             */
	case 0x4e: im = 0; break;						/* im   0           */
	case 0x4f: LD_R_A(); break;						/* LD   r,A         */
	case 0x50: D = IN8(BC); F = (F & CF) | SZP[D]; break;			/* IN   D,(C)       */
	case 0x51: OUT8(BC, D); break;						/* OUT  (C),D       */
	case 0x52: SBC16(de); break;						/* SBC  HL,DE       */
	case 0x53: ea = FETCH16(); WM16(ea, &de); WZ = ea + 1; break;		/* LD   (w),DE      */
	case 0x54: NEG(); break;						/* NEG              */
	case 0x55: RETN(); break;						/* RETN             */
	case 0x56: im = 1; break;						/* im   1           */
	case 0x57: LD_A_I(); break;						/* LD   A,i         */
	case 0x58: E = IN8(BC); F = (F & CF) | SZP[E]; break;			/* IN   E,(C)       */
	case 0x59: OUT8(BC, E); break;						/* OUT  (C),E       */
	case 0x5a: ADC16(de); break;						/* ADC  HL,DE       */
	case 0x5b: ea = FETCH16(); RM16(ea, &de); WZ = ea + 1; break;		/* LD   DE,(w)      */
	case 0x5c: NEG(); break;						/* NEG              */
	case 0x5d: RETI(); break;						/* RETI             */
	case 0x5e: im = 2; break;						/* im   2           */
	case 0x5f: LD_A_R(); break;						/* LD   A,r         */
	case 0x60: H = IN8(BC); F = (F & CF) | SZP[H]; break;			/* IN   H,(C)       */
	case 0x61: OUT8(BC, H); break;						/* OUT  (C),H       */
	case 0x62: SBC16(hl); break;						/* SBC  HL,HL       */
	case 0x63: ea = FETCH16(); WM16(ea, &hl); WZ = ea + 1; break;		/* LD   (w),HL      */
	case 0x64: NEG(); break;						/* NEG              */
	case 0x65: RETN(); break;						/* RETN             */
	case 0x66: im = 0; break;						/* im   0           */
	case 0x67: RRD(); break;						/* RRD  (HL)        */
	case 0x68: L = IN8(BC); F = (F & CF) | SZP[L]; break;			/* IN   L,(C)       */
	case 0x69: OUT8(BC, L); break;						/* OUT  (C),L       */
	case 0x6a: ADC16(hl); break;						/* ADC  HL,HL       */
	case 0x6b: ea = FETCH16(); RM16(ea, &hl); WZ = ea + 1; break;		/* LD   HL,(w)      */
	case 0x6c: NEG(); break;						/* NEG              */
	case 0x6d: RETI(); break;						/* RETI             */
	case 0x6e: im = 0; break;						/* im   0           */
	case 0x6f: RLD(); break;						/* RLD  (HL)        */
	case 0x70: {uint8_t res = IN8(BC); F = (F & CF) | SZP[res];} break;	/* IN   F,(C)       */
	case 0x71: OUT8(BC, 0); break;						/* OUT  (C),0       */
	case 0x72: SBC16(sp); break;						/* SBC  HL,SP       */
	case 0x73: ea = FETCH16(); WM16(ea, &sp); WZ = ea + 1; break;		/* LD   (w),SP      */
	case 0x74: NEG(); break;						/* NEG              */
	case 0x75: RETN(); break;						/* RETN             */
	case 0x76: im = 1; break;						/* im   1           */
	case 0x78: A = IN8(BC); F = (F & CF) | SZP[A]; WZ = BC + 1; break;	/* IN   A,(C)       */
	case 0x79: OUT8(BC, A); WZ = BC + 1; break;				/* OUT  (C),A       */
	case 0x7a: ADC16(sp); break;						/* ADC  HL,SP       */
	case 0x7b: ea = FETCH16(); RM16(ea, &sp); WZ = ea + 1; break;		/* LD   SP,(w)      */
	case 0x7c: NEG(); break;						/* NEG              */
	case 0x7d: RETI(); break;						/* RETI             */
	case 0x7e: im = 2; break;						/* im   2           */
	case 0xa0: LDI(); break;						/* LDI              */
	case 0xa1: CPI(); break;						/* CPI              */
	case 0xa2: INI(); break;						/* INI              */
	case 0xa3: OUTI(); break;						/* OUTI             */
	case 0xa8: LDD(); break;						/* LDD              */
	case 0xa9: CPD(); break;						/* CPD              */
	case 0xaa: IND(); break;						/* IND              */
	case 0xab: OUTD(); break;						/* OUTD             */
	case 0xb0: LDIR(); break;						/* LDIR             */
	case 0xb1: CPIR(); break;						/* CPIR             */
	case 0xb2: INIR(); break;						/* INIR             */
	case 0xb3: OTIR(); break;						/* OTIR             */
	case 0xb8: LDDR(); break;						/* LDDR             */
	case 0xb9: CPDR(); break;						/* CPDR             */
	case 0xba: INDR(); break;						/* INDR             */
	case 0xbb: OTDR(); break;						/* OTDR             */
	default:   OP(code); break;
	}
}

void Z80::OP(uint8_t code)
{
	// Done: M1
	uint8_t v;

	prevpc = PC - 1;
	icount -= cc_op[code];

	switch(code) {
	case 0x00: break;												/* NOP              */
	case 0x01: BC = FETCH16(); break;										/* LD   BC,w        */
	case 0x02: WM8(BC, A); WZ_L = (BC + 1) & 0xff; WZ_H = A; break;							/* LD (BC),A        */
	case 0x03: BC++; break;												/* INC  BC          */
	case 0x04: B = INC(B); break;											/* INC  B           */
	case 0x05: B = DEC(B); break;											/* DEC  B           */
	case 0x06: B = FETCH8(); break;											/* LD   B,n         */
	case 0x07: RLCA(); break;											/* RLCA             */
	case 0x08: EX_AF(); break;											/* EX   AF,AF'      */
	case 0x09: ADD16(hl, bc); break;										/* ADD  HL,BC       */
	case 0x0a: A = RM8(BC); WZ = BC+1; break;									/* LD   A,(BC)      */
	case 0x0b: BC--; break;												/* DEC  BC          */
	case 0x0c: C = INC(C); break;											/* INC  C           */
	case 0x0d: C = DEC(C); break;											/* DEC  C           */
	case 0x0e: C = FETCH8(); break;											/* LD   C,n         */
	case 0x0f: RRCA(); break;											/* RRCA             */
	case 0x10: B--; JR_COND(B, 0x10); break;									/* DJNZ o           */
	case 0x11: DE = FETCH16(); break;										/* LD   DE,w        */
	case 0x12: WM8(DE, A); WZ_L = (DE + 1) & 0xff; WZ_H = A; break;							/* LD (DE),A        */
	case 0x13: DE++; break;												/* INC  DE          */
	case 0x14: D = INC(D); break;											/* INC  D           */
	case 0x15: D = DEC(D); break;											/* DEC  D           */
	case 0x16: D = FETCH8(); break;											/* LD   D,n         */
	case 0x17: RLA(); break;											/* RLA              */
	case 0x18: JR(); break;												/* JR   o           */
	case 0x19: ADD16(hl, de); break;										/* ADD  HL,DE       */
	case 0x1a: A = RM8(DE); WZ = DE + 1; break;									/* LD   A,(DE)      */
	case 0x1b: DE--; break;												/* DEC  DE          */
	case 0x1c: E = INC(E); break;											/* INC  E           */
	case 0x1d: E = DEC(E); break;											/* DEC  E           */
	case 0x1e: E = FETCH8(); break;											/* LD   E,n         */
	case 0x1f: RRA(); break;											/* RRA              */
	case 0x20: JR_COND(!(F & ZF), 0x20); break;									/* JR   NZ,o        */
	case 0x21: HL = FETCH16(); break;										/* LD   HL,w        */
	case 0x22: ea = FETCH16(); WM16(ea, &hl); WZ = ea + 1; break;							/* LD   (w),HL      */
	case 0x23: HL++; break;												/* INC  HL          */
	case 0x24: H = INC(H); break;											/* INC  H           */
	case 0x25: H = DEC(H); break;											/* DEC  H           */
	case 0x26: H = FETCH8(); break;											/* LD   H,n         */
	case 0x27: DAA(); break;											/* DAA              */
	case 0x28: JR_COND(F & ZF, 0x28); break;									/* JR   Z,o         */
	case 0x29: ADD16(hl, hl); break;										/* ADD  HL,HL       */
	case 0x2a: ea = FETCH16(); RM16(ea, &hl); WZ = ea + 1; break;							/* LD   HL,(w)      */
	case 0x2b: HL--; break;												/* DEC  HL          */
	case 0x2c: L = INC(L); break;											/* INC  L           */
	case 0x2d: L = DEC(L); break;											/* DEC  L           */
	case 0x2e: L = FETCH8(); break;											/* LD   L,n         */
	case 0x2f: A ^= 0xff; F = (F & (SF | ZF | PF | CF)) | HF | NF | (A & (YF | XF)); break;				/* CPL              */
	case 0x30: JR_COND(!(F & CF), 0x30); break;									/* JR   NC,o        */
	case 0x31: SP = FETCH16(); break;										/* LD   SP,w        */
	case 0x32: ea = FETCH16(); WM8(ea, A); WZ_L = (ea + 1) & 0xff; WZ_H = A; break;					/* LD   (w),A       */
	case 0x33: SP++; break;												/* INC  SP          */
	case 0x34: v = INC(RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;							/* INC  (HL)        */
	case 0x35: v = DEC(RM8(HL)); CLOCK_IN_OP(1); WM8(HL, v); break;							/* DEC  (HL)        */
	case 0x36: WM8(HL, FETCH8()); break;										/* LD   (HL),n      */
	case 0x37: F = (F & (SF | ZF | YF | XF | PF)) | CF | (A & (YF | XF)); break;					/* SCF              */
	case 0x38: JR_COND(F & CF, 0x38); break;									/* JR   C,o         */
	case 0x39: ADD16(hl, sp); break;										/* ADD  HL,SP       */
	case 0x3a: ea = FETCH16(); A = RM8(ea); WZ = ea + 1; break;							/* LD   A,(w)       */
	case 0x3b: SP--; break;												/* DEC  SP          */
	case 0x3c: A = INC(A); break;											/* INC  A           */
	case 0x3d: A = DEC(A); break;											/* DEC  A           */
	case 0x3e: A = FETCH8(); break;											/* LD   A,n         */
	case 0x3f: F = ((F & (SF | ZF | YF | XF | PF | CF)) | ((F & CF) << 4) | (A & (YF | XF))) ^ CF; break;		/* CCF              */
	case 0x40: break;												/* LD   B,B         */
	case 0x41: B = C; break;											/* LD   B,C         */
	case 0x42: B = D; break;											/* LD   B,D         */
	case 0x43: B = E; break;											/* LD   B,E         */
	case 0x44: B = H; break;											/* LD   B,H         */
	case 0x45: B = L; break;											/* LD   B,L         */
	case 0x46: B = RM8(HL); break;											/* LD   B,(HL)      */
	case 0x47: B = A; break;											/* LD   B,A         */
	case 0x48: C = B; break;											/* LD   C,B         */
	case 0x49: break;												/* LD   C,C         */
	case 0x4a: C = D; break;											/* LD   C,D         */
	case 0x4b: C = E; break;											/* LD   C,E         */
	case 0x4c: C = H; break;											/* LD   C,H         */
	case 0x4d: C = L; break;											/* LD   C,L         */
	case 0x4e: C = RM8(HL); break;											/* LD   C,(HL)      */
	case 0x4f: C = A; break;											/* LD   C,A         */
	case 0x50: D = B; break;											/* LD   D,B         */
	case 0x51: D = C; break;											/* LD   D,C         */
	case 0x52: break;												/* LD   D,D         */
	case 0x53: D = E; break;											/* LD   D,E         */
	case 0x54: D = H; break;											/* LD   D,H         */
	case 0x55: D = L; break;											/* LD   D,L         */
	case 0x56: D = RM8(HL); break;											/* LD   D,(HL)      */
	case 0x57: D = A; break;											/* LD   D,A         */
	case 0x58: E = B; break;											/* LD   E,B         */
	case 0x59: E = C; break;											/* LD   E,C         */
	case 0x5a: E = D; break;											/* LD   E,D         */
	case 0x5b: break;												/* LD   E,E         */
	case 0x5c: E = H; break;											/* LD   E,H         */
	case 0x5d: E = L; break;											/* LD   E,L         */
	case 0x5e: E = RM8(HL); break;											/* LD   E,(HL)      */
	case 0x5f: E = A; break;											/* LD   E,A         */
	case 0x60: H = B; break;											/* LD   H,B         */
	case 0x61: H = C; break;											/* LD   H,C         */
	case 0x62: H = D; break;											/* LD   H,D         */
	case 0x63: H = E; break;											/* LD   H,E         */
	case 0x64: break;												/* LD   H,H         */
	case 0x65: H = L; break;											/* LD   H,L         */
	case 0x66: H = RM8(HL); break;											/* LD   H,(HL)      */
	case 0x67: H = A; break;											/* LD   H,A         */
	case 0x68: L = B; break;											/* LD   L,B         */
	case 0x69: L = C; break;											/* LD   L,C         */
	case 0x6a: L = D; break;											/* LD   L,D         */
	case 0x6b: L = E; break;											/* LD   L,E         */
	case 0x6c: L = H; break;											/* LD   L,H         */
	case 0x6d: break;												/* LD   L,L         */
	case 0x6e: L = RM8(HL); break;											/* LD   L,(HL)      */
	case 0x6f: L = A; break;											/* LD   L,A         */
	case 0x70: WM8(HL, B); break;											/* LD   (HL),B      */
	case 0x71: WM8(HL, C); break;											/* LD   (HL),C      */
	case 0x72: WM8(HL, D); break;											/* LD   (HL),D      */
	case 0x73: WM8(HL, E); break;											/* LD   (HL),E      */
	case 0x74: WM8(HL, H); break;											/* LD   (HL),H      */
	case 0x75: WM8(HL, L); break;											/* LD   (HL),L      */
	case 0x76: ENTER_HALT(); break;											/* halt             */
	case 0x77: WM8(HL, A); break;											/* LD   (HL),A      */
	case 0x78: A = B; break;											/* LD   A,B         */
	case 0x79: A = C; break;											/* LD   A,C         */
	case 0x7a: A = D; break;											/* LD   A,D         */
	case 0x7b: A = E; break;											/* LD   A,E         */
	case 0x7c: A = H; break;											/* LD   A,H         */
	case 0x7d: A = L; break;											/* LD   A,L         */
	case 0x7e: A = RM8(HL); break;											/* LD   A,(HL)      */
	case 0x7f: break;												/* LD   A,A         */
	case 0x80: ADD(B); break;											/* ADD  A,B         */
	case 0x81: ADD(C); break;											/* ADD  A,C         */
	case 0x82: ADD(D); break;											/* ADD  A,D         */
	case 0x83: ADD(E); break;											/* ADD  A,E         */
	case 0x84: ADD(H); break;											/* ADD  A,H         */
	case 0x85: ADD(L); break;											/* ADD  A,L         */
	case 0x86: ADD(RM8(HL)); break;											/* ADD  A,(HL)      */
	case 0x87: ADD(A); break;											/* ADD  A,A         */
	case 0x88: ADC(B); break;											/* ADC  A,B         */
	case 0x89: ADC(C); break;											/* ADC  A,C         */
	case 0x8a: ADC(D); break;											/* ADC  A,D         */
	case 0x8b: ADC(E); break;											/* ADC  A,E         */
	case 0x8c: ADC(H); break;											/* ADC  A,H         */
	case 0x8d: ADC(L); break;											/* ADC  A,L         */
	case 0x8e: ADC(RM8(HL)); break;											/* ADC  A,(HL)      */
	case 0x8f: ADC(A); break;											/* ADC  A,A         */
	case 0x90: SUB(B); break;											/* SUB  B           */
	case 0x91: SUB(C); break;											/* SUB  C           */
	case 0x92: SUB(D); break;											/* SUB  D           */
	case 0x93: SUB(E); break;											/* SUB  E           */
	case 0x94: SUB(H); break;											/* SUB  H           */
	case 0x95: SUB(L); break;											/* SUB  L           */
	case 0x96: SUB(RM8(HL)); break;											/* SUB  (HL)        */
	case 0x97: SUB(A); break;											/* SUB  A           */
	case 0x98: SBC(B); break;											/* SBC  A,B         */
	case 0x99: SBC(C); break;											/* SBC  A,C         */
	case 0x9a: SBC(D); break;											/* SBC  A,D         */
	case 0x9b: SBC(E); break;											/* SBC  A,E         */
	case 0x9c: SBC(H); break;											/* SBC  A,H         */
	case 0x9d: SBC(L); break;											/* SBC  A,L         */
	case 0x9e: SBC(RM8(HL)); break;											/* SBC  A,(HL)      */
	case 0x9f: SBC(A); break;											/* SBC  A,A         */
	case 0xa0: AND(B); break;											/* AND  B           */
	case 0xa1: AND(C); break;											/* AND  C           */
	case 0xa2: AND(D); break;											/* AND  D           */
	case 0xa3: AND(E); break;											/* AND  E           */
	case 0xa4: AND(H); break;											/* AND  H           */
	case 0xa5: AND(L); break;											/* AND  L           */
	case 0xa6: AND(RM8(HL)); break;											/* AND  (HL)        */
	case 0xa7: AND(A); break;											/* AND  A           */
	case 0xa8: XOR(B); break;											/* XOR  B           */
	case 0xa9: XOR(C); break;											/* XOR  C           */
	case 0xaa: XOR(D); break;											/* XOR  D           */
	case 0xab: XOR(E); break;											/* XOR  E           */
	case 0xac: XOR(H); break;											/* XOR  H           */
	case 0xad: XOR(L); break;											/* XOR  L           */
	case 0xae: XOR(RM8(HL)); break;											/* XOR  (HL)        */
	case 0xaf: XOR(A); break;											/* XOR  A           */
	case 0xb0: OR(B); break;											/* OR   B           */
	case 0xb1: OR(C); break;											/* OR   C           */
	case 0xb2: OR(D); break;											/* OR   D           */
	case 0xb3: OR(E); break;											/* OR   E           */
	case 0xb4: OR(H); break;											/* OR   H           */
	case 0xb5: OR(L); break;											/* OR   L           */
	case 0xb6: OR(RM8(HL)); break;											/* OR   (HL)        */
	case 0xb7: OR(A); break;											/* OR   A           */
	case 0xb8: CP(B); break;											/* CP   B           */
	case 0xb9: CP(C); break;											/* CP   C           */
	case 0xba: CP(D); break;											/* CP   D           */
	case 0xbb: CP(E); break;											/* CP   E           */
	case 0xbc: CP(H); break;											/* CP   H           */
	case 0xbd: CP(L); break;											/* CP   L           */
	case 0xbe: CP(RM8(HL)); break;											/* CP   (HL)        */
	case 0xbf: CP(A); break;											/* CP   A           */
	case 0xc0: RET_COND(!(F & ZF), 0xc0); break;									/* RET  NZ          */
	case 0xc1: POP(bc); break;											/* POP  BC          */
	case 0xc2: JP_COND(!(F & ZF)); break;										/* JP   NZ,a        */
	case 0xc3: JP(); break;												/* JP   a           */
	case 0xc4: CALL_COND(!(F & ZF), 0xc4); break;									/* CALL NZ,a        */
	case 0xc5: PUSH(bc); break;											/* PUSH BC          */
	case 0xc6: ADD(FETCH8()); break;										/* ADD  A,n         */
	case 0xc7: RST(0x00); break;											/* RST  0           */
	case 0xc8: RET_COND(F & ZF, 0xc8); break;									/* RET  Z           */
	case 0xc9:
		if(has_pseudo_bios) {
			if(d_bios != NULL) {
				d_bios->bios_ret_z80(prevpc, &af, &bc, &de, &hl, &ix, &iy, &iff1);
			}
		}
		POP(pc); WZ = PCD; break;										/* RET              */
//#else
//	case 0xc9: POP(pc); WZ = PCD; break;										/* RET              */
//#endif
	case 0xca: JP_COND(F & ZF); break;										/* JP   Z,a         */
	case 0xcb: OP_CB(FETCHOP()); break;										/* **** CB xx       */
	case 0xcc: CALL_COND(F & ZF, 0xcc); break;									/* CALL Z,a         */
	case 0xcd: CALL(); break;											/* CALL a           */
	case 0xce: ADC(FETCH8()); break;										/* ADC  A,n         */
	case 0xcf: RST(0x08); break;											/* RST  1           */
	case 0xd0: RET_COND(!(F & CF), 0xd0); break;									/* RET  NC          */
	case 0xd1: POP(de); break;											/* POP  DE          */
	case 0xd2: JP_COND(!(F & CF)); break;										/* JP   NC,a        */
	case 0xd3: {unsigned n = FETCH8() | (A << 8); OUT8(n, A); WZ_L = ((n & 0xff) + 1) & 0xff; WZ_H = A;} break;	/* OUT  (n),A       */
	case 0xd4: CALL_COND(!(F & CF), 0xd4); break;									/* CALL NC,a        */
	case 0xd5: PUSH(de); break;											/* PUSH DE          */
	case 0xd6: SUB(FETCH8()); break;										/* SUB  n           */
	case 0xd7: RST(0x10); break;											/* RST  2           */
	case 0xd8: RET_COND(F & CF, 0xd8); break;									/* RET  C           */
	case 0xd9: EXX(); break;											/* EXX              */
	case 0xda: JP_COND(F & CF); break;										/* JP   C,a         */
	case 0xdb: {unsigned n = FETCH8() | (A << 8); A = IN8(n); WZ = n + 1;} break;					/* IN   A,(n)       */
	case 0xdc: CALL_COND(F & CF, 0xdc); break;									/* CALL C,a         */
	case 0xdd: OP_DD(FETCHOP()); break;										/* **** DD xx       */
	case 0xde: SBC(FETCH8()); break;										/* SBC  A,n         */
	case 0xdf: RST(0x18); break;											/* RST  3           */
	case 0xe0: RET_COND(!(F & PF), 0xe0); break;									/* RET  PO          */
	case 0xe1: POP(hl); break;											/* POP  HL          */
	case 0xe2: JP_COND(!(F & PF)); break;										/* JP   PO,a        */
	case 0xe3: EXSP(hl); break;											/* EX   HL,(SP)     */
	case 0xe4: CALL_COND(!(F & PF), 0xe4); break;									/* CALL PO,a        */
	case 0xe5: PUSH(hl); break;											/* PUSH HL          */
	case 0xe6: AND(FETCH8()); break;										/* AND  n           */
	case 0xe7: RST(0x20); break;											/* RST  4           */
	case 0xe8: RET_COND(F & PF, 0xe8); break;									/* RET  PE          */
	case 0xe9: PC = HL; break;											/* JP   (HL)        */
	case 0xea: JP_COND(F & PF); break;										/* JP   PE,a        */
	case 0xeb: EX_DE_HL(); break;											/* EX   DE,HL       */
	case 0xec: CALL_COND(F & PF, 0xec); break;									/* CALL PE,a        */
	case 0xed: OP_ED(FETCHOP()); break;										/* **** ED xx       */
	case 0xee: XOR(FETCH8()); break;										/* XOR  n           */
	case 0xef: RST(0x28); break;											/* RST  5           */
	case 0xf0: RET_COND(!(F & SF), 0xf0); break;									/* RET  P           */
	case 0xf1: POP(af); break;											/* POP  AF          */
	case 0xf2: JP_COND(!(F & SF)); break;										/* JP   P,a         */
	case 0xf3: iff1 = iff2 = 0; after_di = true; break;										/* DI               */
	case 0xf4: CALL_COND(!(F & SF), 0xf4); break;									/* CALL P,a         */
	case 0xf5: PUSH(af); break;											/* PUSH AF          */
	case 0xf6: OR(FETCH8()); break;											/* OR   n           */
	case 0xf7: RST(0x30); break;											/* RST  6           */
	case 0xf8: RET_COND(F & SF, 0xf8); break;									/* RET  M           */
	case 0xf9: SP = HL; break;											/* LD   SP,HL       */
	case 0xfa: JP_COND(F & SF); break;										/* JP   M,a         */
	case 0xfb: EI(); break;												/* EI               */
	case 0xfc: CALL_COND(F & SF, 0xfc); break;									/* CALL M,a         */
	case 0xfd: OP_FD(FETCHOP()); break;										/* **** FD xx       */
	case 0xfe: CP(FETCH8()); break;											/* CP   n           */
	case 0xff: RST(0x38); break;											/* RST  7           */
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}


void Z80::initialize()
{
	DEVICE::initialize();

	waitfactor = 0;
	waitcount = 0;
	extra_cycles = 0;

	if(!flags_initialized) {
		uint8_t *padd = SZHVC_add;
		uint8_t *padc = SZHVC_add + 256 * 256;
		uint8_t *psub = SZHVC_sub;
		uint8_t *psbc = SZHVC_sub + 256 * 256;

		for(int oldval = 0; oldval < 256; oldval++) {
			for(int newval = 0; newval < 256; newval++) {
				/* add or adc w/o carry set */
				int val = newval - oldval;
				*padd = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
				*padd |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
				if((newval & 0x0f) < (oldval & 0x0f)) *padd |= HF;
				if(newval < oldval) *padd |= CF;
				if((val ^ oldval ^ 0x80) & (val ^ newval) & 0x80) *padd |= VF;
				padd++;

				/* adc with carry set */
				val = newval - oldval - 1;
				*padc = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
				*padc |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
				if((newval & 0x0f) <= (oldval & 0x0f)) *padc |= HF;
				if(newval <= oldval) *padc |= CF;
				if((val ^ oldval ^ 0x80) & (val ^ newval) & 0x80) *padc |= VF;
				padc++;

				/* cp, sub or sbc w/o carry set */
				val = oldval - newval;
				*psub = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
				*psub |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
				if((newval & 0x0f) > (oldval & 0x0f)) *psub |= HF;
				if(newval > oldval) *psub |= CF;
				if((val ^ oldval) & (oldval ^ newval) & 0x80) *psub |= VF;
				psub++;

				/* sbc with carry set */
				val = oldval - newval - 1;
				*psbc = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
				*psbc |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
				if((newval & 0x0f) >= (oldval & 0x0f)) *psbc |= HF;
				if(newval >= oldval) *psbc |= CF;
				if((val ^ oldval) & (oldval ^ newval) & 0x80) *psbc |= VF;
				psbc++;
			}
		}
		for(int i = 0; i < 256; i++) {
			int p = 0;
			if(i & 0x01) ++p;
			if(i & 0x02) ++p;
			if(i & 0x04) ++p;
			if(i & 0x08) ++p;
			if(i & 0x10) ++p;
			if(i & 0x20) ++p;
			if(i & 0x40) ++p;
			if(i & 0x80) ++p;
			SZ[i] = i ? i & SF : ZF;
			SZ[i] |= (i & (YF | XF));	/* undocumented flag bits 5+3 */
			SZ_BIT[i] = i ? i & SF : ZF | PF;
			SZ_BIT[i] |= (i & (YF | XF));	/* undocumented flag bits 5+3 */
			SZP[i] = SZ[i] | ((p & 1) ? 0 : PF);
			SZHV_inc[i] = SZ[i];
			if(i == 0x80) SZHV_inc[i] |= VF;
			if((i & 0x0f) == 0x00) SZHV_inc[i] |= HF;
			SZHV_dec[i] = SZ[i] | NF;
			if(i == 0x7f) SZHV_dec[i] |= VF;
			if((i & 0x0f) == 0x0f) SZHV_dec[i] |= HF;
		}
		flags_initialized = true;
	}
	is_primary = is_primary_cpu(this);

	// Collecting stateus.
	cycles_tmp_count = 0;
	extra_tmp_count = 0;
	insns_count = 0;
	frames_count = 0;
	nmi_count = 0;
	irq_count = 0;
	nsc800_int_count = 0;
	nsc800_rsta_count = 0;
	nsc800_rsta_count = 0;
	nsc800_rsta_count = 0;
	register_frame_event(this);

//#ifdef USE_DEBUGGER
	if((__USE_DEBUGGER) && (d_debugger != NULL)) {
		d_mem_stored = d_mem;
		d_io_stored = d_io;
		d_debugger->set_context_mem(d_mem);
		d_debugger->set_context_io(d_io);
	}
	if(osd->check_feature(_T("CPU_START_ADDR"))) {
		__CPU_START_ADDR = osd->get_feature_uint32_value(_T("CPU_START_ADDR"));
	} else {
		__CPU_START_ADDR = 0;
	}
//#endif
//#ifdef HAS_NSC800
	has_nsc800 = osd->check_feature(_T("HAS_NSC800"));
//#endif
//#ifdef Z80_PSEUDO_BIOS
	has_pseudo_bios = osd->check_feature(_T("Z80_PSEUDO_BIOS"));
//#endif
//#ifdef SINGLE_MODE_DMA
	has_single_mode_dma = osd->check_feature(_T("SINGLE_MODE_DMA"));
//#endif
//#ifdef HAS_LDAIR_QUIRK
	has_ldair_quirk =  osd->check_feature(_T("HAS_LDAIR_QUIRK"));
//#endif
}
void Z80::event_frame()
{
	if(frames_count < 0) {
		cycles_tmp_count = total_icount;
		extra_tmp_count = 0;
		insns_count = 0;
		frames_count = 0;
		nmi_count = 0;
		irq_count = 0;
		nsc800_int_count = 0;
		nsc800_rsta_count = 0;
		nsc800_rstb_count = 0;
		nsc800_rstc_count = 0;
	} else if(frames_count >= 16) {
		uint64_t _icount = total_icount - cycles_tmp_count;
		if(config.print_statistics) {
			if(has_nsc800) {
				out_debug_log(_T("INFO: 16 frames done.\nINFO: CLOCKS = %ld INSNS = %d EXTRA_ICOUNT = %d \nINFO: NMI# = %d IRQ# = %d NSC800_INT# = %d RSTA# = %d RSTB# = %d RSTC# = %d"),
							  _icount, insns_count, extra_tmp_count, nmi_count, irq_count,
							  nsc800_int_count, nsc800_rsta_count, nsc800_rstb_count, nsc800_rstc_count);
			} else {
				out_debug_log(_T("INFO: 16 frames done.\nINFO: CLOCKS = %ld INSNS = %d EXTRA_ICOUNT = %d \nINFO: NMI# = %d IRQ# = %d"), _icount, insns_count, extra_tmp_count, nmi_count, irq_count);
			}
		}
		cycles_tmp_count = total_icount;
		insns_count = 0;
		extra_tmp_count = 0;
		frames_count = 0;
		nmi_count = 0;
		irq_count = 0;
		nsc800_int_count = 0;
		nsc800_rsta_count = 0;
		nsc800_rstb_count = 0;
		nsc800_rstc_count = 0;
	} else {
		frames_count++;
	}

}

void Z80::special_reset(int num)
{
	PCD = __CPU_START_ADDR;
	SPD = 0;
	AFD = BCD = DED = HLD = 0;
	F = ZF;			/* Zero flag is set */
	I = R = R2 = 0;
	WZD = PCD;
	af2.d = bc2.d = de2.d = hl2.d = 0;
	ea = 0;

	im = iff1 = iff2 = icr = 0;
	after_halt = /*after_di = */ after_ei = false;
	after_ldair = false;
	intr_req_bit = intr_pend_bit = 0;
}

void Z80::reset()
{
	IXD = IYD = 0xffff;	/* IX and IY are FFFF after a reset! */
	special_reset(0);
	icount = dma_icount = wait_icount = 0;
	extra_cycles = 0;
}

void Z80::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CPU_IRQ) {
		intr_req_bit = (intr_req_bit & ~mask) | (data & mask);
		// always pending (temporary)
		intr_pend_bit = (intr_pend_bit & ~mask) | (0xffffffff & mask);
		irq_count++;
	} else if(id == SIG_CPU_NMI) {
		intr_req_bit = (data & mask) ? (intr_req_bit | NMI_REQ_BIT) : (intr_req_bit & ~NMI_REQ_BIT);
		nmi_count++;
	} else if(id == SIG_CPU_BUSREQ) {
		busreq = ((data & mask) != 0);
		write_signals(&outputs_busack, busreq ? 0xffffffff : 0);
	} else if(id == SIG_CPU_WAIT) {
		wait = ((data & mask) != 0);
	} else if(id == SIG_CPU_WAIT_FACTOR) {
		waitfactor = data; // 65536.
	} else if(has_nsc800) {
//#ifdef HAS_NSC800
		if(id == SIG_NSC800_INT) {
			intr_req_bit = (data & mask) ? (intr_req_bit | 1) : (intr_req_bit & ~1);
			nsc800_int_count++;
		} else if(id == SIG_NSC800_RSTA) {
			intr_req_bit = (data & mask) ? (intr_req_bit | 8) : (intr_req_bit & ~8);
			nsc800_rsta_count++;
		} else if(id == SIG_NSC800_RSTB) {
			intr_req_bit = (data & mask) ? (intr_req_bit | 4) : (intr_req_bit & ~4);
			nsc800_rstb_count++;
		} else if(id == SIG_NSC800_RSTC) {
			intr_req_bit = (data & mask) ? (intr_req_bit | 2) : (intr_req_bit & ~2);
			nsc800_rstc_count++;
		}
	}
//#endif
}

uint32_t  Z80::read_signal(int id)
{
	if(id == SIG_CPU_IRQ) {
		return intr_req_bit;
	}
	return 0;
}

// Maybe dummy.
void  Z80::debugger_hook(void)
{
#if 0
//#ifdef USE_DEBUGGER
	__LIKELY_IF((__USE_DEBUGGER) && (d_debugger != NULL)) {
		bool now_debugging = d_debugger->now_debugging;
		if(now_debugging) {
			d_debugger->check_break_points(PC);
			if(d_debugger->now_suspended) {
				osd->mute_sound();
				d_debugger->now_waiting = true;
				while(d_debugger->now_debugging && d_debugger->now_suspended) {
					osd->sleep(10);
				}
				d_debugger->now_waiting = false;
			}
			if(d_debugger->now_debugging) {
				d_mem = d_debugger;
			} else {
				now_debugging = false;
			}

			//d_debugger->add_cpu_trace(PC);
			int first_icount = icount;
			//pPPC = pPC;
			if(now_debugging) {
				if(!d_debugger->now_going) {
					d_debugger->now_suspended = true;
				}
				d_mem = d_mem_stored;
			}
		}
	}
//#endif
#endif
}

void Z80::cpu_wait(int clocks)
{
	uint64_t ncount = 0;
	if(clocks < 0) return;
	if(waitfactor == 0) return;

	waitcount += (waitfactor * (uint64_t)clocks);
	const uint64_t _limit = INT64_MAX;
	const int  _i_limit = INT_MAX;
	__UNLIKELY_IF(waitcount >= _limit) {
		waitcount = _limit;
	}
	if(waitcount >= (1 << 16)) {
		ncount = waitcount >> 16;
		waitcount = waitcount - (ncount << 16);
		__LIKELY_IF(ncount > 0) {
			extra_cycles += ncount;
			__UNLIKELY_IF(extra_cycles > _i_limit) {
				extra_cycles = _i_limit;
			}
		}
	}
}

int Z80::run(int clock)
{
	bool __is_use_debugger = 	((__USE_DEBUGGER) && (d_debugger != nullptr)) ? true : false;
	if(clock == -1) {
		// this is primary cpu
		if(wait) {
			// don't run cpu!
			__LIKELY_IF(__is_use_debugger) {
				total_icount += 1;
			}
			int tmp_extra_cycles = 0;
			if(extra_cycles > 0) {
				tmp_extra_cycles = extra_cycles;
			}
			extra_cycles = 0;
			cpu_wait(1);
			return tmp_extra_cycles + 1;
		} else if(wait_icount > 0) {
			// don't run cpu!
			icount = wait_icount;
			wait_icount = 0;

			int tmp_extra_cycles = 0;
			if(extra_cycles > 0) {
				tmp_extra_cycles = extra_cycles;
			}
			extra_cycles = 0;
			__LIKELY_IF(__is_use_debugger) {
				total_icount += icount;
			}
			cpu_wait(icount);
			return icount + tmp_extra_cycles;
		} else if(busreq || dma_icount > 0) {
			// run dma once
			if(has_single_mode_dma) {
				if(d_dma && dma_icount == 0) {
					d_dma->do_dma();
				}
			}
			if(dma_icount > 0) {
				icount = dma_icount;
				dma_icount = 0;
			} else {
				icount = 1;
			}
			int tmp_extra_cycles = 0;
			if(extra_cycles > 0) {
				tmp_extra_cycles = extra_cycles;
			}
			extra_cycles = 0;
			// don't run cpu!
			__LIKELY_IF(__is_use_debugger) {
				total_icount += icount;
			}
			cpu_wait(icount);
			return icount + tmp_extra_cycles;
		} else {
			// run only one opcode
			icount = event_icount = event_done_icount = 0;
			run_one_opecode();
			if(wait || wait_icount > 0) {
				event_icount = (-icount) - (event_icount + event_done_icount);
				#ifdef _DEBUG
					assert(event_icount >= 0);
				#endif
				if(event_icount > 0) wait_icount += event_icount;
			}
			__LIKELY_IF(__is_use_debugger) {
				total_icount += (-icount);
			}
			// run dma once
			if(has_single_mode_dma) {
				if(d_dma && dma_icount == 0 && !(wait || wait_icount > 0)) {
					d_dma->do_dma();
				}
			}
			int tmp_extra_cycles = 0;
			if(extra_cycles > 0) {
				tmp_extra_cycles = extra_cycles;
			}
			extra_cycles = 0;
			cpu_wait(-icount);
			return (-icount) + tmp_extra_cycles;
		}
	} else if((icount += clock) > 0) {
		// Secondary CPU or below.
		int first_icount = icount;
		int tmp_icount;

		if(busreq && !wait) {
			if(dma_icount > 0) {
				tmp_icount = min(icount, dma_icount);
				__LIKELY_IF(__is_use_debugger) {
					total_icount += tmp_icount;
				}
				icount -= tmp_icount;
				dma_icount -= tmp_icount;
			}
			// run dma once
			if(has_single_mode_dma) {
				if(d_dma && dma_icount == 0) {
					d_dma->do_dma();
				}
			}
		} else {
			// run cpu while given clocks remain
			while(icount > 0 && !(busreq || wait)) {
				if(dma_icount > 0) {
					tmp_icount = min(icount, dma_icount);
					__LIKELY_IF(__is_use_debugger) {
						total_icount += tmp_icount;
					}
					icount -= tmp_icount;
					dma_icount -= tmp_icount;
				} else {
					// run only one opcode
					__LIKELY_IF(__is_use_debugger) {
						tmp_icount = icount;
					}
					run_one_opecode();
					__LIKELY_IF(__is_use_debugger) {
						total_icount += tmp_icount - icount;
					}
					// run dma once
					if(has_single_mode_dma) {
						if(d_dma && dma_icount == 0) {
							d_dma->do_dma();
						}
					}
				}
			}
		}
		// spin cpu for remained clocks
		if(icount > 0) {
			if(dma_icount > 0 && !wait) {
				dma_icount -= min(icount, dma_icount);
			}
			__LIKELY_IF(__is_use_debugger) {
				total_icount += icount;
			}
			icount = 0;
		}
		//cpu_wait(first_icount - icount);
		return first_icount - icount;
	} else {
		//int passed_icount = wait_icount;
		//cpu_wait(1); // ToDo
		return 0;
	}
}

void Z80::run_one_opecode()
{
	// rune one opecode
	bool prev_after_ei = after_ei;

	after_halt = after_di = after_ei = false;
	after_ldair = false;

	bool now_debugging = false;
//#ifdef USE_DEBUGGER
	__LIKELY_IF((__USE_DEBUGGER) && (d_debugger != nullptr)) {
		now_debugging = d_debugger->now_debugging;
		if(now_debugging) {
			d_debugger->check_break_points(PC);
			if(d_debugger->now_suspended) {
				d_debugger->now_waiting = true;
				emu->start_waiting_in_debugger();
				while(d_debugger->now_debugging && d_debugger->now_suspended) {
					emu->process_waiting_in_debugger();
				}
				emu->finish_waiting_in_debugger();
				d_debugger->now_waiting = false;
			}
			if(d_debugger->now_debugging) {
				d_mem = d_io = d_debugger;
			} else {
				now_debugging = false;
			}
			d_debugger->add_cpu_trace(PC);
			OP(FETCHOP());
//#if HAS_LDAIR_QUIRK
			if(has_ldair_quirk) {
				if(after_ldair) {
					F &= ~PF;	// reset parity flag after LD A,I or LD A,R
				}
			}
//#endif
			if(!after_ei) {
				// not just after EI is done
				if(prev_after_ei && !after_di) {
					// EI and any instruction (ex. RET, not DI/EI) are done
					d_pic->notify_intr_ei();
				}
				check_interrupt();
			}
			if(now_debugging) {
				if(!d_debugger->now_going) {
					d_debugger->now_suspended = true;
				}
				d_mem = d_mem_stored;
				d_io = d_io_stored;
			}
			return;
		} else {
			d_debugger->add_cpu_trace(PC);
		}
	}
//#endif
	OP(FETCHOP());
//#if HAS_LDAIR_QUIRK
	if(has_ldair_quirk) {
		if(after_ldair) {
			F &= ~PF;	// reset parity flag after LD A,I or LD A,R
		}
	}
//#endif
	if(!after_ei) {
		// not just after EI is done
		if(prev_after_ei && !after_di) {
			// EI and any instruction (ex. RET, not DI/EI) are done
			d_pic->notify_intr_ei();
		}
		check_interrupt();
	}
//#ifdef USE_DEBUGGER
//}
//#endif
}

void Z80::check_interrupt_standard()
{
	// interrupt
	LEAVE_HALT();

	uint32_t vector = d_pic->get_intr_ack();
	if(im == 0) {
		// mode 0 (support NOP/JMP/CALL/RST only)
		switch(vector & 0xff) {
		case 0x00: break;				// NOP
		case 0xc3: PCD = vector >> 8; break;		// JMP
		case 0xcd: PUSH(pc); PCD = vector >> 8; break;	// CALL
		case 0xc7: PUSH(pc); PCD = 0x0000; break;	// RST 00H
		case 0xcf: PUSH(pc); PCD = 0x0008; break;	// RST 08H
		case 0xd7: PUSH(pc); PCD = 0x0010; break;	// RST 10H
		case 0xdf: PUSH(pc); PCD = 0x0018; break;	// RST 18H
		case 0xe7: PUSH(pc); PCD = 0x0020; break;	// RST 20H
		case 0xef: PUSH(pc); PCD = 0x0028; break;	// RST 28H
		case 0xf7: PUSH(pc); PCD = 0x0030; break;	// RST 30H
		case 0xff: PUSH(pc); PCD = 0x0038; break;	// RST 38H
		}
		icount -= cc_op[vector & 0xff] + cc_ex[0xff];
	} else if(im == 1) {
		// mode 1
		PUSH(pc);
		PCD = 0x0038;
		icount -= cc_op[0xff] + cc_ex[0xff];
	} else {
		// mode 2
		PUSH(pc);
		RM16((vector & 0xff) | (I << 8), &pc);
		icount -= cc_op[0xcd] + cc_ex[0xff];
	}
	iff1 = iff2 = 0;
	intr_req_bit = 0;
	WZ = PCD;
}

void Z80::check_interrupt_nsc800()
{
	if((intr_req_bit & 8) && (icr & 8)) {
		// RSTA
		LEAVE_HALT();
		PUSH(pc);
		PCD = WZ = 0x003c;
		icount -= cc_op[0xff] + cc_ex[0xff];
		iff1 = iff2 = 0;
		intr_req_bit &= ~8;
	} else if((intr_req_bit & 4) && (icr & 4)) {
		// RSTB
		LEAVE_HALT();
		PUSH(pc);
		PCD = WZ = 0x0034;
		icount -= cc_op[0xff] + cc_ex[0xff];
		iff1 = iff2 = 0;
		intr_req_bit &= ~4;
	} else if((intr_req_bit & 2) && (icr & 2)) {
		// RSTC
		LEAVE_HALT();
		PUSH(pc);
		PCD = WZ = 0x002c;
		icount -= cc_op[0xff] + cc_ex[0xff];
		iff1 = iff2 = 0;
		intr_req_bit &= ~2;
	} else if((intr_req_bit & 1) && (icr & 1)) {
		// INTR
		LEAVE_HALT();
		PUSH(pc);
		PCD = WZ = d_pic->get_intr_ack() & 0xffff;
		icount -= cc_op[0xcd] + cc_ex[0xff];
		iff1 = iff2 = 0;
		intr_req_bit &= ~1;
	}
}

void Z80::check_interrupt()
{
	// check interrupt
	if(intr_req_bit) {
		// Processing NMI is commonly both Z80 and NSC800 .
		__UNLIKELY_IF(intr_req_bit & NMI_REQ_BIT) {
			// nmi
			LEAVE_HALT();
			PUSH(pc);
			PCD = WZD = 0x0066;
			icount -= 11;
			iff1 = 0;
			intr_req_bit &= ~NMI_REQ_BIT;
		} else if(iff1) {
			__UNLIKELY_IF(has_nsc800) {
				// Another processing is different both Z80 and NSC800 .
				check_interrupt_nsc800();
			} else {
				check_interrupt_standard();
			}
		} else __LIKELY_IF(!(has_nsc800)) {
			intr_req_bit &= intr_pend_bit;
		}
	}
}

//#ifdef USE_DEBUGGER
void  Z80::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait_tmp;
	d_mem_stored->write_data8w(addr, data, &wait_tmp);
}

uint32_t  Z80::read_debug_data8(uint32_t addr)
{
	int wait_tmp;
	return d_mem_stored->read_data8w(addr, &wait_tmp);
}

void  Z80::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait_tmp;
	d_io_stored->write_io8w(addr, data, &wait_tmp);
}

uint32_t  Z80::read_debug_io8(uint32_t addr)
{
	int wait_tmp;
	return d_io_stored->read_io8w(addr, &wait_tmp);
}

bool Z80::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	if(_tcsicmp(reg, _T("PC")) == 0) {
		PC = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		SP = data;
	} else if(_tcsicmp(reg, _T("AF")) == 0) {
		AF = data;
	} else if(_tcsicmp(reg, _T("BC")) == 0) {
		BC = data;
	} else if(_tcsicmp(reg, _T("DE")) == 0) {
		DE = data;
	} else if(_tcsicmp(reg, _T("HL")) == 0) {
		HL = data;
	} else if(_tcsicmp(reg, _T("IX")) == 0) {
		IX = data;
	} else if(_tcsicmp(reg, _T("IY")) == 0) {
		IY = data;
	} else if(_tcsicmp(reg, _T("A")) == 0) {
		A = data;
	} else if(_tcsicmp(reg, _T("F")) == 0) {
		F = data;
	} else if(_tcsicmp(reg, _T("B")) == 0) {
		B = data;
	} else if(_tcsicmp(reg, _T("C")) == 0) {
		C = data;
	} else if(_tcsicmp(reg, _T("D")) == 0) {
		D = data;
	} else if(_tcsicmp(reg, _T("E")) == 0) {
		E = data;
	} else if(_tcsicmp(reg, _T("H")) == 0) {
		H = data;
	} else if(_tcsicmp(reg, _T("L")) == 0) {
		L = data;
	} else if(_tcsicmp(reg, _T("HX")) == 0 || _tcsicmp(reg, _T("XH")) == 0 || _tcsicmp(reg, _T("IXH")) == 0) {
		HX = data;
	} else if(_tcsicmp(reg, _T("LX")) == 0 || _tcsicmp(reg, _T("XL")) == 0 || _tcsicmp(reg, _T("IXL")) == 0) {
		LX = data;
	} else if(_tcsicmp(reg, _T("HY")) == 0 || _tcsicmp(reg, _T("YH")) == 0 || _tcsicmp(reg, _T("IYH")) == 0) {
		HY = data;
	} else if(_tcsicmp(reg, _T("LX")) == 0 || _tcsicmp(reg, _T("YL")) == 0 || _tcsicmp(reg, _T("IYL")) == 0) {
		LY = data;
	} else if(_tcsicmp(reg, _T("I")) == 0) {
		I = data;
	} else if(_tcsicmp(reg, _T("R")) == 0) {
		R = data;
	} else if(_tcsicmp(reg, _T("AF'")) == 0) {
		AF2 = data;
	} else if(_tcsicmp(reg, _T("BC'")) == 0) {
		BC2 = data;
	} else if(_tcsicmp(reg, _T("DE'")) == 0) {
		DE2 = data;
	} else if(_tcsicmp(reg, _T("HL'")) == 0) {
		HL2 = data;
	} else if(_tcsicmp(reg, _T("A'")) == 0) {
		A2 = data;
	} else if(_tcsicmp(reg, _T("F'")) == 0) {
		F2 = data;
	} else if(_tcsicmp(reg, _T("B'")) == 0) {
		B2 = data;
	} else if(_tcsicmp(reg, _T("C'")) == 0) {
		C2 = data;
	} else if(_tcsicmp(reg, _T("D'")) == 0) {
		D2 = data;
	} else if(_tcsicmp(reg, _T("E'")) == 0) {
		E2 = data;
	} else if(_tcsicmp(reg, _T("H'")) == 0) {
		H2 = data;
	} else if(_tcsicmp(reg, _T("L'")) == 0) {
		L2 = data;
	} else {
		return false;
	}
	return true;
}

bool Z80::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
/*
F = [--------]  A = 00  BC = 0000  DE = 0000  HL = 0000  IX = 0000  IY = 0000
F'= [--------]  A'= 00  BC'= 0000  DE'= 0000  HL'= 0000  SP = 0000  PC = 0000
        I = 00  R = 00 (BC)= 0000 (DE)= 0000 (HL)= 0000 (SP)= 0000  EI:IFF2=0
Total CPU Clocks = 0 (0) Since Scanline = 0/0 (0/0)
*/
	int wait_tmp;
	my_stprintf_s(buffer, buffer_len,
	_T("F = [%c%c%c%c%c%c%c%c]  A = %02X  BC = %04X  DE = %04X  HL = %04X  IX = %04X  IY = %04X\n")
	_T("F'= [%c%c%c%c%c%c%c%c]  A'= %02X  BC'= %04X  DE'= %04X  HL'= %04X  SP = %04X  PC = %04X\n")
	_T("        I = %02X  R = %02X (BC)= %04X (DE)= %04X (HL)= %04X (SP)= %04X  %cI:IFF2=%d\n")
	_T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),

	(F & CF) ? _T('C') : _T('-'), (F & NF) ? _T('N') : _T('-'), (F & PF) ? _T('P') : _T('-'), (F & XF) ? _T('X') : _T('-'),
	(F & HF) ? _T('H') : _T('-'), (F & YF) ? _T('Y') : _T('-'), (F & ZF) ? _T('Z') : _T('-'), (F & SF) ? _T('S') : _T('-'),
	A, BC, DE, HL, IX, IY,
	(F2 & CF) ? _T('C') : _T('-'), (F2 & NF) ? _T('N') : _T('-'), (F2 & PF) ? _T('P') : _T('-'), (F2 & XF) ? _T('X') : _T('-'),
	(F2 & HF) ? _T('H') : _T('-'), (F2 & YF) ? _T('Y') : _T('-'), (F2 & ZF) ? _T('Z') : _T('-'), (F2 & SF) ? _T('S') : _T('-'),
	A2, BC2, DE2, HL2, SP, PC,
	I, R,
	d_mem_stored->read_data16w(BC, &wait_tmp), d_mem_stored->read_data16w(DE, &wait_tmp), d_mem_stored->read_data16w(HL, &wait_tmp), d_mem_stored->read_data16w(SP, &wait_tmp),
	iff1 ? _T('E') : _T('D'), iff2,
	total_icount, total_icount - prev_total_icount,
	get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
	prev_total_icount = total_icount;
	return true;
}

// disassembler
extern "C" {
extern int z80_dasm_main(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol);
static void dasm_cb(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol);
static void dasm_dd(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol);
static void dasm_ed(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol);
static void dasm_fd(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol);
static void dasm_ddcb(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol);
static void dasm_fdcb(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol);

uint8_t z80_dasm_ops[4];
int z80_dasm_ptr;

inline uint8_t dasm_fetchop()
{
	return z80_dasm_ops[z80_dasm_ptr++];
}

inline uint8_t debug_fetch8()
{
	return z80_dasm_ops[z80_dasm_ptr++];
}

inline uint16_t debug_fetch16()
{
	uint16_t val = z80_dasm_ops[z80_dasm_ptr] | (z80_dasm_ops[z80_dasm_ptr + 1] << 8);
	z80_dasm_ptr += 2;
	return val;
}

inline int8_t debug_fetch8_rel()
{
	return (int8_t)z80_dasm_ops[z80_dasm_ptr++];
}

inline uint16_t debug_fetch8_relpc(uint32_t pc)
{
	int8_t res = (int8_t)z80_dasm_ops[z80_dasm_ptr++];
	return pc + z80_dasm_ptr + res;
}

int z80_dasm_main(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol)
{
	buffer[0] = _T('\0');
	z80_dasm_ptr = 0;
	uint8_t code = dasm_fetchop();

	switch(code) {
	case 0x00: my_stprintf_s(buffer, buffer_len, _T("NOP")); break;
	case 0x01: my_stprintf_s(buffer, buffer_len, _T("LD BC, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x02: my_stprintf_s(buffer, buffer_len, _T("LD (BC), A")); break;
	case 0x03: my_stprintf_s(buffer, buffer_len, _T("INC BC")); break;
	case 0x04: my_stprintf_s(buffer, buffer_len, _T("INC B")); break;
	case 0x05: my_stprintf_s(buffer, buffer_len, _T("DEC B")); break;
	case 0x06: my_stprintf_s(buffer, buffer_len, _T("LD B, %02x"), debug_fetch8()); break;
	case 0x07: my_stprintf_s(buffer, buffer_len, _T("RLCA")); break;
	case 0x08: my_stprintf_s(buffer, buffer_len, _T("EX AF, AF'")); break;
	case 0x09: my_stprintf_s(buffer, buffer_len, _T("ADD HL, BC")); break;
	case 0x0a: my_stprintf_s(buffer, buffer_len, _T("LD A, (BC)")); break;
	case 0x0b: my_stprintf_s(buffer, buffer_len, _T("DEC BC")); break;
	case 0x0c: my_stprintf_s(buffer, buffer_len, _T("INC C")); break;
	case 0x0d: my_stprintf_s(buffer, buffer_len, _T("DEC C")); break;
	case 0x0e: my_stprintf_s(buffer, buffer_len, _T("LD C, %02x"), debug_fetch8()); break;
	case 0x0f: my_stprintf_s(buffer, buffer_len, _T("RRCA")); break;
	case 0x10: my_stprintf_s(buffer, buffer_len, _T("DJNZ %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch8_relpc(pc))); break;
	case 0x11: my_stprintf_s(buffer, buffer_len, _T("LD DE, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x12: my_stprintf_s(buffer, buffer_len, _T("LD (DE), A")); break;
	case 0x13: my_stprintf_s(buffer, buffer_len, _T("INC DE")); break;
	case 0x14: my_stprintf_s(buffer, buffer_len, _T("INC D")); break;
	case 0x15: my_stprintf_s(buffer, buffer_len, _T("DEC D")); break;
	case 0x16: my_stprintf_s(buffer, buffer_len, _T("LD D, %02x"), debug_fetch8()); break;
	case 0x17: my_stprintf_s(buffer, buffer_len, _T("RLA")); break;
	case 0x18: my_stprintf_s(buffer, buffer_len, _T("JR %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch8_relpc(pc))); break;
	case 0x19: my_stprintf_s(buffer, buffer_len, _T("ADD HL, DE")); break;
	case 0x1a: my_stprintf_s(buffer, buffer_len, _T("LD A, (DE)")); break;
	case 0x1b: my_stprintf_s(buffer, buffer_len, _T("DEC DE")); break;
	case 0x1c: my_stprintf_s(buffer, buffer_len, _T("INC E")); break;
	case 0x1d: my_stprintf_s(buffer, buffer_len, _T("DEC E")); break;
	case 0x1e: my_stprintf_s(buffer, buffer_len, _T("LD E, %02x"), debug_fetch8()); break;
	case 0x1f: my_stprintf_s(buffer, buffer_len, _T("RRA")); break;
	case 0x20: my_stprintf_s(buffer, buffer_len, _T("JR NZ, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch8_relpc(pc))); break;
	case 0x21: my_stprintf_s(buffer, buffer_len, _T("LD HL, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x22: my_stprintf_s(buffer, buffer_len, _T("LD (%s), HL"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x23: my_stprintf_s(buffer, buffer_len, _T("INC HL")); break;
	case 0x24: my_stprintf_s(buffer, buffer_len, _T("INC H")); break;
	case 0x25: my_stprintf_s(buffer, buffer_len, _T("DEC H")); break;
	case 0x26: my_stprintf_s(buffer, buffer_len, _T("LD H, %02x"), debug_fetch8()); break;
	case 0x27: my_stprintf_s(buffer, buffer_len, _T("DAA")); break;
	case 0x28: my_stprintf_s(buffer, buffer_len, _T("JR Z, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch8_relpc(pc))); break;
	case 0x29: my_stprintf_s(buffer, buffer_len, _T("ADD HL, HL")); break;
	case 0x2a: my_stprintf_s(buffer, buffer_len, _T("LD HL, (%s)"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x2b: my_stprintf_s(buffer, buffer_len, _T("DEC HL")); break;
	case 0x2c: my_stprintf_s(buffer, buffer_len, _T("INC L")); break;
	case 0x2d: my_stprintf_s(buffer, buffer_len, _T("DEC L")); break;
	case 0x2e: my_stprintf_s(buffer, buffer_len, _T("LD L, %02x"), debug_fetch8()); break;
	case 0x2f: my_stprintf_s(buffer, buffer_len, _T("CPL")); break;
	case 0x30: my_stprintf_s(buffer, buffer_len, _T("JR NC, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch8_relpc(pc))); break;
	case 0x31: my_stprintf_s(buffer, buffer_len, _T("LD SP, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x32: my_stprintf_s(buffer, buffer_len, _T("LD (%s), A"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x33: my_stprintf_s(buffer, buffer_len, _T("INC SP")); break;
	case 0x34: my_stprintf_s(buffer, buffer_len, _T("INC (HL)")); break;
	case 0x35: my_stprintf_s(buffer, buffer_len, _T("DEC (HL)")); break;
	case 0x36: my_stprintf_s(buffer, buffer_len, _T("LD (HL), %02x"), debug_fetch8()); break;
	case 0x37: my_stprintf_s(buffer, buffer_len, _T("SCF")); break;
	case 0x38: my_stprintf_s(buffer, buffer_len, _T("JR C, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch8_relpc(pc))); break;
	case 0x39: my_stprintf_s(buffer, buffer_len, _T("ADD HL, SP")); break;
	case 0x3a: my_stprintf_s(buffer, buffer_len, _T("LD A, (%s)"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x3b: my_stprintf_s(buffer, buffer_len, _T("DEC SP")); break;
	case 0x3c: my_stprintf_s(buffer, buffer_len, _T("INC A")); break;
	case 0x3d: my_stprintf_s(buffer, buffer_len, _T("DEC A")); break;
	case 0x3e: my_stprintf_s(buffer, buffer_len, _T("LD A, %02x"), debug_fetch8()); break;
	case 0x3f: my_stprintf_s(buffer, buffer_len, _T("CCF")); break;
	case 0x40: my_stprintf_s(buffer, buffer_len, _T("LD B, B")); break;
	case 0x41: my_stprintf_s(buffer, buffer_len, _T("LD B, C")); break;
	case 0x42: my_stprintf_s(buffer, buffer_len, _T("LD B, D")); break;
	case 0x43: my_stprintf_s(buffer, buffer_len, _T("LD B, E")); break;
	case 0x44: my_stprintf_s(buffer, buffer_len, _T("LD B, H")); break;
	case 0x45: my_stprintf_s(buffer, buffer_len, _T("LD B, L")); break;
	case 0x46: my_stprintf_s(buffer, buffer_len, _T("LD B, (HL)")); break;
	case 0x47: my_stprintf_s(buffer, buffer_len, _T("LD B, A")); break;
	case 0x48: my_stprintf_s(buffer, buffer_len, _T("LD C, B")); break;
	case 0x49: my_stprintf_s(buffer, buffer_len, _T("LD C, C")); break;
	case 0x4a: my_stprintf_s(buffer, buffer_len, _T("LD C, D")); break;
	case 0x4b: my_stprintf_s(buffer, buffer_len, _T("LD C, E")); break;
	case 0x4c: my_stprintf_s(buffer, buffer_len, _T("LD C, H")); break;
	case 0x4d: my_stprintf_s(buffer, buffer_len, _T("LD C, L")); break;
	case 0x4e: my_stprintf_s(buffer, buffer_len, _T("LD C, (HL)")); break;
	case 0x4f: my_stprintf_s(buffer, buffer_len, _T("LD C, A")); break;
	case 0x50: my_stprintf_s(buffer, buffer_len, _T("LD D, B")); break;
	case 0x51: my_stprintf_s(buffer, buffer_len, _T("LD D, C")); break;
	case 0x52: my_stprintf_s(buffer, buffer_len, _T("LD D, D")); break;
	case 0x53: my_stprintf_s(buffer, buffer_len, _T("LD D, E")); break;
	case 0x54: my_stprintf_s(buffer, buffer_len, _T("LD D, H")); break;
	case 0x55: my_stprintf_s(buffer, buffer_len, _T("LD D, L")); break;
	case 0x56: my_stprintf_s(buffer, buffer_len, _T("LD D, (HL)")); break;
	case 0x57: my_stprintf_s(buffer, buffer_len, _T("LD D, A")); break;
	case 0x58: my_stprintf_s(buffer, buffer_len, _T("LD E, B")); break;
	case 0x59: my_stprintf_s(buffer, buffer_len, _T("LD E, C")); break;
	case 0x5a: my_stprintf_s(buffer, buffer_len, _T("LD E, D")); break;
	case 0x5b: my_stprintf_s(buffer, buffer_len, _T("LD E, E")); break;
	case 0x5c: my_stprintf_s(buffer, buffer_len, _T("LD E, H")); break;
	case 0x5d: my_stprintf_s(buffer, buffer_len, _T("LD E, L")); break;
	case 0x5e: my_stprintf_s(buffer, buffer_len, _T("LD E, (HL)")); break;
	case 0x5f: my_stprintf_s(buffer, buffer_len, _T("LD E, A")); break;
	case 0x60: my_stprintf_s(buffer, buffer_len, _T("LD H, B")); break;
	case 0x61: my_stprintf_s(buffer, buffer_len, _T("LD H, C")); break;
	case 0x62: my_stprintf_s(buffer, buffer_len, _T("LD H, D")); break;
	case 0x63: my_stprintf_s(buffer, buffer_len, _T("LD H, E")); break;
	case 0x64: my_stprintf_s(buffer, buffer_len, _T("LD H, H")); break;
	case 0x65: my_stprintf_s(buffer, buffer_len, _T("LD H, L")); break;
	case 0x66: my_stprintf_s(buffer, buffer_len, _T("LD H, (HL)")); break;
	case 0x67: my_stprintf_s(buffer, buffer_len, _T("LD H, A")); break;
	case 0x68: my_stprintf_s(buffer, buffer_len, _T("LD L, B")); break;
	case 0x69: my_stprintf_s(buffer, buffer_len, _T("LD L, C")); break;
	case 0x6a: my_stprintf_s(buffer, buffer_len, _T("LD L, D")); break;
	case 0x6b: my_stprintf_s(buffer, buffer_len, _T("LD L, E")); break;
	case 0x6c: my_stprintf_s(buffer, buffer_len, _T("LD L, H")); break;
	case 0x6d: my_stprintf_s(buffer, buffer_len, _T("LD L, L")); break;
	case 0x6e: my_stprintf_s(buffer, buffer_len, _T("LD L, (HL)")); break;
	case 0x6f: my_stprintf_s(buffer, buffer_len, _T("LD L, A")); break;
	case 0x70: my_stprintf_s(buffer, buffer_len, _T("LD (HL), B")); break;
	case 0x71: my_stprintf_s(buffer, buffer_len, _T("LD (HL), C")); break;
	case 0x72: my_stprintf_s(buffer, buffer_len, _T("LD (HL), D")); break;
	case 0x73: my_stprintf_s(buffer, buffer_len, _T("LD (HL), E")); break;
	case 0x74: my_stprintf_s(buffer, buffer_len, _T("LD (HL), H")); break;
	case 0x75: my_stprintf_s(buffer, buffer_len, _T("LD (HL), L")); break;
	case 0x76: my_stprintf_s(buffer, buffer_len, _T("HALT")); break;
	case 0x77: my_stprintf_s(buffer, buffer_len, _T("LD (HL), A")); break;
	case 0x78: my_stprintf_s(buffer, buffer_len, _T("LD A, B")); break;
	case 0x79: my_stprintf_s(buffer, buffer_len, _T("LD A, C")); break;
	case 0x7a: my_stprintf_s(buffer, buffer_len, _T("LD A, D")); break;
	case 0x7b: my_stprintf_s(buffer, buffer_len, _T("LD A, E")); break;
	case 0x7c: my_stprintf_s(buffer, buffer_len, _T("LD A, H")); break;
	case 0x7d: my_stprintf_s(buffer, buffer_len, _T("LD A, L")); break;
	case 0x7e: my_stprintf_s(buffer, buffer_len, _T("LD A, (HL)")); break;
	case 0x7f: my_stprintf_s(buffer, buffer_len, _T("LD A, A")); break;
	case 0x80: my_stprintf_s(buffer, buffer_len, _T("ADD A, B")); break;
	case 0x81: my_stprintf_s(buffer, buffer_len, _T("ADD A, C")); break;
	case 0x82: my_stprintf_s(buffer, buffer_len, _T("ADD A, D")); break;
	case 0x83: my_stprintf_s(buffer, buffer_len, _T("ADD A, E")); break;
	case 0x84: my_stprintf_s(buffer, buffer_len, _T("ADD A, H")); break;
	case 0x85: my_stprintf_s(buffer, buffer_len, _T("ADD A, L")); break;
	case 0x86: my_stprintf_s(buffer, buffer_len, _T("ADD A, (HL)")); break;
	case 0x87: my_stprintf_s(buffer, buffer_len, _T("ADD A, A")); break;
	case 0x88: my_stprintf_s(buffer, buffer_len, _T("ADC A, B")); break;
	case 0x89: my_stprintf_s(buffer, buffer_len, _T("ADC A, C")); break;
	case 0x8a: my_stprintf_s(buffer, buffer_len, _T("ADC A, D")); break;
	case 0x8b: my_stprintf_s(buffer, buffer_len, _T("ADC A, E")); break;
	case 0x8c: my_stprintf_s(buffer, buffer_len, _T("ADC A, H")); break;
	case 0x8d: my_stprintf_s(buffer, buffer_len, _T("ADC A, L")); break;
	case 0x8e: my_stprintf_s(buffer, buffer_len, _T("ADC A, (HL)")); break;
	case 0x8f: my_stprintf_s(buffer, buffer_len, _T("ADC A, A")); break;
	case 0x90: my_stprintf_s(buffer, buffer_len, _T("SUB B")); break;
	case 0x91: my_stprintf_s(buffer, buffer_len, _T("SUB C")); break;
	case 0x92: my_stprintf_s(buffer, buffer_len, _T("SUB D")); break;
	case 0x93: my_stprintf_s(buffer, buffer_len, _T("SUB E")); break;
	case 0x94: my_stprintf_s(buffer, buffer_len, _T("SUB H")); break;
	case 0x95: my_stprintf_s(buffer, buffer_len, _T("SUB L")); break;
	case 0x96: my_stprintf_s(buffer, buffer_len, _T("SUB (HL)")); break;
	case 0x97: my_stprintf_s(buffer, buffer_len, _T("SUB A")); break;
	case 0x98: my_stprintf_s(buffer, buffer_len, _T("SBC A, B")); break;
	case 0x99: my_stprintf_s(buffer, buffer_len, _T("SBC A, C")); break;
	case 0x9a: my_stprintf_s(buffer, buffer_len, _T("SBC A, D")); break;
	case 0x9b: my_stprintf_s(buffer, buffer_len, _T("SBC A, E")); break;
	case 0x9c: my_stprintf_s(buffer, buffer_len, _T("SBC A, H")); break;
	case 0x9d: my_stprintf_s(buffer, buffer_len, _T("SBC A, L")); break;
	case 0x9e: my_stprintf_s(buffer, buffer_len, _T("SBC A, (HL)")); break;
	case 0x9f: my_stprintf_s(buffer, buffer_len, _T("SBC A, A")); break;
	case 0xa0: my_stprintf_s(buffer, buffer_len, _T("AND B")); break;
	case 0xa1: my_stprintf_s(buffer, buffer_len, _T("AND C")); break;
	case 0xa2: my_stprintf_s(buffer, buffer_len, _T("AND D")); break;
	case 0xa3: my_stprintf_s(buffer, buffer_len, _T("AND E")); break;
	case 0xa4: my_stprintf_s(buffer, buffer_len, _T("AND H")); break;
	case 0xa5: my_stprintf_s(buffer, buffer_len, _T("AND L")); break;
	case 0xa6: my_stprintf_s(buffer, buffer_len, _T("AND (HL)")); break;
	case 0xa7: my_stprintf_s(buffer, buffer_len, _T("AND A")); break;
	case 0xa8: my_stprintf_s(buffer, buffer_len, _T("XOR B")); break;
	case 0xa9: my_stprintf_s(buffer, buffer_len, _T("XOR C")); break;
	case 0xaa: my_stprintf_s(buffer, buffer_len, _T("XOR D")); break;
	case 0xab: my_stprintf_s(buffer, buffer_len, _T("XOR E")); break;
	case 0xac: my_stprintf_s(buffer, buffer_len, _T("XOR H")); break;
	case 0xad: my_stprintf_s(buffer, buffer_len, _T("XOR L")); break;
	case 0xae: my_stprintf_s(buffer, buffer_len, _T("XOR (HL)")); break;
	case 0xaf: my_stprintf_s(buffer, buffer_len, _T("XOR A")); break;
	case 0xb0: my_stprintf_s(buffer, buffer_len, _T("OR B")); break;
	case 0xb1: my_stprintf_s(buffer, buffer_len, _T("OR C")); break;
	case 0xb2: my_stprintf_s(buffer, buffer_len, _T("OR D")); break;
	case 0xb3: my_stprintf_s(buffer, buffer_len, _T("OR E")); break;
	case 0xb4: my_stprintf_s(buffer, buffer_len, _T("OR H")); break;
	case 0xb5: my_stprintf_s(buffer, buffer_len, _T("OR L")); break;
	case 0xb6: my_stprintf_s(buffer, buffer_len, _T("OR (HL)")); break;
	case 0xb7: my_stprintf_s(buffer, buffer_len, _T("OR A")); break;
	case 0xb8: my_stprintf_s(buffer, buffer_len, _T("CP B")); break;
	case 0xb9: my_stprintf_s(buffer, buffer_len, _T("CP C")); break;
	case 0xba: my_stprintf_s(buffer, buffer_len, _T("CP D")); break;
	case 0xbb: my_stprintf_s(buffer, buffer_len, _T("CP E")); break;
	case 0xbc: my_stprintf_s(buffer, buffer_len, _T("CP H")); break;
	case 0xbd: my_stprintf_s(buffer, buffer_len, _T("CP L")); break;
	case 0xbe: my_stprintf_s(buffer, buffer_len, _T("CP (HL)")); break;
	case 0xbf: my_stprintf_s(buffer, buffer_len, _T("CP A")); break;
	case 0xc0: my_stprintf_s(buffer, buffer_len, _T("RET NZ")); break;
	case 0xc1: my_stprintf_s(buffer, buffer_len, _T("POP BC")); break;
	case 0xc2: my_stprintf_s(buffer, buffer_len, _T("JP NZ, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xc3: my_stprintf_s(buffer, buffer_len, _T("JP %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xc4: my_stprintf_s(buffer, buffer_len, _T("CALL NZ, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xc5: my_stprintf_s(buffer, buffer_len, _T("PUSH BC")); break;
	case 0xc6: my_stprintf_s(buffer, buffer_len, _T("ADD A, %02x"), debug_fetch8()); break;
	case 0xc7: my_stprintf_s(buffer, buffer_len, _T("RST 00H")); break;
	case 0xc8: my_stprintf_s(buffer, buffer_len, _T("RET Z")); break;
	case 0xc9: my_stprintf_s(buffer, buffer_len, _T("RET")); break;
	case 0xca: my_stprintf_s(buffer, buffer_len, _T("JP Z, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xcb: dasm_cb(pc, buffer, buffer_len, first_symbol); break;
	case 0xcc: my_stprintf_s(buffer, buffer_len, _T("CALL Z, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xcd: my_stprintf_s(buffer, buffer_len, _T("CALL %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xce: my_stprintf_s(buffer, buffer_len, _T("ADC A, %02x"), debug_fetch8()); break;
	case 0xcf: my_stprintf_s(buffer, buffer_len, _T("RST 08H")); break;
	case 0xd0: my_stprintf_s(buffer, buffer_len, _T("RET NC")); break;
	case 0xd1: my_stprintf_s(buffer, buffer_len, _T("POP DE")); break;
	case 0xd2: my_stprintf_s(buffer, buffer_len, _T("JP NC, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xd3: my_stprintf_s(buffer, buffer_len, _T("OUT (%02x), A"), debug_fetch8()); break;
	case 0xd4: my_stprintf_s(buffer, buffer_len, _T("CALL NC, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xd5: my_stprintf_s(buffer, buffer_len, _T("PUSH DE")); break;
	case 0xd6: my_stprintf_s(buffer, buffer_len, _T("SUB %02x"), debug_fetch8()); break;
	case 0xd7: my_stprintf_s(buffer, buffer_len, _T("RST 10H")); break;
	case 0xd8: my_stprintf_s(buffer, buffer_len, _T("RET C")); break;
	case 0xd9: my_stprintf_s(buffer, buffer_len, _T("EXX")); break;
	case 0xda: my_stprintf_s(buffer, buffer_len, _T("JP C, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xdb: my_stprintf_s(buffer, buffer_len, _T("IN A, (%02x)"), debug_fetch8()); break;
	case 0xdc: my_stprintf_s(buffer, buffer_len, _T("CALL C, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xdd: dasm_dd(pc, buffer, buffer_len, first_symbol); break;
	case 0xde: my_stprintf_s(buffer, buffer_len, _T("SBC A, %02x"), debug_fetch8()); break;
	case 0xdf: my_stprintf_s(buffer, buffer_len, _T("RST 18H")); break;
	case 0xe0: my_stprintf_s(buffer, buffer_len, _T("RET PO")); break;
	case 0xe1: my_stprintf_s(buffer, buffer_len, _T("POP HL")); break;
	case 0xe2: my_stprintf_s(buffer, buffer_len, _T("JP PO, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xe3: my_stprintf_s(buffer, buffer_len, _T("EX HL, (SP)")); break;
	case 0xe4: my_stprintf_s(buffer, buffer_len, _T("CALL PO, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xe5: my_stprintf_s(buffer, buffer_len, _T("PUSH HL")); break;
	case 0xe6: my_stprintf_s(buffer, buffer_len, _T("AND %02x"), debug_fetch8()); break;
	case 0xe7: my_stprintf_s(buffer, buffer_len, _T("RST 20H")); break;
	case 0xe8: my_stprintf_s(buffer, buffer_len, _T("RET PE")); break;
	case 0xe9: my_stprintf_s(buffer, buffer_len, _T("JP (HL)")); break;
	case 0xea: my_stprintf_s(buffer, buffer_len, _T("JP PE, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xeb: my_stprintf_s(buffer, buffer_len, _T("EX DE, HL")); break;
	case 0xec: my_stprintf_s(buffer, buffer_len, _T("CALL PE, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xed: dasm_ed(pc, buffer, buffer_len, first_symbol); break;
	case 0xee: my_stprintf_s(buffer, buffer_len, _T("XOR %02x"), debug_fetch8()); break;
	case 0xef: my_stprintf_s(buffer, buffer_len, _T("RST 28H")); break;
	case 0xf0: my_stprintf_s(buffer, buffer_len, _T("RET P")); break;
	case 0xf1: my_stprintf_s(buffer, buffer_len, _T("POP AF")); break;
	case 0xf2: my_stprintf_s(buffer, buffer_len, _T("JP P, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xf3: my_stprintf_s(buffer, buffer_len, _T("DI")); break;
	case 0xf4: my_stprintf_s(buffer, buffer_len, _T("CALL P, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xf5: my_stprintf_s(buffer, buffer_len, _T("PUSH AF")); break;
	case 0xf6: my_stprintf_s(buffer, buffer_len, _T("OR %02x"), debug_fetch8()); break;
	case 0xf7: my_stprintf_s(buffer, buffer_len, _T("RST 30H")); break;
	case 0xf8: my_stprintf_s(buffer, buffer_len, _T("RET M")); break;
	case 0xf9: my_stprintf_s(buffer, buffer_len, _T("LD SP, HL")); break;
	case 0xfa: my_stprintf_s(buffer, buffer_len, _T("JP M, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xfb: my_stprintf_s(buffer, buffer_len, _T("EI")); break;
	case 0xfc: my_stprintf_s(buffer, buffer_len, _T("CALL M, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0xfd: dasm_fd(pc, buffer, buffer_len, first_symbol); break;
	case 0xfe: my_stprintf_s(buffer, buffer_len, _T("CP %02x"), debug_fetch8()); break;
	case 0xff: my_stprintf_s(buffer, buffer_len, _T("RST 38H")); break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
	return z80_dasm_ptr;
}

static void dasm_cb(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol)
{
	uint8_t code = dasm_fetchop();

	switch(code) {
	case 0x00: my_stprintf_s(buffer, buffer_len, _T("RLC B")); break;
	case 0x01: my_stprintf_s(buffer, buffer_len, _T("RLC C")); break;
	case 0x02: my_stprintf_s(buffer, buffer_len, _T("RLC D")); break;
	case 0x03: my_stprintf_s(buffer, buffer_len, _T("RLC E")); break;
	case 0x04: my_stprintf_s(buffer, buffer_len, _T("RLC H")); break;
	case 0x05: my_stprintf_s(buffer, buffer_len, _T("RLC L")); break;
	case 0x06: my_stprintf_s(buffer, buffer_len, _T("RLC (HL)")); break;
	case 0x07: my_stprintf_s(buffer, buffer_len, _T("RLC A")); break;
	case 0x08: my_stprintf_s(buffer, buffer_len, _T("RRC B")); break;
	case 0x09: my_stprintf_s(buffer, buffer_len, _T("RRC C")); break;
	case 0x0a: my_stprintf_s(buffer, buffer_len, _T("RRC D")); break;
	case 0x0b: my_stprintf_s(buffer, buffer_len, _T("RRC E")); break;
	case 0x0c: my_stprintf_s(buffer, buffer_len, _T("RRC H")); break;
	case 0x0d: my_stprintf_s(buffer, buffer_len, _T("RRC L")); break;
	case 0x0e: my_stprintf_s(buffer, buffer_len, _T("RRC (HL)")); break;
	case 0x0f: my_stprintf_s(buffer, buffer_len, _T("RRC A")); break;
	case 0x10: my_stprintf_s(buffer, buffer_len, _T("RL B")); break;
	case 0x11: my_stprintf_s(buffer, buffer_len, _T("RL C")); break;
	case 0x12: my_stprintf_s(buffer, buffer_len, _T("RL D")); break;
	case 0x13: my_stprintf_s(buffer, buffer_len, _T("RL E")); break;
	case 0x14: my_stprintf_s(buffer, buffer_len, _T("RL H")); break;
	case 0x15: my_stprintf_s(buffer, buffer_len, _T("RL L")); break;
	case 0x16: my_stprintf_s(buffer, buffer_len, _T("RL (HL)")); break;
	case 0x17: my_stprintf_s(buffer, buffer_len, _T("RL A")); break;
	case 0x18: my_stprintf_s(buffer, buffer_len, _T("RR B")); break;
	case 0x19: my_stprintf_s(buffer, buffer_len, _T("RR C")); break;
	case 0x1a: my_stprintf_s(buffer, buffer_len, _T("RR D")); break;
	case 0x1b: my_stprintf_s(buffer, buffer_len, _T("RR E")); break;
	case 0x1c: my_stprintf_s(buffer, buffer_len, _T("RR H")); break;
	case 0x1d: my_stprintf_s(buffer, buffer_len, _T("RR L")); break;
	case 0x1e: my_stprintf_s(buffer, buffer_len, _T("RR (HL)")); break;
	case 0x1f: my_stprintf_s(buffer, buffer_len, _T("RR A")); break;
	case 0x20: my_stprintf_s(buffer, buffer_len, _T("SLA B")); break;
	case 0x21: my_stprintf_s(buffer, buffer_len, _T("SLA C")); break;
	case 0x22: my_stprintf_s(buffer, buffer_len, _T("SLA D")); break;
	case 0x23: my_stprintf_s(buffer, buffer_len, _T("SLA E")); break;
	case 0x24: my_stprintf_s(buffer, buffer_len, _T("SLA H")); break;
	case 0x25: my_stprintf_s(buffer, buffer_len, _T("SLA L")); break;
	case 0x26: my_stprintf_s(buffer, buffer_len, _T("SLA (HL)")); break;
	case 0x27: my_stprintf_s(buffer, buffer_len, _T("SLA A")); break;
	case 0x28: my_stprintf_s(buffer, buffer_len, _T("SRA B")); break;
	case 0x29: my_stprintf_s(buffer, buffer_len, _T("SRA C")); break;
	case 0x2a: my_stprintf_s(buffer, buffer_len, _T("SRA D")); break;
	case 0x2b: my_stprintf_s(buffer, buffer_len, _T("SRA E")); break;
	case 0x2c: my_stprintf_s(buffer, buffer_len, _T("SRA H")); break;
	case 0x2d: my_stprintf_s(buffer, buffer_len, _T("SRA L")); break;
	case 0x2e: my_stprintf_s(buffer, buffer_len, _T("SRA (HL)")); break;
	case 0x2f: my_stprintf_s(buffer, buffer_len, _T("SRA A")); break;
	case 0x30: my_stprintf_s(buffer, buffer_len, _T("SLL B")); break;
	case 0x31: my_stprintf_s(buffer, buffer_len, _T("SLL C")); break;
	case 0x32: my_stprintf_s(buffer, buffer_len, _T("SLL D")); break;
	case 0x33: my_stprintf_s(buffer, buffer_len, _T("SLL E")); break;
	case 0x34: my_stprintf_s(buffer, buffer_len, _T("SLL H")); break;
	case 0x35: my_stprintf_s(buffer, buffer_len, _T("SLL L")); break;
	case 0x36: my_stprintf_s(buffer, buffer_len, _T("SLL (HL)")); break;
	case 0x37: my_stprintf_s(buffer, buffer_len, _T("SLL A")); break;
	case 0x38: my_stprintf_s(buffer, buffer_len, _T("SRL B")); break;
	case 0x39: my_stprintf_s(buffer, buffer_len, _T("SRL C")); break;
	case 0x3a: my_stprintf_s(buffer, buffer_len, _T("SRL D")); break;
	case 0x3b: my_stprintf_s(buffer, buffer_len, _T("SRL E")); break;
	case 0x3c: my_stprintf_s(buffer, buffer_len, _T("SRL H")); break;
	case 0x3d: my_stprintf_s(buffer, buffer_len, _T("SRL L")); break;
	case 0x3e: my_stprintf_s(buffer, buffer_len, _T("SRL (HL)")); break;
	case 0x3f: my_stprintf_s(buffer, buffer_len, _T("SRL A")); break;
	case 0x40: my_stprintf_s(buffer, buffer_len, _T("BIT 0, B")); break;
	case 0x41: my_stprintf_s(buffer, buffer_len, _T("BIT 0, C")); break;
	case 0x42: my_stprintf_s(buffer, buffer_len, _T("BIT 0, D")); break;
	case 0x43: my_stprintf_s(buffer, buffer_len, _T("BIT 0, E")); break;
	case 0x44: my_stprintf_s(buffer, buffer_len, _T("BIT 0, H")); break;
	case 0x45: my_stprintf_s(buffer, buffer_len, _T("BIT 0, L")); break;
	case 0x46: my_stprintf_s(buffer, buffer_len, _T("BIT 0, (HL)")); break;
	case 0x47: my_stprintf_s(buffer, buffer_len, _T("BIT 0, A")); break;
	case 0x48: my_stprintf_s(buffer, buffer_len, _T("BIT 1, B")); break;
	case 0x49: my_stprintf_s(buffer, buffer_len, _T("BIT 1, C")); break;
	case 0x4a: my_stprintf_s(buffer, buffer_len, _T("BIT 1, D")); break;
	case 0x4b: my_stprintf_s(buffer, buffer_len, _T("BIT 1, E")); break;
	case 0x4c: my_stprintf_s(buffer, buffer_len, _T("BIT 1, H")); break;
	case 0x4d: my_stprintf_s(buffer, buffer_len, _T("BIT 1, L")); break;
	case 0x4e: my_stprintf_s(buffer, buffer_len, _T("BIT 1, (HL)")); break;
	case 0x4f: my_stprintf_s(buffer, buffer_len, _T("BIT 1, A")); break;
	case 0x50: my_stprintf_s(buffer, buffer_len, _T("BIT 2, B")); break;
	case 0x51: my_stprintf_s(buffer, buffer_len, _T("BIT 2, C")); break;
	case 0x52: my_stprintf_s(buffer, buffer_len, _T("BIT 2, D")); break;
	case 0x53: my_stprintf_s(buffer, buffer_len, _T("BIT 2, E")); break;
	case 0x54: my_stprintf_s(buffer, buffer_len, _T("BIT 2, H")); break;
	case 0x55: my_stprintf_s(buffer, buffer_len, _T("BIT 2, L")); break;
	case 0x56: my_stprintf_s(buffer, buffer_len, _T("BIT 2, (HL)")); break;
	case 0x57: my_stprintf_s(buffer, buffer_len, _T("BIT 2, A")); break;
	case 0x58: my_stprintf_s(buffer, buffer_len, _T("BIT 3, B")); break;
	case 0x59: my_stprintf_s(buffer, buffer_len, _T("BIT 3, C")); break;
	case 0x5a: my_stprintf_s(buffer, buffer_len, _T("BIT 3, D")); break;
	case 0x5b: my_stprintf_s(buffer, buffer_len, _T("BIT 3, E")); break;
	case 0x5c: my_stprintf_s(buffer, buffer_len, _T("BIT 3, H")); break;
	case 0x5d: my_stprintf_s(buffer, buffer_len, _T("BIT 3, L")); break;
	case 0x5e: my_stprintf_s(buffer, buffer_len, _T("BIT 3, (HL)")); break;
	case 0x5f: my_stprintf_s(buffer, buffer_len, _T("BIT 3, A")); break;
	case 0x60: my_stprintf_s(buffer, buffer_len, _T("BIT 4, B")); break;
	case 0x61: my_stprintf_s(buffer, buffer_len, _T("BIT 4, C")); break;
	case 0x62: my_stprintf_s(buffer, buffer_len, _T("BIT 4, D")); break;
	case 0x63: my_stprintf_s(buffer, buffer_len, _T("BIT 4, E")); break;
	case 0x64: my_stprintf_s(buffer, buffer_len, _T("BIT 4, H")); break;
	case 0x65: my_stprintf_s(buffer, buffer_len, _T("BIT 4, L")); break;
	case 0x66: my_stprintf_s(buffer, buffer_len, _T("BIT 4, (HL)")); break;
	case 0x67: my_stprintf_s(buffer, buffer_len, _T("BIT 4, A")); break;
	case 0x68: my_stprintf_s(buffer, buffer_len, _T("BIT 5, B")); break;
	case 0x69: my_stprintf_s(buffer, buffer_len, _T("BIT 5, C")); break;
	case 0x6a: my_stprintf_s(buffer, buffer_len, _T("BIT 5, D")); break;
	case 0x6b: my_stprintf_s(buffer, buffer_len, _T("BIT 5, E")); break;
	case 0x6c: my_stprintf_s(buffer, buffer_len, _T("BIT 5, H")); break;
	case 0x6d: my_stprintf_s(buffer, buffer_len, _T("BIT 5, L")); break;
	case 0x6e: my_stprintf_s(buffer, buffer_len, _T("BIT 5, (HL)")); break;
	case 0x6f: my_stprintf_s(buffer, buffer_len, _T("BIT 5, A")); break;
	case 0x70: my_stprintf_s(buffer, buffer_len, _T("BIT 6, B")); break;
	case 0x71: my_stprintf_s(buffer, buffer_len, _T("BIT 6, C")); break;
	case 0x72: my_stprintf_s(buffer, buffer_len, _T("BIT 6, D")); break;
	case 0x73: my_stprintf_s(buffer, buffer_len, _T("BIT 6, E")); break;
	case 0x74: my_stprintf_s(buffer, buffer_len, _T("BIT 6, H")); break;
	case 0x75: my_stprintf_s(buffer, buffer_len, _T("BIT 6, L")); break;
	case 0x76: my_stprintf_s(buffer, buffer_len, _T("BIT 6, (HL)")); break;
	case 0x77: my_stprintf_s(buffer, buffer_len, _T("BIT 6, A")); break;
	case 0x78: my_stprintf_s(buffer, buffer_len, _T("BIT 7, B")); break;
	case 0x79: my_stprintf_s(buffer, buffer_len, _T("BIT 7, C")); break;
	case 0x7a: my_stprintf_s(buffer, buffer_len, _T("BIT 7, D")); break;
	case 0x7b: my_stprintf_s(buffer, buffer_len, _T("BIT 7, E")); break;
	case 0x7c: my_stprintf_s(buffer, buffer_len, _T("BIT 7, H")); break;
	case 0x7d: my_stprintf_s(buffer, buffer_len, _T("BIT 7, L")); break;
	case 0x7e: my_stprintf_s(buffer, buffer_len, _T("BIT 7, (HL)")); break;
	case 0x7f: my_stprintf_s(buffer, buffer_len, _T("BIT 7, A")); break;
	case 0x80: my_stprintf_s(buffer, buffer_len, _T("RES 0, B")); break;
	case 0x81: my_stprintf_s(buffer, buffer_len, _T("RES 0, C")); break;
	case 0x82: my_stprintf_s(buffer, buffer_len, _T("RES 0, D")); break;
	case 0x83: my_stprintf_s(buffer, buffer_len, _T("RES 0, E")); break;
	case 0x84: my_stprintf_s(buffer, buffer_len, _T("RES 0, H")); break;
	case 0x85: my_stprintf_s(buffer, buffer_len, _T("RES 0, L")); break;
	case 0x86: my_stprintf_s(buffer, buffer_len, _T("RES 0, (HL)")); break;
	case 0x87: my_stprintf_s(buffer, buffer_len, _T("RES 0, A")); break;
	case 0x88: my_stprintf_s(buffer, buffer_len, _T("RES 1, B")); break;
	case 0x89: my_stprintf_s(buffer, buffer_len, _T("RES 1, C")); break;
	case 0x8a: my_stprintf_s(buffer, buffer_len, _T("RES 1, D")); break;
	case 0x8b: my_stprintf_s(buffer, buffer_len, _T("RES 1, E")); break;
	case 0x8c: my_stprintf_s(buffer, buffer_len, _T("RES 1, H")); break;
	case 0x8d: my_stprintf_s(buffer, buffer_len, _T("RES 1, L")); break;
	case 0x8e: my_stprintf_s(buffer, buffer_len, _T("RES 1, (HL)")); break;
	case 0x8f: my_stprintf_s(buffer, buffer_len, _T("RES 1, A")); break;
	case 0x90: my_stprintf_s(buffer, buffer_len, _T("RES 2, B")); break;
	case 0x91: my_stprintf_s(buffer, buffer_len, _T("RES 2, C")); break;
	case 0x92: my_stprintf_s(buffer, buffer_len, _T("RES 2, D")); break;
	case 0x93: my_stprintf_s(buffer, buffer_len, _T("RES 2, E")); break;
	case 0x94: my_stprintf_s(buffer, buffer_len, _T("RES 2, H")); break;
	case 0x95: my_stprintf_s(buffer, buffer_len, _T("RES 2, L")); break;
	case 0x96: my_stprintf_s(buffer, buffer_len, _T("RES 2, (HL)")); break;
	case 0x97: my_stprintf_s(buffer, buffer_len, _T("RES 2, A")); break;
	case 0x98: my_stprintf_s(buffer, buffer_len, _T("RES 3, B")); break;
	case 0x99: my_stprintf_s(buffer, buffer_len, _T("RES 3, C")); break;
	case 0x9a: my_stprintf_s(buffer, buffer_len, _T("RES 3, D")); break;
	case 0x9b: my_stprintf_s(buffer, buffer_len, _T("RES 3, E")); break;
	case 0x9c: my_stprintf_s(buffer, buffer_len, _T("RES 3, H")); break;
	case 0x9d: my_stprintf_s(buffer, buffer_len, _T("RES 3, L")); break;
	case 0x9e: my_stprintf_s(buffer, buffer_len, _T("RES 3, (HL)")); break;
	case 0x9f: my_stprintf_s(buffer, buffer_len, _T("RES 3, A")); break;
	case 0xa0: my_stprintf_s(buffer, buffer_len, _T("RES 4, B")); break;
	case 0xa1: my_stprintf_s(buffer, buffer_len, _T("RES 4, C")); break;
	case 0xa2: my_stprintf_s(buffer, buffer_len, _T("RES 4, D")); break;
	case 0xa3: my_stprintf_s(buffer, buffer_len, _T("RES 4, E")); break;
	case 0xa4: my_stprintf_s(buffer, buffer_len, _T("RES 4, H")); break;
	case 0xa5: my_stprintf_s(buffer, buffer_len, _T("RES 4, L")); break;
	case 0xa6: my_stprintf_s(buffer, buffer_len, _T("RES 4, (HL)")); break;
	case 0xa7: my_stprintf_s(buffer, buffer_len, _T("RES 4, A")); break;
	case 0xa8: my_stprintf_s(buffer, buffer_len, _T("RES 5, B")); break;
	case 0xa9: my_stprintf_s(buffer, buffer_len, _T("RES 5, C")); break;
	case 0xaa: my_stprintf_s(buffer, buffer_len, _T("RES 5, D")); break;
	case 0xab: my_stprintf_s(buffer, buffer_len, _T("RES 5, E")); break;
	case 0xac: my_stprintf_s(buffer, buffer_len, _T("RES 5, H")); break;
	case 0xad: my_stprintf_s(buffer, buffer_len, _T("RES 5, L")); break;
	case 0xae: my_stprintf_s(buffer, buffer_len, _T("RES 5, (HL)")); break;
	case 0xaf: my_stprintf_s(buffer, buffer_len, _T("RES 5, A")); break;
	case 0xb0: my_stprintf_s(buffer, buffer_len, _T("RES 6, B")); break;
	case 0xb1: my_stprintf_s(buffer, buffer_len, _T("RES 6, C")); break;
	case 0xb2: my_stprintf_s(buffer, buffer_len, _T("RES 6, D")); break;
	case 0xb3: my_stprintf_s(buffer, buffer_len, _T("RES 6, E")); break;
	case 0xb4: my_stprintf_s(buffer, buffer_len, _T("RES 6, H")); break;
	case 0xb5: my_stprintf_s(buffer, buffer_len, _T("RES 6, L")); break;
	case 0xb6: my_stprintf_s(buffer, buffer_len, _T("RES 6, (HL)")); break;
	case 0xb7: my_stprintf_s(buffer, buffer_len, _T("RES 6, A")); break;
	case 0xb8: my_stprintf_s(buffer, buffer_len, _T("RES 7, B")); break;
	case 0xb9: my_stprintf_s(buffer, buffer_len, _T("RES 7, C")); break;
	case 0xba: my_stprintf_s(buffer, buffer_len, _T("RES 7, D")); break;
	case 0xbb: my_stprintf_s(buffer, buffer_len, _T("RES 7, E")); break;
	case 0xbc: my_stprintf_s(buffer, buffer_len, _T("RES 7, H")); break;
	case 0xbd: my_stprintf_s(buffer, buffer_len, _T("RES 7, L")); break;
	case 0xbe: my_stprintf_s(buffer, buffer_len, _T("RES 7, (HL)")); break;
	case 0xbf: my_stprintf_s(buffer, buffer_len, _T("RES 7, A")); break;
	case 0xc0: my_stprintf_s(buffer, buffer_len, _T("SET 0, B")); break;
	case 0xc1: my_stprintf_s(buffer, buffer_len, _T("SET 0, C")); break;
	case 0xc2: my_stprintf_s(buffer, buffer_len, _T("SET 0, D")); break;
	case 0xc3: my_stprintf_s(buffer, buffer_len, _T("SET 0, E")); break;
	case 0xc4: my_stprintf_s(buffer, buffer_len, _T("SET 0, H")); break;
	case 0xc5: my_stprintf_s(buffer, buffer_len, _T("SET 0, L")); break;
	case 0xc6: my_stprintf_s(buffer, buffer_len, _T("SET 0, (HL)")); break;
	case 0xc7: my_stprintf_s(buffer, buffer_len, _T("SET 0, A")); break;
	case 0xc8: my_stprintf_s(buffer, buffer_len, _T("SET 1, B")); break;
	case 0xc9: my_stprintf_s(buffer, buffer_len, _T("SET 1, C")); break;
	case 0xca: my_stprintf_s(buffer, buffer_len, _T("SET 1, D")); break;
	case 0xcb: my_stprintf_s(buffer, buffer_len, _T("SET 1, E")); break;
	case 0xcc: my_stprintf_s(buffer, buffer_len, _T("SET 1, H")); break;
	case 0xcd: my_stprintf_s(buffer, buffer_len, _T("SET 1, L")); break;
	case 0xce: my_stprintf_s(buffer, buffer_len, _T("SET 1, (HL)")); break;
	case 0xcf: my_stprintf_s(buffer, buffer_len, _T("SET 1, A")); break;
	case 0xd0: my_stprintf_s(buffer, buffer_len, _T("SET 2, B")); break;
	case 0xd1: my_stprintf_s(buffer, buffer_len, _T("SET 2, C")); break;
	case 0xd2: my_stprintf_s(buffer, buffer_len, _T("SET 2, D")); break;
	case 0xd3: my_stprintf_s(buffer, buffer_len, _T("SET 2, E")); break;
	case 0xd4: my_stprintf_s(buffer, buffer_len, _T("SET 2, H")); break;
	case 0xd5: my_stprintf_s(buffer, buffer_len, _T("SET 2, L")); break;
	case 0xd6: my_stprintf_s(buffer, buffer_len, _T("SET 2, (HL)")); break;
	case 0xd7: my_stprintf_s(buffer, buffer_len, _T("SET 2, A")); break;
	case 0xd8: my_stprintf_s(buffer, buffer_len, _T("SET 3, B")); break;
	case 0xd9: my_stprintf_s(buffer, buffer_len, _T("SET 3, C")); break;
	case 0xda: my_stprintf_s(buffer, buffer_len, _T("SET 3, D")); break;
	case 0xdb: my_stprintf_s(buffer, buffer_len, _T("SET 3, E")); break;
	case 0xdc: my_stprintf_s(buffer, buffer_len, _T("SET 3, H")); break;
	case 0xdd: my_stprintf_s(buffer, buffer_len, _T("SET 3, L")); break;
	case 0xde: my_stprintf_s(buffer, buffer_len, _T("SET 3, (HL)")); break;
	case 0xdf: my_stprintf_s(buffer, buffer_len, _T("SET 3, A")); break;
	case 0xe0: my_stprintf_s(buffer, buffer_len, _T("SET 4, B")); break;
	case 0xe1: my_stprintf_s(buffer, buffer_len, _T("SET 4, C")); break;
	case 0xe2: my_stprintf_s(buffer, buffer_len, _T("SET 4, D")); break;
	case 0xe3: my_stprintf_s(buffer, buffer_len, _T("SET 4, E")); break;
	case 0xe4: my_stprintf_s(buffer, buffer_len, _T("SET 4, H")); break;
	case 0xe5: my_stprintf_s(buffer, buffer_len, _T("SET 4, L")); break;
	case 0xe6: my_stprintf_s(buffer, buffer_len, _T("SET 4, (HL)")); break;
	case 0xe7: my_stprintf_s(buffer, buffer_len, _T("SET 4, A")); break;
	case 0xe8: my_stprintf_s(buffer, buffer_len, _T("SET 5, B")); break;
	case 0xe9: my_stprintf_s(buffer, buffer_len, _T("SET 5, C")); break;
	case 0xea: my_stprintf_s(buffer, buffer_len, _T("SET 5, D")); break;
	case 0xeb: my_stprintf_s(buffer, buffer_len, _T("SET 5, E")); break;
	case 0xec: my_stprintf_s(buffer, buffer_len, _T("SET 5, H")); break;
	case 0xed: my_stprintf_s(buffer, buffer_len, _T("SET 5, L")); break;
	case 0xee: my_stprintf_s(buffer, buffer_len, _T("SET 5, (HL)")); break;
	case 0xef: my_stprintf_s(buffer, buffer_len, _T("SET 5, A")); break;
	case 0xf0: my_stprintf_s(buffer, buffer_len, _T("SET 6, B")); break;
	case 0xf1: my_stprintf_s(buffer, buffer_len, _T("SET 6, C")); break;
	case 0xf2: my_stprintf_s(buffer, buffer_len, _T("SET 6, D")); break;
	case 0xf3: my_stprintf_s(buffer, buffer_len, _T("SET 6, E")); break;
	case 0xf4: my_stprintf_s(buffer, buffer_len, _T("SET 6, H")); break;
	case 0xf5: my_stprintf_s(buffer, buffer_len, _T("SET 6, L")); break;
	case 0xf6: my_stprintf_s(buffer, buffer_len, _T("SET 6, (HL)")); break;
	case 0xf7: my_stprintf_s(buffer, buffer_len, _T("SET 6, A")); break;
	case 0xf8: my_stprintf_s(buffer, buffer_len, _T("SET 7, B")); break;
	case 0xf9: my_stprintf_s(buffer, buffer_len, _T("SET 7, C")); break;
	case 0xfa: my_stprintf_s(buffer, buffer_len, _T("SET 7, D")); break;
	case 0xfb: my_stprintf_s(buffer, buffer_len, _T("SET 7, E")); break;
	case 0xfc: my_stprintf_s(buffer, buffer_len, _T("SET 7, H")); break;
	case 0xfd: my_stprintf_s(buffer, buffer_len, _T("SET 7, L")); break;
	case 0xfe: my_stprintf_s(buffer, buffer_len, _T("SET 7, (HL)")); break;
	case 0xff: my_stprintf_s(buffer, buffer_len, _T("SET 7, A")); break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}

static void dasm_dd(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol)
{
	uint8_t code = dasm_fetchop();
	int8_t ofs;

	switch(code) {
	case 0x09: my_stprintf_s(buffer, buffer_len, _T("ADD IX, BC")); break;
	case 0x19: my_stprintf_s(buffer, buffer_len, _T("ADD IX, DE")); break;
	case 0x21: my_stprintf_s(buffer, buffer_len, _T("LD IX, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x22: my_stprintf_s(buffer, buffer_len, _T("LD (%s), IX"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x23: my_stprintf_s(buffer, buffer_len, _T("INC IX")); break;
	case 0x24: my_stprintf_s(buffer, buffer_len, _T("INC HX")); break;
	case 0x25: my_stprintf_s(buffer, buffer_len, _T("DEC HX")); break;
	case 0x26: my_stprintf_s(buffer, buffer_len, _T("LD HX, %02x"), debug_fetch8()); break;
	case 0x29: my_stprintf_s(buffer, buffer_len, _T("ADD IX, IX")); break;
	case 0x2a: my_stprintf_s(buffer, buffer_len, _T("LD IX, (%s)"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x2b: my_stprintf_s(buffer, buffer_len, _T("DEC IX")); break;
	case 0x2c: my_stprintf_s(buffer, buffer_len, _T("INC LX")); break;
	case 0x2d: my_stprintf_s(buffer, buffer_len, _T("DEC LX")); break;
	case 0x2e: my_stprintf_s(buffer, buffer_len, _T("LD LX, %02x"), debug_fetch8()); break;
	case 0x34: my_stprintf_s(buffer, buffer_len, _T("INC (IX+(%d))"), debug_fetch8_rel()); break;
	case 0x35: my_stprintf_s(buffer, buffer_len, _T("DEC (IX+(%d))"), debug_fetch8_rel()); break;
	case 0x36: ofs = debug_fetch8_rel(); my_stprintf_s(buffer, buffer_len, _T("LD (IX+(%d)), %02x"), ofs, debug_fetch8()); break;
	case 0x39: my_stprintf_s(buffer, buffer_len, _T("ADD IX, SP")); break;
	case 0x44: my_stprintf_s(buffer, buffer_len, _T("LD B, HX")); break;
	case 0x45: my_stprintf_s(buffer, buffer_len, _T("LD B, LX")); break;
	case 0x46: my_stprintf_s(buffer, buffer_len, _T("LD B, (IX+(%d))"), debug_fetch8_rel()); break;
	case 0x4c: my_stprintf_s(buffer, buffer_len, _T("LD C, HX")); break;
	case 0x4d: my_stprintf_s(buffer, buffer_len, _T("LD C, LX")); break;
	case 0x4e: my_stprintf_s(buffer, buffer_len, _T("LD C, (IX+(%d))"), debug_fetch8_rel()); break;
	case 0x54: my_stprintf_s(buffer, buffer_len, _T("LD D, HX")); break;
	case 0x55: my_stprintf_s(buffer, buffer_len, _T("LD D, LX")); break;
	case 0x56: my_stprintf_s(buffer, buffer_len, _T("LD D, (IX+(%d))"), debug_fetch8_rel()); break;
	case 0x5c: my_stprintf_s(buffer, buffer_len, _T("LD E, HX")); break;
	case 0x5d: my_stprintf_s(buffer, buffer_len, _T("LD E, LX")); break;
	case 0x5e: my_stprintf_s(buffer, buffer_len, _T("LD E, (IX+(%d))"), debug_fetch8_rel()); break;
	case 0x60: my_stprintf_s(buffer, buffer_len, _T("LD HX, B")); break;
	case 0x61: my_stprintf_s(buffer, buffer_len, _T("LD HX, C")); break;
	case 0x62: my_stprintf_s(buffer, buffer_len, _T("LD HX, D")); break;
	case 0x63: my_stprintf_s(buffer, buffer_len, _T("LD HX, E")); break;
	case 0x64: my_stprintf_s(buffer, buffer_len, _T("LD HX, HX")); break;
	case 0x65: my_stprintf_s(buffer, buffer_len, _T("LD HX, LX")); break;
	case 0x66: my_stprintf_s(buffer, buffer_len, _T("LD H, (IX+(%d))"), debug_fetch8_rel()); break;
	case 0x67: my_stprintf_s(buffer, buffer_len, _T("LD HX, A")); break;
	case 0x68: my_stprintf_s(buffer, buffer_len, _T("LD LX, B")); break;
	case 0x69: my_stprintf_s(buffer, buffer_len, _T("LD LX, C")); break;
	case 0x6a: my_stprintf_s(buffer, buffer_len, _T("LD LX, D")); break;
	case 0x6b: my_stprintf_s(buffer, buffer_len, _T("LD LX, E")); break;
	case 0x6c: my_stprintf_s(buffer, buffer_len, _T("LD LX, HX")); break;
	case 0x6d: my_stprintf_s(buffer, buffer_len, _T("LD LX, LX")); break;
	case 0x6e: my_stprintf_s(buffer, buffer_len, _T("LD L, (IX+(%d))"), debug_fetch8_rel()); break;
	case 0x6f: my_stprintf_s(buffer, buffer_len, _T("LD LX, A")); break;
	case 0x70: my_stprintf_s(buffer, buffer_len, _T("LD (IX+(%d)), B"), debug_fetch8_rel()); break;
	case 0x71: my_stprintf_s(buffer, buffer_len, _T("LD (IX+(%d)), C"), debug_fetch8_rel()); break;
	case 0x72: my_stprintf_s(buffer, buffer_len, _T("LD (IX+(%d)), D"), debug_fetch8_rel()); break;
	case 0x73: my_stprintf_s(buffer, buffer_len, _T("LD (IX+(%d)), E"), debug_fetch8_rel()); break;
	case 0x74: my_stprintf_s(buffer, buffer_len, _T("LD (IX+(%d)), H"), debug_fetch8_rel()); break;
	case 0x75: my_stprintf_s(buffer, buffer_len, _T("LD (IX+(%d)), L"), debug_fetch8_rel()); break;
	case 0x77: my_stprintf_s(buffer, buffer_len, _T("LD (IX+(%d)), A"), debug_fetch8_rel()); break;
	case 0x7c: my_stprintf_s(buffer, buffer_len, _T("LD A, HX")); break;
	case 0x7d: my_stprintf_s(buffer, buffer_len, _T("LD A, LX")); break;
	case 0x7e: my_stprintf_s(buffer, buffer_len, _T("LD A, (IX+(%d))"), debug_fetch8_rel()); break;
	case 0x84: my_stprintf_s(buffer, buffer_len, _T("ADD A, HX")); break;
	case 0x85: my_stprintf_s(buffer, buffer_len, _T("ADD A, LX")); break;
	case 0x86: my_stprintf_s(buffer, buffer_len, _T("ADD A, (IX+(%d))"), debug_fetch8_rel()); break;
	case 0x8c: my_stprintf_s(buffer, buffer_len, _T("ADC A, HX")); break;
	case 0x8d: my_stprintf_s(buffer, buffer_len, _T("ADC A, LX")); break;
	case 0x8e: my_stprintf_s(buffer, buffer_len, _T("ADC A, (IX+(%d))"), debug_fetch8_rel()); break;
	case 0x94: my_stprintf_s(buffer, buffer_len, _T("SUB HX")); break;
	case 0x95: my_stprintf_s(buffer, buffer_len, _T("SUB LX")); break;
	case 0x96: my_stprintf_s(buffer, buffer_len, _T("SUB (IX+(%d))"), debug_fetch8_rel()); break;
	case 0x9c: my_stprintf_s(buffer, buffer_len, _T("SBC A, HX")); break;
	case 0x9d: my_stprintf_s(buffer, buffer_len, _T("SBC A, LX")); break;
	case 0x9e: my_stprintf_s(buffer, buffer_len, _T("SBC A, (IX+(%d))"), debug_fetch8_rel()); break;
	case 0xa4: my_stprintf_s(buffer, buffer_len, _T("AND HX")); break;
	case 0xa5: my_stprintf_s(buffer, buffer_len, _T("AND LX")); break;
	case 0xa6: my_stprintf_s(buffer, buffer_len, _T("AND (IX+(%d))"), debug_fetch8_rel()); break;
	case 0xac: my_stprintf_s(buffer, buffer_len, _T("XOR HX")); break;
	case 0xad: my_stprintf_s(buffer, buffer_len, _T("XOR LX")); break;
	case 0xae: my_stprintf_s(buffer, buffer_len, _T("XOR (IX+(%d))"), debug_fetch8_rel()); break;
	case 0xb4: my_stprintf_s(buffer, buffer_len, _T("OR HX")); break;
	case 0xb5: my_stprintf_s(buffer, buffer_len, _T("OR LX")); break;
	case 0xb6: my_stprintf_s(buffer, buffer_len, _T("OR (IX+(%d))"), debug_fetch8_rel()); break;
	case 0xbc: my_stprintf_s(buffer, buffer_len, _T("CP HX")); break;
	case 0xbd: my_stprintf_s(buffer, buffer_len, _T("CP LX")); break;
	case 0xbe: my_stprintf_s(buffer, buffer_len, _T("CP (IX+(%d))"), debug_fetch8_rel()); break;
	case 0xcb: dasm_ddcb(pc, buffer, buffer_len, first_symbol); break;
	case 0xe1: my_stprintf_s(buffer, buffer_len, _T("POP IX")); break;
	case 0xe3: my_stprintf_s(buffer, buffer_len, _T("EX (SP), IX")); break;
	case 0xe5: my_stprintf_s(buffer, buffer_len, _T("PUSH IX")); break;
	case 0xe9: my_stprintf_s(buffer, buffer_len, _T("JP (IX)")); break;
	case 0xf9: my_stprintf_s(buffer, buffer_len, _T("LD SP, IX")); break;
	default:   my_stprintf_s(buffer, buffer_len, _T("DB dd")); z80_dasm_ptr--; break;
	}
}

void dasm_ed(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol)
{
	uint8_t code = dasm_fetchop();

	switch(code) {
	case 0x40: my_stprintf_s(buffer, buffer_len, _T("IN B, (C)")); break;
	case 0x41: my_stprintf_s(buffer, buffer_len, _T("OUT (C), B")); break;
	case 0x42: my_stprintf_s(buffer, buffer_len, _T("SBC HL, BC")); break;
	case 0x43: my_stprintf_s(buffer, buffer_len, _T("LD (%s), BC"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x44: my_stprintf_s(buffer, buffer_len, _T("NEG")); break;
	case 0x45: my_stprintf_s(buffer, buffer_len, _T("RETN")); break;
	case 0x46: my_stprintf_s(buffer, buffer_len, _T("IM 0")); break;
	case 0x47: my_stprintf_s(buffer, buffer_len, _T("LD I, A")); break;
	case 0x48: my_stprintf_s(buffer, buffer_len, _T("IN C, (C)")); break;
	case 0x49: my_stprintf_s(buffer, buffer_len, _T("OUT (C), C")); break;
	case 0x4a: my_stprintf_s(buffer, buffer_len, _T("ADC HL, BC")); break;
	case 0x4b: my_stprintf_s(buffer, buffer_len, _T("LD BC, (%s)"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x4c: my_stprintf_s(buffer, buffer_len, _T("NEG")); break;
	case 0x4d: my_stprintf_s(buffer, buffer_len, _T("RETI")); break;
	case 0x4e: my_stprintf_s(buffer, buffer_len, _T("IM 0")); break;
	case 0x4f: my_stprintf_s(buffer, buffer_len, _T("LD R, A")); break;
	case 0x50: my_stprintf_s(buffer, buffer_len, _T("IN D, (C)")); break;
	case 0x51: my_stprintf_s(buffer, buffer_len, _T("OUT (C), D")); break;
	case 0x52: my_stprintf_s(buffer, buffer_len, _T("SBC HL, DE")); break;
	case 0x53: my_stprintf_s(buffer, buffer_len, _T("LD (%s), DE"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x54: my_stprintf_s(buffer, buffer_len, _T("NEG")); break;
	case 0x55: my_stprintf_s(buffer, buffer_len, _T("RETN")); break;
	case 0x56: my_stprintf_s(buffer, buffer_len, _T("IM 1")); break;
	case 0x57: my_stprintf_s(buffer, buffer_len, _T("LD A, I")); break;
	case 0x58: my_stprintf_s(buffer, buffer_len, _T("IN E, (C)")); break;
	case 0x59: my_stprintf_s(buffer, buffer_len, _T("OUT (C), E")); break;
	case 0x5a: my_stprintf_s(buffer, buffer_len, _T("ADC HL, DE")); break;
	case 0x5b: my_stprintf_s(buffer, buffer_len, _T("LD DE, (%s)"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x5c: my_stprintf_s(buffer, buffer_len, _T("NEG")); break;
	case 0x5d: my_stprintf_s(buffer, buffer_len, _T("RETI")); break;
	case 0x5e: my_stprintf_s(buffer, buffer_len, _T("IM 2")); break;
	case 0x5f: my_stprintf_s(buffer, buffer_len, _T("LD A, R")); break;
	case 0x60: my_stprintf_s(buffer, buffer_len, _T("IN H, (C)")); break;
	case 0x61: my_stprintf_s(buffer, buffer_len, _T("OUT (C), H")); break;
	case 0x62: my_stprintf_s(buffer, buffer_len, _T("SBC HL, HL")); break;
	case 0x63: my_stprintf_s(buffer, buffer_len, _T("LD (%s), HL"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x64: my_stprintf_s(buffer, buffer_len, _T("NEG")); break;
	case 0x65: my_stprintf_s(buffer, buffer_len, _T("RETN")); break;
	case 0x66: my_stprintf_s(buffer, buffer_len, _T("IM 0")); break;
	case 0x67: my_stprintf_s(buffer, buffer_len, _T("RRD (HL)")); break;
	case 0x68: my_stprintf_s(buffer, buffer_len, _T("IN L, (C)")); break;
	case 0x69: my_stprintf_s(buffer, buffer_len, _T("OUT (C), L")); break;
	case 0x6a: my_stprintf_s(buffer, buffer_len, _T("ADC HL, HL")); break;
	case 0x6b: my_stprintf_s(buffer, buffer_len, _T("LD HL, (%s)"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x6c: my_stprintf_s(buffer, buffer_len, _T("NEG")); break;
	case 0x6d: my_stprintf_s(buffer, buffer_len, _T("RETI")); break;
	case 0x6e: my_stprintf_s(buffer, buffer_len, _T("IM 0")); break;
	case 0x6f: my_stprintf_s(buffer, buffer_len, _T("RLD (HL)")); break;
	case 0x70: my_stprintf_s(buffer, buffer_len, _T("IN F, (C)")); break;
	case 0x71: my_stprintf_s(buffer, buffer_len, _T("OUT (C), 0")); break;
	case 0x72: my_stprintf_s(buffer, buffer_len, _T("SBC HL, SP")); break;
	case 0x73: my_stprintf_s(buffer, buffer_len, _T("LD (%s), SP"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x74: my_stprintf_s(buffer, buffer_len, _T("NEG")); break;
	case 0x75: my_stprintf_s(buffer, buffer_len, _T("RETN")); break;
	case 0x76: my_stprintf_s(buffer, buffer_len, _T("IM 1")); break;
	case 0x78: my_stprintf_s(buffer, buffer_len, _T("IN A, (C)")); break;
	case 0x79: my_stprintf_s(buffer, buffer_len, _T("OUT (C), A")); break;
	case 0x7a: my_stprintf_s(buffer, buffer_len, _T("ADC HL, SP")); break;
	case 0x7b: my_stprintf_s(buffer, buffer_len, _T("LD SP, (%s)"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x7c: my_stprintf_s(buffer, buffer_len, _T("NEG")); break;
	case 0x7d: my_stprintf_s(buffer, buffer_len, _T("RETI")); break;
	case 0x7e: my_stprintf_s(buffer, buffer_len, _T("IM 2")); break;
	case 0xa0: my_stprintf_s(buffer, buffer_len, _T("LDI")); break;
	case 0xa1: my_stprintf_s(buffer, buffer_len, _T("CPI")); break;
	case 0xa2: my_stprintf_s(buffer, buffer_len, _T("INI")); break;
	case 0xa3: my_stprintf_s(buffer, buffer_len, _T("OUTI")); break;
	case 0xa8: my_stprintf_s(buffer, buffer_len, _T("LDD")); break;
	case 0xa9: my_stprintf_s(buffer, buffer_len, _T("CPD")); break;
	case 0xaa: my_stprintf_s(buffer, buffer_len, _T("IND")); break;
	case 0xab: my_stprintf_s(buffer, buffer_len, _T("OUTD")); break;
	case 0xb0: my_stprintf_s(buffer, buffer_len, _T("LDIR")); break;
	case 0xb1: my_stprintf_s(buffer, buffer_len, _T("CPIR")); break;
	case 0xb2: my_stprintf_s(buffer, buffer_len, _T("INIR")); break;
	case 0xb3: my_stprintf_s(buffer, buffer_len, _T("OTIR")); break;
	case 0xb8: my_stprintf_s(buffer, buffer_len, _T("LDDR")); break;
	case 0xb9: my_stprintf_s(buffer, buffer_len, _T("CPDR")); break;
	case 0xba: my_stprintf_s(buffer, buffer_len, _T("INDR")); break;
	case 0xbb: my_stprintf_s(buffer, buffer_len, _T("OTDR")); break;
	default:   my_stprintf_s(buffer, buffer_len, _T("DB ed")); z80_dasm_ptr--; break;
	}
}

void dasm_fd(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol)
{
	uint8_t code = dasm_fetchop();
	int8_t ofs;

	switch(code) {
	case 0x09: my_stprintf_s(buffer, buffer_len, _T("ADD IY, BC")); break;
	case 0x19: my_stprintf_s(buffer, buffer_len, _T("ADD IY, DE")); break;
	case 0x21: my_stprintf_s(buffer, buffer_len, _T("LD IY, %s"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x22: my_stprintf_s(buffer, buffer_len, _T("LD (%s), IY"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x23: my_stprintf_s(buffer, buffer_len, _T("INC IY")); break;
	case 0x24: my_stprintf_s(buffer, buffer_len, _T("INC HY")); break;
	case 0x25: my_stprintf_s(buffer, buffer_len, _T("DEC HY")); break;
	case 0x26: my_stprintf_s(buffer, buffer_len, _T("LD HY, %02x"), debug_fetch8()); break;
	case 0x29: my_stprintf_s(buffer, buffer_len, _T("ADD IY, IY")); break;
	case 0x2a: my_stprintf_s(buffer, buffer_len, _T("LD IY, (%s)"), get_value_or_symbol(first_symbol, _T("%04x"), debug_fetch16())); break;
	case 0x2b: my_stprintf_s(buffer, buffer_len, _T("DEC IY")); break;
	case 0x2c: my_stprintf_s(buffer, buffer_len, _T("INC LY")); break;
	case 0x2d: my_stprintf_s(buffer, buffer_len, _T("DEC LY")); break;
	case 0x2e: my_stprintf_s(buffer, buffer_len, _T("LD LY, %02x"), debug_fetch8()); break;
	case 0x34: my_stprintf_s(buffer, buffer_len, _T("INC (IY+(%d))"), debug_fetch8_rel()); break;
	case 0x35: my_stprintf_s(buffer, buffer_len, _T("DEC (IY+(%d))"), debug_fetch8_rel()); break;
	case 0x36: ofs = debug_fetch8_rel(); my_stprintf_s(buffer, buffer_len, _T("LD (IY+(%d)), %02x"), ofs, debug_fetch8()); break;
	case 0x39: my_stprintf_s(buffer, buffer_len, _T("ADD IY, SP")); break;
	case 0x44: my_stprintf_s(buffer, buffer_len, _T("LD B, HY")); break;
	case 0x45: my_stprintf_s(buffer, buffer_len, _T("LD B, LY")); break;
	case 0x46: my_stprintf_s(buffer, buffer_len, _T("LD B, (IY+(%d))"), debug_fetch8_rel()); break;
	case 0x4c: my_stprintf_s(buffer, buffer_len, _T("LD C, HY")); break;
	case 0x4d: my_stprintf_s(buffer, buffer_len, _T("LD C, LY")); break;
	case 0x4e: my_stprintf_s(buffer, buffer_len, _T("LD C, (IY+(%d))"), debug_fetch8_rel()); break;
	case 0x54: my_stprintf_s(buffer, buffer_len, _T("LD D, HY")); break;
	case 0x55: my_stprintf_s(buffer, buffer_len, _T("LD D, LY")); break;
	case 0x56: my_stprintf_s(buffer, buffer_len, _T("LD D, (IY+(%d))"), debug_fetch8_rel()); break;
	case 0x5c: my_stprintf_s(buffer, buffer_len, _T("LD E, HY")); break;
	case 0x5d: my_stprintf_s(buffer, buffer_len, _T("LD E, LY")); break;
	case 0x5e: my_stprintf_s(buffer, buffer_len, _T("LD E, (IY+(%d))"), debug_fetch8_rel()); break;
	case 0x60: my_stprintf_s(buffer, buffer_len, _T("LD HY, B")); break;
	case 0x61: my_stprintf_s(buffer, buffer_len, _T("LD HY, C")); break;
	case 0x62: my_stprintf_s(buffer, buffer_len, _T("LD HY, D")); break;
	case 0x63: my_stprintf_s(buffer, buffer_len, _T("LD HY, E")); break;
	case 0x64: my_stprintf_s(buffer, buffer_len, _T("LD HY, HY")); break;
	case 0x65: my_stprintf_s(buffer, buffer_len, _T("LD HY, LY")); break;
	case 0x66: my_stprintf_s(buffer, buffer_len, _T("LD H, (IY+(%d))"), debug_fetch8_rel()); break;
	case 0x67: my_stprintf_s(buffer, buffer_len, _T("LD HY, A")); break;
	case 0x68: my_stprintf_s(buffer, buffer_len, _T("LD LY, B")); break;
	case 0x69: my_stprintf_s(buffer, buffer_len, _T("LD LY, C")); break;
	case 0x6a: my_stprintf_s(buffer, buffer_len, _T("LD LY, D")); break;
	case 0x6b: my_stprintf_s(buffer, buffer_len, _T("LD LY, E")); break;
	case 0x6c: my_stprintf_s(buffer, buffer_len, _T("LD LY, HY")); break;
	case 0x6d: my_stprintf_s(buffer, buffer_len, _T("LD LY, LY")); break;
	case 0x6e: my_stprintf_s(buffer, buffer_len, _T("LD L, (IY+(%d))"), debug_fetch8_rel()); break;
	case 0x6f: my_stprintf_s(buffer, buffer_len, _T("LD LY, A")); break;
	case 0x70: my_stprintf_s(buffer, buffer_len, _T("LD (IY+(%d)), B"), debug_fetch8_rel()); break;
	case 0x71: my_stprintf_s(buffer, buffer_len, _T("LD (IY+(%d)), C"), debug_fetch8_rel()); break;
	case 0x72: my_stprintf_s(buffer, buffer_len, _T("LD (IY+(%d)), D"), debug_fetch8_rel()); break;
	case 0x73: my_stprintf_s(buffer, buffer_len, _T("LD (IY+(%d)), E"), debug_fetch8_rel()); break;
	case 0x74: my_stprintf_s(buffer, buffer_len, _T("LD (IY+(%d)), H"), debug_fetch8_rel()); break;
	case 0x75: my_stprintf_s(buffer, buffer_len, _T("LD (IY+(%d)), L"), debug_fetch8_rel()); break;
	case 0x77: my_stprintf_s(buffer, buffer_len, _T("LD (IY+(%d)), A"), debug_fetch8_rel()); break;
	case 0x7c: my_stprintf_s(buffer, buffer_len, _T("LD A, HY")); break;
	case 0x7d: my_stprintf_s(buffer, buffer_len, _T("LD A, LY")); break;
	case 0x7e: my_stprintf_s(buffer, buffer_len, _T("LD A, (IY+(%d))"), debug_fetch8_rel()); break;
	case 0x84: my_stprintf_s(buffer, buffer_len, _T("ADD A, HY")); break;
	case 0x85: my_stprintf_s(buffer, buffer_len, _T("ADD A, LY")); break;
	case 0x86: my_stprintf_s(buffer, buffer_len, _T("ADD A, (IY+(%d))"), debug_fetch8_rel()); break;
	case 0x8c: my_stprintf_s(buffer, buffer_len, _T("ADC A, HY")); break;
	case 0x8d: my_stprintf_s(buffer, buffer_len, _T("ADC A, LY")); break;
	case 0x8e: my_stprintf_s(buffer, buffer_len, _T("ADC A, (IY+(%d))"), debug_fetch8_rel()); break;
	case 0x94: my_stprintf_s(buffer, buffer_len, _T("SUB HY")); break;
	case 0x95: my_stprintf_s(buffer, buffer_len, _T("SUB LY")); break;
	case 0x96: my_stprintf_s(buffer, buffer_len, _T("SUB (IY+(%d))"), debug_fetch8_rel()); break;
	case 0x9c: my_stprintf_s(buffer, buffer_len, _T("SBC A, HY")); break;
	case 0x9d: my_stprintf_s(buffer, buffer_len, _T("SBC A, LY")); break;
	case 0x9e: my_stprintf_s(buffer, buffer_len, _T("SBC A, (IY+(%d))"), debug_fetch8_rel()); break;
	case 0xa4: my_stprintf_s(buffer, buffer_len, _T("AND HY")); break;
	case 0xa5: my_stprintf_s(buffer, buffer_len, _T("AND LY")); break;
	case 0xa6: my_stprintf_s(buffer, buffer_len, _T("AND (IY+(%d))"), debug_fetch8_rel()); break;
	case 0xac: my_stprintf_s(buffer, buffer_len, _T("XOR HY")); break;
	case 0xad: my_stprintf_s(buffer, buffer_len, _T("XOR LY")); break;
	case 0xae: my_stprintf_s(buffer, buffer_len, _T("XOR (IY+(%d))"), debug_fetch8_rel()); break;
	case 0xb4: my_stprintf_s(buffer, buffer_len, _T("OR HY")); break;
	case 0xb5: my_stprintf_s(buffer, buffer_len, _T("OR LY")); break;
	case 0xb6: my_stprintf_s(buffer, buffer_len, _T("OR (IY+(%d))"), debug_fetch8_rel()); break;
	case 0xbc: my_stprintf_s(buffer, buffer_len, _T("CP HY")); break;
	case 0xbd: my_stprintf_s(buffer, buffer_len, _T("CP LY")); break;
	case 0xbe: my_stprintf_s(buffer, buffer_len, _T("CP (IY+(%d))"), debug_fetch8_rel()); break;
	case 0xcb: dasm_fdcb(pc, buffer, buffer_len, first_symbol); break;
	case 0xe1: my_stprintf_s(buffer, buffer_len, _T("POP IY")); break;
	case 0xe3: my_stprintf_s(buffer, buffer_len, _T("EX (SP), IY")); break;
	case 0xe5: my_stprintf_s(buffer, buffer_len, _T("PUSH IY")); break;
	case 0xe9: my_stprintf_s(buffer, buffer_len, _T("JP (IY)")); break;
	case 0xf9: my_stprintf_s(buffer, buffer_len, _T("LD SP, IY")); break;
	default:   my_stprintf_s(buffer, buffer_len, _T("DB fd")); z80_dasm_ptr--; break;
	}
}

static void dasm_ddcb(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol)
{
	int8_t ofs = debug_fetch8_rel();
	uint8_t code = debug_fetch8();

	switch(code) {
	case 0x00: my_stprintf_s(buffer, buffer_len, _T("RLC B=(IX+(%d))"), ofs); break;
	case 0x01: my_stprintf_s(buffer, buffer_len, _T("RLC C=(IX+(%d))"), ofs); break;
	case 0x02: my_stprintf_s(buffer, buffer_len, _T("RLC D=(IX+(%d))"), ofs); break;
	case 0x03: my_stprintf_s(buffer, buffer_len, _T("RLC E=(IX+(%d))"), ofs); break;
	case 0x04: my_stprintf_s(buffer, buffer_len, _T("RLC H=(IX+(%d))"), ofs); break;
	case 0x05: my_stprintf_s(buffer, buffer_len, _T("RLC L=(IX+(%d))"), ofs); break;
	case 0x06: my_stprintf_s(buffer, buffer_len, _T("RLC (IX+(%d))"), ofs); break;
	case 0x07: my_stprintf_s(buffer, buffer_len, _T("RLC A=(IX+(%d))"), ofs); break;
	case 0x08: my_stprintf_s(buffer, buffer_len, _T("RRC B=(IX+(%d))"), ofs); break;
	case 0x09: my_stprintf_s(buffer, buffer_len, _T("RRC C=(IX+(%d))"), ofs); break;
	case 0x0a: my_stprintf_s(buffer, buffer_len, _T("RRC D=(IX+(%d))"), ofs); break;
	case 0x0b: my_stprintf_s(buffer, buffer_len, _T("RRC E=(IX+(%d))"), ofs); break;
	case 0x0c: my_stprintf_s(buffer, buffer_len, _T("RRC H=(IX+(%d))"), ofs); break;
	case 0x0d: my_stprintf_s(buffer, buffer_len, _T("RRC L=(IX+(%d))"), ofs); break;
	case 0x0e: my_stprintf_s(buffer, buffer_len, _T("RRC (IX+(%d))"), ofs); break;
	case 0x0f: my_stprintf_s(buffer, buffer_len, _T("RRC A=(IX+(%d))"), ofs); break;
	case 0x10: my_stprintf_s(buffer, buffer_len, _T("RL B=(IX+(%d))"), ofs); break;
	case 0x11: my_stprintf_s(buffer, buffer_len, _T("RL C=(IX+(%d))"), ofs); break;
	case 0x12: my_stprintf_s(buffer, buffer_len, _T("RL D=(IX+(%d))"), ofs); break;
	case 0x13: my_stprintf_s(buffer, buffer_len, _T("RL E=(IX+(%d))"), ofs); break;
	case 0x14: my_stprintf_s(buffer, buffer_len, _T("RL H=(IX+(%d))"), ofs); break;
	case 0x15: my_stprintf_s(buffer, buffer_len, _T("RL L=(IX+(%d))"), ofs); break;
	case 0x16: my_stprintf_s(buffer, buffer_len, _T("RL (IX+(%d))"), ofs); break;
	case 0x17: my_stprintf_s(buffer, buffer_len, _T("RL A=(IX+(%d))"), ofs); break;
	case 0x18: my_stprintf_s(buffer, buffer_len, _T("RR B=(IX+(%d))"), ofs); break;
	case 0x19: my_stprintf_s(buffer, buffer_len, _T("RR C=(IX+(%d))"), ofs); break;
	case 0x1a: my_stprintf_s(buffer, buffer_len, _T("RR D=(IX+(%d))"), ofs); break;
	case 0x1b: my_stprintf_s(buffer, buffer_len, _T("RR E=(IX+(%d))"), ofs); break;
	case 0x1c: my_stprintf_s(buffer, buffer_len, _T("RR H=(IX+(%d))"), ofs); break;
	case 0x1d: my_stprintf_s(buffer, buffer_len, _T("RR L=(IX+(%d))"), ofs); break;
	case 0x1e: my_stprintf_s(buffer, buffer_len, _T("RR (IX+(%d))"), ofs); break;
	case 0x1f: my_stprintf_s(buffer, buffer_len, _T("RR A=(IX+(%d))"), ofs); break;
	case 0x20: my_stprintf_s(buffer, buffer_len, _T("SLA B=(IX+(%d))"), ofs); break;
	case 0x21: my_stprintf_s(buffer, buffer_len, _T("SLA C=(IX+(%d))"), ofs); break;
	case 0x22: my_stprintf_s(buffer, buffer_len, _T("SLA D=(IX+(%d))"), ofs); break;
	case 0x23: my_stprintf_s(buffer, buffer_len, _T("SLA E=(IX+(%d))"), ofs); break;
	case 0x24: my_stprintf_s(buffer, buffer_len, _T("SLA H=(IX+(%d))"), ofs); break;
	case 0x25: my_stprintf_s(buffer, buffer_len, _T("SLA L=(IX+(%d))"), ofs); break;
	case 0x26: my_stprintf_s(buffer, buffer_len, _T("SLA (IX+(%d))"), ofs); break;
	case 0x27: my_stprintf_s(buffer, buffer_len, _T("SLA A=(IX+(%d))"), ofs); break;
	case 0x28: my_stprintf_s(buffer, buffer_len, _T("SRA B=(IX+(%d))"), ofs); break;
	case 0x29: my_stprintf_s(buffer, buffer_len, _T("SRA C=(IX+(%d))"), ofs); break;
	case 0x2a: my_stprintf_s(buffer, buffer_len, _T("SRA D=(IX+(%d))"), ofs); break;
	case 0x2b: my_stprintf_s(buffer, buffer_len, _T("SRA E=(IX+(%d))"), ofs); break;
	case 0x2c: my_stprintf_s(buffer, buffer_len, _T("SRA H=(IX+(%d))"), ofs); break;
	case 0x2d: my_stprintf_s(buffer, buffer_len, _T("SRA L=(IX+(%d))"), ofs); break;
	case 0x2e: my_stprintf_s(buffer, buffer_len, _T("SRA (IX+(%d))"), ofs); break;
	case 0x2f: my_stprintf_s(buffer, buffer_len, _T("SRA A=(IX+(%d))"), ofs); break;
	case 0x30: my_stprintf_s(buffer, buffer_len, _T("SLL B=(IX+(%d))"), ofs); break;
	case 0x31: my_stprintf_s(buffer, buffer_len, _T("SLL C=(IX+(%d))"), ofs); break;
	case 0x32: my_stprintf_s(buffer, buffer_len, _T("SLL D=(IX+(%d))"), ofs); break;
	case 0x33: my_stprintf_s(buffer, buffer_len, _T("SLL E=(IX+(%d))"), ofs); break;
	case 0x34: my_stprintf_s(buffer, buffer_len, _T("SLL H=(IX+(%d))"), ofs); break;
	case 0x35: my_stprintf_s(buffer, buffer_len, _T("SLL L=(IX+(%d))"), ofs); break;
	case 0x36: my_stprintf_s(buffer, buffer_len, _T("SLL (IX+(%d))"), ofs); break;
	case 0x37: my_stprintf_s(buffer, buffer_len, _T("SLL A=(IX+(%d))"), ofs); break;
	case 0x38: my_stprintf_s(buffer, buffer_len, _T("SRL B=(IX+(%d))"), ofs); break;
	case 0x39: my_stprintf_s(buffer, buffer_len, _T("SRL C=(IX+(%d))"), ofs); break;
	case 0x3a: my_stprintf_s(buffer, buffer_len, _T("SRL D=(IX+(%d))"), ofs); break;
	case 0x3b: my_stprintf_s(buffer, buffer_len, _T("SRL E=(IX+(%d))"), ofs); break;
	case 0x3c: my_stprintf_s(buffer, buffer_len, _T("SRL H=(IX+(%d))"), ofs); break;
	case 0x3d: my_stprintf_s(buffer, buffer_len, _T("SRL L=(IX+(%d))"), ofs); break;
	case 0x3e: my_stprintf_s(buffer, buffer_len, _T("SRL (IX+(%d))"), ofs); break;
	case 0x3f: my_stprintf_s(buffer, buffer_len, _T("SRL A=(IX+(%d))"), ofs); break;
	case 0x40: my_stprintf_s(buffer, buffer_len, _T("BIT 0, B=(IX+(%d))"), ofs); break;
	case 0x41: my_stprintf_s(buffer, buffer_len, _T("BIT 0, C=(IX+(%d))"), ofs); break;
	case 0x42: my_stprintf_s(buffer, buffer_len, _T("BIT 0, D=(IX+(%d))"), ofs); break;
	case 0x43: my_stprintf_s(buffer, buffer_len, _T("BIT 0, E=(IX+(%d))"), ofs); break;
	case 0x44: my_stprintf_s(buffer, buffer_len, _T("BIT 0, H=(IX+(%d))"), ofs); break;
	case 0x45: my_stprintf_s(buffer, buffer_len, _T("BIT 0, L=(IX+(%d))"), ofs); break;
	case 0x46: my_stprintf_s(buffer, buffer_len, _T("BIT 0, (IX+(%d))"), ofs); break;
	case 0x47: my_stprintf_s(buffer, buffer_len, _T("BIT 0, A=(IX+(%d))"), ofs); break;
	case 0x48: my_stprintf_s(buffer, buffer_len, _T("BIT 1, B=(IX+(%d))"), ofs); break;
	case 0x49: my_stprintf_s(buffer, buffer_len, _T("BIT 1, C=(IX+(%d))"), ofs); break;
	case 0x4a: my_stprintf_s(buffer, buffer_len, _T("BIT 1, D=(IX+(%d))"), ofs); break;
	case 0x4b: my_stprintf_s(buffer, buffer_len, _T("BIT 1, E=(IX+(%d))"), ofs); break;
	case 0x4c: my_stprintf_s(buffer, buffer_len, _T("BIT 1, H=(IX+(%d))"), ofs); break;
	case 0x4d: my_stprintf_s(buffer, buffer_len, _T("BIT 1, L=(IX+(%d))"), ofs); break;
	case 0x4e: my_stprintf_s(buffer, buffer_len, _T("BIT 1, (IX+(%d))"), ofs); break;
	case 0x4f: my_stprintf_s(buffer, buffer_len, _T("BIT 1, A=(IX+(%d))"), ofs); break;
	case 0x50: my_stprintf_s(buffer, buffer_len, _T("BIT 2, B=(IX+(%d))"), ofs); break;
	case 0x51: my_stprintf_s(buffer, buffer_len, _T("BIT 2, C=(IX+(%d))"), ofs); break;
	case 0x52: my_stprintf_s(buffer, buffer_len, _T("BIT 2, D=(IX+(%d))"), ofs); break;
	case 0x53: my_stprintf_s(buffer, buffer_len, _T("BIT 2, E=(IX+(%d))"), ofs); break;
	case 0x54: my_stprintf_s(buffer, buffer_len, _T("BIT 2, H=(IX+(%d))"), ofs); break;
	case 0x55: my_stprintf_s(buffer, buffer_len, _T("BIT 2, L=(IX+(%d))"), ofs); break;
	case 0x56: my_stprintf_s(buffer, buffer_len, _T("BIT 2, (IX+(%d))"), ofs); break;
	case 0x57: my_stprintf_s(buffer, buffer_len, _T("BIT 2, A=(IX+(%d))"), ofs); break;
	case 0x58: my_stprintf_s(buffer, buffer_len, _T("BIT 3, B=(IX+(%d))"), ofs); break;
	case 0x59: my_stprintf_s(buffer, buffer_len, _T("BIT 3, C=(IX+(%d))"), ofs); break;
	case 0x5a: my_stprintf_s(buffer, buffer_len, _T("BIT 3, D=(IX+(%d))"), ofs); break;
	case 0x5b: my_stprintf_s(buffer, buffer_len, _T("BIT 3, E=(IX+(%d))"), ofs); break;
	case 0x5c: my_stprintf_s(buffer, buffer_len, _T("BIT 3, H=(IX+(%d))"), ofs); break;
	case 0x5d: my_stprintf_s(buffer, buffer_len, _T("BIT 3, L=(IX+(%d))"), ofs); break;
	case 0x5e: my_stprintf_s(buffer, buffer_len, _T("BIT 3, (IX+(%d))"), ofs); break;
	case 0x5f: my_stprintf_s(buffer, buffer_len, _T("BIT 3, A=(IX+(%d))"), ofs); break;
	case 0x60: my_stprintf_s(buffer, buffer_len, _T("BIT 4, B=(IX+(%d))"), ofs); break;
	case 0x61: my_stprintf_s(buffer, buffer_len, _T("BIT 4, C=(IX+(%d))"), ofs); break;
	case 0x62: my_stprintf_s(buffer, buffer_len, _T("BIT 4, D=(IX+(%d))"), ofs); break;
	case 0x63: my_stprintf_s(buffer, buffer_len, _T("BIT 4, E=(IX+(%d))"), ofs); break;
	case 0x64: my_stprintf_s(buffer, buffer_len, _T("BIT 4, H=(IX+(%d))"), ofs); break;
	case 0x65: my_stprintf_s(buffer, buffer_len, _T("BIT 4, L=(IX+(%d))"), ofs); break;
	case 0x66: my_stprintf_s(buffer, buffer_len, _T("BIT 4, (IX+(%d))"), ofs); break;
	case 0x67: my_stprintf_s(buffer, buffer_len, _T("BIT 4, A=(IX+(%d))"), ofs); break;
	case 0x68: my_stprintf_s(buffer, buffer_len, _T("BIT 5, B=(IX+(%d))"), ofs); break;
	case 0x69: my_stprintf_s(buffer, buffer_len, _T("BIT 5, C=(IX+(%d))"), ofs); break;
	case 0x6a: my_stprintf_s(buffer, buffer_len, _T("BIT 5, D=(IX+(%d))"), ofs); break;
	case 0x6b: my_stprintf_s(buffer, buffer_len, _T("BIT 5, E=(IX+(%d))"), ofs); break;
	case 0x6c: my_stprintf_s(buffer, buffer_len, _T("BIT 5, H=(IX+(%d))"), ofs); break;
	case 0x6d: my_stprintf_s(buffer, buffer_len, _T("BIT 5, L=(IX+(%d))"), ofs); break;
	case 0x6e: my_stprintf_s(buffer, buffer_len, _T("BIT 5, (IX+(%d))"), ofs); break;
	case 0x6f: my_stprintf_s(buffer, buffer_len, _T("BIT 5, A=(IX+(%d))"), ofs); break;
	case 0x70: my_stprintf_s(buffer, buffer_len, _T("BIT 6, B=(IX+(%d))"), ofs); break;
	case 0x71: my_stprintf_s(buffer, buffer_len, _T("BIT 6, C=(IX+(%d))"), ofs); break;
	case 0x72: my_stprintf_s(buffer, buffer_len, _T("BIT 6, D=(IX+(%d))"), ofs); break;
	case 0x73: my_stprintf_s(buffer, buffer_len, _T("BIT 6, E=(IX+(%d))"), ofs); break;
	case 0x74: my_stprintf_s(buffer, buffer_len, _T("BIT 6, H=(IX+(%d))"), ofs); break;
	case 0x75: my_stprintf_s(buffer, buffer_len, _T("BIT 6, L=(IX+(%d))"), ofs); break;
	case 0x76: my_stprintf_s(buffer, buffer_len, _T("BIT 6, (IX+(%d))"), ofs); break;
	case 0x77: my_stprintf_s(buffer, buffer_len, _T("BIT 6, A=(IX+(%d))"), ofs); break;
	case 0x78: my_stprintf_s(buffer, buffer_len, _T("BIT 7, B=(IX+(%d))"), ofs); break;
	case 0x79: my_stprintf_s(buffer, buffer_len, _T("BIT 7, C=(IX+(%d))"), ofs); break;
	case 0x7a: my_stprintf_s(buffer, buffer_len, _T("BIT 7, D=(IX+(%d))"), ofs); break;
	case 0x7b: my_stprintf_s(buffer, buffer_len, _T("BIT 7, E=(IX+(%d))"), ofs); break;
	case 0x7c: my_stprintf_s(buffer, buffer_len, _T("BIT 7, H=(IX+(%d))"), ofs); break;
	case 0x7d: my_stprintf_s(buffer, buffer_len, _T("BIT 7, L=(IX+(%d))"), ofs); break;
	case 0x7e: my_stprintf_s(buffer, buffer_len, _T("BIT 7, (IX+(%d))"), ofs); break;
	case 0x7f: my_stprintf_s(buffer, buffer_len, _T("BIT 7, A=(IX+(%d))"), ofs); break;
	case 0x80: my_stprintf_s(buffer, buffer_len, _T("RES 0, B=(IX+(%d))"), ofs); break;
	case 0x81: my_stprintf_s(buffer, buffer_len, _T("RES 0, C=(IX+(%d))"), ofs); break;
	case 0x82: my_stprintf_s(buffer, buffer_len, _T("RES 0, D=(IX+(%d))"), ofs); break;
	case 0x83: my_stprintf_s(buffer, buffer_len, _T("RES 0, E=(IX+(%d))"), ofs); break;
	case 0x84: my_stprintf_s(buffer, buffer_len, _T("RES 0, H=(IX+(%d))"), ofs); break;
	case 0x85: my_stprintf_s(buffer, buffer_len, _T("RES 0, L=(IX+(%d))"), ofs); break;
	case 0x86: my_stprintf_s(buffer, buffer_len, _T("RES 0, (IX+(%d))"), ofs); break;
	case 0x87: my_stprintf_s(buffer, buffer_len, _T("RES 0, A=(IX+(%d))"), ofs); break;
	case 0x88: my_stprintf_s(buffer, buffer_len, _T("RES 1, B=(IX+(%d))"), ofs); break;
	case 0x89: my_stprintf_s(buffer, buffer_len, _T("RES 1, C=(IX+(%d))"), ofs); break;
	case 0x8a: my_stprintf_s(buffer, buffer_len, _T("RES 1, D=(IX+(%d))"), ofs); break;
	case 0x8b: my_stprintf_s(buffer, buffer_len, _T("RES 1, E=(IX+(%d))"), ofs); break;
	case 0x8c: my_stprintf_s(buffer, buffer_len, _T("RES 1, H=(IX+(%d))"), ofs); break;
	case 0x8d: my_stprintf_s(buffer, buffer_len, _T("RES 1, L=(IX+(%d))"), ofs); break;
	case 0x8e: my_stprintf_s(buffer, buffer_len, _T("RES 1, (IX+(%d))"), ofs); break;
	case 0x8f: my_stprintf_s(buffer, buffer_len, _T("RES 1, A=(IX+(%d))"), ofs); break;
	case 0x90: my_stprintf_s(buffer, buffer_len, _T("RES 2, B=(IX+(%d))"), ofs); break;
	case 0x91: my_stprintf_s(buffer, buffer_len, _T("RES 2, C=(IX+(%d))"), ofs); break;
	case 0x92: my_stprintf_s(buffer, buffer_len, _T("RES 2, D=(IX+(%d))"), ofs); break;
	case 0x93: my_stprintf_s(buffer, buffer_len, _T("RES 2, E=(IX+(%d))"), ofs); break;
	case 0x94: my_stprintf_s(buffer, buffer_len, _T("RES 2, H=(IX+(%d))"), ofs); break;
	case 0x95: my_stprintf_s(buffer, buffer_len, _T("RES 2, L=(IX+(%d))"), ofs); break;
	case 0x96: my_stprintf_s(buffer, buffer_len, _T("RES 2, (IX+(%d))"), ofs); break;
	case 0x97: my_stprintf_s(buffer, buffer_len, _T("RES 2, A=(IX+(%d))"), ofs); break;
	case 0x98: my_stprintf_s(buffer, buffer_len, _T("RES 3, B=(IX+(%d))"), ofs); break;
	case 0x99: my_stprintf_s(buffer, buffer_len, _T("RES 3, C=(IX+(%d))"), ofs); break;
	case 0x9a: my_stprintf_s(buffer, buffer_len, _T("RES 3, D=(IX+(%d))"), ofs); break;
	case 0x9b: my_stprintf_s(buffer, buffer_len, _T("RES 3, E=(IX+(%d))"), ofs); break;
	case 0x9c: my_stprintf_s(buffer, buffer_len, _T("RES 3, H=(IX+(%d))"), ofs); break;
	case 0x9d: my_stprintf_s(buffer, buffer_len, _T("RES 3, L=(IX+(%d))"), ofs); break;
	case 0x9e: my_stprintf_s(buffer, buffer_len, _T("RES 3, (IX+(%d))"), ofs); break;
	case 0x9f: my_stprintf_s(buffer, buffer_len, _T("RES 3, A=(IX+(%d))"), ofs); break;
	case 0xa0: my_stprintf_s(buffer, buffer_len, _T("RES 4, B=(IX+(%d))"), ofs); break;
	case 0xa1: my_stprintf_s(buffer, buffer_len, _T("RES 4, C=(IX+(%d))"), ofs); break;
	case 0xa2: my_stprintf_s(buffer, buffer_len, _T("RES 4, D=(IX+(%d))"), ofs); break;
	case 0xa3: my_stprintf_s(buffer, buffer_len, _T("RES 4, E=(IX+(%d))"), ofs); break;
	case 0xa4: my_stprintf_s(buffer, buffer_len, _T("RES 4, H=(IX+(%d))"), ofs); break;
	case 0xa5: my_stprintf_s(buffer, buffer_len, _T("RES 4, L=(IX+(%d))"), ofs); break;
	case 0xa6: my_stprintf_s(buffer, buffer_len, _T("RES 4, (IX+(%d))"), ofs); break;
	case 0xa7: my_stprintf_s(buffer, buffer_len, _T("RES 4, A=(IX+(%d))"), ofs); break;
	case 0xa8: my_stprintf_s(buffer, buffer_len, _T("RES 5, B=(IX+(%d))"), ofs); break;
	case 0xa9: my_stprintf_s(buffer, buffer_len, _T("RES 5, C=(IX+(%d))"), ofs); break;
	case 0xaa: my_stprintf_s(buffer, buffer_len, _T("RES 5, D=(IX+(%d))"), ofs); break;
	case 0xab: my_stprintf_s(buffer, buffer_len, _T("RES 5, E=(IX+(%d))"), ofs); break;
	case 0xac: my_stprintf_s(buffer, buffer_len, _T("RES 5, H=(IX+(%d))"), ofs); break;
	case 0xad: my_stprintf_s(buffer, buffer_len, _T("RES 5, L=(IX+(%d))"), ofs); break;
	case 0xae: my_stprintf_s(buffer, buffer_len, _T("RES 5, (IX+(%d))"), ofs); break;
	case 0xaf: my_stprintf_s(buffer, buffer_len, _T("RES 5, A=(IX+(%d))"), ofs); break;
	case 0xb0: my_stprintf_s(buffer, buffer_len, _T("RES 6, B=(IX+(%d))"), ofs); break;
	case 0xb1: my_stprintf_s(buffer, buffer_len, _T("RES 6, C=(IX+(%d))"), ofs); break;
	case 0xb2: my_stprintf_s(buffer, buffer_len, _T("RES 6, D=(IX+(%d))"), ofs); break;
	case 0xb3: my_stprintf_s(buffer, buffer_len, _T("RES 6, E=(IX+(%d))"), ofs); break;
	case 0xb4: my_stprintf_s(buffer, buffer_len, _T("RES 6, H=(IX+(%d))"), ofs); break;
	case 0xb5: my_stprintf_s(buffer, buffer_len, _T("RES 6, L=(IX+(%d))"), ofs); break;
	case 0xb6: my_stprintf_s(buffer, buffer_len, _T("RES 6, (IX+(%d))"), ofs); break;
	case 0xb7: my_stprintf_s(buffer, buffer_len, _T("RES 6, A=(IX+(%d))"), ofs); break;
	case 0xb8: my_stprintf_s(buffer, buffer_len, _T("RES 7, B=(IX+(%d))"), ofs); break;
	case 0xb9: my_stprintf_s(buffer, buffer_len, _T("RES 7, C=(IX+(%d))"), ofs); break;
	case 0xba: my_stprintf_s(buffer, buffer_len, _T("RES 7, D=(IX+(%d))"), ofs); break;
	case 0xbb: my_stprintf_s(buffer, buffer_len, _T("RES 7, E=(IX+(%d))"), ofs); break;
	case 0xbc: my_stprintf_s(buffer, buffer_len, _T("RES 7, H=(IX+(%d))"), ofs); break;
	case 0xbd: my_stprintf_s(buffer, buffer_len, _T("RES 7, L=(IX+(%d))"), ofs); break;
	case 0xbe: my_stprintf_s(buffer, buffer_len, _T("RES 7, (IX+(%d))"), ofs); break;
	case 0xbf: my_stprintf_s(buffer, buffer_len, _T("RES 7, A=(IX+(%d))"), ofs); break;
	case 0xc0: my_stprintf_s(buffer, buffer_len, _T("SET 0, B=(IX+(%d))"), ofs); break;
	case 0xc1: my_stprintf_s(buffer, buffer_len, _T("SET 0, C=(IX+(%d))"), ofs); break;
	case 0xc2: my_stprintf_s(buffer, buffer_len, _T("SET 0, D=(IX+(%d))"), ofs); break;
	case 0xc3: my_stprintf_s(buffer, buffer_len, _T("SET 0, E=(IX+(%d))"), ofs); break;
	case 0xc4: my_stprintf_s(buffer, buffer_len, _T("SET 0, H=(IX+(%d))"), ofs); break;
	case 0xc5: my_stprintf_s(buffer, buffer_len, _T("SET 0, L=(IX+(%d))"), ofs); break;
	case 0xc6: my_stprintf_s(buffer, buffer_len, _T("SET 0, (IX+(%d))"), ofs); break;
	case 0xc7: my_stprintf_s(buffer, buffer_len, _T("SET 0, A=(IX+(%d))"), ofs); break;
	case 0xc8: my_stprintf_s(buffer, buffer_len, _T("SET 1, B=(IX+(%d))"), ofs); break;
	case 0xc9: my_stprintf_s(buffer, buffer_len, _T("SET 1, C=(IX+(%d))"), ofs); break;
	case 0xca: my_stprintf_s(buffer, buffer_len, _T("SET 1, D=(IX+(%d))"), ofs); break;
	case 0xcb: my_stprintf_s(buffer, buffer_len, _T("SET 1, E=(IX+(%d))"), ofs); break;
	case 0xcc: my_stprintf_s(buffer, buffer_len, _T("SET 1, H=(IX+(%d))"), ofs); break;
	case 0xcd: my_stprintf_s(buffer, buffer_len, _T("SET 1, L=(IX+(%d))"), ofs); break;
	case 0xce: my_stprintf_s(buffer, buffer_len, _T("SET 1, (IX+(%d))"), ofs); break;
	case 0xcf: my_stprintf_s(buffer, buffer_len, _T("SET 1, A=(IX+(%d))"), ofs); break;
	case 0xd0: my_stprintf_s(buffer, buffer_len, _T("SET 2, B=(IX+(%d))"), ofs); break;
	case 0xd1: my_stprintf_s(buffer, buffer_len, _T("SET 2, C=(IX+(%d))"), ofs); break;
	case 0xd2: my_stprintf_s(buffer, buffer_len, _T("SET 2, D=(IX+(%d))"), ofs); break;
	case 0xd3: my_stprintf_s(buffer, buffer_len, _T("SET 2, E=(IX+(%d))"), ofs); break;
	case 0xd4: my_stprintf_s(buffer, buffer_len, _T("SET 2, H=(IX+(%d))"), ofs); break;
	case 0xd5: my_stprintf_s(buffer, buffer_len, _T("SET 2, L=(IX+(%d))"), ofs); break;
	case 0xd6: my_stprintf_s(buffer, buffer_len, _T("SET 2, (IX+(%d))"), ofs); break;
	case 0xd7: my_stprintf_s(buffer, buffer_len, _T("SET 2, A=(IX+(%d))"), ofs); break;
	case 0xd8: my_stprintf_s(buffer, buffer_len, _T("SET 3, B=(IX+(%d))"), ofs); break;
	case 0xd9: my_stprintf_s(buffer, buffer_len, _T("SET 3, C=(IX+(%d))"), ofs); break;
	case 0xda: my_stprintf_s(buffer, buffer_len, _T("SET 3, D=(IX+(%d))"), ofs); break;
	case 0xdb: my_stprintf_s(buffer, buffer_len, _T("SET 3, E=(IX+(%d))"), ofs); break;
	case 0xdc: my_stprintf_s(buffer, buffer_len, _T("SET 3, H=(IX+(%d))"), ofs); break;
	case 0xdd: my_stprintf_s(buffer, buffer_len, _T("SET 3, L=(IX+(%d))"), ofs); break;
	case 0xde: my_stprintf_s(buffer, buffer_len, _T("SET 3, (IX+(%d))"), ofs); break;
	case 0xdf: my_stprintf_s(buffer, buffer_len, _T("SET 3, A=(IX+(%d))"), ofs); break;
	case 0xe0: my_stprintf_s(buffer, buffer_len, _T("SET 4, B=(IX+(%d))"), ofs); break;
	case 0xe1: my_stprintf_s(buffer, buffer_len, _T("SET 4, C=(IX+(%d))"), ofs); break;
	case 0xe2: my_stprintf_s(buffer, buffer_len, _T("SET 4, D=(IX+(%d))"), ofs); break;
	case 0xe3: my_stprintf_s(buffer, buffer_len, _T("SET 4, E=(IX+(%d))"), ofs); break;
	case 0xe4: my_stprintf_s(buffer, buffer_len, _T("SET 4, H=(IX+(%d))"), ofs); break;
	case 0xe5: my_stprintf_s(buffer, buffer_len, _T("SET 4, L=(IX+(%d))"), ofs); break;
	case 0xe6: my_stprintf_s(buffer, buffer_len, _T("SET 4, (IX+(%d))"), ofs); break;
	case 0xe7: my_stprintf_s(buffer, buffer_len, _T("SET 4, A=(IX+(%d))"), ofs); break;
	case 0xe8: my_stprintf_s(buffer, buffer_len, _T("SET 5, B=(IX+(%d))"), ofs); break;
	case 0xe9: my_stprintf_s(buffer, buffer_len, _T("SET 5, C=(IX+(%d))"), ofs); break;
	case 0xea: my_stprintf_s(buffer, buffer_len, _T("SET 5, D=(IX+(%d))"), ofs); break;
	case 0xeb: my_stprintf_s(buffer, buffer_len, _T("SET 5, E=(IX+(%d))"), ofs); break;
	case 0xec: my_stprintf_s(buffer, buffer_len, _T("SET 5, H=(IX+(%d))"), ofs); break;
	case 0xed: my_stprintf_s(buffer, buffer_len, _T("SET 5, L=(IX+(%d))"), ofs); break;
	case 0xee: my_stprintf_s(buffer, buffer_len, _T("SET 5, (IX+(%d))"), ofs); break;
	case 0xef: my_stprintf_s(buffer, buffer_len, _T("SET 5, A=(IX+(%d))"), ofs); break;
	case 0xf0: my_stprintf_s(buffer, buffer_len, _T("SET 6, B=(IX+(%d))"), ofs); break;
	case 0xf1: my_stprintf_s(buffer, buffer_len, _T("SET 6, C=(IX+(%d))"), ofs); break;
	case 0xf2: my_stprintf_s(buffer, buffer_len, _T("SET 6, D=(IX+(%d))"), ofs); break;
	case 0xf3: my_stprintf_s(buffer, buffer_len, _T("SET 6, E=(IX+(%d))"), ofs); break;
	case 0xf4: my_stprintf_s(buffer, buffer_len, _T("SET 6, H=(IX+(%d))"), ofs); break;
	case 0xf5: my_stprintf_s(buffer, buffer_len, _T("SET 6, L=(IX+(%d))"), ofs); break;
	case 0xf6: my_stprintf_s(buffer, buffer_len, _T("SET 6, (IX+(%d))"), ofs); break;
	case 0xf7: my_stprintf_s(buffer, buffer_len, _T("SET 6, A=(IX+(%d))"), ofs); break;
	case 0xf8: my_stprintf_s(buffer, buffer_len, _T("SET 7, B=(IX+(%d))"), ofs); break;
	case 0xf9: my_stprintf_s(buffer, buffer_len, _T("SET 7, C=(IX+(%d))"), ofs); break;
	case 0xfa: my_stprintf_s(buffer, buffer_len, _T("SET 7, D=(IX+(%d))"), ofs); break;
	case 0xfb: my_stprintf_s(buffer, buffer_len, _T("SET 7, E=(IX+(%d))"), ofs); break;
	case 0xfc: my_stprintf_s(buffer, buffer_len, _T("SET 7, H=(IX+(%d))"), ofs); break;
	case 0xfd: my_stprintf_s(buffer, buffer_len, _T("SET 7, L=(IX+(%d))"), ofs); break;
	case 0xfe: my_stprintf_s(buffer, buffer_len, _T("SET 7, (IX+(%d))"), ofs); break;
	case 0xff: my_stprintf_s(buffer, buffer_len, _T("SET 7, A=(IX+(%d))"), ofs); break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}

static void dasm_fdcb(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol)
{
	int8_t ofs = debug_fetch8_rel();
	uint8_t code = debug_fetch8();

	switch(code) {
	case 0x00: my_stprintf_s(buffer, buffer_len, _T("RLC B=(IY+(%d))"), ofs); break;
	case 0x01: my_stprintf_s(buffer, buffer_len, _T("RLC C=(IY+(%d))"), ofs); break;
	case 0x02: my_stprintf_s(buffer, buffer_len, _T("RLC D=(IY+(%d))"), ofs); break;
	case 0x03: my_stprintf_s(buffer, buffer_len, _T("RLC E=(IY+(%d))"), ofs); break;
	case 0x04: my_stprintf_s(buffer, buffer_len, _T("RLC H=(IY+(%d))"), ofs); break;
	case 0x05: my_stprintf_s(buffer, buffer_len, _T("RLC L=(IY+(%d))"), ofs); break;
	case 0x06: my_stprintf_s(buffer, buffer_len, _T("RLC (IY+(%d))"), ofs); break;
	case 0x07: my_stprintf_s(buffer, buffer_len, _T("RLC A=(IY+(%d))"), ofs); break;
	case 0x08: my_stprintf_s(buffer, buffer_len, _T("RRC B=(IY+(%d))"), ofs); break;
	case 0x09: my_stprintf_s(buffer, buffer_len, _T("RRC C=(IY+(%d))"), ofs); break;
	case 0x0a: my_stprintf_s(buffer, buffer_len, _T("RRC D=(IY+(%d))"), ofs); break;
	case 0x0b: my_stprintf_s(buffer, buffer_len, _T("RRC E=(IY+(%d))"), ofs); break;
	case 0x0c: my_stprintf_s(buffer, buffer_len, _T("RRC H=(IY+(%d))"), ofs); break;
	case 0x0d: my_stprintf_s(buffer, buffer_len, _T("RRC L=(IY+(%d))"), ofs); break;
	case 0x0e: my_stprintf_s(buffer, buffer_len, _T("RRC (IY+(%d))"), ofs); break;
	case 0x0f: my_stprintf_s(buffer, buffer_len, _T("RRC A=(IY+(%d))"), ofs); break;
	case 0x10: my_stprintf_s(buffer, buffer_len, _T("RL B=(IY+(%d))"), ofs); break;
	case 0x11: my_stprintf_s(buffer, buffer_len, _T("RL C=(IY+(%d))"), ofs); break;
	case 0x12: my_stprintf_s(buffer, buffer_len, _T("RL D=(IY+(%d))"), ofs); break;
	case 0x13: my_stprintf_s(buffer, buffer_len, _T("RL E=(IY+(%d))"), ofs); break;
	case 0x14: my_stprintf_s(buffer, buffer_len, _T("RL H=(IY+(%d))"), ofs); break;
	case 0x15: my_stprintf_s(buffer, buffer_len, _T("RL L=(IY+(%d))"), ofs); break;
	case 0x16: my_stprintf_s(buffer, buffer_len, _T("RL (IY+(%d))"), ofs); break;
	case 0x17: my_stprintf_s(buffer, buffer_len, _T("RL A=(IY+(%d))"), ofs); break;
	case 0x18: my_stprintf_s(buffer, buffer_len, _T("RR B=(IY+(%d))"), ofs); break;
	case 0x19: my_stprintf_s(buffer, buffer_len, _T("RR C=(IY+(%d))"), ofs); break;
	case 0x1a: my_stprintf_s(buffer, buffer_len, _T("RR D=(IY+(%d))"), ofs); break;
	case 0x1b: my_stprintf_s(buffer, buffer_len, _T("RR E=(IY+(%d))"), ofs); break;
	case 0x1c: my_stprintf_s(buffer, buffer_len, _T("RR H=(IY+(%d))"), ofs); break;
	case 0x1d: my_stprintf_s(buffer, buffer_len, _T("RR L=(IY+(%d))"), ofs); break;
	case 0x1e: my_stprintf_s(buffer, buffer_len, _T("RR (IY+(%d))"), ofs); break;
	case 0x1f: my_stprintf_s(buffer, buffer_len, _T("RR A=(IY+(%d))"), ofs); break;
	case 0x20: my_stprintf_s(buffer, buffer_len, _T("SLA B=(IY+(%d))"), ofs); break;
	case 0x21: my_stprintf_s(buffer, buffer_len, _T("SLA C=(IY+(%d))"), ofs); break;
	case 0x22: my_stprintf_s(buffer, buffer_len, _T("SLA D=(IY+(%d))"), ofs); break;
	case 0x23: my_stprintf_s(buffer, buffer_len, _T("SLA E=(IY+(%d))"), ofs); break;
	case 0x24: my_stprintf_s(buffer, buffer_len, _T("SLA H=(IY+(%d))"), ofs); break;
	case 0x25: my_stprintf_s(buffer, buffer_len, _T("SLA L=(IY+(%d))"), ofs); break;
	case 0x26: my_stprintf_s(buffer, buffer_len, _T("SLA (IY+(%d))"), ofs); break;
	case 0x27: my_stprintf_s(buffer, buffer_len, _T("SLA A=(IY+(%d))"), ofs); break;
	case 0x28: my_stprintf_s(buffer, buffer_len, _T("SRA B=(IY+(%d))"), ofs); break;
	case 0x29: my_stprintf_s(buffer, buffer_len, _T("SRA C=(IY+(%d))"), ofs); break;
	case 0x2a: my_stprintf_s(buffer, buffer_len, _T("SRA D=(IY+(%d))"), ofs); break;
	case 0x2b: my_stprintf_s(buffer, buffer_len, _T("SRA E=(IY+(%d))"), ofs); break;
	case 0x2c: my_stprintf_s(buffer, buffer_len, _T("SRA H=(IY+(%d))"), ofs); break;
	case 0x2d: my_stprintf_s(buffer, buffer_len, _T("SRA L=(IY+(%d))"), ofs); break;
	case 0x2e: my_stprintf_s(buffer, buffer_len, _T("SRA (IY+(%d))"), ofs); break;
	case 0x2f: my_stprintf_s(buffer, buffer_len, _T("SRA A=(IY+(%d))"), ofs); break;
	case 0x30: my_stprintf_s(buffer, buffer_len, _T("SLL B=(IY+(%d))"), ofs); break;
	case 0x31: my_stprintf_s(buffer, buffer_len, _T("SLL C=(IY+(%d))"), ofs); break;
	case 0x32: my_stprintf_s(buffer, buffer_len, _T("SLL D=(IY+(%d))"), ofs); break;
	case 0x33: my_stprintf_s(buffer, buffer_len, _T("SLL E=(IY+(%d))"), ofs); break;
	case 0x34: my_stprintf_s(buffer, buffer_len, _T("SLL H=(IY+(%d))"), ofs); break;
	case 0x35: my_stprintf_s(buffer, buffer_len, _T("SLL L=(IY+(%d))"), ofs); break;
	case 0x36: my_stprintf_s(buffer, buffer_len, _T("SLL (IY+(%d))"), ofs); break;
	case 0x37: my_stprintf_s(buffer, buffer_len, _T("SLL A=(IY+(%d))"), ofs); break;
	case 0x38: my_stprintf_s(buffer, buffer_len, _T("SRL B=(IY+(%d))"), ofs); break;
	case 0x39: my_stprintf_s(buffer, buffer_len, _T("SRL C=(IY+(%d))"), ofs); break;
	case 0x3a: my_stprintf_s(buffer, buffer_len, _T("SRL D=(IY+(%d))"), ofs); break;
	case 0x3b: my_stprintf_s(buffer, buffer_len, _T("SRL E=(IY+(%d))"), ofs); break;
	case 0x3c: my_stprintf_s(buffer, buffer_len, _T("SRL H=(IY+(%d))"), ofs); break;
	case 0x3d: my_stprintf_s(buffer, buffer_len, _T("SRL L=(IY+(%d))"), ofs); break;
	case 0x3e: my_stprintf_s(buffer, buffer_len, _T("SRL (IY+(%d))"), ofs); break;
	case 0x3f: my_stprintf_s(buffer, buffer_len, _T("SRL A=(IY+(%d))"), ofs); break;
	case 0x40: my_stprintf_s(buffer, buffer_len, _T("BIT 0, B=(IY+(%d))"), ofs); break;
	case 0x41: my_stprintf_s(buffer, buffer_len, _T("BIT 0, C=(IY+(%d))"), ofs); break;
	case 0x42: my_stprintf_s(buffer, buffer_len, _T("BIT 0, D=(IY+(%d))"), ofs); break;
	case 0x43: my_stprintf_s(buffer, buffer_len, _T("BIT 0, E=(IY+(%d))"), ofs); break;
	case 0x44: my_stprintf_s(buffer, buffer_len, _T("BIT 0, H=(IY+(%d))"), ofs); break;
	case 0x45: my_stprintf_s(buffer, buffer_len, _T("BIT 0, L=(IY+(%d))"), ofs); break;
	case 0x46: my_stprintf_s(buffer, buffer_len, _T("BIT 0, (IY+(%d))"), ofs); break;
	case 0x47: my_stprintf_s(buffer, buffer_len, _T("BIT 0, A=(IY+(%d))"), ofs); break;
	case 0x48: my_stprintf_s(buffer, buffer_len, _T("BIT 1, B=(IY+(%d))"), ofs); break;
	case 0x49: my_stprintf_s(buffer, buffer_len, _T("BIT 1, C=(IY+(%d))"), ofs); break;
	case 0x4a: my_stprintf_s(buffer, buffer_len, _T("BIT 1, D=(IY+(%d))"), ofs); break;
	case 0x4b: my_stprintf_s(buffer, buffer_len, _T("BIT 1, E=(IY+(%d))"), ofs); break;
	case 0x4c: my_stprintf_s(buffer, buffer_len, _T("BIT 1, H=(IY+(%d))"), ofs); break;
	case 0x4d: my_stprintf_s(buffer, buffer_len, _T("BIT 1, L=(IY+(%d))"), ofs); break;
	case 0x4e: my_stprintf_s(buffer, buffer_len, _T("BIT 1, (IY+(%d))"), ofs); break;
	case 0x4f: my_stprintf_s(buffer, buffer_len, _T("BIT 1, A=(IY+(%d))"), ofs); break;
	case 0x50: my_stprintf_s(buffer, buffer_len, _T("BIT 2, B=(IY+(%d))"), ofs); break;
	case 0x51: my_stprintf_s(buffer, buffer_len, _T("BIT 2, C=(IY+(%d))"), ofs); break;
	case 0x52: my_stprintf_s(buffer, buffer_len, _T("BIT 2, D=(IY+(%d))"), ofs); break;
	case 0x53: my_stprintf_s(buffer, buffer_len, _T("BIT 2, E=(IY+(%d))"), ofs); break;
	case 0x54: my_stprintf_s(buffer, buffer_len, _T("BIT 2, H=(IY+(%d))"), ofs); break;
	case 0x55: my_stprintf_s(buffer, buffer_len, _T("BIT 2, L=(IY+(%d))"), ofs); break;
	case 0x56: my_stprintf_s(buffer, buffer_len, _T("BIT 2, (IY+(%d))"), ofs); break;
	case 0x57: my_stprintf_s(buffer, buffer_len, _T("BIT 2, A=(IY+(%d))"), ofs); break;
	case 0x58: my_stprintf_s(buffer, buffer_len, _T("BIT 3, B=(IY+(%d))"), ofs); break;
	case 0x59: my_stprintf_s(buffer, buffer_len, _T("BIT 3, C=(IY+(%d))"), ofs); break;
	case 0x5a: my_stprintf_s(buffer, buffer_len, _T("BIT 3, D=(IY+(%d))"), ofs); break;
	case 0x5b: my_stprintf_s(buffer, buffer_len, _T("BIT 3, E=(IY+(%d))"), ofs); break;
	case 0x5c: my_stprintf_s(buffer, buffer_len, _T("BIT 3, H=(IY+(%d))"), ofs); break;
	case 0x5d: my_stprintf_s(buffer, buffer_len, _T("BIT 3, L=(IY+(%d))"), ofs); break;
	case 0x5e: my_stprintf_s(buffer, buffer_len, _T("BIT 3, (IY+(%d))"), ofs); break;
	case 0x5f: my_stprintf_s(buffer, buffer_len, _T("BIT 3, A=(IY+(%d))"), ofs); break;
	case 0x60: my_stprintf_s(buffer, buffer_len, _T("BIT 4, B=(IY+(%d))"), ofs); break;
	case 0x61: my_stprintf_s(buffer, buffer_len, _T("BIT 4, C=(IY+(%d))"), ofs); break;
	case 0x62: my_stprintf_s(buffer, buffer_len, _T("BIT 4, D=(IY+(%d))"), ofs); break;
	case 0x63: my_stprintf_s(buffer, buffer_len, _T("BIT 4, E=(IY+(%d))"), ofs); break;
	case 0x64: my_stprintf_s(buffer, buffer_len, _T("BIT 4, H=(IY+(%d))"), ofs); break;
	case 0x65: my_stprintf_s(buffer, buffer_len, _T("BIT 4, L=(IY+(%d))"), ofs); break;
	case 0x66: my_stprintf_s(buffer, buffer_len, _T("BIT 4, (IY+(%d))"), ofs); break;
	case 0x67: my_stprintf_s(buffer, buffer_len, _T("BIT 4, A=(IY+(%d))"), ofs); break;
	case 0x68: my_stprintf_s(buffer, buffer_len, _T("BIT 5, B=(IY+(%d))"), ofs); break;
	case 0x69: my_stprintf_s(buffer, buffer_len, _T("BIT 5, C=(IY+(%d))"), ofs); break;
	case 0x6a: my_stprintf_s(buffer, buffer_len, _T("BIT 5, D=(IY+(%d))"), ofs); break;
	case 0x6b: my_stprintf_s(buffer, buffer_len, _T("BIT 5, E=(IY+(%d))"), ofs); break;
	case 0x6c: my_stprintf_s(buffer, buffer_len, _T("BIT 5, H=(IY+(%d))"), ofs); break;
	case 0x6d: my_stprintf_s(buffer, buffer_len, _T("BIT 5, L=(IY+(%d))"), ofs); break;
	case 0x6e: my_stprintf_s(buffer, buffer_len, _T("BIT 5, (IY+(%d))"), ofs); break;
	case 0x6f: my_stprintf_s(buffer, buffer_len, _T("BIT 5, A=(IY+(%d))"), ofs); break;
	case 0x70: my_stprintf_s(buffer, buffer_len, _T("BIT 6, B=(IY+(%d))"), ofs); break;
	case 0x71: my_stprintf_s(buffer, buffer_len, _T("BIT 6, C=(IY+(%d))"), ofs); break;
	case 0x72: my_stprintf_s(buffer, buffer_len, _T("BIT 6, D=(IY+(%d))"), ofs); break;
	case 0x73: my_stprintf_s(buffer, buffer_len, _T("BIT 6, E=(IY+(%d))"), ofs); break;
	case 0x74: my_stprintf_s(buffer, buffer_len, _T("BIT 6, H=(IY+(%d))"), ofs); break;
	case 0x75: my_stprintf_s(buffer, buffer_len, _T("BIT 6, L=(IY+(%d))"), ofs); break;
	case 0x76: my_stprintf_s(buffer, buffer_len, _T("BIT 6, (IY+(%d))"), ofs); break;
	case 0x77: my_stprintf_s(buffer, buffer_len, _T("BIT 6, A=(IY+(%d))"), ofs); break;
	case 0x78: my_stprintf_s(buffer, buffer_len, _T("BIT 7, B=(IY+(%d))"), ofs); break;
	case 0x79: my_stprintf_s(buffer, buffer_len, _T("BIT 7, C=(IY+(%d))"), ofs); break;
	case 0x7a: my_stprintf_s(buffer, buffer_len, _T("BIT 7, D=(IY+(%d))"), ofs); break;
	case 0x7b: my_stprintf_s(buffer, buffer_len, _T("BIT 7, E=(IY+(%d))"), ofs); break;
	case 0x7c: my_stprintf_s(buffer, buffer_len, _T("BIT 7, H=(IY+(%d))"), ofs); break;
	case 0x7d: my_stprintf_s(buffer, buffer_len, _T("BIT 7, L=(IY+(%d))"), ofs); break;
	case 0x7e: my_stprintf_s(buffer, buffer_len, _T("BIT 7, (IY+(%d))"), ofs); break;
	case 0x7f: my_stprintf_s(buffer, buffer_len, _T("BIT 7, A=(IY+(%d))"), ofs); break;
	case 0x80: my_stprintf_s(buffer, buffer_len, _T("RES 0, B=(IY+(%d))"), ofs); break;
	case 0x81: my_stprintf_s(buffer, buffer_len, _T("RES 0, C=(IY+(%d))"), ofs); break;
	case 0x82: my_stprintf_s(buffer, buffer_len, _T("RES 0, D=(IY+(%d))"), ofs); break;
	case 0x83: my_stprintf_s(buffer, buffer_len, _T("RES 0, E=(IY+(%d))"), ofs); break;
	case 0x84: my_stprintf_s(buffer, buffer_len, _T("RES 0, H=(IY+(%d))"), ofs); break;
	case 0x85: my_stprintf_s(buffer, buffer_len, _T("RES 0, L=(IY+(%d))"), ofs); break;
	case 0x86: my_stprintf_s(buffer, buffer_len, _T("RES 0, (IY+(%d))"), ofs); break;
	case 0x87: my_stprintf_s(buffer, buffer_len, _T("RES 0, A=(IY+(%d))"), ofs); break;
	case 0x88: my_stprintf_s(buffer, buffer_len, _T("RES 1, B=(IY+(%d))"), ofs); break;
	case 0x89: my_stprintf_s(buffer, buffer_len, _T("RES 1, C=(IY+(%d))"), ofs); break;
	case 0x8a: my_stprintf_s(buffer, buffer_len, _T("RES 1, D=(IY+(%d))"), ofs); break;
	case 0x8b: my_stprintf_s(buffer, buffer_len, _T("RES 1, E=(IY+(%d))"), ofs); break;
	case 0x8c: my_stprintf_s(buffer, buffer_len, _T("RES 1, H=(IY+(%d))"), ofs); break;
	case 0x8d: my_stprintf_s(buffer, buffer_len, _T("RES 1, L=(IY+(%d))"), ofs); break;
	case 0x8e: my_stprintf_s(buffer, buffer_len, _T("RES 1, (IY+(%d))"), ofs); break;
	case 0x8f: my_stprintf_s(buffer, buffer_len, _T("RES 1, A=(IY+(%d))"), ofs); break;
	case 0x90: my_stprintf_s(buffer, buffer_len, _T("RES 2, B=(IY+(%d))"), ofs); break;
	case 0x91: my_stprintf_s(buffer, buffer_len, _T("RES 2, C=(IY+(%d))"), ofs); break;
	case 0x92: my_stprintf_s(buffer, buffer_len, _T("RES 2, D=(IY+(%d))"), ofs); break;
	case 0x93: my_stprintf_s(buffer, buffer_len, _T("RES 2, E=(IY+(%d))"), ofs); break;
	case 0x94: my_stprintf_s(buffer, buffer_len, _T("RES 2, H=(IY+(%d))"), ofs); break;
	case 0x95: my_stprintf_s(buffer, buffer_len, _T("RES 2, L=(IY+(%d))"), ofs); break;
	case 0x96: my_stprintf_s(buffer, buffer_len, _T("RES 2, (IY+(%d))"), ofs); break;
	case 0x97: my_stprintf_s(buffer, buffer_len, _T("RES 2, A=(IY+(%d))"), ofs); break;
	case 0x98: my_stprintf_s(buffer, buffer_len, _T("RES 3, B=(IY+(%d))"), ofs); break;
	case 0x99: my_stprintf_s(buffer, buffer_len, _T("RES 3, C=(IY+(%d))"), ofs); break;
	case 0x9a: my_stprintf_s(buffer, buffer_len, _T("RES 3, D=(IY+(%d))"), ofs); break;
	case 0x9b: my_stprintf_s(buffer, buffer_len, _T("RES 3, E=(IY+(%d))"), ofs); break;
	case 0x9c: my_stprintf_s(buffer, buffer_len, _T("RES 3, H=(IY+(%d))"), ofs); break;
	case 0x9d: my_stprintf_s(buffer, buffer_len, _T("RES 3, L=(IY+(%d))"), ofs); break;
	case 0x9e: my_stprintf_s(buffer, buffer_len, _T("RES 3, (IY+(%d))"), ofs); break;
	case 0x9f: my_stprintf_s(buffer, buffer_len, _T("RES 3, A=(IY+(%d))"), ofs); break;
	case 0xa0: my_stprintf_s(buffer, buffer_len, _T("RES 4, B=(IY+(%d))"), ofs); break;
	case 0xa1: my_stprintf_s(buffer, buffer_len, _T("RES 4, C=(IY+(%d))"), ofs); break;
	case 0xa2: my_stprintf_s(buffer, buffer_len, _T("RES 4, D=(IY+(%d))"), ofs); break;
	case 0xa3: my_stprintf_s(buffer, buffer_len, _T("RES 4, E=(IY+(%d))"), ofs); break;
	case 0xa4: my_stprintf_s(buffer, buffer_len, _T("RES 4, H=(IY+(%d))"), ofs); break;
	case 0xa5: my_stprintf_s(buffer, buffer_len, _T("RES 4, L=(IY+(%d))"), ofs); break;
	case 0xa6: my_stprintf_s(buffer, buffer_len, _T("RES 4, (IY+(%d))"), ofs); break;
	case 0xa7: my_stprintf_s(buffer, buffer_len, _T("RES 4, A=(IY+(%d))"), ofs); break;
	case 0xa8: my_stprintf_s(buffer, buffer_len, _T("RES 5, B=(IY+(%d))"), ofs); break;
	case 0xa9: my_stprintf_s(buffer, buffer_len, _T("RES 5, C=(IY+(%d))"), ofs); break;
	case 0xaa: my_stprintf_s(buffer, buffer_len, _T("RES 5, D=(IY+(%d))"), ofs); break;
	case 0xab: my_stprintf_s(buffer, buffer_len, _T("RES 5, E=(IY+(%d))"), ofs); break;
	case 0xac: my_stprintf_s(buffer, buffer_len, _T("RES 5, H=(IY+(%d))"), ofs); break;
	case 0xad: my_stprintf_s(buffer, buffer_len, _T("RES 5, L=(IY+(%d))"), ofs); break;
	case 0xae: my_stprintf_s(buffer, buffer_len, _T("RES 5, (IY+(%d))"), ofs); break;
	case 0xaf: my_stprintf_s(buffer, buffer_len, _T("RES 5, A=(IY+(%d))"), ofs); break;
	case 0xb0: my_stprintf_s(buffer, buffer_len, _T("RES 6, B=(IY+(%d))"), ofs); break;
	case 0xb1: my_stprintf_s(buffer, buffer_len, _T("RES 6, C=(IY+(%d))"), ofs); break;
	case 0xb2: my_stprintf_s(buffer, buffer_len, _T("RES 6, D=(IY+(%d))"), ofs); break;
	case 0xb3: my_stprintf_s(buffer, buffer_len, _T("RES 6, E=(IY+(%d))"), ofs); break;
	case 0xb4: my_stprintf_s(buffer, buffer_len, _T("RES 6, H=(IY+(%d))"), ofs); break;
	case 0xb5: my_stprintf_s(buffer, buffer_len, _T("RES 6, L=(IY+(%d))"), ofs); break;
	case 0xb6: my_stprintf_s(buffer, buffer_len, _T("RES 6, (IY+(%d))"), ofs); break;
	case 0xb7: my_stprintf_s(buffer, buffer_len, _T("RES 6, A=(IY+(%d))"), ofs); break;
	case 0xb8: my_stprintf_s(buffer, buffer_len, _T("RES 7, B=(IY+(%d))"), ofs); break;
	case 0xb9: my_stprintf_s(buffer, buffer_len, _T("RES 7, C=(IY+(%d))"), ofs); break;
	case 0xba: my_stprintf_s(buffer, buffer_len, _T("RES 7, D=(IY+(%d))"), ofs); break;
	case 0xbb: my_stprintf_s(buffer, buffer_len, _T("RES 7, E=(IY+(%d))"), ofs); break;
	case 0xbc: my_stprintf_s(buffer, buffer_len, _T("RES 7, H=(IY+(%d))"), ofs); break;
	case 0xbd: my_stprintf_s(buffer, buffer_len, _T("RES 7, L=(IY+(%d))"), ofs); break;
	case 0xbe: my_stprintf_s(buffer, buffer_len, _T("RES 7, (IY+(%d))"), ofs); break;
	case 0xbf: my_stprintf_s(buffer, buffer_len, _T("RES 7, A=(IY+(%d))"), ofs); break;
	case 0xc0: my_stprintf_s(buffer, buffer_len, _T("SET 0, B=(IY+(%d))"), ofs); break;
	case 0xc1: my_stprintf_s(buffer, buffer_len, _T("SET 0, C=(IY+(%d))"), ofs); break;
	case 0xc2: my_stprintf_s(buffer, buffer_len, _T("SET 0, D=(IY+(%d))"), ofs); break;
	case 0xc3: my_stprintf_s(buffer, buffer_len, _T("SET 0, E=(IY+(%d))"), ofs); break;
	case 0xc4: my_stprintf_s(buffer, buffer_len, _T("SET 0, H=(IY+(%d))"), ofs); break;
	case 0xc5: my_stprintf_s(buffer, buffer_len, _T("SET 0, L=(IY+(%d))"), ofs); break;
	case 0xc6: my_stprintf_s(buffer, buffer_len, _T("SET 0, (IY+(%d))"), ofs); break;
	case 0xc7: my_stprintf_s(buffer, buffer_len, _T("SET 0, A=(IY+(%d))"), ofs); break;
	case 0xc8: my_stprintf_s(buffer, buffer_len, _T("SET 1, B=(IY+(%d))"), ofs); break;
	case 0xc9: my_stprintf_s(buffer, buffer_len, _T("SET 1, C=(IY+(%d))"), ofs); break;
	case 0xca: my_stprintf_s(buffer, buffer_len, _T("SET 1, D=(IY+(%d))"), ofs); break;
	case 0xcb: my_stprintf_s(buffer, buffer_len, _T("SET 1, E=(IY+(%d))"), ofs); break;
	case 0xcc: my_stprintf_s(buffer, buffer_len, _T("SET 1, H=(IY+(%d))"), ofs); break;
	case 0xcd: my_stprintf_s(buffer, buffer_len, _T("SET 1, L=(IY+(%d))"), ofs); break;
	case 0xce: my_stprintf_s(buffer, buffer_len, _T("SET 1, (IY+(%d))"), ofs); break;
	case 0xcf: my_stprintf_s(buffer, buffer_len, _T("SET 1, A=(IY+(%d))"), ofs); break;
	case 0xd0: my_stprintf_s(buffer, buffer_len, _T("SET 2, B=(IY+(%d))"), ofs); break;
	case 0xd1: my_stprintf_s(buffer, buffer_len, _T("SET 2, C=(IY+(%d))"), ofs); break;
	case 0xd2: my_stprintf_s(buffer, buffer_len, _T("SET 2, D=(IY+(%d))"), ofs); break;
	case 0xd3: my_stprintf_s(buffer, buffer_len, _T("SET 2, E=(IY+(%d))"), ofs); break;
	case 0xd4: my_stprintf_s(buffer, buffer_len, _T("SET 2, H=(IY+(%d))"), ofs); break;
	case 0xd5: my_stprintf_s(buffer, buffer_len, _T("SET 2, L=(IY+(%d))"), ofs); break;
	case 0xd6: my_stprintf_s(buffer, buffer_len, _T("SET 2, (IY+(%d))"), ofs); break;
	case 0xd7: my_stprintf_s(buffer, buffer_len, _T("SET 2, A=(IY+(%d))"), ofs); break;
	case 0xd8: my_stprintf_s(buffer, buffer_len, _T("SET 3, B=(IY+(%d))"), ofs); break;
	case 0xd9: my_stprintf_s(buffer, buffer_len, _T("SET 3, C=(IY+(%d))"), ofs); break;
	case 0xda: my_stprintf_s(buffer, buffer_len, _T("SET 3, D=(IY+(%d))"), ofs); break;
	case 0xdb: my_stprintf_s(buffer, buffer_len, _T("SET 3, E=(IY+(%d))"), ofs); break;
	case 0xdc: my_stprintf_s(buffer, buffer_len, _T("SET 3, H=(IY+(%d))"), ofs); break;
	case 0xdd: my_stprintf_s(buffer, buffer_len, _T("SET 3, L=(IY+(%d))"), ofs); break;
	case 0xde: my_stprintf_s(buffer, buffer_len, _T("SET 3, (IY+(%d))"), ofs); break;
	case 0xdf: my_stprintf_s(buffer, buffer_len, _T("SET 3, A=(IY+(%d))"), ofs); break;
	case 0xe0: my_stprintf_s(buffer, buffer_len, _T("SET 4, B=(IY+(%d))"), ofs); break;
	case 0xe1: my_stprintf_s(buffer, buffer_len, _T("SET 4, C=(IY+(%d))"), ofs); break;
	case 0xe2: my_stprintf_s(buffer, buffer_len, _T("SET 4, D=(IY+(%d))"), ofs); break;
	case 0xe3: my_stprintf_s(buffer, buffer_len, _T("SET 4, E=(IY+(%d))"), ofs); break;
	case 0xe4: my_stprintf_s(buffer, buffer_len, _T("SET 4, H=(IY+(%d))"), ofs); break;
	case 0xe5: my_stprintf_s(buffer, buffer_len, _T("SET 4, L=(IY+(%d))"), ofs); break;
	case 0xe6: my_stprintf_s(buffer, buffer_len, _T("SET 4, (IY+(%d))"), ofs); break;
	case 0xe7: my_stprintf_s(buffer, buffer_len, _T("SET 4, A=(IY+(%d))"), ofs); break;
	case 0xe8: my_stprintf_s(buffer, buffer_len, _T("SET 5, B=(IY+(%d))"), ofs); break;
	case 0xe9: my_stprintf_s(buffer, buffer_len, _T("SET 5, C=(IY+(%d))"), ofs); break;
	case 0xea: my_stprintf_s(buffer, buffer_len, _T("SET 5, D=(IY+(%d))"), ofs); break;
	case 0xeb: my_stprintf_s(buffer, buffer_len, _T("SET 5, E=(IY+(%d))"), ofs); break;
	case 0xec: my_stprintf_s(buffer, buffer_len, _T("SET 5, H=(IY+(%d))"), ofs); break;
	case 0xed: my_stprintf_s(buffer, buffer_len, _T("SET 5, L=(IY+(%d))"), ofs); break;
	case 0xee: my_stprintf_s(buffer, buffer_len, _T("SET 5, (IY+(%d))"), ofs); break;
	case 0xef: my_stprintf_s(buffer, buffer_len, _T("SET 5, A=(IY+(%d))"), ofs); break;
	case 0xf0: my_stprintf_s(buffer, buffer_len, _T("SET 6, B=(IY+(%d))"), ofs); break;
	case 0xf1: my_stprintf_s(buffer, buffer_len, _T("SET 6, C=(IY+(%d))"), ofs); break;
	case 0xf2: my_stprintf_s(buffer, buffer_len, _T("SET 6, D=(IY+(%d))"), ofs); break;
	case 0xf3: my_stprintf_s(buffer, buffer_len, _T("SET 6, E=(IY+(%d))"), ofs); break;
	case 0xf4: my_stprintf_s(buffer, buffer_len, _T("SET 6, H=(IY+(%d))"), ofs); break;
	case 0xf5: my_stprintf_s(buffer, buffer_len, _T("SET 6, L=(IY+(%d))"), ofs); break;
	case 0xf6: my_stprintf_s(buffer, buffer_len, _T("SET 6, (IY+(%d))"), ofs); break;
	case 0xf7: my_stprintf_s(buffer, buffer_len, _T("SET 6, A=(IY+(%d))"), ofs); break;
	case 0xf8: my_stprintf_s(buffer, buffer_len, _T("SET 7, B=(IY+(%d))"), ofs); break;
	case 0xf9: my_stprintf_s(buffer, buffer_len, _T("SET 7, C=(IY+(%d))"), ofs); break;
	case 0xfa: my_stprintf_s(buffer, buffer_len, _T("SET 7, D=(IY+(%d))"), ofs); break;
	case 0xfb: my_stprintf_s(buffer, buffer_len, _T("SET 7, E=(IY+(%d))"), ofs); break;
	case 0xfc: my_stprintf_s(buffer, buffer_len, _T("SET 7, H=(IY+(%d))"), ofs); break;
	case 0xfd: my_stprintf_s(buffer, buffer_len, _T("SET 7, L=(IY+(%d))"), ofs); break;
	case 0xfe: my_stprintf_s(buffer, buffer_len, _T("SET 7, (IY+(%d))"), ofs); break;
	case 0xff: my_stprintf_s(buffer, buffer_len, _T("SET 7, A=(IY+(%d))"), ofs); break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}

} // extern "C"

int Z80::debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata )
{
	for(int i = 0; i < 4; i++) {
		int wait_tmp;
		z80_dasm_ops[i] = d_mem_stored->read_data8w(pc + i, &wait_tmp);
	}
	return z80_dasm_main(pc, buffer, buffer_len, d_debugger->first_symbol);
}

#define STATE_VERSION 6

bool Z80::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}

	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(total_icount);

	state_fio->StateValue(icount);
	state_fio->StateValue(dma_icount);
	state_fio->StateValue(wait_icount);
	state_fio->StateValue(prevpc);
	state_fio->StateValue(pc.d);
	state_fio->StateValue(sp.d);
	state_fio->StateValue(af.d);
	state_fio->StateValue(bc.d);
	state_fio->StateValue(de.d);
	state_fio->StateValue(hl.d);
	state_fio->StateValue(ix.d);
	state_fio->StateValue(iy.d);
	state_fio->StateValue(wz.d);
	state_fio->StateValue(af2.d);
	state_fio->StateValue(bc2.d);
	state_fio->StateValue(de2.d);
	state_fio->StateValue(hl2.d);
	state_fio->StateValue(I);
	state_fio->StateValue(R);
	state_fio->StateValue(R2);
	state_fio->StateValue(ea);
	state_fio->StateValue(busreq);
	state_fio->StateValue(wait);
	state_fio->StateValue(after_halt);
	state_fio->StateValue(im);
	state_fio->StateValue(iff1);
	state_fio->StateValue(iff2);
	state_fio->StateValue(icr);
	state_fio->StateValue(after_di);
	state_fio->StateValue(after_ei);
	state_fio->StateValue(after_ldair);
	state_fio->StateValue(intr_req_bit);
	state_fio->StateValue(intr_pend_bit);
	state_fio->StateValue(waitfactor);
	state_fio->StateValue(waitcount);
	state_fio->StateValue(extra_cycles);

	// post process
	if(loading) {
		prev_total_icount = total_icount;
		// Post process for collecting statistics.
		cycles_tmp_count = total_icount;
		extra_tmp_count = 0;
		insns_count = 0;
		frames_count = 0;
		nmi_count = 0;
		irq_count = 0;
		nsc800_int_count = 0;
		nsc800_rsta_count = 0;
		nsc800_rsta_count = 0;
		nsc800_rsta_count = 0;
	}
 	return true;
}


//#endif
