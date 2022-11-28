/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date   : 2011.05.06-

	[ MC6809 ]
        Notes from K.Ohta <whatisthis.sowhat _at_ gmail.com> at Jan 16, 2015:
              All of undocumented instructions (i.e. ngc, flag16) of MC6809(not HD6309) are written by me.
              These behaviors of undocumented insns are refered from "vm/cpu_x86.asm" (ia32 assembly codefor nasm) within XM7
              written by Ryu Takegami , and older article wrote in magazine, "I/O" at 1985.
              But, these C implements are written from scratch by me , and I tested many years at XM7/SDL.
              Perhaps, these insns. are not implement MAME/MESS yet.
*/

// Fixed IRQ/FIRQ by Mr.Sasaji at 2011.06.17
#include "./mc6809.h"
#include "./mc6809_consts.h"
#include "../common.h"
#include "../config.h"

#define OP_HANDLER(_name) void MC6809_BASE::_name (void)

static void (MC6809_BASE::*m6809_main[0x100]) (void) = {
/*          0xX0,   0xX1,     0xX2,    0xX3,    0xX4,    0xX5,    0xX6,    0xX7,
            0xX8,   0xX9,     0xXA,    0xXB,    0xXC,    0xXD,    0xXE,    0xXF   */

/* 0x0X */
	&MC6809_BASE::neg_di, &MC6809_BASE::neg_di, &MC6809_BASE::ngc_di, &MC6809_BASE::com_di,
	&MC6809_BASE::lsr_di, &MC6809_BASE::lsr_di, &MC6809_BASE::ror_di, &MC6809_BASE::asr_di,
	&MC6809_BASE::asl_di, &MC6809_BASE::rol_di, &MC6809_BASE::dec_di, &MC6809_BASE::dcc_di,
	&MC6809_BASE::inc_di, &MC6809_BASE::tst_di, &MC6809_BASE::jmp_di, &MC6809_BASE::clr_di,
/* 0x1X */
	&MC6809_BASE::pref10, &MC6809_BASE::pref11, &MC6809_BASE::nop, &MC6809_BASE::sync_09,
	&MC6809_BASE::trap, &MC6809_BASE::trap, &MC6809_BASE::lbra, &MC6809_BASE::lbsr,
	&MC6809_BASE::aslcc_in, &MC6809_BASE::daa, &MC6809_BASE::orcc, &MC6809_BASE::nop,
	&MC6809_BASE::andcc, &MC6809_BASE::sex, &MC6809_BASE::exg, &MC6809_BASE::tfr,
/* 0x2X */
	&MC6809_BASE::bra, &MC6809_BASE::brn, &MC6809_BASE::bhi, &MC6809_BASE::bls,
	&MC6809_BASE::bcc, &MC6809_BASE::bcs, &MC6809_BASE::bne, &MC6809_BASE::beq,
	&MC6809_BASE::bvc, &MC6809_BASE::bvs, &MC6809_BASE::bpl, &MC6809_BASE::bmi,
	&MC6809_BASE::bge, &MC6809_BASE::blt, &MC6809_BASE::bgt, &MC6809_BASE::ble,
/* 0x3X */
	&MC6809_BASE::leax, &MC6809_BASE::leay, &MC6809_BASE::leas, &MC6809_BASE::leau,
	&MC6809_BASE::pshs, &MC6809_BASE::puls, &MC6809_BASE::pshu, &MC6809_BASE::pulu,
	&MC6809_BASE::andcc, &MC6809_BASE::rts, &MC6809_BASE::abx, &MC6809_BASE::rti,
	&MC6809_BASE::cwai, &MC6809_BASE::mul, &MC6809_BASE::rst, &MC6809_BASE::swi,
/* 0x4X */
	&MC6809_BASE::nega, &MC6809_BASE::nega, &MC6809_BASE::ngca, &MC6809_BASE::coma,
	&MC6809_BASE::lsra, &MC6809_BASE::lsra, &MC6809_BASE::rora, &MC6809_BASE::asra,
	&MC6809_BASE::asla, &MC6809_BASE::rola, &MC6809_BASE::deca, &MC6809_BASE::dcca,
	&MC6809_BASE::inca, &MC6809_BASE::tsta, &MC6809_BASE::clca, &MC6809_BASE::clra,
/* 0x5X */
	&MC6809_BASE::negb, &MC6809_BASE::negb, &MC6809_BASE::ngcb, &MC6809_BASE::comb,
	&MC6809_BASE::lsrb, &MC6809_BASE::lsrb, &MC6809_BASE::rorb, &MC6809_BASE::asrb,
	&MC6809_BASE::aslb, &MC6809_BASE::rolb, &MC6809_BASE::decb, &MC6809_BASE::dccb,
	&MC6809_BASE::incb, &MC6809_BASE::tstb, &MC6809_BASE::clcb, &MC6809_BASE::clrb,
/* 0x6X */
	&MC6809_BASE::neg_ix, &MC6809_BASE::neg_ix, &MC6809_BASE::ngc_ix, &MC6809_BASE::com_ix,
	&MC6809_BASE::lsr_ix, &MC6809_BASE::lsr_ix, &MC6809_BASE::ror_ix, &MC6809_BASE::asr_ix,
	&MC6809_BASE::asl_ix, &MC6809_BASE::rol_ix, &MC6809_BASE::dec_ix, &MC6809_BASE::dcc_ix,
	&MC6809_BASE::inc_ix, &MC6809_BASE::tst_ix, &MC6809_BASE::jmp_ix, &MC6809_BASE::clr_ix,
/* 0x7X */
	&MC6809_BASE::neg_ex, &MC6809_BASE::neg_ex, &MC6809_BASE::ngc_ex, &MC6809_BASE::com_ex,
	&MC6809_BASE::lsr_ex, &MC6809_BASE::lsr_ex, &MC6809_BASE::ror_ex, &MC6809_BASE::asr_ex,
	&MC6809_BASE::asl_ex, &MC6809_BASE::rol_ex, &MC6809_BASE::dec_ex, &MC6809_BASE::dcc_ex,
	&MC6809_BASE::inc_ex, &MC6809_BASE::tst_ex, &MC6809_BASE::jmp_ex, &MC6809_BASE::clr_ex,
/* 0x8X */
	&MC6809_BASE::suba_im, &MC6809_BASE::cmpa_im, &MC6809_BASE::sbca_im, &MC6809_BASE::subd_im,
	&MC6809_BASE::anda_im, &MC6809_BASE::bita_im, &MC6809_BASE::lda_im, &MC6809_BASE::flag8_im,
	&MC6809_BASE::eora_im, &MC6809_BASE::adca_im, &MC6809_BASE::ora_im, &MC6809_BASE::adda_im,
	&MC6809_BASE::cmpx_im, &MC6809_BASE::bsr, &MC6809_BASE::ldx_im, &MC6809_BASE::flag16_im,
/* 0x9X */
	&MC6809_BASE::suba_di, &MC6809_BASE::cmpa_di, &MC6809_BASE::sbca_di, &MC6809_BASE::subd_di,
	&MC6809_BASE::anda_di, &MC6809_BASE::bita_di, &MC6809_BASE::lda_di, &MC6809_BASE::sta_di,
	&MC6809_BASE::eora_di, &MC6809_BASE::adca_di, &MC6809_BASE::ora_di, &MC6809_BASE::adda_di,
	&MC6809_BASE::cmpx_di, &MC6809_BASE::jsr_di, &MC6809_BASE::ldx_di, &MC6809_BASE::stx_di,
/* 0xAX */
	&MC6809_BASE::suba_ix, &MC6809_BASE::cmpa_ix, &MC6809_BASE::sbca_ix, &MC6809_BASE::subd_ix,
	&MC6809_BASE::anda_ix, &MC6809_BASE::bita_ix, &MC6809_BASE::lda_ix, &MC6809_BASE::sta_ix,
	&MC6809_BASE::eora_ix, &MC6809_BASE::adca_ix, &MC6809_BASE::ora_ix, &MC6809_BASE::adda_ix,
	&MC6809_BASE::cmpx_ix, &MC6809_BASE::jsr_ix, &MC6809_BASE::ldx_ix, &MC6809_BASE::stx_ix,
/* 0xBX */
	&MC6809_BASE::suba_ex, &MC6809_BASE::cmpa_ex, &MC6809_BASE::sbca_ex, &MC6809_BASE::subd_ex,
	&MC6809_BASE::anda_ex, &MC6809_BASE::bita_ex, &MC6809_BASE::lda_ex, &MC6809_BASE::sta_ex,
	&MC6809_BASE::eora_ex, &MC6809_BASE::adca_ex, &MC6809_BASE::ora_ex, &MC6809_BASE::adda_ex,
	&MC6809_BASE::cmpx_ex, &MC6809_BASE::jsr_ex, &MC6809_BASE::ldx_ex, &MC6809_BASE::stx_ex,
/* 0xCX */
	&MC6809_BASE::subb_im, &MC6809_BASE::cmpb_im, &MC6809_BASE::sbcb_im, &MC6809_BASE::addd_im,
	&MC6809_BASE::andb_im, &MC6809_BASE::bitb_im, &MC6809_BASE::ldb_im, &MC6809_BASE::flag8_im,
	&MC6809_BASE::eorb_im, &MC6809_BASE::adcb_im, &MC6809_BASE::orb_im, &MC6809_BASE::addb_im,
	&MC6809_BASE::ldd_im, &MC6809_BASE::trap, &MC6809_BASE::ldu_im, &MC6809_BASE::flag16_im,
/* 0xDX */
	&MC6809_BASE::subb_di, &MC6809_BASE::cmpb_di, &MC6809_BASE::sbcb_di, &MC6809_BASE::addd_di,
	&MC6809_BASE::andb_di, &MC6809_BASE::bitb_di, &MC6809_BASE::ldb_di, &MC6809_BASE::stb_di,
	&MC6809_BASE::eorb_di, &MC6809_BASE::adcb_di, &MC6809_BASE::orb_di, &MC6809_BASE::addb_di,
	&MC6809_BASE::ldd_di, &MC6809_BASE::std_di, &MC6809_BASE::ldu_di, &MC6809_BASE::stu_di,
/* 0xEX */
	&MC6809_BASE::subb_ix, &MC6809_BASE::cmpb_ix, &MC6809_BASE::sbcb_ix, &MC6809_BASE::addd_ix,
	&MC6809_BASE::andb_ix, &MC6809_BASE::bitb_ix, &MC6809_BASE::ldb_ix, &MC6809_BASE::stb_ix,
	&MC6809_BASE::eorb_ix, &MC6809_BASE::adcb_ix, &MC6809_BASE::orb_ix, &MC6809_BASE::addb_ix,
	&MC6809_BASE::ldd_ix, &MC6809_BASE::std_ix, &MC6809_BASE::ldu_ix, &MC6809_BASE::stu_ix,
/* 0xFX */
	&MC6809_BASE::subb_ex, &MC6809_BASE::cmpb_ex, &MC6809_BASE::sbcb_ex, &MC6809_BASE::addd_ex,
	&MC6809_BASE::andb_ex, &MC6809_BASE::bitb_ex, &MC6809_BASE::ldb_ex, &MC6809_BASE::stb_ex,
	&MC6809_BASE::eorb_ex, &MC6809_BASE::adcb_ex, &MC6809_BASE::orb_ex, &MC6809_BASE::addb_ex,
	&MC6809_BASE::ldd_ex, &MC6809_BASE::std_ex, &MC6809_BASE::ldu_ex, &MC6809_BASE::stu_ex
	};
/* macros for branch instructions */
inline void MC6809_BASE::BRANCH(bool cond)
{
	uint8_t t;
	IMMBYTE(t);
	if(!cond) return;
	PC = PC + SIGNED(t);
	PC = PC & 0xffff;
}

inline void MC6809_BASE::LBRANCH(bool cond)
{
	pair32_t t;
	IMMWORD(t);
	if(!cond) return;
	icount -= 1;
	PC += t.w.l;
	PC = PC & 0xffff;
}

/* macros for setting/getting registers in TFR/EXG instructions */

inline pair32_t MC6809_BASE::RM16_PAIR(uint32_t addr)
{
	pair32_t b;
	b.d = 0;
	b.b.h = RM(addr);
	b.b.l = RM((addr + 1));
	return b;
}

inline void MC6809_BASE::WM16(uint32_t Addr, pair32_t *p)
{
	WM(Addr , p->b.h);
	WM((Addr + 1), p->b.l);
}

void MC6809_BASE::reset()
{
	//extar_tmp_count += extra_icount;
	
	icount = 0;
	waitcount = 0;
	int_state &= MC6809_HALT_BIT;
	extra_icount = 0;
	//busreq = false;
   
	DPD = 0;	/* Reset direct page register */
	CC = 0;
	D = 0;
	X = 0;
	Y = 0;
	U = 0;
	S = 0;
	EA = 0;
//#if defined(_FM7) || defined(_FM8) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	clr_used = false;

	write_signals(&outputs_bus_clr, 0x00000000);
	if((req_halt_on) && !(req_halt_off)) {
		int_state |= MC6809_HALT_BIT;
	} else {
		req_halt_on = req_halt_off = false;
	}
	if((int_state & MC6809_HALT_BIT) != 0) {
		write_signals(&outputs_bus_ba, 0xffffffff);
		write_signals(&outputs_bus_bs, 0xffffffff);
	} else {
		write_signals(&outputs_bus_ba, 0x00000000);
		write_signals(&outputs_bus_bs, 0x00000000);
	}

//#endif
	CC |= CC_II;	/* IRQ disabled */
	CC |= CC_IF;	/* FIRQ disabled */
	
	pPC = RM16_PAIR(0xfffe);
}


void MC6809_BASE::initialize()
{
	DEVICE::initialize();
	int_state = 0;
	busreq = false;
	icount = 0;
	extra_icount = 0;
	req_halt_on = req_halt_off = false;
	cycles_tmp_count = 0;
	insns_count = 0;

#ifdef _MSC_VER
	#ifdef USE_DEBUGGER
		__USE_DEBUGGER = true;
	#else
		__USE_DEBUGGER = false;
	#endif
#else
	__USE_DEBUGGER = osd->check_feature(_T("USE_DEBUGGER"));
#endif

	insns_count = 0;
	frames_count = 0;
	cycles_tmp_count = 0;
	nmi_count = 0;
	firq_count = 0;
	irq_count = 0;
	if(config.print_statistics) {
		register_frame_event(this);
	}
	waitfactor = 0;
	waitcount = 0;
}

void MC6809_BASE::event_frame()
{
	if(frames_count < 0) {
		cycles_tmp_count = total_icount;
		extra_tmp_count = 0;
		insns_count = 0;
		frames_count = 0;
		nmi_count = 0;
		firq_count = 0;
		irq_count = 0;
	} else if(frames_count >= 16) {
		uint64_t _icount = total_icount - cycles_tmp_count;
		if(config.print_statistics) {
			out_debug_log(_T("INFO: 16 frames done.\nINFO: CLOCKS = %ld INSNS = %ld EXTRA_ICOUNT = %ld \nINFO: NMI# = %d FIRQ# = %d IRQ# = %d"), _icount, insns_count, extra_tmp_count, nmi_count, firq_count, irq_count);
		}
		cycles_tmp_count = total_icount;
		insns_count = 0;
		extra_tmp_count = 0;
		frames_count = 0;
		nmi_count = 0;
		firq_count = 0;
		irq_count = 0;
	} else {
		frames_count++;
	}
}

void MC6809_BASE::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CPU_IRQ) {
		if(data & mask) {
			int_state |= MC6809_IRQ_BIT;
			irq_count++;
		} else {
			int_state &= ~MC6809_IRQ_BIT;
		}
	} else if(id == SIG_CPU_FIRQ) {
		if(data & mask) {
			int_state |= MC6809_FIRQ_BIT;
			firq_count++;
		} else {
			int_state &= ~MC6809_FIRQ_BIT;
		}
	} else if(id == SIG_CPU_NMI) {
		if(data & mask) {
			int_state |= MC6809_NMI_BIT;
			nmi_count++;
		} else {
			int_state &= ~MC6809_NMI_BIT;
		}
	} else if(id == SIG_CPU_BUSREQ) {
		if(data & mask) {
			req_halt_on = false;
			req_halt_off = false;
			int_state |= MC6809_HALT_BIT;
			busreq = false;
		} else {
			req_halt_on = false;
			req_halt_off = false;
			int_state &= ~MC6809_HALT_BIT;
		}
	} else if(id == SIG_CPU_HALTREQ) {
		if(data & mask) {
			req_halt_on = true;
			//int_state |= MC6809_HALT_BIT;
		} else {
			if(req_halt_on) {
				req_halt_off = true;
			}
			//int_state &= ~MC6809_HALT_BIT;
		}
	} else if(id == SIG_CPU_WAIT_FACTOR) {
		waitfactor = data; // 65536.
	}
}

void MC6809_BASE::cpu_nmi_push(void)
{
	if ((int_state & MC6809_CWAI_IN) == 0) {
		CC |= CC_E;
		PUSHWORD(pPC);
		PUSHWORD(pU);
		PUSHWORD(pY);
		PUSHWORD(pX);
		PUSHBYTE(DP);
		PUSHBYTE(B);
		PUSHBYTE(A);
		PUSHBYTE(CC);
	}
	return;
}

void MC6809_BASE::cpu_nmi_fetch_vector_address(void)
{
	pPC = RM16_PAIR(0xfffc);
//	printf("NMI occured PC=0x%04x VECTOR=%04x SP=%04x \n",rpc.w.l,pPC.w.l,S);
	int_state |= MC6809_CWAI_OUT;
	//int_state &= ~(MC6809_NMI_BIT | MC6809_SYNC_IN | MC6809_SYNC_OUT | MC6809_CWAI_IN);	// $FF1E
	return;
}



void MC6809_BASE::cpu_firq_fetch_vector_address(void)
{
	pPC = RM16_PAIR(0xfff6);
	int_state |= MC6809_CWAI_OUT;
	int_state &= ~(MC6809_SYNC_IN | MC6809_SYNC_OUT );
	return;
}

void MC6809_BASE::cpu_firq_push(void)
{
	//pair32_t rpc = pPC;
	if ((int_state & MC6809_CWAI_IN) == 0) {
		/* NORMAL */
		CC &= ~CC_E;
		PUSHWORD(pPC);
		PUSHBYTE(CC);
	}
//	printf("Firq occured PC=0x%04x VECTOR=%04x SP=%04x \n",rpc.w.l,pPC.w.l,S);
}
void MC6809_BASE::cpu_irq_push(void)
{
	if ((int_state & MC6809_CWAI_IN) == 0) {
		CC |= CC_E;
		PUSHWORD(pPC);
		PUSHWORD(pU);
		PUSHWORD(pY);
		PUSHWORD(pX);
		PUSHBYTE(DP);
		PUSHBYTE(B);
		PUSHBYTE(A);
		PUSHBYTE(CC);
	}
}
void MC6809_BASE::cpu_irq_fetch_vector_address(void)
{
	//pair32_t rpc = pPC;
	pPC = RM16_PAIR(0xfff8);
	int_state |= MC6809_CWAI_OUT;
	int_state &= ~(MC6809_SYNC_IN | MC6809_SYNC_OUT);
}

void MC6809_BASE::cpu_wait(int clocks)
{
	uint32_t ncount = 0;
	if(clocks < 0) return;
	if(waitfactor == 0) return;
	waitcount += (waitfactor * (uint32_t)clocks);
	if(waitcount >= 65536) {
		ncount = waitcount >> 16;
		waitcount = waitcount - (ncount << 16);
	}
	if(ncount > 0) extra_icount += ncount;
}


int MC6809_BASE::run(int clock)
{
	int cycle = 0;
	int first_icount = 0;
	int passed_icount = 0;
	first_icount = icount;
	if(extra_icount > 0) {
		extra_tmp_count += extra_icount;
	}
	
	if((req_halt_on) && !(req_halt_off)) {
		int_state |= MC6809_HALT_BIT;
	} else	if(req_halt_on && req_halt_off) { // HALT OFF
		int_state &= ~MC6809_HALT_BIT;
		req_halt_on = req_halt_off = false;
	}

	if ((int_state & MC6809_HALT_BIT) != 0) {	// 0x80
		if(clock <= 0) {
			clock = 1;
		} else {
			icount += clock;
		}
		first_icount = icount;
		if(!busreq) {
			write_signals(&outputs_bus_ba, 0);
			write_signals(&outputs_bus_bs, 0);
			busreq = true;
			icount -= clock;
			icount -= extra_icount;
			extra_icount = 0;
			passed_icount = first_icount - icount;
			total_icount += passed_icount;
			
			write_signals(&outputs_bus_ba, 0xffffffff);
			write_signals(&outputs_bus_bs, 0xffffffff);
			debugger_hook();
			cpu_wait(passed_icount);
			return passed_icount;
		} else {
			icount -= clock;
			icount -= extra_icount;
			extra_icount = 0;
			passed_icount = first_icount - icount;
			total_icount += passed_icount;
			debugger_hook();
			cpu_wait(passed_icount);
			return passed_icount;
		}
	}
	if(busreq) { // Exit from BUSREQ state.
		if((int_state & MC6809_SYNC_IN) != 0) {
			write_signals(&outputs_bus_ba, 0xffffffff);
		} else {
			write_signals(&outputs_bus_ba, 0x00000000);
		}				
		write_signals(&outputs_bus_bs, 0x00000000);
		busreq = false;
	}
	if((int_state & MC6809_INSN_HALT) != 0) {	// 0x80
		if(clock <= 1) clock = 1;
		icount += clock;
		first_icount = icount;
		while(icount > 0) {
			RM(PCD); //Will save.Need to keep.
			icount -= 1;
		}
  		icount -= extra_icount;
  		passed_icount = first_icount - icount;
		extra_icount = 0;
		PC++;
		debugger_hook();
		total_icount += passed_icount;
		cpu_wait(passed_icount);
		return passed_icount;
	}
 	/*
	 * Check Interrupt
	 */
check_nmi:
	if ((int_state & (MC6809_NMI_BIT | MC6809_FIRQ_BIT | MC6809_IRQ_BIT)) != 0) {	// 0x0007
		if ((int_state & MC6809_NMI_BIT) == 0)
			goto check_firq;
		int_state &= ~MC6809_SYNC_IN; // Thanks to Ryu Takegami.
		write_signals(&outputs_bus_ba, 0x00000000);
		write_signals(&outputs_bus_bs, 0x00000000);
		if((int_state & MC6809_CWAI_IN) == 0) {
			CC = CC | CC_E;
			cycle += 14;
			cpu_nmi_push();
		}
		write_signals(&outputs_bus_bs, 0xffffffff);
		CC = CC | CC_II | CC_IF;	// 0x50
		cycle += 2;
		cpu_nmi_fetch_vector_address();
		cycle += 3;
		write_signals(&outputs_bus_bs, 0x00000000);
		int_state &= ~(MC6809_NMI_BIT | MC6809_SYNC_IN | MC6809_SYNC_OUT);	// $FF1E
		goto int_cycle;
	} else {
		// OK, none interrupts.
		goto check_ok;
	}
check_firq:
	if ((int_state & MC6809_FIRQ_BIT) != 0) {
		int_state &= ~MC6809_SYNC_IN; // Moved to before checking MASK.Thanks to Ryu Takegami.
		if ((CC & CC_IF) != 0)
			goto check_irq;
		write_signals(&outputs_bus_bs, 0x00000000);
		write_signals(&outputs_bus_ba, 0x00000000);
		if((int_state & MC6809_CWAI_IN) == 0) {
			CC = CC & (uint8_t)(~CC_E);
			cycle += 5;
			cpu_firq_push();
		}
		write_signals(&outputs_bus_bs, 0xffffffff);
		CC = CC | CC_II | CC_IF;	// 0x50
		cycle += 2;
		cpu_firq_fetch_vector_address();
		cycle += 3;
		write_signals(&outputs_bus_bs, 0x00000000);
		int_state &= ~(MC6809_SYNC_IN | MC6809_SYNC_OUT);	// $FF1E
		goto int_cycle;

	}
check_irq:
	if ((int_state & MC6809_IRQ_BIT) != 0) {
		int_state &= ~MC6809_SYNC_IN; // Moved to before checking MASK.Thanks to Ryu Takegami.
		if ((CC & CC_II) != 0)
			goto check_ok;
		write_signals(&outputs_bus_bs, 0x00000000);
		write_signals(&outputs_bus_ba, 0x00000000);
		if((int_state & MC6809_CWAI_IN) == 0) {
			CC = CC | CC_E;
			cycle += 14;
   			cpu_irq_push();
		}
		write_signals(&outputs_bus_bs, 0xffffffff);
		cycle += 2;
		CC = CC | CC_II;	// 0x50
		cpu_irq_fetch_vector_address();
		cycle += 3;
		write_signals(&outputs_bus_bs, 0x00000000);
		int_state &= ~(MC6809_SYNC_IN | MC6809_SYNC_OUT);	// $FF1E
		goto int_cycle;
	}
	/*
	 * NO INTERRUPT
	 */
	goto check_ok;
	/*
	 * INTERRUPT
	 */
int_cycle:
	if((int_state & MC6809_CWAI_IN) != 0) {
		int_state &= ~MC6809_CWAI_IN;
	}
	if(clock >= 0) icount += clock;
	first_icount = icount;
	icount -= cycle;
	debugger_hook();
	icount -= extra_icount;
	passed_icount = first_icount - icount;
	extra_icount = 0;
	total_icount += (uint64_t)passed_icount;
	cpu_wait(passed_icount);
#if 1	
	if((icount <= 0) || (clock <= passed_icount)) return passed_icount;
	clock -= passed_icount;
#else
	return passed_icount;
#endif
	// goto check_ok;
	// run cpu
check_ok:
	if((int_state & MC6809_SYNC_IN) != 0) {
		int tmp_passed_icount = 0;
		first_icount = icount;
		if(clock < 1) clock = 1;
		icount -= extra_icount;
		icount -= clock;
		extra_icount = 0;
		debugger_hook();
		tmp_passed_icount = first_icount - icount;
		total_icount += (uint64_t)passed_icount;
		cpu_wait(tmp_passed_icount);
		return passed_icount + tmp_passed_icount;
	}
	if((int_state & MC6809_CWAI_IN) == 0) {
		if(clock <= -1) {
			// run only one opcode
			int tmp_passed_icount = 0;
			first_icount = icount;
			insns_count++;
			run_one_opecode();
			tmp_passed_icount = first_icount - icount;
			cpu_wait(tmp_passed_icount);
			return passed_icount + tmp_passed_icount;;
		} else {
			// run cpu while given clocks
			int tmp_passed_icount = 0;
			icount += clock;
			first_icount = icount;
			while((icount > 0) && (!(req_halt_on) && !(req_halt_off)) && (!busreq)) {
				insns_count++;
				run_one_opecode();
			}
			tmp_passed_icount = first_icount - icount;
			cpu_wait(tmp_passed_icount);
			return tmp_passed_icount + passed_icount;
		}
	} else { // CWAI_IN
		int tmp_passed_icount = 0;
		first_icount = icount;
		if(clock < 1) clock = 1;
		icount -= extra_icount;
		icount -= clock;
		extra_icount = 0;
		debugger_hook();
		tmp_passed_icount = first_icount - icount;
		total_icount += tmp_passed_icount;
		cpu_wait(tmp_passed_icount);
		return passed_icount + tmp_passed_icount;
	}

}

void MC6809_BASE::debugger_hook()
{
	
}

void MC6809_BASE::run_one_opecode()
{
	pPPC = pPC;
	uint8_t ireg = ROP(PCD);
	PC++;
	icount -= cycles1[ireg];
	icount -= extra_icount;
	extra_icount = 0;
	op(ireg);
}

void MC6809_BASE::op(uint8_t ireg)
{
	if(ireg == 0x0f) { // clr_di()
		write_signals(&outputs_bus_clr, 0x00000001);
		clr_used = true;
	} else if((ireg == 0x6f) || (ireg == 0x7f)){ //clr_ex() clr_ix()
		write_signals(&outputs_bus_clr, 0x00000002);
		clr_used = true;
	} else {
		if(clr_used) write_signals(&outputs_bus_clr, 0x00000000);
		clr_used = false;
	}
//#endif
	//printf("CPU(%08x) PC=%04x OP=%02x %02x %02x %02x %02x\n", (void *)this, PC, ireg, RM(PC), RM(PC + 1), RM(PC + 2), RM(PC + 3));

	(this->*m6809_main[ireg])();
}


void MC6809_BASE::write_debug_data8(uint32_t addr, uint32_t data)
{
	if(__USE_DEBUGGER) d_mem_stored->write_data8(addr, data);
}

uint32_t MC6809_BASE::read_debug_data8(uint32_t addr)
{
	if(__USE_DEBUGGER) {
		return d_mem_stored->read_data8(addr);
	}
	return 0xff;
}

void MC6809_BASE::write_debug_io8(uint32_t addr, uint32_t data)
{
	if(__USE_DEBUGGER) d_mem_stored->write_io8(addr, data);
}

uint32_t MC6809_BASE::read_debug_io8(uint32_t addr)
{
	if(__USE_DEBUGGER) {
		uint8_t val = d_mem_stored->read_io8(addr);
		return val;
	}
	return 0xff;
}


bool MC6809_BASE::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
//#ifdef USE_DEBUGGER
	if(_tcsicmp(reg, _T("PC")) == 0) {
		PC = data;
	} else if(_tcsicmp(reg, _T("DP")) == 0) {
		DP = data;
	} else if(_tcsicmp(reg, _T("A")) == 0) {
		A = data;
	} else if(_tcsicmp(reg, _T("B")) == 0) {
		B = data;
	} else if(_tcsicmp(reg, _T("D")) == 0) {
		D = data;
	} else if(_tcsicmp(reg, _T("U")) == 0) {
		U = data;
	} else if(_tcsicmp(reg, _T("X")) == 0) {
		X = data;
	} else if(_tcsicmp(reg, _T("Y")) == 0) {
		Y = data;
	} else if(_tcsicmp(reg, _T("S")) == 0) {
		S = data;
	} else if(_tcsicmp(reg, _T("CC")) == 0) {
		CC = data;
	} else {
		return false;
	}
//#endif
	return true;
}

/*
PC = 0000 PPC = 0000
INTR=[ IRQ FIRQ  NMI HALT][CI CO SI SO TRAP] CC =[EFHINZVC]
A = 00 B = 00 DP = 00 X = 0000 Y = 0000 U = 0000 S = 0000 EA = 0000
Clocks = 0 (0)  Since Scanline = 0/0 (0/0)
*/
bool MC6809_BASE::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
//#ifdef USE_DEBUGGER
	my_stprintf_s(buffer, buffer_len,
		 _T("PC = %04x PPC = %04x\n")
		 _T("INTR = [%s %s %s %s][%s %s %s %s %s] CC = [%c%c%c%c%c%c%c%c]\n")
		 _T("A = %02x B = %02x DP = %02x X = %04x Y = %04x U = %04x S = %04x EA = %04x\n")
		 _T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
		 PC,
		 PPC,
		 ((int_state & MC6809_IRQ_BIT) == 0)   ? _T("----") : _T(" IRQ"),
		 ((int_state & MC6809_FIRQ_BIT) == 0)  ? _T("----") : _T("FIRQ"),
		 ((int_state & MC6809_NMI_BIT) == 0)   ? _T("----") : _T(" NMI"),
		 ((int_state & MC6809_HALT_BIT) == 0)  ? _T("----") : _T("HALT"),
		 ((int_state & MC6809_CWAI_IN) == 0)   ? _T("--") : _T("CI"),
		 ((int_state & MC6809_CWAI_OUT) == 0)  ? _T("--") : _T("CO"),
		 ((int_state & MC6809_SYNC_IN) == 0)   ? _T("--") : _T("SI"),
		 ((int_state & MC6809_SYNC_OUT) == 0)  ? _T("--") : _T("SO"),
		 ((int_state & MC6809_INSN_HALT) == 0) ? _T("----") : _T("TRAP"),
		 ((CC & CC_E) == 0)  ? _T('-') : _T('E'), 
		 ((CC & CC_IF) == 0) ? _T('-') : _T('F'), 
		 ((CC & CC_H) == 0)  ? _T('-') : _T('H'), 
		 ((CC & CC_II) == 0) ? _T('-') : _T('I'), 
		 ((CC & CC_N) == 0)  ? _T('-') : _T('N'), 
		 ((CC & CC_Z) == 0)  ? _T('-') : _T('Z'), 
		 ((CC & CC_V) == 0)  ? _T('-') : _T('V'), 
		 ((CC & CC_C) == 0)  ? _T('-') : _T('C'),
		 A, B, DP,
		 X, Y, U, S,
		 EAD,
		  total_icount, total_icount - prev_total_icount,
		 get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame()
		);
	prev_total_icount = total_icount;
//#endif
	return true;
}  

uint32_t MC6809_BASE::cpu_disassemble_m6809(_TCHAR *buffer, uint32_t pc, const uint8_t *oprom, const uint8_t *opram)
{
	return 0;
}

int MC6809_BASE::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	return 0;
}



inline void MC6809_BASE::fetch_effective_address()
{
	uint8_t postbyte;
	uint8_t upper, lower;

	IMMBYTE(postbyte);
	
	upper = (postbyte >> 4) & 0x0f;
	lower = postbyte & 0x0f;
	switch (upper) {
		case 0x00:
			EA = X + lower;
			break;
		case 0x01:
			EA = X - 16 + lower;
			break;
		case 0x02:
			EA = Y + lower;
			break;
		case 0x03:
			EA = Y - 16 + lower;
			break;
		case 0x04:
			EA = U + lower;
			break;
		case 0x05:
			EA = U - 16 + lower;
			break;
		case 0x06:
			EA = S + lower;
			break;
		case 0x07:
			EA = S - 16 + lower;
			break;
		default:
			fetch_effective_address_IDX(upper, lower);
			break;
	}
	EAD &= 0xffff;
	icount -= index_cycle_em[postbyte];
}

inline void MC6809_BASE::fetch_effective_address_IDX(uint8_t upper, uint8_t lower)
{
	bool indirect = false;
	uint16_t *reg;
	uint8_t bx_p;
	pair32_t pp;
	
	indirect = ((upper & 0x01) != 0) ? true : false;

	switch ((upper >> 1) & 0x03) {	// $8-$f >> 1 = $4 - $7 : delete bit2 
		case 0:	// $8x,$9x
			reg = &X;
			break;
		case 1:	// $ax,$bx
			reg = &Y;
			break;
		case 2:	// $cx,$dx
			reg = &U;
			break;
		case 3:	// $ex,$fx
			reg = &S;
			break;
	}

	switch (lower) {
		case 0:	// ,r+ 
			EA = *reg;
			*reg = *reg + 1;
			*reg = *reg & 0xffff;
			break;
		case 1:	// ,r++
			EA = *reg;
			*reg = *reg + 2;
			*reg = *reg & 0xffff;
			break;
		case 2:	// ,-r
			*reg = *reg - 1;
			*reg = *reg & 0xffff;
			EA = *reg;
			break;
		case 3:	// ,--r
			*reg = *reg - 2;
			*reg = *reg & 0xffff;
			EA = *reg;
			break;
		case 4:	// ,r
			EA = *reg;
			break;
		case 5:	// b,r
			EA = *reg + SIGNED(B);
			break;
		case 6:	// a,r
		case 7:
			EA = *reg + SIGNED(A);
			break;
		case 8:	// $xx,r
			IMMBYTE(bx_p);
			EA = *reg + SIGNED(bx_p);
			break;
		case 9:	// $xxxx, r
			IMMWORD(EAP);
			EA = EA + *reg;
			break;
		case 0x0a:	// Undocumented
			EA = PC;
			EA++;
			EAP.w.l |= 0x00ff;
			break;
		case 0x0b:	// D,r
			EA = *reg + D;
			break;
		case 0x0c:	// xx,pc
			IMMBYTE(bx_p);
			EA = PC + SIGNED(bx_p);
			break;
		case 0x0d:	// xxxx,pc
			IMMWORD(EAP);
			EA = EA + PC;
			break;
		case 0x0e:	// Undocumented
			EA = 0xffff;
			break;
		case 0x0f:
			IMMWORD(EAP);
			break;
	}
	EAP.w.h = 0x0000;
	// $9x,$bx,$dx,$fx = INDIRECT
	if (indirect) {
		pp = EAP;
		EAP = RM16_PAIR(pp.d);
	}
}

#define IIError() illegal()

OP_HANDLER(illegal)
{
	//logerror("M6809: illegal opcode at %04x\n",PC);
	//printf("M6809: illegal opcode at %04x %02x %02x %02x %02x %02x \n",
	//	 PC - 2, RM(PC - 2), RM(PC - 1), RM(PC), RM(PC + 1), RM(PC + 2));
//        PC-=1;
}

inline uint8_t MC6809_BASE::GET_INDEXED_DATA(void)
{
	uint8_t t;
	fetch_effective_address();
	t = RM(EAD);
	return t;
}

inline pair32_t MC6809_BASE::GET_INDEXED_DATA16(void)
{
	pair32_t t;
	fetch_effective_address();
	t = RM16_PAIR(EAD);
	return t;
}

// $x0, $x1
inline void MC6809_BASE::NEG_MEM(uint8_t a_neg)
{							
	uint16_t r_neg;					
	r_neg = 0 - (uint16_t)a_neg;
	CLR_NZVC;
	SET_FLAGS8(0, a_neg, r_neg);
	WM(EAD, r_neg);					
}

inline uint8_t MC6809_BASE::NEG_REG(uint8_t a_neg)
{
	uint16_t r_neg;
	r_neg = 0 - (uint16_t)a_neg;
	CLR_NZVC;
	SET_FLAGS8(0, a_neg, r_neg);
	return (uint8_t)r_neg;
}


// $x2
inline void MC6809_BASE::COM_MEM(uint8_t a_com)
{			 
	uint8_t t_com;		 
	t_com = ~a_com;	 
	CLR_NZVC;		 
	SET_NZ8(t_com);	 
	SEC;
	WM(EAD, t_com);
}

inline uint8_t MC6809_BASE::COM_REG(uint8_t r_com)
{
	r_com = ~r_com;
	CLR_NZVC;
	SET_NZ8(r_com);
	SEC;
	return r_com;
}

inline void MC6809_BASE::LSR_MEM(uint8_t t)
{
	CLR_NZC;
	CC = CC | (t & CC_C);
	t >>= 1;
	SET_NZ8(t);
	WM(EAD, t);
}

inline uint8_t MC6809_BASE::LSR_REG(uint8_t r)
{
	CLR_NZC;
	CC |= (r & CC_C);
	r >>= 1;
	SET_NZ8(r);
	return r;
}

inline void MC6809_BASE::ROR_MEM(uint8_t t)
{
	uint8_t r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	t >>= 1;
	r |= t;
	SET_NZ8(r); //NZ8?
	WM(EAD, r);
}

inline uint8_t MC6809_BASE::ROR_REG(uint8_t t)
{
	uint8_t r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	t >>= 1;
	r |= t;
	SET_NZ8(r); //NZ8?
	return r;
}


inline void MC6809_BASE::ASR_MEM(uint8_t t)
{
	uint8_t r;
	CLR_NZC;
	CC = CC | (t & CC_C);
	r = (t & 0x80) | (t >> 1);
	// H is undefined
	SET_NZ8(r);
	//SET_H(t, t, r);
	WM(EAD, r);
}

inline uint8_t MC6809_BASE::ASR_REG(uint8_t t)
{
	uint8_t r;
	CLR_NZC;
	CC = CC | (t & CC_C);
	r = (t & 0x80) | (t >> 1);
	// H is undefined
	SET_NZ8(r);
	//SET_H(t, t, r);
	return r;
}

inline void MC6809_BASE::ASL_MEM(uint8_t t)
{
	uint16_t r, tt;
	tt = (uint16_t)t & 0x00ff;
	r = tt << 1;
	CLR_NZVC;
	SET_FLAGS8(tt, tt, r);
	//SET_H(tt, tt, r);
	WM(EAD, (uint8_t)r);
}

inline uint8_t MC6809_BASE::ASL_REG(uint8_t t)
{
	uint16_t r, tt;
	tt = (uint16_t)t & 0x00ff;
	r = tt << 1;
	CLR_NZVC;
	SET_FLAGS8(tt, tt, r);
	//SET_H(tt, tt, r);
	return (uint8_t)r;
}

inline void MC6809_BASE::ROL_MEM(uint8_t t)
{
	uint16_t r, tt;
	tt = (uint16_t)t & 0x00ff;
	r = (CC & CC_C) | (tt << 1);
	CLR_NZVC;
	//SET_NZ8(r);
	//if(t & 0x80) {
	//	SEC;
	//	if((r & 0x80) == 0)SEV;
	//} else {
	//	if((r & 0x80) != 0) SEV;
	//}	  
	SET_FLAGS8(tt, tt, r);
	WM(EAD, (uint8_t)r);
}

inline uint8_t MC6809_BASE::ROL_REG(uint8_t t)
{
	uint16_t r, tt;
	tt = (uint16_t)t & 0x00ff;
	r = (CC & CC_C) | (tt << 1);
	CLR_NZVC;
	//SET_NZ8(r);
	//if(t & 0x80) {
	//	SEC;
	//	if((r & 0x80) == 0) SEV;
	//} else {
	//	if((r & 0x80) != 0) SEV;
	//}	  
	SET_FLAGS8(tt, tt, r);
	return (uint8_t)r;
}

inline void MC6809_BASE::DEC_MEM(uint8_t t)
{
	uint16_t tt;
	tt = t - 1;
	CLR_NZV;
	SET_FLAGS8D(tt);
	WM(EAD, tt);
}

inline uint8_t MC6809_BASE::DEC_REG(uint8_t t)
{
	uint8_t tt;
	tt = t - 1;
	CLR_NZV;
	SET_FLAGS8D(tt);
	return tt;
}

inline void MC6809_BASE::DCC_MEM(uint8_t t)
{
	uint16_t tt, ss;
	tt = t - 1;
	CLR_NZVC;
	SET_FLAGS8D(tt);
	ss = CC;
	ss >>= 2;
	ss = ~ss;
	ss = ss & CC_C;
	CC = ss | CC;
	WM(EAD, tt);
}

inline uint8_t MC6809_BASE::DCC_REG(uint8_t t)
{
	uint16_t tt, ss;
	tt = t - 1;
	CLR_NZVC;
	SET_FLAGS8D(tt);
	ss = CC;
	ss >>= 2;
	ss = ~ss;
	ss = ss & CC_C;
	CC = ss | CC;
	return (uint8_t)tt;
}

inline void MC6809_BASE::INC_MEM(uint8_t t)
{
	uint16_t tt = t + 1;
	CLR_NZV;
	SET_FLAGS8I(tt);
	WM(EAD, tt);
}

inline uint8_t MC6809_BASE::INC_REG(uint8_t t)
{
	uint16_t tt = t + 1;
	CLR_NZV;
	SET_FLAGS8I(tt);
	return (uint8_t)tt;
}

inline void MC6809_BASE::TST_MEM(uint8_t t)
{
	CLR_NZV;
	SET_NZ8(t);
}

inline uint8_t MC6809_BASE::TST_REG(uint8_t t)
{
	CLR_NZV;
	SET_NZ8(t);
	return t;
}

inline uint8_t MC6809_BASE::CLC_REG(uint8_t t)
{
	uint8_t r;
	r = 0;
	CLR_NZV;
	SEZ;
	return r;
}
  

inline void MC6809_BASE::CLR_MEM(uint8_t t)
{
	WM(EAD, 0);
	CLR_NZVC;
	SEZ;
}

inline uint8_t MC6809_BASE::CLR_REG(uint8_t t)
{
	CLR_NZVC;
	SEZ;
	return 0;
}

inline uint8_t MC6809_BASE::SUB8_REG(uint8_t reg, uint8_t data)
{
	uint16_t r;
	r = (uint16_t)reg - (uint16_t)data;
	CLR_HNZVC;
	// H is undefined
	SET_FLAGS8(reg, data, r);
	return (uint8_t)r;
}

inline uint8_t MC6809_BASE::CMP8_REG(uint8_t reg, uint8_t data)
{
	uint16_t r;
	r = (uint16_t)reg - (uint16_t)data;
	CLR_NZVC;
	// H is undefined
	SET_FLAGS8(reg, data, r);
	return reg;
}

inline uint8_t MC6809_BASE::SBC8_REG(uint8_t reg, uint8_t data)
{
	uint16_t r;
	uint8_t cc_c = CC & CC_C;
	r = (uint16_t)reg - (uint16_t)data - (uint16_t)cc_c;
	CLR_HNZVC;
	SET_FLAGS8(reg, (data + cc_c) , r);
	return (uint8_t)r;
}

inline uint8_t MC6809_BASE::AND8_REG(uint8_t reg, uint8_t data)
{
	uint8_t r = reg;
	r &= data;
	CLR_NZV;
	SET_NZ8(r);
	return r;
}

inline uint8_t MC6809_BASE::BIT8_REG(uint8_t reg, uint8_t data)
{
	uint16_t r;
	r = reg & data;
	CLR_NZV;
	SET_NZ8(r);
	SET_V8(reg, data, r);
	return reg;
}

inline uint8_t MC6809_BASE::EOR8_REG(uint8_t reg, uint8_t data)
{
	uint8_t r = reg;
	r ^= data;
	CLR_NZV;
	SET_NZ8(r);
	return r;
}

inline uint8_t MC6809_BASE::OR8_REG(uint8_t reg, uint8_t data)
{
	uint8_t r = reg;
	r |= data;
	CLR_NZV;
	SET_NZ8(r);
	return r;
}

inline uint8_t MC6809_BASE::ADD8_REG(uint8_t reg, uint8_t data)
{
	uint16_t t, r;
	t = (uint16_t) data;
	t &= 0x00ff;
	r = reg + t;
	CLR_HNZVC;
	SET_HNZVC8(reg, t, r);
	return (uint8_t)r;
}

inline uint8_t MC6809_BASE::ADC8_REG(uint8_t reg, uint8_t data)
{
	uint16_t t, r;
	uint8_t c_cc = CC & CC_C;
	t = (uint16_t) data;
	t &= 0x00ff;
	r = reg + t + c_cc;
	CLR_HNZVC;
	SET_HNZVC8(reg, (t + c_cc), r);
	return (uint8_t)r;
}	

inline uint8_t MC6809_BASE::LOAD8_REG(uint8_t reg)
{
	CLR_NZV;
	SET_NZ8(reg);
	return reg;
}

inline void MC6809_BASE::STORE8_REG(uint8_t reg)
{
	CLR_NZV;
	SET_NZ8(reg);
	WM(EAD, reg);
}

inline uint16_t MC6809_BASE::LOAD16_REG(uint16_t reg)
{
	CLR_NZV;
	SET_NZ16(reg);
	return reg;
}
  

inline uint16_t MC6809_BASE::SUB16_REG(uint16_t reg, uint16_t data)
{
	uint32_t r, d;
	d = reg;
	r = d - data;
	CLR_NZVC;
	SET_FLAGS16(d, data, r);
	return (uint16_t)r;
}

inline uint16_t MC6809_BASE::ADD16_REG(uint16_t reg, uint16_t data)
{
	uint32_t r, d;
	d = reg;
	r = d + (uint32_t)data;
	CLR_HNZVC;
	SET_HNZVC16(d, data, r);
	return (uint16_t)r;
}

inline uint16_t MC6809_BASE::CMP16_REG(uint16_t reg, uint16_t data)
{
	uint32_t r, d;
	d = reg;
	r = d - data;
	CLR_NZVC;
	SET_FLAGS16(d, data, r);
	return reg;
}

inline void MC6809_BASE::STORE16_REG(pair32_t *p)
{
	CLR_NZV;
	SET_NZ16(p->w.l);
	WM16(EAD, p);
}


/* $00 NEG direct ?**** */
OP_HANDLER(neg_di) {
	uint8_t t;
	DIRBYTE(t);
	NEG_MEM(t);
}

/* $01 Undefined Neg */
/* $03 COM direct -**01 */
OP_HANDLER(com_di) {
	uint8_t t;
	DIRBYTE(t);
	COM_MEM(t);
}

/* $02 NGC Direct (Undefined) */
OP_HANDLER(ngc_di) {
	if ((CC & CC_C) == 0) {
		neg_di();
	}
	else {
		com_di();
	}
}


/* $04 LSR direct -0*-* */
OP_HANDLER(lsr_di) {
	uint8_t t;
	DIRBYTE(t);
	LSR_MEM(t);
}

/* $05 ILLEGAL */

/* $06 ROR direct -**-* */
OP_HANDLER(ror_di) {
	uint8_t t;
	DIRBYTE(t);
	ROR_MEM(t);
}

/* $07 ASR direct ?**-* */
OP_HANDLER(asr_di) {
	uint8_t t;
	DIRBYTE(t);
	ASR_MEM(t);
}

/* $08 ASL direct ?**** */
OP_HANDLER(asl_di) {
	uint8_t t;
	DIRBYTE(t);
	ASL_MEM(t);
}

/* $09 ROL direct -**** */
OP_HANDLER(rol_di) {
	uint8_t t;
	DIRBYTE(t);
	ROL_MEM(t);
}

/* $0A DEC direct -***- */
OP_HANDLER(dec_di) {
	uint8_t t;
	DIRBYTE(t);
	DEC_MEM(t);
}

/* $0B DCC direct */
OP_HANDLER(dcc_di) {
	uint8_t t;
	DIRBYTE(t);
	DCC_MEM(t);
}


/* $OC INC direct -***- */
OP_HANDLER(inc_di) {
	uint8_t t;
	DIRBYTE(t);
	INC_MEM(t);
}

/* $OD TST direct -**0- */
OP_HANDLER(tst_di) {
	uint8_t t;
	DIRBYTE(t);
	t = RM(EAD);
	TST_MEM(t);
}

/* $0E JMP direct ----- */
OP_HANDLER(jmp_di) {
	DIRECT;
	PC = EA;
}

/* $0F CLR direct -0100 */
OP_HANDLER(clr_di) {
	uint8_t dummy;
	DIRECT;
	dummy = RM(EAD);	// Dummy Read(Alpha etc...)
	CLR_MEM(dummy);
}

/* $10 FLAG */

/* $11 FLAG */

/* $12 NOP inherent ----- */
OP_HANDLER(nop) {
	;
}

/* $13 SYNC inherent ----- */
OP_HANDLER(sync_09)	// Rename 20101110
{
	int_state |= MC6809_SYNC_IN;
	write_signals(&outputs_bus_ba, 0xffffffff);
	write_signals(&outputs_bus_bs, 0x00000000);
}



/* $14 trap(HALT) */
OP_HANDLER(trap) {
	int_state |= MC6809_INSN_HALT;	// HALTフラグ
	// Debug: トラチE�E要因
	this->out_debug_log(_T("TRAP(HALT) @%04x %02x %02x\n"), PC - 1, RM((PC - 1)), RM(PC));
}

/* $15 trap */

/* $16 LBRA relative ----- */
OP_HANDLER(lbra) {
	LBRANCH(true);
}

/* $17 LBSR relative ----- */
OP_HANDLER(lbsr) {
	IMMWORD(EAP);
	PUSHWORD(pPC);
	PC += EAD;
}

/* $18 ASLCC */

OP_HANDLER(aslcc_in) {
	uint8_t cc_r = CC;
	if ((cc_r & CC_Z) != 0x00) { //20100824 Fix
		cc_r |= CC_C;
	}
	cc_r <<= 1;
	cc_r &= 0x3e;
	CC = cc_r;
}

/* $19 DAA inherent (A) -**0* */
OP_HANDLER(daa) {
	uint8_t msn, lsn;
	uint16_t t, cf = 0;
	msn = A & 0xf0;
	lsn = A & 0x0f;
	if (lsn > 0x09 || CC & CC_H)
		cf |= 0x06;
	if (msn > 0x80 && lsn > 0x09)
		cf |= 0x60;
	if (msn > 0x90 || CC & CC_C)
		cf |= 0x60;
	t = cf + A;
	CLR_NZV;	/* keep carry from previous operation */
	SET_NZ8((uint8_t) t);
	SET_C8(t);
	A = (uint8_t)t;
}


/* $1A ORCC immediate ##### */
OP_HANDLER(orcc) {
	uint8_t t;
	IMMBYTE(t);
	CC |= t;
}

/* $1B ILLEGAL */


/* $1C ANDCC immediate ##### */
OP_HANDLER(andcc) {
	uint8_t t;
	IMMBYTE(t);
	CC &= t;
//  check_irq_lines(); /* HJB 990116 */
}

/* $1D SEX inherent -**-- */
OP_HANDLER(sex) {
	uint16_t t;
	t = SIGNED(B);
	D = t; // Endian OK?
	//  CLR_NZV;    Tim Lindner 20020905: verified that V flag is not affected
	CLR_NZ;
	SET_NZ16(t);
}

	/* $1E EXG inherent ----- */// 20100825
OP_HANDLER(exg) {
	pair32_t t1, t2;
	uint8_t tb;
	IMMBYTE(tb);
	t1.d = 0;
	t2.d = 0;
	/*
	 * 20111011: 16bit vs 16Bitの演算にする(XM7/ cpu_x86.asmより
	 */
	{
		switch ((tb >> 4) & 15) {
			case 0:
				t1.w.l = D;
				break;
			case 1:
				t1.w.l = X;
				break;
			case 2:
				t1.w.l = Y;
				break;
			case 3:
				t1.w.l = U;
				break;
			case 4:
				t1.w.l = S;
				break;
			case 5:
				t1.w.l = PC;
				break;
			case 8:
				t1.b.l = A;
				t1.b.h = 0xff;
				break;
			case 9:
				t1.b.l = B;
				t1.b.h = 0xff;
				break;
			case 10:
				t1.b.l = CC;
				t1.b.h = 0xff;
				break;
			case 11:
				t1.b.l = DP;
				t1.b.h = 0xff;
				break;
			default:
				t1.w.l = 0xffff;
				break;
		}
		switch (tb & 15) {
			case 0:
				t2.w.l = D;
				break;
			case 1:
				t2.w.l = X;
				break;
			case 2:
				t2.w.l = Y;
				break;
			case 3:
				t2.w.l = U;
				break;
			case 4:
				t2.w.l = S;
				break;
			case 5:
				t2.w.l = PC;
				break;
			case 8:
				t2.b.l = A;
				t2.b.h = 0xff;
				break;
			case 9:
				t2.b.l = B;
				t2.b.h = 0xff;
				break;
			case 10:
				t2.b.l = CC;
				t2.b.h = 0xff;
				break;
			case 11:
				t2.b.l = DP;
				t2.b.h = 0xff;
				break;
			default:
				t2.w.l = 0xffff;
				break;
		}
	}
	switch ((tb >> 4) & 15) {
		case 0:
			D = t2.w.l;
			break;
		case 1:
			X = t2.w.l;
			break;
		case 2:
			Y = t2.w.l;
			break;
		case 3:
			U = t2.w.l;
			break;
		case 4:
			S = t2.w.l;
			int_state |= MC6809_LDS;
			break;
		case 5:
			PC = t2.w.l;
			break;
		case 8:
			A = t2.b.l;
			break;
		case 9:
			B = t2.b.l;
			break;
		case 10:
			CC = t2.b.l;
			break;
		case 11:
			DP = t2.b.l;
			break;
	}
	switch (tb & 15) {
		case 0:
			D = t1.w.l;
			break;
		case 1:
			X = t1.w.l;
			break;
		case 2:
			Y = t1.w.l;
			break;
		case 3:
			U = t1.w.l;
			break;
		case 4:
			S = t1.w.l;
			int_state |= MC6809_LDS;
			break;
		case 5:
			PC = t1.w.l;
			break;
		case 8:
			A = t1.b.l;
			break;
		case 9:
			B = t1.b.l;
			break;
		case 10:
			CC = t1.b.l;
			break;
		case 11:
			DP = t1.b.l;
			break;
	}
}

/* $1F TFR inherent ----- */
OP_HANDLER(tfr) {
	uint8_t tb;
	pair32_t t;
	IMMBYTE(tb);
	t.d = 0;
	/*
	 * 20111011: 16bit vs 16Bitの演算にする(XM7/ cpu_x86.asmより)
	 */
	{
		switch ((tb >> 4) & 15) {
			case 0:
				t.w.l = D;
				break;
			case 1:
				t.w.l = X;
				break;
			case 2:
				t.w.l = Y;
				break;
			case 3:
				t.w.l = U;
				break;
			case 4:
				t.w.l = S;
				break;
			case 5:
				t.w.l = PC;
				break;
			case 8:
				t.b.l = A;
				t.b.h = 0xff;
				break;
			case 9:
				t.b.l = B;
				t.b.h = 0xff;
				break;
			case 10:
				t.b.l = CC;
				t.b.h = 0xff;
				break;
			case 11:
				t.b.l = DP;
				t.b.h = 0xff;
				break;
			default:
				t.w.l = 0xffff;
				break;
		}
	}
	switch (tb & 15) {
		case 0:
			D = t.w.l;
			break;
		case 1:
			X = t.w.l;
			break;
		case 2:
			Y = t.w.l;
			break;
		case 3:
			U = t.w.l;
			break;
		case 4:
			S = t.w.l;
			int_state |= MC6809_LDS;
			break;
		case 5:
			PC = t.w.l;
			break;
		case 8:
			A = t.b.l;
			break;
		case 9:
			B = t.b.l;
			break;
		case 10:
			CC = t.b.l;
			break;
		case 11:
			DP = t.b.l;
			break;
	}
}

/* $20 BRA relative ----- */
OP_HANDLER(bra) {
	BRANCH(true);
}

/* $21 BRN relative ----- */
OP_HANDLER(brn) {
	BRANCH(false);
}

/* $1021 LBRN relative ----- */
OP_HANDLER(lbrn) {
	LBRANCH(false);
}

/* $22 BHI relative ----- */
OP_HANDLER(bhi) {
	BRANCH(((CC & (CC_Z | CC_C)) == 0));
}

/* $1022 LBHI relative ----- */
OP_HANDLER(lbhi) {
	LBRANCH(((CC & (CC_Z | CC_C)) == 0));
}

/* $23 BLS relative ----- */
OP_HANDLER(bls) {
	BRANCH(((CC & (CC_Z | CC_C)) != 0));
}

/* $1023 LBLS relative ----- */
OP_HANDLER(lbls) {
	LBRANCH(((CC & (CC_Z | CC_C)) != 0));
	//LBRANCH((CC & (CC_Z | CC_C)));
}

/* $24 BCC relative ----- */
OP_HANDLER(bcc) {
	BRANCH((CC & CC_C) == 0);
}

/* $1024 LBCC relative ----- */
OP_HANDLER(lbcc) {
	LBRANCH((CC & CC_C) == 0);
}

/* $25 BCS relative ----- */
OP_HANDLER(bcs) {
	BRANCH((CC & CC_C) != 0);
}

/* $1025 LBCS relative ----- */
OP_HANDLER(lbcs) {
	LBRANCH((CC & CC_C) != 0);
}

/* $26 BNE relative ----- */
OP_HANDLER(bne) {
	BRANCH((CC & CC_Z) == 0);
}

/* $1026 LBNE relative ----- */
OP_HANDLER(lbne) {
	LBRANCH((CC & CC_Z) == 0);
}

/* $27 BEQ relative ----- */
OP_HANDLER(beq) {
	BRANCH((CC & CC_Z) != 0);
}

/* $1027 LBEQ relative ----- */
OP_HANDLER(lbeq) {
	LBRANCH((CC & CC_Z) != 0);
}

/* $28 BVC relative ----- */
OP_HANDLER(bvc) {
	BRANCH((CC & CC_V) == 0);
}

/* $1028 LBVC relative ----- */
OP_HANDLER(lbvc) {
	LBRANCH((CC & CC_V) == 0);
}

/* $29 BVS relative ----- */
OP_HANDLER(bvs) {
	BRANCH((CC & CC_V) != 0);
}

/* $1029 LBVS relative ----- */
OP_HANDLER(lbvs) {
	LBRANCH((CC & CC_V) != 0);
}

/* $2A BPL relative ----- */
OP_HANDLER(bpl) {
	BRANCH((CC & CC_N) == 0);
}

/* $102A LBPL relative ----- */
OP_HANDLER(lbpl) {
	LBRANCH((CC & CC_N) == 0);
}

/* $2B BMI relative ----- */
OP_HANDLER(bmi) {
	BRANCH((CC & CC_N) != 0);
}

/* $102B LBMI relative ----- */
OP_HANDLER(lbmi) {
	LBRANCH((CC & CC_N) != 0);
}

/* $2C BGE relative ----- */
OP_HANDLER(bge) {
	BRANCH(!NXORV);
}

/* $102C LBGE relative ----- */
OP_HANDLER(lbge) {
	LBRANCH(!NXORV);
}

/* $2D BLT relative ----- */
OP_HANDLER(blt) {
	BRANCH(NXORV);
}

/* $102D LBLT relative ----- */
OP_HANDLER(lblt) {
	LBRANCH(NXORV);
}

/* $2E BGT relative ----- */
OP_HANDLER(bgt) {
	BRANCH(!(NXORV || (CC & CC_Z)));
}

/* $102E LBGT relative ----- */
OP_HANDLER(lbgt) {
	LBRANCH(!(NXORV || (CC & CC_Z)));
}

/* $2F BLE relative ----- */
OP_HANDLER(ble) {
	BRANCH((NXORV || (CC & CC_Z)));
}

/* $102F LBLE relative ----- */
OP_HANDLER(lble) {
	LBRANCH((NXORV || (CC & CC_Z)));
}

/* $30 LEAX indexed --*-- */
OP_HANDLER(leax) {
	fetch_effective_address();
	X = EA;
	CLR_Z;
	SET_Z16(X);
}

/* $31 LEAY indexed --*-- */
OP_HANDLER(leay) {
	fetch_effective_address();
	Y = EA;
	CLR_Z;
	SET_Z16(Y);
}

/* $32 LEAS indexed ----- */
OP_HANDLER(leas) {
	fetch_effective_address();
	S = EA;
	int_state |= MC6809_LDS;
}

/* $33 LEAU indexed ----- */
OP_HANDLER(leau) {
	fetch_effective_address();
	U = EA;
}

/* $34 PSHS inherent ----- */
OP_HANDLER(pshs) {
		uint8_t t;
		IMMBYTE(t);
		//dmy = RM(S);	// Add 20100825
		RM(S);	// Add 20100825
		if (t & 0x80) {
			PUSHWORD(pPC);
			icount -= 2;
		}
		if (t & 0x40) {
			PUSHWORD(pU);
			icount -= 2;
		}
		if (t & 0x20) {
			PUSHWORD(pY);
			icount -= 2;
		}
		if (t & 0x10) {
			PUSHWORD(pX);
			icount -= 2;
		}
		if (t & 0x08) {
			PUSHBYTE(DP);
			icount -= 1;
		}
		if (t & 0x04) {
			PUSHBYTE(B);
			icount -= 1;
		}
		if (t & 0x02) {
			PUSHBYTE(A);
			icount -= 1;
		}
		if (t & 0x01) {
			PUSHBYTE(CC);
			icount -= 1;
		}
	}

/* 35 PULS inherent ----- */
OP_HANDLER(puls) {
		uint8_t t;
		IMMBYTE(t);
		if (t & 0x01) {
			PULLBYTE(CC);
			icount -= 1;
		}
		if (t & 0x02) {
			PULLBYTE(A);
			icount -= 1;
		}
		if (t & 0x04) {
			PULLBYTE(B);
			icount -= 1;
		}
		if (t & 0x08) {
			PULLBYTE(DP);
			icount -= 1;
		}
		if (t & 0x10) {
			PULLWORD(pX);
			icount -= 2;
		}
		if (t & 0x20) {
			PULLWORD(pY);
			icount -= 2;
		}
		if (t & 0x40) {
			PULLWORD(pU);
			icount -= 2;
		}
		if (t & 0x80) {
			PULLWORD(pPC);
			icount -= 2;
		}
		//dmy = RM(S);	// Add 20100825
		RM(S);	// Add 20100825
		/* HJB 990225: moved check after all PULLs */
//  if( t&0x01 ) { check_irq_lines(); }
	}

/* $36 PSHU inherent ----- */
OP_HANDLER(pshu) {
		uint8_t t;
		IMMBYTE(t);
		//dmy = RM(U);	// Add 20100825
		RM(U);	// Add 20100825
		if (t & 0x80) {
			PSHUWORD(pPC);
			icount -= 2;
		}
		if (t & 0x40) {
			PSHUWORD(pS);
			icount -= 2;
		}
		if (t & 0x20) {
			PSHUWORD(pY);
			icount -= 2;
		}
		if (t & 0x10) {
			PSHUWORD(pX);
			icount -= 2;
		}
		if (t & 0x08) {
			PSHUBYTE(DP);
			icount -= 1;
		}
		if (t & 0x04) {
			PSHUBYTE(B);
			icount -= 1;
		}
		if (t & 0x02) {
			PSHUBYTE(A);
			icount -= 1;
		}
		if (t & 0x01) {
			PSHUBYTE(CC);
			icount -= 1;
		}
	}

/* 37 PULU inherent ----- */
OP_HANDLER(pulu) {
		uint8_t t;
		IMMBYTE(t);
		if (t & 0x01) {
			PULUBYTE(CC);
			icount -= 1;
		}
		if (t & 0x02) {
			PULUBYTE(A);
			icount -= 1;
		}
		if (t & 0x04) {
			PULUBYTE(B);
			icount -= 1;
		}
		if (t & 0x08) {
			PULUBYTE(DP);
			icount -= 1;
		}
		if (t & 0x10) {
			PULUWORD(pX);
			icount -= 2;
		}
		if (t & 0x20) {
			PULUWORD(pY);
			icount -= 2;
		}
		if (t & 0x40) {
			PULUWORD(pS);
			icount -= 2;
		}
		if (t & 0x80) {
			PULUWORD(pPC);
			icount -= 2;
		}
		//dmy = RM(U);	// Add 20100825
		RM(U);	// Add 20100825
		/* HJB 990225: moved check after all PULLs */
		//if( t&0x01 ) { check_irq_lines(); }
}

/* $38 ILLEGAL */

/* $39 RTS inherent ----- */
OP_HANDLER(rts) {
	//printf("RTS: Before PC=%04x", pPC.w.l);
	PULLWORD(pPC);
	//printf(" After PC=%04x\n", pPC.w.l);
}

/* $3A ABX inherent ----- */
OP_HANDLER(abx) {
	pair32_t bt;
	bt.d = 0;
	bt.b.l = B;
	X = X + bt.w.l;
}

/* $3B RTI inherent ##### */
OP_HANDLER(rti) {
		PULLBYTE(CC);
//  t = CC & CC_E;    /* HJB 990225: entire state saved? */
	if ((CC & CC_E) != 0) {	// NMIIRQ
		icount -= 9;
		PULLBYTE(A);
		PULLBYTE(B);
		PULLBYTE(DP);
		PULLWORD(pX);
		PULLWORD(pY);
		PULLWORD(pU);
	}
	PULLWORD(pPC);
//  check_irq_lines(); /* HJB 990116 */
}

/* $3C CWAI inherent ----1 */
OP_HANDLER(cwai) {
	uint8_t t;
	IMMBYTE(t);
	CC = CC & t;
	CC |= CC_E;	/* HJB 990225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);

	int_state = int_state | MC6809_CWAI_IN;
	int_state &= ~MC6809_CWAI_OUT;	// 0xfeff
	return;
}

/* $3D MUL inherent --*-@ */
OP_HANDLER(mul) {
	pair32_t t, r;
	t.d = 0;
	r.d = 0;
	t.b.l = A;
	r.b.l = B;
	t.d = t.d * r.d;
	CLR_ZC;
	SET_Z16(t.w.l);
	if (t.b.l & 0x80) SEC;
	A = t.b.h;
	B = t.b.l;
}

/* $3E RST */
OP_HANDLER(rst) {
	this->reset();
}


/* $3F SWI (SWI2 SWI3) absolute indirect ----- */
OP_HANDLER(swi) {
		CC |= CC_E;	/* HJB 980225: save entire state */
		PUSHWORD(pPC);
		PUSHWORD(pU);
		PUSHWORD(pY);
		PUSHWORD(pX);
		PUSHBYTE(DP);
		PUSHBYTE(B);
		PUSHBYTE(A);
		PUSHBYTE(CC);
		CC |= CC_IF | CC_II;	/* inhibit FIRQ and IRQ */
		pPC = RM16_PAIR(0xfffa);
}

/* $103F SWI2 absolute indirect ----- */
OP_HANDLER(swi2) {
		CC |= CC_E;	/* HJB 980225: save entire state */
		PUSHWORD(pPC);
		PUSHWORD(pU);
		PUSHWORD(pY);
		PUSHWORD(pX);
		PUSHBYTE(DP);
		PUSHBYTE(B);
		PUSHBYTE(A);
		PUSHBYTE(CC);
		pPC = RM16_PAIR(0xfff4);
}

/* $113F SWI3 absolute indirect ----- */
OP_HANDLER(swi3) {
		CC |= CC_E;	/* HJB 980225: save entire state */
		PUSHWORD(pPC);
		PUSHWORD(pU);
		PUSHWORD(pY);
		PUSHWORD(pX);
		PUSHBYTE(DP);
		PUSHBYTE(B);
		PUSHBYTE(A);
		PUSHBYTE(CC);
		pPC = RM16_PAIR(0xfff2);
}

/* $40 NEGA inherent ?**** */
OP_HANDLER(nega) {
	A = NEG_REG(A);
}

/* $41 NEGA */


/* $43 COMA inherent -**01 */
OP_HANDLER(coma) {
	A = COM_REG(A);
}

/* $42 NGCA */
OP_HANDLER(ngca) {
	if ((CC & CC_C) == 0) {
		nega();
	} else {
		coma();
	}
}

/* $44 LSRA inherent -0*-* */
OP_HANDLER(lsra) {
	A = LSR_REG(A);
}

/* $45 LSRA */

/* $46 RORA inherent -**-* */
OP_HANDLER(rora) {
	A = ROR_REG(A);
}

/* $47 ASRA inherent ?**-* */
OP_HANDLER(asra) {
	A = ASR_REG(A);
}

/* $48 ASLA inherent ?**** */
OP_HANDLER(asla) {
	A = ASL_REG(A);
}

/* $49 ROLA inherent -**** */
OP_HANDLER(rola) {
	A = ROL_REG(A);
}

/* $4A DECA inherent -***- */
OP_HANDLER(deca) {
	A = DEC_REG(A);
}


/* $4B DCCA */
OP_HANDLER(dcca) {
	A = DCC_REG(A);
}

/* $4C INCA inherent -***- */
OP_HANDLER(inca) {
	A = INC_REG(A);
}

/* $4D TSTA inherent -**0- */
OP_HANDLER(tsta) {
	A = TST_REG(A);
}

/* $4E ILLEGAL */
OP_HANDLER(clca) {
	A = CLC_REG(A);
}

/* $4F CLRA inherent -0100 */
OP_HANDLER(clra) {
	A = CLR_REG(A);
}

/* $50 NEGB inherent ?**** */
OP_HANDLER(negb) {
	B = NEG_REG(B);
}

/* $51 NEGB */

/* $52 NGCB */

/* $53 COMB inherent -**01 */
OP_HANDLER(comb) {
	B = COM_REG(B);
}

/* $52 NGCB */
OP_HANDLER(ngcb) {
	if ((CC & CC_C) == 0) {
		negb();
	} else {
		comb();
	}
}

/* $54 LSRB inherent -0*-* */
OP_HANDLER(lsrb) {
	B = LSR_REG(B);
}

/* $55 LSRB */

/* $56 RORB inherent -**-* */
OP_HANDLER(rorb) {
	B = ROR_REG(B);
}

/* $57 ASRB inherent ?**-* */
OP_HANDLER(asrb) {
	B = ASR_REG(B);
}

/* $58 ASLB inherent ?**** */
OP_HANDLER(aslb) {
	B = ASL_REG(B);
}

/* $59 ROLB inherent -**** */
OP_HANDLER(rolb) {
	B = ROL_REG(B);
}

/* $5A DECB inherent -***- */
OP_HANDLER(decb) {
	B = DEC_REG(B);
}

/* $5B DCCB */
OP_HANDLER(dccb) {
	B = DCC_REG(B);
}

/* $5C INCB inherent -***- */
OP_HANDLER(incb) {
	B = INC_REG(B);
}

/* $5D TSTB inherent -**0- */
OP_HANDLER(tstb) {
	B = TST_REG(B);
}

/* $5E ILLEGAL */
OP_HANDLER(clcb) {
	B = CLC_REG(B);
}

/* $5F CLRB inherent -0100 */
OP_HANDLER(clrb) {
	B = CLR_REG(B);
}

/* $60 NEG indexed ?**** */
OP_HANDLER(neg_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	NEG_MEM(t);
}

/* $61 ILLEGAL */


/* $63 COM indexed -**01 */
OP_HANDLER(com_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	COM_MEM(t);
}

/* $62 ILLEGAL */
OP_HANDLER(ngc_ix) {
	if ((CC & CC_C) == 0) {
		neg_ix();
	} else {
		com_ix();
	}
}

/* $64 LSR indexed -0*-* */
OP_HANDLER(lsr_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	LSR_MEM(t);
}

/* $65 ILLEGAL */

/* $66 ROR indexed -**-* */
OP_HANDLER(ror_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	ROR_MEM(t);
}

/* $67 ASR indexed ?**-* */
OP_HANDLER(asr_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	ASR_MEM(t);
}

/* $68 ASL indexed ?**** */
OP_HANDLER(asl_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	ASL_MEM(t);
}

/* $69 ROL indexed -**** */
OP_HANDLER(rol_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	ROL_MEM(t);
}

/* $6A DEC indexed -***- */
OP_HANDLER(dec_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	DEC_MEM(t);
}

/* $6B DCC index */
OP_HANDLER(dcc_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	DCC_MEM(t);
}

/* $6C INC indexed -***- */
OP_HANDLER(inc_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	INC_MEM(t);
}

/* $6D TST indexed -**0- */
OP_HANDLER(tst_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	TST_MEM(t);
}

/* $6E JMP indexed ----- */
OP_HANDLER(jmp_ix) {
	fetch_effective_address();
	PCD = EAD;
}

/* $6F CLR indexed -0100 */
OP_HANDLER(clr_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	//dummy = RM(EAD);	// Dummy Read(Alpha etc...)
	RM(EAD);	// Dummy Read(Alpha etc...)
	CLR_MEM(t);
}

/* $70 NEG extended ?**** */
OP_HANDLER(neg_ex) {
	uint8_t t;
	EXTBYTE(t);
	NEG_MEM(t);
}


/* $73 COM extended -**01 */
OP_HANDLER(com_ex) {
	uint8_t t;
	EXTBYTE(t);
	COM_MEM(t);
}

/* $72 NGC extended */
OP_HANDLER(ngc_ex) {
	if ((CC & CC_C) == 0) {
		neg_ex();
	} else {
		com_ex();
	}
}

/* $74 LSR extended -0*-* */
OP_HANDLER(lsr_ex) {
	uint8_t t;
	EXTBYTE(t);
	LSR_MEM(t);
}

/* $75 ILLEGAL */

/* $76 ROR extended -**-* */
OP_HANDLER(ror_ex) {
	uint8_t t;
	EXTBYTE(t);
	ROR_MEM(t);
}

/* $77 ASR extended ?**-* */
OP_HANDLER(asr_ex) {
	uint8_t t;
	EXTBYTE(t);
	ASR_MEM(t);
}

/* $78 ASL extended ?**** */
OP_HANDLER(asl_ex) {
	uint8_t t;
	EXTBYTE(t);
	ASL_MEM(t);
}

/* $79 ROL extended -**** */
OP_HANDLER(rol_ex) {
	uint8_t t;
	EXTBYTE(t);
	ROL_MEM(t);
}

/* $7A DEC extended -***- */
OP_HANDLER(dec_ex) {
	uint8_t t;
	EXTBYTE(t);
	DEC_MEM(t);
}

/* $7B ILLEGAL */
/* $6B DCC index */
OP_HANDLER(dcc_ex) {
	uint8_t t;
	EXTBYTE(t);
	DCC_MEM(t);
}

/* $7C INC extended -***- */
OP_HANDLER(inc_ex) {
	uint8_t t;
	EXTBYTE(t);
	INC_MEM(t);
}

/* $7D TST extended -**0- */
OP_HANDLER(tst_ex) {
	uint8_t t;
	EXTBYTE(t);
	TST_MEM(t);
}

/* $7E JMP extended ----- */
OP_HANDLER(jmp_ex) {
	EXTENDED;
	PCD = EAD;
}

/* $7F CLR extended -0100 */
OP_HANDLER(clr_ex) {
	uint8_t dummy;
	EXTENDED;
	dummy = RM(EAD);
	CLR_MEM(dummy);
}

/* $80 SUBA immediate ?**** */
OP_HANDLER(suba_im) {
	uint8_t t;
	IMMBYTE(t);
	A = SUB8_REG(A, t);
}

/* $81 CMPA immediate ?**** */
OP_HANDLER(cmpa_im) {
	uint8_t t;
	IMMBYTE(t);
	A = CMP8_REG(A, t);
}

/* $82 SBCA immediate ?**** */
OP_HANDLER(sbca_im) {
	uint8_t t;
	IMMBYTE(t);
	A = SBC8_REG(A, t);
}

/* $83 SUBD (CMPD CMPU) immediate -**** */
OP_HANDLER(subd_im) {
	pair32_t b;
	IMMWORD(b);
	D = SUB16_REG(D, b.w.l);
}

/* $1083 CMPD immediate -**** */
OP_HANDLER(cmpd_im) {
	pair32_t b;
	IMMWORD(b);
	D = CMP16_REG(D, b.w.l);
}

/* $1183 CMPU immediate -**** */
OP_HANDLER(cmpu_im) {
	pair32_t b;
	IMMWORD(b);
	U = CMP16_REG(U, b.w.l);
}

/* $84 ANDA immediate -**0- */
OP_HANDLER(anda_im) {
	uint8_t t;
	IMMBYTE(t);
	A = AND8_REG(A, t);
}

/* $85 BITA immediate -**0- */
OP_HANDLER(bita_im) {
	uint8_t t;
	IMMBYTE(t);
	A = BIT8_REG(A, t);
}

/* $86 LDA immediate -**0- */
OP_HANDLER(lda_im) {
	IMMBYTE(A);
	A = LOAD8_REG(A);
}

/* is this a legal instruction? */
/* $87 STA immediate -**0- */
OP_HANDLER(sta_im) {
		CLR_NZV;
		SET_NZ8(A);
		IMM8;
		WM(EAD, A);
	}

/*
 * $87 , $C7: FLAG8
 */
OP_HANDLER(flag8_im) {
		// 20111117
		//uint8_t t;
		// IMMBYTE(t);
		ROP_ARG(PCD);
		PC++;
		CLR_NZV;
		CC |= CC_N;
	}


/* $88 EORA immediate -**0- */
OP_HANDLER(eora_im) {
		uint8_t t;
		IMMBYTE(t);
		A = EOR8_REG(A, t);
}

/* $89 ADCA immediate ***** */
OP_HANDLER(adca_im) {
	uint8_t t;
	IMMBYTE(t);
	A = ADC8_REG(A, t);
}

/* $8A ORA immediate -**0- */
OP_HANDLER(ora_im) {
	uint8_t t;
	IMMBYTE(t);
	A = OR8_REG(A, t);
}

/* $8B ADDA immediate ***** */
OP_HANDLER(adda_im) {
	uint8_t t;
	IMMBYTE(t);
	A = ADD8_REG(A, t);
}

/* $8C CMPX (CMPY CMPS) immediate -**** */
OP_HANDLER(cmpx_im) {
	pair32_t b;
	IMMWORD(b);
	X = CMP16_REG(X, b.w.l);
}

/* $108C CMPY immediate -**** */
OP_HANDLER(cmpy_im) {
	pair32_t b;
	IMMWORD(b);
	Y = CMP16_REG(Y, b.w.l);
}

/* $118C CMPS immediate -**** */
OP_HANDLER(cmps_im) {
	pair32_t b;
	IMMWORD(b);
	S = CMP16_REG(S, b.w.l);
}

/* $8D BSR ----- */
OP_HANDLER(bsr) {
		uint8_t t;
		IMMBYTE(t);
		PUSHWORD(pPC);
		PC += SIGNED(t);
	}

/* $8E LDX (LDY) immediate -**0- */
OP_HANDLER(ldx_im) {
	IMMWORD(pX);
	X = LOAD16_REG(X);
}

/* $108E LDY immediate -**0- */
OP_HANDLER(ldy_im) {
	IMMWORD(pY);
	Y = LOAD16_REG(Y);
}

/* is this a legal instruction? */
/* $8F STX (STY) immediate -**0- */
OP_HANDLER(stx_im) {
		CLR_NZV;
		SET_NZ16(X);
		IMM16;
		WM16(EAD, &pX);
	}

/*
 * $8F , $CF: FLAG16
 */
OP_HANDLER(flag16_im) {
		pair32_t t;
		IMMWORD(t);
		CLR_NZV;
		CC |= CC_N;
}


/* is this a legal instruction? */
/* $108F STY immediate -**0- */
OP_HANDLER(sty_im) {
		CLR_NZV;
		SET_NZ16(Y);
		IMM16;
		WM16(EAD, &pY);
	}

/* $90 SUBA direct ?**** */
OP_HANDLER(suba_di) {
	uint8_t t;
	DIRBYTE(t);
	A = SUB8_REG(A, t);
}

/* $91 CMPA direct ?**** */
OP_HANDLER(cmpa_di) {
	uint8_t t;
	DIRBYTE(t);
	A = CMP8_REG(A, t); 
}

/* $92 SBCA direct ?**** */
OP_HANDLER(sbca_di) {
	uint8_t t;
	DIRBYTE(t);
	A = SBC8_REG(A, t);
}

/* $93 SUBD (CMPD CMPU) direct -**** */
OP_HANDLER(subd_di) {
	pair32_t b;
	DIRWORD(b);
	D = SUB16_REG(D, b.w.l);
}

/* $1093 CMPD direct -**** */
OP_HANDLER(cmpd_di) {
	pair32_t b;
	DIRWORD(b);
	D = CMP16_REG(D, b.w.l);
}

/* $1193 CMPU direct -**** */
OP_HANDLER(cmpu_di) {
	pair32_t b;
	DIRWORD(b);
	U = CMP16_REG(U, b.w.l);
}

/* $94 ANDA direct -**0- */
OP_HANDLER(anda_di) {
	uint8_t t;
	DIRBYTE(t);
	A = AND8_REG(A, t);
}

/* $95 BITA direct -**0- */
OP_HANDLER(bita_di) {
	uint8_t t;
	DIRBYTE(t);
	A = BIT8_REG(A, t);
}

/* $96 LDA direct -**0- */
OP_HANDLER(lda_di) {
	DIRBYTE(A);
	A = LOAD8_REG(A);
}

/* $97 STA direct -**0- */
OP_HANDLER(sta_di) {
	DIRECT;
	STORE8_REG(A);
}

/* $98 EORA direct -**0- */
OP_HANDLER(eora_di) {
	uint8_t t;
	DIRBYTE(t);
	A = EOR8_REG(A, t);
}

/* $99 ADCA direct ***** */
OP_HANDLER(adca_di) {
	uint8_t t;
	DIRBYTE(t);
	A = ADC8_REG(A, t);
}

/* $9A ORA direct -**0- */
OP_HANDLER(ora_di) {
	uint8_t t;
	DIRBYTE(t);
	A = OR8_REG(A, t);
}

/* $9B ADDA direct ***** */
OP_HANDLER(adda_di) {
	uint8_t t;
	DIRBYTE(t);
	A = ADD8_REG(A, t);
}

/* $9C CMPX (CMPY CMPS) direct -**** */
OP_HANDLER(cmpx_di) {
	pair32_t b;
	DIRWORD(b);
	X = CMP16_REG(X, b.w.l);
}

/* $109C CMPY direct -**** */
OP_HANDLER(cmpy_di) {
	pair32_t b;
	DIRWORD(b);
	Y = CMP16_REG(Y, b.w.l);
}

/* $119C CMPS direct -**** */
OP_HANDLER(cmps_di) {
	pair32_t b;
	DIRWORD(b);
	S = CMP16_REG(S, b.w.l);
}

/* $9D JSR direct ----- */
OP_HANDLER(jsr_di) {
		DIRECT;
		PUSHWORD(pPC);
		PCD = EAD;
	}

/* $9E LDX (LDY) direct -**0- */
OP_HANDLER(ldx_di) {
	DIRWORD(pX);
	X = LOAD16_REG(X);
}

/* $109E LDY direct -**0- */
OP_HANDLER(ldy_di) {
	DIRWORD(pY);
	Y = LOAD16_REG(Y);
}

/* $9F STX (STY) direct -**0- */
OP_HANDLER(stx_di) {
	DIRECT;
	STORE16_REG(&pX);
}

/* $109F STY direct -**0- */
OP_HANDLER(sty_di) {
	DIRECT;
	STORE16_REG(&pY);
}

/* $a0 SUBA indexed ?**** */
OP_HANDLER(suba_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	A = SUB8_REG(A, t); 
}

/* $a1 CMPA indexed ?**** */
OP_HANDLER(cmpa_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	A = CMP8_REG(A, t); 
}

/* $a2 SBCA indexed ?**** */
OP_HANDLER(sbca_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	A = SBC8_REG(A, t); 
}

/* $a3 SUBD (CMPD CMPU) indexed -**** */
OP_HANDLER(subd_ix) {
	pair32_t b;
	b = GET_INDEXED_DATA16();
	D = SUB16_REG(D, b.w.l);
}

/* $10a3 CMPD indexed -**** */
OP_HANDLER(cmpd_ix) {
	pair32_t b;
	b = GET_INDEXED_DATA16();
	D = CMP16_REG(D, b.w.l);
}

/* $11a3 CMPU indexed -**** */
OP_HANDLER(cmpu_ix) {
	pair32_t b;
	b = GET_INDEXED_DATA16();
	U = CMP16_REG(U, b.w.l);
}

/* $a4 ANDA indexed -**0- */
OP_HANDLER(anda_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	A = AND8_REG(A, t);
}

/* $a5 BITA indexed -**0- */
OP_HANDLER(bita_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	A = BIT8_REG(A, t);
}

/* $a6 LDA indexed -**0- */
OP_HANDLER(lda_ix) {
	A = GET_INDEXED_DATA();
	A = LOAD8_REG(A);
}

/* $a7 STA indexed -**0- */
OP_HANDLER(sta_ix) {
	fetch_effective_address();
	STORE8_REG(A);
}

/* $a8 EORA indexed -**0- */
OP_HANDLER(eora_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	A = EOR8_REG(A, t);
}

/* $a9 ADCA indexed ***** */
OP_HANDLER(adca_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	A = ADC8_REG(A, t);
}

/* $aA ORA indexed -**0- */
OP_HANDLER(ora_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	A = OR8_REG(A, t);
}

/* $aB ADDA indexed ***** */
OP_HANDLER(adda_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	A = ADD8_REG(A, t);
}

/* $aC CMPX (CMPY CMPS) indexed -**** */
OP_HANDLER(cmpx_ix) {
	pair32_t b;
	b = GET_INDEXED_DATA16();
	X = CMP16_REG(X, b.w.l);
}

/* $10aC CMPY indexed -**** */
OP_HANDLER(cmpy_ix) {
	pair32_t b;
	b = GET_INDEXED_DATA16();
	Y = CMP16_REG(Y, b.w.l);
}

/* $11aC CMPS indexed -**** */
OP_HANDLER(cmps_ix) {
	pair32_t b;
	b = GET_INDEXED_DATA16();
	S = CMP16_REG(S, b.w.l);
}

/* $aD JSR indexed ----- */
OP_HANDLER(jsr_ix) {
	fetch_effective_address();
	PUSHWORD(pPC);
	PCD = EAD;
}

/* $aE LDX (LDY) indexed -**0- */
OP_HANDLER(ldx_ix) {
	pair32_t t;
	t = GET_INDEXED_DATA16();
	X = t.w.l;
	X = LOAD16_REG(X);
}

/* $10aE LDY indexed -**0- */
OP_HANDLER(ldy_ix) {
	pair32_t t;
	t = GET_INDEXED_DATA16();
	Y = t.w.l;
	Y = LOAD16_REG(Y);
}

/* $aF STX (STY) indexed -**0- */
OP_HANDLER(stx_ix) {
	fetch_effective_address();
	STORE16_REG(&pX);
}

/* $10aF STY indexed -**0- */
OP_HANDLER(sty_ix) {
	fetch_effective_address();
	STORE16_REG(&pY);
}

/* $b0 SUBA extended ?**** */
OP_HANDLER(suba_ex) {
	uint8_t t;
	EXTBYTE(t);
	A = SUB8_REG(A, t);
}

/* $b1 CMPA extended ?**** */
OP_HANDLER(cmpa_ex) {
	uint8_t t;
	EXTBYTE(t);
	A = CMP8_REG(A, t);
}

/* $b2 SBCA extended ?**** */
OP_HANDLER(sbca_ex) {
	uint8_t t;
	EXTBYTE(t);
	A = SBC8_REG(A, t);
}

/* $b3 SUBD (CMPD CMPU) extended -**** */
OP_HANDLER(subd_ex) {
	pair32_t b;
	EXTWORD(b);
	D = SUB16_REG(D, b.w.l);
}

/* $10b3 CMPD extended -**** */
OP_HANDLER(cmpd_ex) {
	pair32_t b;
	EXTWORD(b);
	D = CMP16_REG(D, b.w.l);
}

/* $11b3 CMPU extended -**** */
OP_HANDLER(cmpu_ex) {
	pair32_t b;
	EXTWORD(b);
	U = CMP16_REG(U, b.w.l);
}

/* $b4 ANDA extended -**0- */
OP_HANDLER(anda_ex) {
	uint8_t t;
	EXTBYTE(t);
	A = AND8_REG(A, t);
}

/* $b5 BITA extended -**0- */
OP_HANDLER(bita_ex) {
	uint8_t t;
	EXTBYTE(t);
	A = BIT8_REG(A, t);
}

/* $b6 LDA extended -**0- */
OP_HANDLER(lda_ex) {
	EXTBYTE(A);
	A = LOAD8_REG(A);
}

/* $b7 STA extended -**0- */
OP_HANDLER(sta_ex) {
	EXTENDED;
	STORE8_REG(A);
}

/* $b8 EORA extended -**0- */
OP_HANDLER(eora_ex) {
	uint8_t t;
	EXTBYTE(t);
	A = EOR8_REG(A, t);
}

/* $b9 ADCA extended ***** */
OP_HANDLER(adca_ex) {
	uint8_t t;
	EXTBYTE(t);
	A = ADC8_REG(A, t);
}

/* $bA ORA extended -**0- */
OP_HANDLER(ora_ex) {
	uint8_t t;
	EXTBYTE(t);
	A = OR8_REG(A, t);
}

/* $bB ADDA extended ***** */
OP_HANDLER(adda_ex) {
	uint8_t t;
	EXTBYTE(t);
	A = ADD8_REG(A, t);
}

/* $bC CMPX (CMPY CMPS) extended -**** */
OP_HANDLER(cmpx_ex) {
	pair32_t b;
	EXTWORD(b);
	X = CMP16_REG(X, b.w.l);
}

/* $10bC CMPY extended -**** */
OP_HANDLER(cmpy_ex) {
	pair32_t b;
	EXTWORD(b);
	Y = CMP16_REG(Y, b.w.l);
}

/* $11bC CMPS extended -**** */
OP_HANDLER(cmps_ex) {
	pair32_t b;
	EXTWORD(b);
	S = CMP16_REG(S, b.w.l);
}

/* $bD JSR extended ----- */
OP_HANDLER(jsr_ex) {
		EXTENDED;
		PUSHWORD(pPC);
		PCD = EAD;
}

/* $bE LDX (LDY) extended -**0- */
OP_HANDLER(ldx_ex) {
	EXTWORD(pX);
	X = LOAD16_REG(X);
}

/* $10bE LDY extended -**0- */
OP_HANDLER(ldy_ex) {
	EXTWORD(pY);
	Y = LOAD16_REG(Y);
}

/* $bF STX (STY) extended -**0- */
OP_HANDLER(stx_ex) {
	EXTENDED;
	STORE16_REG(&pX);
}

/* $10bF STY extended -**0- */
OP_HANDLER(sty_ex) {
	EXTENDED;
	STORE16_REG(&pY);
}

/* $c0 SUBB immediate ?**** */
OP_HANDLER(subb_im) {
	uint8_t t;
	IMMBYTE(t);
	B = SUB8_REG(B, t);
}

/* $c1 CMPB immediate ?**** */
OP_HANDLER(cmpb_im) {
	uint8_t t;
	IMMBYTE(t);
	B = CMP8_REG(B, t);
}

/* $c2 SBCB immediate ?**** */
OP_HANDLER(sbcb_im) {
	uint8_t t;
	IMMBYTE(t);
	B = SBC8_REG(B, t);
}

/* $c3 ADDD immediate -**** */
OP_HANDLER(addd_im) {
	pair32_t b;
	IMMWORD(b);
	D = ADD16_REG(D, b.w.l);
}

/* $c4 ANDB immediate -**0- */
OP_HANDLER(andb_im) {
	uint8_t t;
	IMMBYTE(t);
	B = AND8_REG(B, t);
}

/* $c5 BITB immediate -**0- */
OP_HANDLER(bitb_im) {
	uint8_t t;
	IMMBYTE(t);
	B = BIT8_REG(B, t);
}

/* $c6 LDB immediate -**0- */
OP_HANDLER(ldb_im) {
	IMMBYTE(B);
	B = LOAD8_REG(B);
}

/* is this a legal instruction? */
/* $c7 STB immediate -**0- */
OP_HANDLER(stb_im) {
	CLR_NZV;
	SET_NZ8(B);
	IMM8;
	WM(EAD, B);
}

/* $c8 EORB immediate -**0- */
OP_HANDLER(eorb_im) {
	uint8_t t;
	IMMBYTE(t);
	B = EOR8_REG(B, t);
}

/* $c9 ADCB immediate ***** */
OP_HANDLER(adcb_im) {
	uint8_t t;
	IMMBYTE(t);
	B = ADC8_REG(B, t);
}

/* $cA ORB immediate -**0- */
OP_HANDLER(orb_im) {
	uint8_t t;
	IMMBYTE(t);
	B = OR8_REG(B, t);
}

/* $cB ADDB immediate ***** */
OP_HANDLER(addb_im) {
	uint8_t t;
	IMMBYTE(t);
	B = ADD8_REG(B, t);
}

/* $cC LDD immediate -**0- */
OP_HANDLER(ldd_im) {
	IMMWORD(pD);
	D = LOAD16_REG(D);
}

/* is this a legal instruction? */
/* $cD STD immediate -**0- */
OP_HANDLER(std_im) {
		CLR_NZV;
		SET_NZ16(D);
		IMM16;
		WM(EAD, D);
}

/* $cE LDU (LDS) immediate -**0- */
OP_HANDLER(ldu_im) {
	IMMWORD(pU);
	U = LOAD16_REG(U);
}

/* $10cE LDS immediate -**0- */
OP_HANDLER(lds_im) {
	IMMWORD(pS);
	S = LOAD16_REG(S);
	int_state |= MC6809_LDS;
}

/* is this a legal instruction? */
/* $cF STU (STS) immediate -**0- */
OP_HANDLER(stu_im) {
		CLR_NZV;
		SET_NZ16(U);
		IMM16;
		WM16(EAD, &pU);
	}

/* is this a legal instruction? */
/* $10cF STS immediate -**0- */
OP_HANDLER(sts_im) {
		CLR_NZV;
		SET_NZ16(S);
		IMM16;
		WM16(EAD, &pS);
	}

/* $d0 SUBB direct ?**** */
OP_HANDLER(subb_di) {
	uint8_t t;
	DIRBYTE(t);
	B = SUB8_REG(B, t);
}
/* $d1 CMPB direct ?**** */
OP_HANDLER(cmpb_di) {
	uint8_t t;
	DIRBYTE(t);
	B = CMP8_REG(B, t);
}

/* $d2 SBCB direct ?**** */
OP_HANDLER(sbcb_di) {
	uint8_t t;
	DIRBYTE(t);
	B = SBC8_REG(B, t);
}

/* $d3 ADDD direct -**** */
OP_HANDLER(addd_di) {
	pair32_t b;
	DIRWORD(b);
	D = ADD16_REG(D, b.w.l);
}

/* $d4 ANDB direct -**0- */
OP_HANDLER(andb_di) {
	uint8_t t;
	DIRBYTE(t);
	B = AND8_REG(B, t);
}

/* $d5 BITB direct -**0- */
OP_HANDLER(bitb_di) {
	uint8_t t;
	DIRBYTE(t);
	B = BIT8_REG(B, t);
}

/* $d6 LDB direct -**0- */
OP_HANDLER(ldb_di) {
	DIRBYTE(B);
	B = LOAD8_REG(B);
}

/* $d7 STB direct -**0- */
OP_HANDLER(stb_di) {
	DIRECT;
	STORE8_REG(B);
}

/* $d8 EORB direct -**0- */
OP_HANDLER(eorb_di) {
	uint8_t t;
	DIRBYTE(t);
	B = EOR8_REG(B, t);
}

/* $d9 ADCB direct ***** */
OP_HANDLER(adcb_di) {
	uint8_t t;
	DIRBYTE(t);
	B = ADC8_REG(B, t);
}

/* $dA ORB direct -**0- */
OP_HANDLER(orb_di) {
	uint8_t t;
	DIRBYTE(t);
	B = OR8_REG(B, t);
}

/* $dB ADDB direct ***** */
OP_HANDLER(addb_di) {
	uint8_t t;
	DIRBYTE(t);
	B = ADD8_REG(B, t);
}

/* $dC LDD direct -**0- */
OP_HANDLER(ldd_di) {
	DIRWORD(pD);
	D = LOAD16_REG(D);
}

/* $dD STD direct -**0- */
OP_HANDLER(std_di) {
	DIRECT;
	STORE16_REG(&pD);
}

/* $dE LDU (LDS) direct -**0- */
OP_HANDLER(ldu_di) {
	DIRWORD(pU);
	U = LOAD16_REG(U);
}

/* $10dE LDS direct -**0- */
OP_HANDLER(lds_di) {
	DIRWORD(pS);
	S = LOAD16_REG(S);
	int_state |= MC6809_LDS;
}

/* $dF STU (STS) direct -**0- */
OP_HANDLER(stu_di) {
	DIRECT;
	STORE16_REG(&pU);
}

/* $10dF STS direct -**0- */
OP_HANDLER(sts_di) {
	DIRECT;
	STORE16_REG(&pS);
}

/* $e0 SUBB indexed ?**** */
OP_HANDLER(subb_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	B = SUB8_REG(B, t);
}

/* $e1 CMPB indexed ?**** */
OP_HANDLER(cmpb_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	B = CMP8_REG(B, t);
}

/* $e2 SBCB indexed ?**** */
OP_HANDLER(sbcb_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	B = SBC8_REG(B, t);
}

/* $e3 ADDD indexed -**** */
OP_HANDLER(addd_ix) {
	pair32_t b;
	b = GET_INDEXED_DATA16();
	D = ADD16_REG(D, b.w.l);
}

/* $e4 ANDB indexed -**0- */
OP_HANDLER(andb_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	B = AND8_REG(B, t);
}

/* $e5 BITB indexed -**0- */
OP_HANDLER(bitb_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	B = BIT8_REG(B, t);
}

/* $e6 LDB indexed -**0- */
OP_HANDLER(ldb_ix) {
	B = GET_INDEXED_DATA();
	B = LOAD8_REG(B);
}

/* $e7 STB indexed -**0- */
OP_HANDLER(stb_ix) {
	fetch_effective_address();
	STORE8_REG(B);
}

/* $e8 EORB indexed -**0- */
OP_HANDLER(eorb_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	B = EOR8_REG(B, t);
}

/* $e9 ADCB indexed ***** */
OP_HANDLER(adcb_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	B = ADC8_REG(B, t);
}

/* $eA ORB indexed -**0- */
OP_HANDLER(orb_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	B = OR8_REG(B, t);
}

/* $eB ADDB indexed ***** */
OP_HANDLER(addb_ix) {
	uint8_t t;
	t = GET_INDEXED_DATA();
	B = ADD8_REG(B, t);
}

/* $eC LDD indexed -**0- */
OP_HANDLER(ldd_ix) {
	pair32_t t;
	t = GET_INDEXED_DATA16();
	D = t.w.l;
	D = LOAD16_REG(D);
}

/* $eD STD indexed -**0- */
OP_HANDLER(std_ix) {
	fetch_effective_address();
	STORE16_REG(&pD);
}

/* $eE LDU (LDS) indexed -**0- */
OP_HANDLER(ldu_ix) {
	pair32_t t;
	t = GET_INDEXED_DATA16();
	U = t.w.l;
	U = LOAD16_REG(U);
}

/* $10eE LDS indexed -**0- */
OP_HANDLER(lds_ix) {
	pair32_t t;
	t = GET_INDEXED_DATA16();
	S = t.w.l;
	S = LOAD16_REG(S);
	int_state |= MC6809_LDS;
}

/* $eF STU (STS) indexed -**0- */
OP_HANDLER(stu_ix) {
	fetch_effective_address();
	STORE16_REG(&pU);
}

/* $10eF STS indexed -**0- */
OP_HANDLER(sts_ix) {
	fetch_effective_address();
	STORE16_REG(&pS);
}

/* $f0 SUBB extended ?**** */
OP_HANDLER(subb_ex) {
	uint8_t t;
	EXTBYTE(t);
	B = SUB8_REG(B, t);
}

/* $f1 CMPB extended ?**** */
OP_HANDLER(cmpb_ex) {
	uint8_t t;
	EXTBYTE(t);
	B = CMP8_REG(B, t);
}

/* $f2 SBCB extended ?**** */
OP_HANDLER(sbcb_ex) {
	uint8_t t;
	EXTBYTE(t);
	B = SBC8_REG(B, t);
}

/* $f3 ADDD extended -**** */
OP_HANDLER(addd_ex) {
	pair32_t b;
	EXTWORD(b);
	D = ADD16_REG(D, b.w.l);
}

/* $f4 ANDB extended -**0- */
OP_HANDLER(andb_ex) {
	uint8_t t;
	EXTBYTE(t);
	B = AND8_REG(B, t);
}

/* $f5 BITB extended -**0- */
OP_HANDLER(bitb_ex) {
	uint8_t t;
	EXTBYTE(t);
	B = BIT8_REG(B, t);
}

/* $f6 LDB extended -**0- */
OP_HANDLER(ldb_ex) {
	EXTBYTE(B);
	B = LOAD8_REG(B);
}

/* $f7 STB extended -**0- */
OP_HANDLER(stb_ex) {
	EXTENDED;
	STORE8_REG(B);
}

/* $f8 EORB extended -**0- */
OP_HANDLER(eorb_ex) {
	uint8_t t;
	EXTBYTE(t);
	B = EOR8_REG(B, t);
}

/* $f9 ADCB extended ***** */
OP_HANDLER(adcb_ex) {
	uint8_t t;
	EXTBYTE(t);
	B = ADC8_REG(B, t);
}

/* $fA ORB extended -**0- */
OP_HANDLER(orb_ex) {
	uint8_t t;
	EXTBYTE(t);
	B = OR8_REG(B, t);
}

/* $fB ADDB extended ***** */
OP_HANDLER(addb_ex) {
	uint8_t t;
	EXTBYTE(t);
	B = ADD8_REG(B, t);
}

/* $fC LDD extended -**0- */
OP_HANDLER(ldd_ex) {
	EXTWORD(pD);
	D = LOAD16_REG(D);
}

/* $fD STD extended -**0- */
OP_HANDLER(std_ex) {
	EXTENDED;
	STORE16_REG(&pD);
}

/* $fE LDU (LDS) extended -**0- */
OP_HANDLER(ldu_ex) {
	EXTWORD(pU);
	U = LOAD16_REG(U);
}

/* $10fE LDS extended -**0- */
OP_HANDLER(lds_ex) {
	EXTWORD(pS);
	S = LOAD16_REG(S);
	int_state |= MC6809_LDS;
}

/* $fF STU (STS) extended -**0- */
OP_HANDLER(stu_ex) {
	EXTENDED;
	STORE16_REG(&pU);
}

/* $10fF STS extended -**0- */
OP_HANDLER(sts_ex) {
	EXTENDED;
	STORE16_REG(&pS);
}


/* $10xx opcodes */
OP_HANDLER(pref10) {
	uint8_t ireg2 = ROP_ARG(PCD);
	PC++;
	switch (ireg2) {
		case 0x20:
			lbra();
			icount -= 5;
			break;	// 20111217
		case 0x21:
			lbrn();
			icount -= 5;
			break;
		case 0x22:
			lbhi();
			icount -= 5;
			break;
		case 0x23:
			lbls();
			icount -= 5;
			break;
		case 0x24:
			lbcc();
			icount -= 5;
			break;
		case 0x25:
			lbcs();
			icount -= 5;
			break;
		case 0x26:
			lbne();
			icount -= 5;
			break;
		case 0x27:
			lbeq();
			icount -= 5;
			break;
		case 0x28:
			lbvc();
			icount -= 5;
			break;
		case 0x29:
			lbvs();
			icount -= 5;
			break;
		case 0x2a:
			lbpl();
			icount -= 5;
			break;
		case 0x2b:
			lbmi();
			icount -= 5;
			break;
		case 0x2c:
			lbge();
			icount -= 5;
			break;
		case 0x2d:
			lblt();
			icount -= 5;
			break;
		case 0x2e:
			lbgt();
			icount -= 5;
			break;
		case 0x2f:
			lble();
			icount -= 5;
			break;
		case 0x3f:
			swi2();
			icount -= 20;
			break;
		case 0x83:
			cmpd_im();
			icount -= 5;
			break;
		case 0x8c:
			cmpy_im();
			icount -= 5;
			break;
		case 0x8d:
			lbsr();
			icount -= 9;
			break;
		case 0x8e:
			ldy_im();
			icount -= 4;
			break;
//    case 0x8f: flag16_im();->cycle=4; break; // 20130417
		case 0x93:
			cmpd_di();
			icount -= 7;
			break;
		case 0x9c:
			cmpy_di();
			icount -= 7;
			break;
		case 0x9e:
			ldy_di();
			icount -= 6;
			break;
		case 0x9f:
			sty_di();
			icount -= 6;
			break;
		case 0xa3:
			cmpd_ix();
			icount -= 7;
			break;
		case 0xac:
			cmpy_ix();
			icount -= 7;
			break;
		case 0xae:
			ldy_ix();
			icount -= 6;
			break;
		case 0xaf:
			sty_ix();
			icount -= 6;
			break;
		case 0xb3:
			cmpd_ex();
			icount -= 8;
			break;
		case 0xbc:
			cmpy_ex();
			icount -= 8;
			break;
		case 0xbe:
			ldy_ex();
			icount -= 7;
			break;
		case 0xbf:
			sty_ex();
			icount -= 7;
			break;
		case 0xce:
			lds_im();
			icount -= 4;
			break;
//    case 0xcf: flag16_im();->cycle=4; break;
		case 0xde:
			lds_di();
			icount -= 6;
			break;
		case 0xdf:
			sts_di();
			icount -= 6;
			break;
		case 0xee:
			lds_ix();
			icount -= 6;
			break;
		case 0xef:
			sts_ix();
			icount -= 6;
			break;
		case 0xfe:
			lds_ex();
			icount -= 7;
			break;
		case 0xff:
			sts_ex();
			icount -= 7;
			break;
		default:
			PC--;
			IIError();
			break;
	}
}

/* $11xx opcodes */
OP_HANDLER(pref11) {
		uint8_t ireg2 = ROP_ARG(PCD);
		PC++;
		switch (ireg2) {
			case 0x3f:
				swi3();
				icount -= 20;
				break;

			case 0x83:
				cmpu_im();
				icount -= 5;
				break;
			case 0x8c:
				cmps_im();
				icount -= 5;
				break;

			case 0x93:
				cmpu_di();
				icount -= 7;
				break;
			case 0x9c:
				cmps_di();
				icount -= 7;
				break;

			case 0xa3:
				cmpu_ix();
				icount -= 7;
				break;
			case 0xac:
				cmps_ix();
				icount -= 7;
				break;

			case 0xb3:
				cmpu_ex();
				icount -= 8;
				break;
			case 0xbc:
				cmps_ex();
				icount -= 8;
				break;

			default:
				PC--;
				IIError();
				break;
		}
	}

#define STATE_VERSION	5

bool MC6809_BASE::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(icount);
	state_fio->StateValue(extra_icount);
	state_fio->StateValue(int_state);

	state_fio->StateValue(pc.d);
	state_fio->StateValue(ppc.d);
	state_fio->StateValue(acc.d);
	state_fio->StateValue(dp.d);
	state_fio->StateValue(u.d);
	state_fio->StateValue(s.d);
	state_fio->StateValue(x.d);
	state_fio->StateValue(y.d);
	state_fio->StateValue(cc);
	state_fio->StateValue(ea.d);

	// V2
	state_fio->StateValue(req_halt_on);
	state_fio->StateValue(req_halt_off);
	state_fio->StateValue(busreq);
	
	state_fio->StateValue(total_icount);
	state_fio->StateValue(waitfactor);
	state_fio->StateValue(waitcount);
	
	// post process
	if(loading) {
		prev_total_icount = total_icount;
	}
	return true;
}


