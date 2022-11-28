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

#include "vm.h"
#include "../emu.h"
#include "mc6809.h"
#include "mc6809_consts.h"

#ifdef USE_DEBUGGER
#include "debugger.h"
#endif
#include "../config.h"

void MC6809::initialize()
{
	MC6809_BASE::initialize();
	int_state = 0;
	busreq = false;

	if(__USE_DEBUGGER) {
		d_mem_stored = d_mem;
		d_debugger->set_context_mem(d_mem);
	}
}

void MC6809::run_one_opecode()
{
	if(__USE_DEBUGGER) {
		bool now_debugging = d_debugger->now_debugging;
		if(now_debugging) {
			d_debugger->check_break_points(PC);
			if(d_debugger->now_suspended) {
				d_debugger->now_waiting = true;
//#ifdef _MSC_VER
				emu->start_waiting_in_debugger();
//#else
//				osd->start_waiting_in_debugger();
//#endif
				while(d_debugger->now_debugging && d_debugger->now_suspended) {
//#ifdef _MSC_VER
					emu->process_waiting_in_debugger();
//#else
//					osd->process_waiting_in_debugger();
//#endif
				}
//#ifdef _MSC_VER
				emu->finish_waiting_in_debugger();
//#else
//				osd->finish_waiting_in_debugger();
//#endif
				d_debugger->now_waiting = false;
			}
			if(d_debugger->now_debugging) {
				d_mem = d_debugger;
			} else {
				now_debugging = false;
			}
		
			d_debugger->add_cpu_trace(PC);
			int first_icount = icount;
			pPPC = pPC;
			uint8_t ireg = ROP(PCD);
			PC++;
			icount -= cycles1[ireg];
			icount -= extra_icount;
			extra_icount = 0;
			op(ireg);
			total_icount += first_icount - icount;
		
			if(now_debugging) {
				if(!d_debugger->now_going) {
					d_debugger->now_suspended = true;
				}
				d_mem = d_mem_stored;
			}
		} else {
			d_debugger->add_cpu_trace(PC);
			int first_icount = icount;
			pPPC = pPC;
			uint8_t ireg = ROP(PCD);
			PC++;
			icount -= cycles1[ireg];
			icount -= extra_icount;
			extra_icount = 0;
			op(ireg);
			total_icount += first_icount - icount;
		}
	} else {
		pPPC = pPC;
		uint8_t ireg = ROP(PCD);
		PC++;
		icount -= cycles1[ireg];
		icount -= extra_icount;
		extra_icount = 0;
		op(ireg);
	}
}

void MC6809::debugger_hook()
{
	if(__USE_DEBUGGER) {
		bool now_debugging = d_debugger->now_debugging;
		if(now_debugging) {
			d_debugger->check_break_points(PC);
			if(d_debugger->now_suspended) {
				d_debugger->now_waiting = true;
#ifdef _MSC_VER
				emu->start_waiting_in_debugger();
#else
				osd->start_waiting_in_debugger();
#endif
				while(d_debugger->now_debugging && d_debugger->now_suspended) {
#ifdef _MSC_VER
					emu->process_waiting_in_debugger();
#else
					osd->process_waiting_in_debugger();
#endif
				}
#ifdef _MSC_VER
				emu->finish_waiting_in_debugger();
#else
				osd->finish_waiting_in_debugger();
#endif
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
}


// from MAME 0.160


#ifdef USE_DEBUGGER

/*****************************************************************************

    6809dasm.c - a 6809 opcode disassembler
    Version 1.4 1-MAR-95
    Copyright Sean Riddle

    Thanks to Franklin Bowen for bug fixes, ideas

    Freely distributable on any medium given all copyrights are retained
    by the author and no charge greater than $7.00 is made for obtaining
    this software

    Please send all bug reports, update ideas and data files to:
    sriddle@ionet.net

*****************************************************************************/
// Opcode structure
struct opcodeinfo
{
	uint8_t   opcode;     // 8-bit opcode value
	uint8_t   length;     // Opcode length in bytes
	_TCHAR  name[6];    // Opcode name
	uint8_t   mode;       // Addressing mode
//	unsigned flags;     // Disassembly flags
};

enum m6809_addressing_modes
{
	INH,                // Inherent
	DIR,                // Direct
	IND,                // Indexed
	REL,                // Relative (8 bit)
	LREL,               // Long relative (16 bit)
	EXT,                // Extended
	IMM,                // Immediate
	IMM_RR,             // Register-to-register
	PG1,                // Switch to page 1 opcodes
	PG2                 // Switch to page 2 opcodes
};

// Page 0 opcodes (single byte)
static const opcodeinfo m6809_pg0opcodes[] =
{
	{ 0x00, 2, _T("NEG"),   DIR    },
	{ 0x01, 2, _T("NEG"),   DIR    },
	{ 0x02, 2, _T("NGC"),   DIR    },
	{ 0x03, 2, _T("COM"),   DIR    },
	{ 0x04, 2, _T("LSR"),   DIR    },
	{ 0x05, 2, _T("LSR"),   DIR    },
	{ 0x06, 2, _T("ROR"),   DIR    },
	{ 0x07, 2, _T("ASR"),   DIR    },
	{ 0x08, 2, _T("ASL"),   DIR    },
	{ 0x09, 2, _T("ROL"),   DIR    },
	{ 0x0A, 2, _T("DEC"),   DIR    },
	{ 0x0B, 2, _T("DCC"),   DIR    },
	{ 0x0C, 2, _T("INC"),   DIR    },
	{ 0x0D, 2, _T("TST"),   DIR    },
	{ 0x0E, 2, _T("JMP"),   DIR    },
	{ 0x0F, 2, _T("CLR"),   DIR    },

	{ 0x10, 1, _T("page1"), PG1    },
	{ 0x11, 1, _T("page2"), PG2    },
	{ 0x12, 1, _T("NOP"),   INH    },
	{ 0x13, 1, _T("SYNC"),  INH    },
	{ 0x14, 1, _T("HALT"),  INH    },
	{ 0x15, 1, _T("HALT"),  INH    },
	{ 0x16, 3, _T("LBRA"),  LREL   },
	{ 0x17, 3, _T("LBSR"),  LREL   },
	{ 0x18, 1, _T("ASLCC"), INH    },
	{ 0x19, 1, _T("DAA"),   INH    },
	{ 0x1A, 2, _T("ORCC"),  IMM    },
	{ 0x1B, 1, _T("NOP"),   INH    },
	{ 0x1C, 2, _T("ANDCC"), IMM    },
	{ 0x1D, 1, _T("SEX"),   INH    },
	{ 0x1E, 2, _T("EXG"),   IMM_RR },
	{ 0x1F, 2, _T("TFR"),   IMM_RR },

	{ 0x20, 2, _T("BRA"),   REL    },
	{ 0x21, 2, _T("BRN"),   REL    },
	{ 0x22, 2, _T("BHI"),   REL    },
	{ 0x23, 2, _T("BLS"),   REL    },
	{ 0x24, 2, _T("BCC"),   REL    },
	{ 0x25, 2, _T("BCS"),   REL    },
	{ 0x26, 2, _T("BNE"),   REL    },
	{ 0x27, 2, _T("BEQ"),   REL    },
	{ 0x28, 2, _T("BVC"),   REL    },
	{ 0x29, 2, _T("BVS"),   REL    },
	{ 0x2A, 2, _T("BPL"),   REL    },
	{ 0x2B, 2, _T("BMI"),   REL    },
	{ 0x2C, 2, _T("BGE"),   REL    },
	{ 0x2D, 2, _T("BLT"),   REL    },
	{ 0x2E, 2, _T("BGT"),   REL    },
	{ 0x2F, 2, _T("BLE"),   REL    },

	{ 0x30, 2, _T("LEAX"),  IND    },
	{ 0x31, 2, _T("LEAY"),  IND    },
	{ 0x32, 2, _T("LEAS"),  IND    },
	{ 0x33, 2, _T("LEAU"),  IND    },
	{ 0x34, 2, _T("PSHS"),  INH    },
	{ 0x35, 2, _T("PULS"),  INH    },
	{ 0x36, 2, _T("PSHU"),  INH    },
	{ 0x37, 2, _T("PULU"),  INH    },
	{ 0x38, 2, _T("ANDCC"), IMM    },
	{ 0x39, 1, _T("RTS"),   INH    },
	{ 0x3A, 1, _T("ABX"),   INH    },
	{ 0x3B, 1, _T("RTI"),   INH    },
	{ 0x3C, 2, _T("CWAI"),  IMM    },
	{ 0x3D, 1, _T("MUL"),   INH    },
	{ 0x3F, 1, _T("SWI"),   INH    },

	{ 0x40, 1, _T("NEGA"),  INH    },
	{ 0x41, 1, _T("NEGA"),  INH    },
	{ 0x42, 1, _T("NGGA"),  INH    },
	{ 0x43, 1, _T("COMA"),  INH    },
	{ 0x44, 1, _T("LSRA"),  INH    },
	{ 0x45, 1, _T("LSRA"),  INH    },
	{ 0x46, 1, _T("RORA"),  INH    },
	{ 0x47, 1, _T("ASRA"),  INH    },
	{ 0x48, 1, _T("ASLA"),  INH    },
	{ 0x49, 1, _T("ROLA"),  INH    },
	{ 0x4A, 1, _T("DECA"),  INH    },
	{ 0x4B, 1, _T("DCCA"),  INH    },
	{ 0x4C, 1, _T("INCA"),  INH    },
	{ 0x4D, 1, _T("TSTA"),  INH    },
	{ 0x4E, 1, _T("CLCA"),  INH    },
	{ 0x4F, 1, _T("CLRA"),  INH    },

	{ 0x50, 1, _T("NEGB"),  INH    },
	{ 0x51, 1, _T("NEGB"),  INH    },
	{ 0x52, 1, _T("NGGB"),  INH    },
	{ 0x53, 1, _T("COMB"),  INH    },
	{ 0x54, 1, _T("LSRB"),  INH    },
	{ 0x55, 1, _T("LSRB"),  INH    },
	{ 0x56, 1, _T("RORB"),  INH    },
	{ 0x57, 1, _T("ASRB"),  INH    },
	{ 0x58, 1, _T("ASLB"),  INH    },
	{ 0x59, 1, _T("ROLB"),  INH    },
	{ 0x5A, 1, _T("DECB"),  INH    },
	{ 0x5B, 1, _T("DCCB"),  INH    },
	{ 0x5C, 1, _T("INCB"),  INH    },
	{ 0x5D, 1, _T("TSTB"),  INH    },
	{ 0x5E, 1, _T("CLCB"),  INH    },
	{ 0x5F, 1, _T("CLRB"),  INH    },

	{ 0x60, 2, _T("NEG"),   IND    },
	{ 0x61, 2, _T("NEG"),   IND    },
	{ 0x62, 2, _T("NGC"),   IND    },
	{ 0x63, 2, _T("COM"),   IND    },
	{ 0x64, 2, _T("LSR"),   IND    },
	{ 0x65, 2, _T("LSR"),   IND    },
	{ 0x66, 2, _T("ROR"),   IND    },
	{ 0x67, 2, _T("ASR"),   IND    },
	{ 0x68, 2, _T("ASL"),   IND    },
	{ 0x69, 2, _T("ROL"),   IND    },
	{ 0x6A, 2, _T("DEC"),   IND    },
	{ 0x6B, 2, _T("DCC"),   IND    },
	{ 0x6C, 2, _T("INC"),   IND    },
	{ 0x6D, 2, _T("TST"),   IND    },
	{ 0x6E, 2, _T("JMP"),   IND    },
	{ 0x6F, 2, _T("CLR"),   IND    },

	{ 0x70, 3, _T("NEG"),   EXT    },
	{ 0x71, 3, _T("NEG"),   EXT    },
	{ 0x72, 3, _T("NGC"),   EXT    },
	{ 0x73, 3, _T("COM"),   EXT    },
	{ 0x74, 3, _T("LSR"),   EXT    },
	{ 0x75, 3, _T("LSR"),   EXT    },
	{ 0x76, 3, _T("ROR"),   EXT    },
	{ 0x77, 3, _T("ASR"),   EXT    },
	{ 0x78, 3, _T("ASL"),   EXT    },
	{ 0x79, 3, _T("ROL"),   EXT    },
	{ 0x7A, 3, _T("DEC"),   EXT    },
	{ 0x7B, 3, _T("DCC"),   EXT    },
	{ 0x7C, 3, _T("INC"),   EXT    },
	{ 0x7D, 3, _T("TST"),   EXT    },
	{ 0x7E, 3, _T("JMP"),   EXT    },
	{ 0x7F, 3, _T("CLR"),   EXT    },

	{ 0x80, 2, _T("SUBA"),  IMM    },
	{ 0x81, 2, _T("CMPA"),  IMM    },
	{ 0x82, 2, _T("SBCA"),  IMM    },
	{ 0x83, 3, _T("SUBD"),  IMM    },
	{ 0x84, 2, _T("ANDA"),  IMM    },
	{ 0x85, 2, _T("BITA"),  IMM    },
	{ 0x86, 2, _T("LDA"),   IMM    },
	{ 0x87, 2, _T("FLAG"),  IMM    },
	{ 0x88, 2, _T("EORA"),  IMM    },
	{ 0x89, 2, _T("ADCA"),  IMM    },
	{ 0x8A, 2, _T("ORA"),   IMM    },
	{ 0x8B, 2, _T("ADDA"),  IMM    },
	{ 0x8C, 3, _T("CMPX"),  IMM    },
	{ 0x8D, 2, _T("BSR"),   REL    },
	{ 0x8E, 3, _T("LDX"),   IMM    },
	{ 0x8F, 3, _T("FLAG"),  IMM    },

	{ 0x90, 2, _T("SUBA"),  DIR    },
	{ 0x91, 2, _T("CMPA"),  DIR    },
	{ 0x92, 2, _T("SBCA"),  DIR    },
	{ 0x93, 2, _T("SUBD"),  DIR    },
	{ 0x94, 2, _T("ANDA"),  DIR    },
	{ 0x95, 2, _T("BITA"),  DIR    },
	{ 0x96, 2, _T("LDA"),   DIR    },
	{ 0x97, 2, _T("STA"),   DIR    },
	{ 0x98, 2, _T("EORA"),  DIR    },
	{ 0x99, 2, _T("ADCA"),  DIR    },
	{ 0x9A, 2, _T("ORA"),   DIR    },
	{ 0x9B, 2, _T("ADDA"),  DIR    },
	{ 0x9C, 2, _T("CMPX"),  DIR    },
	{ 0x9D, 2, _T("JSR"),   DIR    },
	{ 0x9E, 2, _T("LDX"),   DIR    },
	{ 0x9F, 2, _T("STX"),   DIR    },

	{ 0xA0, 2, _T("SUBA"),  IND    },
	{ 0xA1, 2, _T("CMPA"),  IND    },
	{ 0xA2, 2, _T("SBCA"),  IND    },
	{ 0xA3, 2, _T("SUBD"),  IND    },
	{ 0xA4, 2, _T("ANDA"),  IND    },
	{ 0xA5, 2, _T("BITA"),  IND    },
	{ 0xA6, 2, _T("LDA"),   IND    },
	{ 0xA7, 2, _T("STA"),   IND    },
	{ 0xA8, 2, _T("EORA"),  IND    },
	{ 0xA9, 2, _T("ADCA"),  IND    },
	{ 0xAA, 2, _T("ORA"),   IND    },
	{ 0xAB, 2, _T("ADDA"),  IND    },
	{ 0xAC, 2, _T("CMPX"),  IND    },
	{ 0xAD, 2, _T("JSR"),   IND    },
	{ 0xAE, 2, _T("LDX"),   IND    },
	{ 0xAF, 2, _T("STX"),   IND    },

	{ 0xB0, 3, _T("SUBA"),  EXT    },
	{ 0xB1, 3, _T("CMPA"),  EXT    },
	{ 0xB2, 3, _T("SBCA"),  EXT    },
	{ 0xB3, 3, _T("SUBD"),  EXT    },
	{ 0xB4, 3, _T("ANDA"),  EXT    },
	{ 0xB5, 3, _T("BITA"),  EXT    },
	{ 0xB6, 3, _T("LDA"),   EXT    },
	{ 0xB7, 3, _T("STA"),   EXT    },
	{ 0xB8, 3, _T("EORA"),  EXT    },
	{ 0xB9, 3, _T("ADCA"),  EXT    },
	{ 0xBA, 3, _T("ORA"),   EXT    },
	{ 0xBB, 3, _T("ADDA"),  EXT    },
	{ 0xBC, 3, _T("CMPX"),  EXT    },
	{ 0xBD, 3, _T("JSR"),   EXT    },
	{ 0xBE, 3, _T("LDX"),   EXT    },
	{ 0xBF, 3, _T("STX"),   EXT    },

	{ 0xC0, 2, _T("SUBB"),  IMM    },
	{ 0xC1, 2, _T("CMPB"),  IMM    },
	{ 0xC2, 2, _T("SBCB"),  IMM    },
	{ 0xC3, 3, _T("ADDD"),  IMM    },
	{ 0xC4, 2, _T("ANDB"),  IMM    },
	{ 0xC5, 2, _T("BITB"),  IMM    },
	{ 0xC6, 2, _T("LDB"),   IMM    },
	{ 0xC7, 2, _T("FLAG"),  IMM    },
	{ 0xC8, 2, _T("EORB"),  IMM    },
	{ 0xC9, 2, _T("ADCB"),  IMM    },
	{ 0xCA, 2, _T("ORB"),   IMM    },
	{ 0xCB, 2, _T("ADDB"),  IMM    },
	{ 0xCC, 3, _T("LDD"),   IMM    },
	{ 0xCD, 1, _T("HALT"),  INH    },
	{ 0xCE, 3, _T("LDU"),   IMM    },
	{ 0xCF, 3, _T("FLAG"),  IMM    },

	{ 0xD0, 2, _T("SUBB"),  DIR    },
	{ 0xD1, 2, _T("CMPB"),  DIR    },
	{ 0xD2, 2, _T("SBCB"),  DIR    },
	{ 0xD3, 2, _T("ADDD"),  DIR    },
	{ 0xD4, 2, _T("ANDB"),  DIR    },
	{ 0xD5, 2, _T("BITB"),  DIR    },
	{ 0xD6, 2, _T("LDB"),   DIR    },
	{ 0xD7, 2, _T("STB"),   DIR    },
	{ 0xD8, 2, _T("EORB"),  DIR    },
	{ 0xD9, 2, _T("ADCB"),  DIR    },
	{ 0xDA, 2, _T("ORB"),   DIR    },
	{ 0xDB, 2, _T("ADDB"),  DIR    },
	{ 0xDC, 2, _T("LDD"),   DIR    },
	{ 0xDD, 2, _T("STD"),   DIR    },
	{ 0xDE, 2, _T("LDU"),   DIR    },
	{ 0xDF, 2, _T("STU"),   DIR    },

	{ 0xE0, 2, _T("SUBB"),  IND    },
	{ 0xE1, 2, _T("CMPB"),  IND    },
	{ 0xE2, 2, _T("SBCB"),  IND    },
	{ 0xE3, 2, _T("ADDD"),  IND    },
	{ 0xE4, 2, _T("ANDB"),  IND    },
	{ 0xE5, 2, _T("BITB"),  IND    },
	{ 0xE6, 2, _T("LDB"),   IND    },
	{ 0xE7, 2, _T("STB"),   IND    },
	{ 0xE8, 2, _T("EORB"),  IND    },
	{ 0xE9, 2, _T("ADCB"),  IND    },
	{ 0xEA, 2, _T("ORB"),   IND    },
	{ 0xEB, 2, _T("ADDB"),  IND    },
	{ 0xEC, 2, _T("LDD"),   IND    },
	{ 0xED, 2, _T("STD"),   IND    },
	{ 0xEE, 2, _T("LDU"),   IND    },
	{ 0xEF, 2, _T("STU"),   IND    },

	{ 0xF0, 3, _T("SUBB"),  EXT    },
	{ 0xF1, 3, _T("CMPB"),  EXT    },
	{ 0xF2, 3, _T("SBCB"),  EXT    },
	{ 0xF3, 3, _T("ADDD"),  EXT    },
	{ 0xF4, 3, _T("ANDB"),  EXT    },
	{ 0xF5, 3, _T("BITB"),  EXT    },
	{ 0xF6, 3, _T("LDB"),   EXT    },
	{ 0xF7, 3, _T("STB"),   EXT    },
	{ 0xF8, 3, _T("EORB"),  EXT    },
	{ 0xF9, 3, _T("ADCB"),  EXT    },
	{ 0xFA, 3, _T("ORB"),   EXT    },
	{ 0xFB, 3, _T("ADDB"),  EXT    },
	{ 0xFC, 3, _T("LDD"),   EXT    },
	{ 0xFD, 3, _T("STD"),   EXT    },
	{ 0xFE, 3, _T("LDU"),   EXT    },
	{ 0xFF, 3, _T("STU"),   EXT    }
};

// Page 1 opcodes (0x10 0x..)
static const opcodeinfo m6809_pg1opcodes[] =
{
	{ 0x20, 4, _T("LBRA"),  LREL   },
	{ 0x21, 4, _T("LBRN"),  LREL   },
	{ 0x22, 4, _T("LBHI"),  LREL   },
	{ 0x23, 4, _T("LBLS"),  LREL   },
	{ 0x24, 4, _T("LBCC"),  LREL   },
	{ 0x25, 4, _T("LBCS"),  LREL   },
	{ 0x26, 4, _T("LBNE"),  LREL   },
	{ 0x27, 4, _T("LBEQ"),  LREL   },
	{ 0x28, 4, _T("LBVC"),  LREL   },
	{ 0x29, 4, _T("LBVS"),  LREL   },
	{ 0x2A, 4, _T("LBPL"),  LREL   },
	{ 0x2B, 4, _T("LBMI"),  LREL   },
	{ 0x2C, 4, _T("LBGE"),  LREL   },
	{ 0x2D, 4, _T("LBLT"),  LREL   },
	{ 0x2E, 4, _T("LBGT"),  LREL   },
	{ 0x2F, 4, _T("LBLE"),  LREL   },
	{ 0x3F, 2, _T("SWI2"),  INH    },
	{ 0x83, 4, _T("CMPD"),  IMM    },
	{ 0x8C, 4, _T("CMPY"),  IMM    },
	{ 0x8D, 4, _T("LBSR"),  LREL   },
	{ 0x8E, 4, _T("LDY"),   IMM    },
	{ 0x93, 3, _T("CMPD"),  DIR    },
	{ 0x9C, 3, _T("CMPY"),  DIR    },
	{ 0x9E, 3, _T("LDY"),   DIR    },
	{ 0x9F, 3, _T("STY"),   DIR    },
	{ 0xA3, 3, _T("CMPD"),  IND    },
	{ 0xAC, 3, _T("CMPY"),  IND    },
	{ 0xAE, 3, _T("LDY"),   IND    },
	{ 0xAF, 3, _T("STY"),   IND    },
	{ 0xB3, 4, _T("CMPD"),  EXT    },
	{ 0xBC, 4, _T("CMPY"),  EXT    },
	{ 0xBE, 4, _T("LDY"),   EXT    },
	{ 0xBF, 4, _T("STY"),   EXT    },
	{ 0xCE, 4, _T("LDS"),   IMM    },
	{ 0xDE, 3, _T("LDS"),   DIR    },
	{ 0xDF, 3, _T("STS"),   DIR    },
	{ 0xEE, 3, _T("LDS"),   IND    },
	{ 0xEF, 3, _T("STS"),   IND    },
	{ 0xFE, 4, _T("LDS"),   EXT    },
	{ 0xFF, 4, _T("STS"),   EXT    }
};

// Page 2 opcodes (0x11 0x..)
static const opcodeinfo m6809_pg2opcodes[] =
{
	{ 0x3F, 2, _T("SWI3"),  INH    },
	{ 0x83, 4, _T("CMPU"),  IMM    },
	{ 0x8C, 4, _T("CMPS"),  IMM    },
	{ 0x93, 3, _T("CMPU"),  DIR    },
	{ 0x9C, 3, _T("CMPS"),  DIR    },
	{ 0xA3, 3, _T("CMPU"),  IND    },
	{ 0xAC, 3, _T("CMPS"),  IND    },
	{ 0xB3, 4, _T("CMPU"),  EXT    },
	{ 0xBC, 4, _T("CMPS"),  EXT    }
};

static const opcodeinfo *const m6809_pgpointers[3] =
{
	m6809_pg0opcodes, m6809_pg1opcodes, m6809_pg2opcodes
};

static const int m6809_numops[3] =
{
	array_length(m6809_pg0opcodes),
	array_length(m6809_pg1opcodes),
	array_length(m6809_pg2opcodes)
};

static const _TCHAR *const m6809_regs[5] = { _T("X"), _T("Y"), _T("U"), _T("S"), _T("PC") };

static const _TCHAR *const m6809_regs_te[16] =
{
	_T("D"), _T("X"),  _T("Y"),  _T("U"),   _T("S"),  _T("PC"), _T("inv"), _T("inv"),
	_T("A"), _T("B"), _T("CC"), _T("DP"), _T("inv"), _T("inv"), _T("inv"), _T("inv")
};
#endif /* USE_DEBUGGER */

uint32_t MC6809::cpu_disassemble_m6809(_TCHAR *buffer, uint32_t pc, const uint8_t *oprom, const uint8_t *opram)
{
#ifdef USE_DEBUGGER
	uint8_t opcode, mode, pb, pbm, reg;
	const uint8_t *operandarray;
	unsigned int ea;//, flags;
	int numoperands, offset;
	int i, p = 0, page = 0;
	bool opcode_found = false;
	bool indirect;
	const _TCHAR *name = NULL;

	do {
		opcode = oprom[p++];

		for (i = 0; i < m6809_numops[page]; i++)
			if (m6809_pgpointers[page][i].opcode == opcode)
				break;

		if (i < m6809_numops[page])
			opcode_found = true;
		else
		{
			_stprintf(buffer, _T("Illegal Opcode %02X"), opcode);
			return p;
		}

		if (m6809_pgpointers[page][i].mode >= PG1)
		{
			page = m6809_pgpointers[page][i].mode - PG1 + 1;
			opcode_found = false;
		}
	} while (!opcode_found);

	if (page == 0)
		numoperands = m6809_pgpointers[page][i].length - 1;
	else
		numoperands = m6809_pgpointers[page][i].length - 2;

	operandarray = &opram[p];
	p += numoperands;
	pc += p;
	mode = m6809_pgpointers[page][i].mode;
//	flags = m6809_pgpointers[page][i].flags;

	buffer += _stprintf(buffer, _T("%-6s"), m6809_pgpointers[page][i].name);

	switch (mode)
	{
	case INH:
		switch (opcode)
		{
		case 0x34:  // PSHS
		case 0x36:  // PSHU
			pb = operandarray[0];
			if (pb & 0x80)
				buffer += _stprintf(buffer, _T("PC"));
			if (pb & 0x40)
				buffer += _stprintf(buffer, _T("%s%s"), (pb&0x80)?_T(","):_T(""), (opcode==0x34)?"U":"S");
			if (pb & 0x20)
				buffer += _stprintf(buffer, _T("%sY"),  (pb&0xc0)?_T(","):_T(""));
			if (pb & 0x10)
				buffer += _stprintf(buffer, _T("%sX"),  (pb&0xe0)?_T(","):_T(""));
			if (pb & 0x08)
				buffer += _stprintf(buffer, _T("%sDP"), (pb&0xf0)?_T(","):_T(""));
			if (pb & 0x04)
				buffer += _stprintf(buffer, _T("%sB"),  (pb&0xf8)?_T(","):_T(""));
			if (pb & 0x02)
				buffer += _stprintf(buffer, _T("%sA"),  (pb&0xfc)?_T(","):_T(""));
			if (pb & 0x01)
				buffer += _stprintf(buffer, _T("%sCC"), (pb&0xfe)?_T(","):_T(""));
			break;
		case 0x35:  // PULS
		case 0x37:  // PULU
			pb = operandarray[0];
			if (pb & 0x01)
				buffer += _stprintf(buffer, _T("CC"));
			if (pb & 0x02)
				buffer += _stprintf(buffer, _T("%sA"),  (pb&0x01)?_T(","):_T(""));
			if (pb & 0x04)
				buffer += _stprintf(buffer, _T("%sB"),  (pb&0x03)?_T(","):_T(""));
			if (pb & 0x08)
				buffer += _stprintf(buffer, _T("%sDP"), (pb&0x07)?_T(","):_T(""));
			if (pb & 0x10)
				buffer += _stprintf(buffer, _T("%sX"),  (pb&0x0f)?_T(","):_T(""));
			if (pb & 0x20)
				buffer += _stprintf(buffer, _T("%sY"),  (pb&0x1f)?_T(","):_T(""));
			if (pb & 0x40)
				buffer += _stprintf(buffer, _T("%s%s"), (pb&0x3f)?_T(","):_T(""), (opcode==0x35)?_T("U"):_T("S"));
			if (pb & 0x80)
				buffer += _stprintf(buffer, _T("%sPC ; (PUL? PC=RTS)"), (pb&0x7f)?_T(","):_T(""));
			break;
		default:
			// No operands
			break;
		}
		break;

	case DIR:
		ea = operandarray[0];
		buffer += _stprintf(buffer, _T("$%02X"), ea);
		break;

	case REL:
		offset = (int8_t)operandarray[0];
		buffer += _stprintf(buffer, _T("%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04X"), (pc + offset) & 0xffff));
		break;

	case LREL:
		offset = (int16_t)((operandarray[0] << 8) + operandarray[1]);
		buffer += _stprintf(buffer, _T("%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04X"), (pc + offset) & 0xffff));
		break;

	case EXT:
		ea = (operandarray[0] << 8) + operandarray[1];
		buffer += _stprintf(buffer, _T("%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04X"), ea));
		break;

	case IND:
		pb = operandarray[0];
		reg = (pb >> 5) & 3;
		pbm = pb & 0x8f;
		indirect = ((pb & 0x90) == 0x90 )? true : false;

		// open brackets if indirect
		if (indirect && pbm != 0x80 && pbm != 0x82)
			buffer += _stprintf(buffer, _T("["));

		switch (pbm)
		{
		case 0x80:  // ,R+
			if (indirect)
				_tcscpy(buffer, _T("Illegal Postbyte"));
			else
				buffer += _stprintf(buffer, _T(",%s+"), m6809_regs[reg]);
			break;

		case 0x81:  // ,R++
			buffer += _stprintf(buffer, _T(",%s++"), m6809_regs[reg]);
			break;

		case 0x82:  // ,-R
		  //if (indirect)
		  //	_tcscpy(buffer, _T("Illegal Postbyte"));
		  //	else
				buffer += _stprintf(buffer, _T(",-%s"), m6809_regs[reg]);
			break;

		case 0x83:  // ,--R
			buffer += _stprintf(buffer, _T(",--%s"), m6809_regs[reg]);
			break;

		case 0x84:  // ,R
			buffer += _stprintf(buffer, _T(",%s"), m6809_regs[reg]);
			break;

		case 0x85:  // (+/- B),R
			buffer += _stprintf(buffer, _T("B,%s"), m6809_regs[reg]);
			break;

		case 0x86:  // (+/- A),R
			buffer += _stprintf(buffer, _T("A,%s"), m6809_regs[reg]);
			break;

		case 0x87:  // (+/- A),R // Also 0x*6.
			buffer += _stprintf(buffer, _T("A,%s"), m6809_regs[reg]);
			break;
			//case 0x87:
			//_tcscpy(buffer, _T("Illegal Postbyte"));
			//break;

		case 0x88:  // (+/- 7 bit offset),R
			offset = (int8_t)opram[p++];
			buffer += _stprintf(buffer, _T("%s"), (offset < 0) ? "-" : "");
			buffer += _stprintf(buffer, _T("$%02X,"), (offset < 0) ? -offset : offset);
			buffer += _stprintf(buffer, _T("%s"), m6809_regs[reg]);
			break;

		case 0x89:  // (+/- 15 bit offset),R
			offset = (int16_t)((opram[p+0] << 8) + opram[p+1]);
			p += 2;
			buffer += _stprintf(buffer, _T("%s"), (offset < 0) ? "-" : "");
			buffer += _stprintf(buffer, _T("$%04X,"), (offset < 0) ? -offset : offset);
			buffer += _stprintf(buffer, _T("%s"), m6809_regs[reg]);
			break;

		case 0x8a:
			_tcscpy(buffer, _T("Illegal Postbyte"));
			break;

		case 0x8b:  // (+/- D),R
			buffer += _stprintf(buffer, _T("D,%s"), m6809_regs[reg]);
			break;

		case 0x8c:  // (+/- 7 bit offset),PC
			offset = (int8_t)opram[p++];
			if((name = get_symbol(d_debugger->first_symbol, (p - 1 + offset) & 0xffff)) != NULL) {
				buffer += _stprintf(buffer, _T("%s,PCR"), name);
			} else {
				buffer += _stprintf(buffer, _T("%s"), (offset < 0) ? "-" : "");
				buffer += _stprintf(buffer, _T("$%02X,PC"), (offset < 0) ? -offset : offset);
			}
			break;

		case 0x8d:  // (+/- 15 bit offset),PC
			offset = (int16_t)((opram[p+0] << 8) + opram[p+1]);
			p += 2;
			if((name = get_symbol(d_debugger->first_symbol, (p - 2 + offset) & 0xffff)) != NULL) {
				buffer += _stprintf(buffer, _T("%s,PCR"), name);
			} else {
				buffer += _stprintf(buffer, _T("%s"), (offset < 0) ? "-" : "");
				buffer += _stprintf(buffer, _T("$%04X,PC"), (offset < 0) ? -offset : offset);
			}
			break;

		case 0x8e: // $FFFFF
		  //_tcscpy(buffer, _T("Illegal Postbyte"));
			offset = (int16_t)0xffff;
			//p += 2;
			buffer += _stprintf(buffer, _T("$%04X"), offset);
			break;

		case 0x8f:  // address
			ea = (uint16_t)((opram[p+0] << 8) + opram[p+1]);
			p += 2;
			buffer += _stprintf(buffer, _T("%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04X"), ea));
			break;

		default:    // (+/- 4 bit offset),R
			offset = pb & 0x1f;
			if (offset > 15)
				offset = offset - 32;
			buffer += _stprintf(buffer, _T("%s"), (offset < 0) ? "-" : "");
			buffer += _stprintf(buffer, _T("$%X,"), (offset < 0) ? -offset : offset);
			buffer += _stprintf(buffer, _T("%s"), m6809_regs[reg]);
			break;
		}

		// close brackets if indirect
		if (indirect && pbm != 0x80 && pbm != 0x82)
			buffer += _stprintf(buffer, _T("]"));
		break;

	case IMM:
		if (numoperands == 2)
		{
			ea = (operandarray[0] << 8) + operandarray[1];
			buffer += _stprintf(buffer, _T("#%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04X"), ea));
		}
		else
		if (numoperands == 1)
		{
			ea = operandarray[0];
			buffer += _stprintf(buffer, _T("#$%02X"), ea);
		}
		break;

	case IMM_RR:
		pb = operandarray[0];
		buffer += _stprintf(buffer, _T("%s,%s"), m6809_regs_te[(pb >> 4) & 0xf], m6809_regs_te[pb & 0xf]);
		break;
	}

	return p;
#else
	return 0;
#endif
}

int MC6809::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	if(__USE_DEBUGGER) {
		_TCHAR buffer_tmp[1024]; // enough ???
		uint8_t ops[4];
		for(int i = 0; i < 4; i++) {
			ops[i] = d_mem_stored->read_data8(pc + i);
		}
		int length = cpu_disassemble_m6809(buffer_tmp, pc, ops, ops);
		my_tcscpy_s(buffer, buffer_len, buffer_tmp);
		return length;
	}
	return 0;
}


