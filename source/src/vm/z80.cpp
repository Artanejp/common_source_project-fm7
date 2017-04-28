/*
	Skelton for retropc emulator

	Origin : MAME 0.145
	Author : Takeda.Toshiya
	Date   : 2012.02.15-

	[ Z80 ]
*/

#include "vm.h"
#include "../emu.h"
#include "z80.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#ifndef CPU_START_ADDR
#define CPU_START_ADDR	0
#endif

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
	halt = true; \
} while(0)

#define LEAVE_HALT() do { \
	if(halt) { \
		halt = false; \
		PC++; \
	} \
} while(0)

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
	} else PC++; \
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
	d_pic->notify_intr_reti(); \
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

// main
Z80::Z80(VM* parent_vm, EMU* parent_emu) : Z80_BASE(parent_vm, parent_emu)
{
#ifdef HAS_NSC800
	has_nsc800 = true;
#endif
#ifdef Z80_PSEUDO_BIOS
	has_pseudo_bios = true;
#endif
#ifdef SINGLE_MODE_DMA
	has_single_mode_dma = true;
#endif
#ifdef Z80_MEMORY_WAIT
	has_memory_wait = true;
#endif
#ifdef Z80_IO_WAIT
	has_io_wait = true;
#endif
#ifdef HAS_LDAIR_QUIRK
	has_ldair_quirk = true;
#endif
}

Z80::~Z80()
{
}

void Z80::initialize()
{
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
#ifdef USE_DEBUGGER
	d_mem_stored = d_mem;
	d_io_stored = d_io;
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
#endif
}

void Z80::reset()
{
	PCD = CPU_START_ADDR;
	SPD = 0;
	AFD = BCD = DED = HLD = 0;
	IXD = IYD = 0xffff;	/* IX and IY are FFFF after a reset! */
	F = ZF;			/* Zero flag is set */
	I = R = R2 = 0;
	WZD = PCD;
	af2.d = bc2.d = de2.d = hl2.d = 0;
	ea = 0;
	
	im = iff1 = iff2 = icr = 0;
	halt = false;
	after_ei = after_ldair = false;
	intr_req_bit = intr_pend_bit = 0;
	
	icount = extra_icount = 0;
}

void Z80::run_one_opecode()
{
	// rune one opecode
#ifdef USE_DEBUGGER
	bool now_debugging = d_debugger->now_debugging;
	if(now_debugging) {
		d_debugger->check_break_points(PC);
		if(d_debugger->now_suspended) {
			emu->mute_sound();
			while(d_debugger->now_debugging && d_debugger->now_suspended) {
				emu->sleep(10);
			}
		}
		if(d_debugger->now_debugging) {
			d_mem = d_io = d_debugger;
		} else {
			now_debugging = false;
		}
		
		after_ei = after_ldair = false;
		OP(FETCHOP());
#if HAS_LDAIR_QUIRK
		if(after_ldair) F &= ~PF;	// reset parity flag after LD A,I or LD A,R
#endif
		
		if(now_debugging) {
			if(!d_debugger->now_going) {
				d_debugger->now_suspended = true;
			}
			d_mem = d_mem_stored;
			d_io = d_io_stored;
		}
	} else {
#endif
		after_ei = after_ldair = false;
		OP(FETCHOP());
#if HAS_LDAIR_QUIRK
		if(after_ldair) F &= ~PF;	// reset parity flag after LD A,I or LD A,R
#endif
#ifdef USE_DEBUGGER
	}
#endif
	
	// ei: run next opecode
	if(after_ei) {
#ifdef USE_DEBUGGER
		bool now_debugging = d_debugger->now_debugging;
		if(now_debugging) {
			d_debugger->check_break_points(PC);
			if(d_debugger->now_suspended) {
				emu->mute_sound();
				while(d_debugger->now_debugging && d_debugger->now_suspended) {
					emu->sleep(10);
				}
			}
			if(d_debugger->now_debugging) {
				d_mem = d_io = d_debugger;
			} else {
				now_debugging = false;
			}
			
			after_ldair = false;
			OP(FETCHOP());
#if HAS_LDAIR_QUIRK
			if(after_ldair) F &= ~PF;	// reset parity flag after LD A,I or LD A,R
#endif
			d_pic->notify_intr_ei();
			
			if(now_debugging) {
				if(!d_debugger->now_going) {
					d_debugger->now_suspended = true;
				}
				d_mem = d_mem_stored;
				d_io = d_io_stored;
			}
		} else {
#endif
			after_ldair = false;
			OP(FETCHOP());
#if HAS_LDAIR_QUIRK
			if(after_ldair) F &= ~PF;	// reset parity flag after LD A,I or LD A,R
#endif
			d_pic->notify_intr_ei();
#ifdef USE_DEBUGGER
		}
#endif
	}
	
	// check interrupt
	if(intr_req_bit) {
		if(intr_req_bit & NMI_REQ_BIT) {
			// nmi
			LEAVE_HALT();
			PUSH(pc);
			PCD = WZD = 0x0066;
			icount -= 11;
			iff1 = 0;
			intr_req_bit &= ~NMI_REQ_BIT;
//#ifdef HAS_NSC800
		} else if(has_nsc800) {
			if((intr_req_bit & 1) && (icr & 1)) {
				// INTR
				LEAVE_HALT();
				PUSH(pc);
				PCD = WZ = d_pic->get_intr_ack() & 0xffff;
				icount -= cc_op[0xcd] + cc_ex[0xff];
				iff1 = iff2 = 0;
				intr_req_bit &= ~1;
			} else if((intr_req_bit & 8) && (icr & 8)) {
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
			}
		} else { // Normal Z80
			if(iff1) {
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
			} else {
				intr_req_bit &= intr_pend_bit;
//#endif
			}
//#else
		}
	}
//#ifdef SINGLE_MODE_DMA
	if(d_dma) {
		d_dma->do_dma();
	}
//#endif
	icount -= extra_icount;
	extra_icount = 0;
}

#ifdef USE_DEBUGGER
void Z80::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem_stored->write_data8w(addr, data, &wait);
}

uint32_t Z80::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem_stored->read_data8w(addr, &wait);
}

void Z80::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	d_io_stored->write_io8w(addr, data, &wait);
}

uint32_t Z80::read_debug_io8(uint32_t addr)
{
	int wait;
	return d_io_stored->read_io8w(addr, &wait);
}

// disassembler
extern "C" {
extern int z80_dasm_main(uint32_t pc, _TCHAR *buffer, size_t buffer_len, symbol_t *first_symbol);

extern uint8_t z80_dasm_ops[4];
extern int z80_dasm_ptr;
}

int Z80::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	for(int i = 0; i < 4; i++) {
		int wait;
		z80_dasm_ops[i] = d_mem_stored->read_data8w(pc + i, &wait);
	}
	return z80_dasm_main(pc, buffer, buffer_len, d_debugger->first_symbol);
}

#endif


