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
	after_halt = true; \
} while(0)

#define LEAVE_HALT() do { \
	if(after_halt) { \
		after_halt = false; \
		PC++; \
	} \
} while(0)


#define POP(DR) do { \
	RM16(SPD, &DR); \
	SP += 2; \
} while(0)

#define PUSH(SR) do { \
	SP -= 2; \
	WM16(SPD, &SR); \
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
	Z80_BASE::initialize();
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
	after_halt = false;
	after_ei = after_ldair = false;
	intr_req_bit = intr_pend_bit = 0;
	
	icount = extra_icount = 0;
}

void Z80::debugger_hook(void)
{
#ifdef USE_DEBUGGER
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
#endif
}

int Z80::run(int clock)
{
	if(clock == -1) {
		if(busreq) {
			// run dma once
			#ifdef SINGLE_MODE_DMA
				if(d_dma) {
					d_dma->do_dma();
				}
			#endif
			// don't run cpu!
			int passed_icount = max(1, extra_icount);
			// this is main cpu, icount is not used
			/*icount = */extra_icount = 0;
			#ifdef USE_DEBUGGER
				total_icount += passed_icount;
				debugger_hook();
			#endif
			return passed_icount;
		} else {
			// run only one opcode
			#ifdef USE_DEBUGGER
				total_icount += extra_icount;
			#endif
			icount = -extra_icount;
			extra_icount = 0;
			run_one_opecode();
			return -icount;
		}
	} else {
		icount += clock;
		int first_icount = icount;
		#ifdef USE_DEBUGGER
			total_icount += extra_icount;
		#endif
		icount -= extra_icount;
		extra_icount = 0;
		
		if(busreq) {
			// run dma once
			#ifdef USE_DEBUGGER
				debugger_hook();
			#endif
			#ifdef SINGLE_MODE_DMA
				if(d_dma) {
					d_dma->do_dma();
				}
			#endif
		} else {
			// run cpu while given clocks
			while(icount > 0 && !busreq) {
				run_one_opecode();
			}
		}
		// if busreq is raised, spin cpu while remained clock
		if(icount > 0 && busreq) {
			#ifdef USE_DEBUGGER
				total_icount += icount;
			#endif
			icount = 0;
		}
		return first_icount - icount;
	}
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
			d_debugger->now_waiting = true;
			while(d_debugger->now_debugging && d_debugger->now_suspended) {
				emu->sleep(10);
			}
			d_debugger->now_waiting = false;
		}
		if(d_debugger->now_debugging) {
			d_mem = d_io = d_debugger;
		} else {
			now_debugging = false;
		}
		
		after_halt = after_ei = false;
#if HAS_LDAIR_QUIRK
		after_ldair = false;
#endif
		OP(FETCHOP());
#if HAS_LDAIR_QUIRK
		if(after_ldair) {
			F &= ~PF;	// reset parity flag after LD A,I or LD A,R
		}
#endif
#ifdef SINGLE_MODE_DMA
		if(d_dma) {
			d_dma->do_dma();
		}
#endif
		if(!after_ei) {
			check_interrupt();
		}
		
		if(now_debugging) {
			if(!d_debugger->now_going) {
				d_debugger->now_suspended = true;
			}
			d_mem = d_mem_stored;
			d_io = d_io_stored;
		}
	} else {
#endif
		after_halt = after_ei = false;
#if HAS_LDAIR_QUIRK
		after_ldair = false;
#endif
		d_debugger->add_cpu_trace(PC);
		int first_icount = icount;
		OP(FETCHOP());
		icount -= extra_icount;
		extra_icount = 0;
		total_icount += first_icount - icount;
#if HAS_LDAIR_QUIRK
		if(after_ldair) {
			F &= ~PF;	// reset parity flag after LD A,I or LD A,R
		}
#endif
#ifdef SINGLE_MODE_DMA
		if(d_dma) {
			d_dma->do_dma();
		}
#endif
		if(!after_ei) {
			check_interrupt();
		}
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
				d_debugger->now_waiting = true;
				while(d_debugger->now_debugging && d_debugger->now_suspended) {
					emu->sleep(10);
				}
				d_debugger->now_waiting = false;
			}
			if(d_debugger->now_debugging) {
				d_mem = d_io = d_debugger;
			} else {
				now_debugging = false;
			}
			
			after_halt = false;
#if HAS_LDAIR_QUIRK
			after_ldair = false;
#endif
			OP(FETCHOP());
#if HAS_LDAIR_QUIRK
			if(after_ldair) {
				F &= ~PF;	// reset parity flag after LD A,I or LD A,R
			}
#endif
#ifdef SINGLE_MODE_DMA
			if(d_dma) {
				d_dma->do_dma();
			}
#endif
			if(d_pic != NULL) d_pic->notify_intr_ei();
			check_interrupt();
			
			if(now_debugging) {
				if(!d_debugger->now_going) {
					d_debugger->now_suspended = true;
				}
				d_mem = d_mem_stored;
				d_io = d_io_stored;
			}
		} else {
#endif
			after_halt = false;
#if HAS_LDAIR_QUIRK
			after_ldair = false;
#endif
			d_debugger->add_cpu_trace(PC);
			int first_icount = icount;
			OP(FETCHOP());
			icount -= extra_icount;
			extra_icount = 0;
			total_icount += first_icount - icount;
#if HAS_LDAIR_QUIRK
			if(after_ldair) {
				F &= ~PF;	// reset parity flag after LD A,I or LD A,R
			}
#endif
#ifdef SINGLE_MODE_DMA
			if(d_dma) {
				d_dma->do_dma();
			}
#endif
			if(d_pic != NULL) d_pic->notify_intr_ei();
			check_interrupt();
#ifdef USE_DEBUGGER
		}
#endif
	}
#ifndef USE_DEBUGGER
	icount -= extra_icount;
	extra_icount = 0;
#endif
}

void Z80::check_interrupt()
{
#ifdef USE_DEBUGGER
	int first_icount = icount;
#endif
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
				if(d_pic != NULL) { // OK?
					PCD = WZ = d_pic->get_intr_ack() & 0xffff;
				} else {
					PCD = WZ = (PCD & 0xff00) | 0xcd;
				}
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
				
				uint32_t vector = 0xcd;
				if(d_pic != NULL) vector = d_pic->get_intr_ack();
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
#ifdef USE_DEBUGGER
	total_icount += first_icount - icount;
#endif
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

void Z80::decl_state(void)
{
	Z80_BASE::decl_state();
}

void Z80::save_state(FILEIO* state_fio)
{
	Z80_BASE::save_state(state_fio);
}

bool Z80::load_state(FILEIO* state_fio)
{
	bool mb = false;
	mb = Z80_BASE::load_state(state_fio);
	if(!mb) return false;
	prev_total_icount = total_icount;
	return true;
}


