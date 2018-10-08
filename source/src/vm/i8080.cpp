/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2008.11.04 -

	[ i8080 / i8085 ]
*/

#include "./vm.h"
#include "../emu.h"
#include "i8080.h"
#include "./i8080_regdef.h"
//#ifdef USE_DEBUGGER
#include "debugger.h"
//#endif


#ifndef CPU_START_ADDR
#define CPU_START_ADDR	0
#endif


#ifdef HAS_I8085
void I8080::ANA(uint8_t v)
{
	_A &= v; 
	_F = ZSP[_A];
	_F |= HF;
}
#else
void I8080::ANA(uint8_t v)
{										
	int i = (((_A | v) >> 3) & 1) * HF;
	_A &= v;
	_F = ZSP[_A];
	_F |= i;
}
#endif

#ifdef HAS_I8085
void I8080::JMP(uint8_t c)
{
	if(c) {
		PC = FETCH16();
	} else {
		PC += 2;
		count += 3;
	}
}
void I8080::CALL(uint8_t c)
{
	if(c) {
		uint16_t a = FETCH16();
		count -= 7;
		PUSH16(PC);
		PC = a;
	} else {
		PC += 2;
		count += 2;
	}
}
#else
void I8080::JMP(uint8_t c)
{
	if(c) {
		PC = FETCH16();
	} else {
		PC += 2;
	}
}

void I8080::CALL(uint8_t c)
{
	if(c) {
		uint16_t a = FETCH16();
		count -= 6;
		PUSH16(PC);
		PC = a;
	} else {
		PC += 2;
	}
}
#endif

void I8080::dec_count(uint8_t code)
{
#ifdef HAS_I8085
	count -= cc_op_8085[code];
#else
	count -= cc_op_8080[code];
#endif
}

void I8080::check_reg_c(uint8_t val)
{
#ifdef HAS_I8085
	if(_C == val) _F |= XF; else _F &= ~XF;
#endif
}

void I8080::check_reg_e(uint8_t val)
{
#ifdef HAS_I8085
	if(_E == val) _F |= XF; else _F &= ~XF;
#endif
}

void I8080::check_reg_l(uint8_t val)
{
#ifdef HAS_I8085
	if(_L == val) _F |= XF; else _F &= ~XF;
#endif
}

void I8080::check_reg_sp(uint8_t val)
{
#ifdef HAS_I8085
	if((SP & 0xff) == val) _F |= XF; else _F &= ~XF;
#endif
}

void I8080::INSN_0x08(void)
{
#ifdef HAS_I8085
	DSUB();
#endif
}

void I8080::INSN_0x10(void)
{
#ifdef HAS_I8085
	// ASRH
	_F = (_F & ~CF) | (_L & CF);
	HL = (HL >> 1);
#endif
}


void I8080::RLDE(void)
{ // 0x18
#ifdef HAS_I8085
	_F = (_F & ~(CF | VF)) | (_D >> 7);
	DE = (DE << 1) | (DE >> 15);
	if(0 != (((DE >> 15) ^ _F) & CF)) {
		_F |= VF;
	}
#endif
}

void I8080::RIM(void)
{ // 0x20
#ifdef HAS_I8085
	_A = (IM & 0x7f) | (SID ? 0x80 : 0) | RIM_IEN;
	RIM_IEN = 0;
#endif
}

void I8080::_DAA(void)
{
	// 0x27
	uint16_t tmp16;
	tmp16 = _A;
	if(_F & CF) tmp16 |= 0x100;
	if(_F & HF) tmp16 |= 0x200;
	if(_F & NF) tmp16 |= 0x400;
	AF = DAA[tmp16];
#ifdef HAS_I8080
	_F &= 0xd5;
#endif
}

void I8080::LDEH(void)
{// 0x28
#ifdef HAS_I8085
	DE = (HL + FETCH8()) & 0xffff;
#endif
}
	
void I8080::CMA(void)
{ // 0x2f
#ifdef HAS_I8085
	_A ^= 0xff;
	_F |= HF + NF;
#else
	_A ^= 0xff;
#endif

}

void I8080::SIM(void)
{
#ifdef HAS_I8085
	if(_A & 0x40) {
		write_signals(&outputs_sod, (_A & 0x80) ? 0xffffffff : 0);
	}
	if(_A & 0x10) {
		IM &= ~IM_I7;
	}
	if(_A & 8) {
		IM = (IM & ~(IM_M5 | IM_M6 | IM_M7)) | (_A & (IM_M5 | IM_M6 | IM_M7));
	}
#endif
}

void I8080::LDES(void)
{
#ifdef HAS_I8085
		DE = (SP + FETCH8()) & 0xffff;
#endif
}

void I8080::INSN_0xcb(void)
{ // RST 8
#ifdef HAS_I8085
	if(_F & VF) {
		count -= 12;
		RST(8);
	} else {
		count -= 6;
	}
#else
	JMP(1);
#endif
}

#ifndef HAS_I8085
#define RET(c) { \
	if(c) { \
		count -= 6; \
		PC = POP16(); \
	}	\
}
#endif
void I8080::INSN_0xd9(void)
{
#ifdef HAS_I8085
		WM16(DE, HL);
#else
		RET(1);
#endif
}
void I8080::INSN_0xdd(void)
{
#ifdef HAS_I8085
		JMP(!(_F & XF));
#else
		CALL(1);
#endif
}

void I8080::INSN_0xed(void)
{
#ifdef HAS_I8085
	HL = RM16(DE); // LHLX
#else
	CALL(1);
#endif
}

void I8080::INSN_0xfd(void)
{
#ifdef HAS_I8085
		JMP(_F & XF);
#else
		CALL(1);
#endif
}


// main

void I8080::initialize()
{
	I8080_BASE::initialize();
#ifdef USE_DEBUGGER
	d_mem_stored = d_mem;
	d_io_stored = d_io;
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
#endif
}

void I8080::reset()
{
	I8080_BASE::reset();
	// reset
	PC = CPU_START_ADDR;
}

void I8080::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CPU_NMI) {
		if(data & mask) {
			IM |= IM_NMI;
		} else {
			IM &= ~IM_NMI;
		}
	} else if(id == SIG_CPU_BUSREQ) {
		BUSREQ = ((data & mask) != 0);
		write_signals(&outputs_busack, BUSREQ ? 0xffffffff : 0);
	} else if(id == SIG_I8080_INTR) {
		if(data & mask) {
			IM |= IM_INT;
		} else {
			IM &= ~IM_INT;
		}
#ifdef HAS_I8085
	} else if(id == SIG_I8085_RST5) {
		if(data & mask) {
			IM |= IM_I5;
		} else {
			IM &= ~IM_I5;
		}
	} else if(id == SIG_I8085_RST6) {
		if(data & mask) {
			IM |= IM_I6;
		} else {
			IM &= ~IM_I6;
		}
	} else if(id == SIG_I8085_RST7) {
		if(data & mask) {
			IM |= IM_I7;
		} else {
			IM &= ~IM_I7;
		}
	} else if(id == SIG_I8085_SID) {
		SID = ((data & mask) != 0);
#endif
	}
}


int I8080::run(int clock)
{
	if(clock == -1) {
		if(BUSREQ) {
			// don't run cpu!
#ifdef USE_DEBUGGER
			total_count += 1;
#endif
			return 1;
		} else {
			// run only one opcode
			count = 0;
			run_one_opecode();
			return -count;
		}
	} else {
		count += clock;
		int first_count = count;
		
		// run cpu while given clocks
		while(count > 0 && !BUSREQ) {
			run_one_opecode();
		}
		// if busreq is raised, spin cpu while remained clock
		if(count > 0 && BUSREQ) {
#ifdef USE_DEBUGGER
			total_count += count;
#endif
			count = 0;
		}
		return first_count - count;
	}
}

void I8080::run_one_opecode()
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
		
		afterHALT = afterEI = false;
		d_debugger->add_cpu_trace(PC);
		int first_count = count;
		OP(FETCHOP());
		total_count += first_count - count;
		if(!afterEI) {
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
		afterHALT = afterEI = false;
#ifdef USE_DEBUGGER
		d_debugger->add_cpu_trace(PC);
		int first_count = count;
#endif
		OP(FETCHOP());
#ifdef USE_DEBUGGER
		total_count += first_count - count;
#endif
		if(!afterEI) {
			check_interrupt();
		}
#ifdef USE_DEBUGGER
	}
#endif
	
	// ei: run next opecode
	if(afterEI) {
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
			
			afterHALT = false;
			d_debugger->add_cpu_trace(PC);
			int first_count = count;
			OP(FETCHOP());
			total_count += first_count - count;
			d_pic->notify_intr_ei();
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
			afterHALT = false;
#ifdef USE_DEBUGGER
			d_debugger->add_cpu_trace(PC);
			int first_count = count;
#endif
			OP(FETCHOP());
#ifdef USE_DEBUGGER
			total_count += first_count - count;
#endif
			d_pic->notify_intr_ei();
			check_interrupt();
#ifdef USE_DEBUGGER
		}
#endif
	}
}

void I8080::check_interrupt()
{
#ifdef USE_DEBUGGER
	int first_count = count;
#endif
	// check interrupt
	if(IM & IM_REQ) {
		if(IM & IM_NMI) {
			INT(0x24);
			count -= 5;	// unknown
			RIM_IEN = IM & IM_IEN;
			IM &= ~(IM_IEN | IM_NMI);
		} else if(IM & IM_IEN) {
#ifdef HAS_I8085
#ifdef _FP200
			if(/*!(IM & IM_M7) &&*/ (IM & IM_I7)) {
#else
			if(!(IM & IM_M7) && (IM & IM_I7)) {
#endif
				INT(0x3c);
				count -= 7;	// unknown
				RIM_IEN = 0;
				IM &= ~(IM_IEN | IM_I7);
			} else if(!(IM & IM_M6) && (IM & IM_I6)) {
				INT(0x34);
				count -= 7;	// unknown
				RIM_IEN = 0;
				IM &= ~(IM_IEN | IM_I6);
			} else if(!(IM & IM_M5) && (IM & IM_I5)) {
				INT(0x2c);
				count -= 7;	// unknown
				RIM_IEN = 0;
				IM &= ~(IM_IEN | IM_I5);
			} else
#endif
			if(IM & IM_INT) {
				uint32_t vector = ACK_INTR();
				uint8_t v0 = vector;
				uint16_t v12 = vector >> 8;
				// support JMP/CALL/RST only
				//count -= cc_op[v0];
#ifdef HAS_I8085
				count -= cc_op_8085[v0];
#else
				count -= cc_op_8080[v0];
#endif
				switch(v0) {
				case 0x00:	// NOP
					break;
				case 0xc3:	// JMP
					PC = v12;
					break;
				case 0xcd:	// CALL
					PUSH16(PC);
					PC = v12;
#ifdef HAS_I8085
					count -= 7;
#else
					count -= 6;
#endif
					break;
				case 0xc7:	// RST 0
					RST(0);
					break;
				case 0xcf:	// RST 1
					RST(1);
					break;
				case 0xd7:	// RST 2
					RST(2);
					break;
				case 0xdf:	// RST 3
					RST(3);
					break;
				case 0xe7:	// RST 4
					RST(4);
					break;
				case 0xef:	// RST 5
					RST(5);
					break;
				case 0xf7:	// RST 6
					RST(6);
					break;
				case 0xff:	// RST 7
					RST(7);
					break;
				}
				RIM_IEN = 0;
				IM &= ~(IM_IEN | IM_INT);
			}
		}
	}
#ifdef USE_DEBUGGER
	total_count += first_count - count;
#endif
	
}

#define STATE_VERSION	2

bool I8080::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
#ifdef USE_DEBUGGER
	state_fio->StateUint64(total_count);
#endif
	state_fio->StateInt32(count);
	state_fio->StateBuffer(regs, sizeof(regs), 1); // ToDo:
	state_fio->StateUint16(SP);
	state_fio->StateUint16(PC);
	state_fio->StateUint16(prevPC);
	state_fio->StateUint16(IM);
	state_fio->StateUint16(RIM_IEN);
	state_fio->StateBool(afterHALT);
	state_fio->StateBool(BUSREQ);
	state_fio->StateBool(SID);
	state_fio->StateBool(afterEI);
	
#ifdef USE_DEBUGGER
	// post process
	if(loading) {
		prev_total_count = total_count;
	}
#endif
	return true;
}

