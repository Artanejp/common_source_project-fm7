/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date  : 2011.04.23-

	[ MB8861 ]
*/
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4996 )
#endif

#include "mb8861.h"
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
#include "../fifo.h"
//#endif
//#ifdef USE_DEBUGGER
#include "debugger.h"
#include "mc6800_consts.h"


int MB8861::debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata )
{
	uint8_t ops[4];
	if(d_mem_stored != NULL) {
		for(int i = 0; i < 4; i++) {
			int wait;
			ops[i] = d_mem_stored->read_data8w(pc + i, &wait);
		}
	}
	return Dasm680x(6800, buffer, pc, ops, ops, d_debugger->first_symbol);	// FIXME
	return 0;
}

void MB8861::insn(uint8_t code)
{
	switch(code) {
	case 0x00: illegal(); break;
	case 0x01: nop(); break;
	case 0x02: illegal(); break;
	case 0x03: illegal(); break;
	case 0x04: illegal(); break;
	case 0x05: illegal(); break;
	case 0x06: tap(); break;
	case 0x07: tpa(); break;
	case 0x08: inx(); break;
	case 0x09: dex(); break;
	case 0x0a: clv(); break;
	case 0x0b: sev(); break;
	case 0x0c: clc(); break;
	case 0x0d: sec(); break;
	case 0x0e: cli(); break;
	case 0x0f: sei(); break;
	case 0x10: sba(); break;
	case 0x11: cba(); break;
	case 0x12: illegal(); break;
	case 0x13: illegal(); break;
	case 0x14: illegal(); break;
	case 0x15: illegal(); break;
	case 0x16: tab(); break;
	case 0x17: tba(); break;
	case 0x18: illegal(); break;
	case 0x19: daa(); break;
	case 0x1a: illegal(); break;
	case 0x1b: aba(); break;
	case 0x1c: illegal(); break;
	case 0x1d: illegal(); break;
	case 0x1e: illegal(); break;
	case 0x1f: illegal(); break;
	case 0x20: bra(); break;
	case 0x21: brn(); break;
	case 0x22: bhi(); break;
	case 0x23: bls(); break;
	case 0x24: bcc(); break;
	case 0x25: bcs(); break;
	case 0x26: bne(); break;
	case 0x27: beq(); break;
	case 0x28: bvc(); break;
	case 0x29: bvs(); break;
	case 0x2a: bpl(); break;
	case 0x2b: bmi(); break;
	case 0x2c: bge(); break;
	case 0x2d: blt(); break;
	case 0x2e: bgt(); break;
	case 0x2f: ble(); break;
	case 0x30: tsx(); break;
	case 0x31: ins(); break;
	case 0x32: pula(); break;
	case 0x33: pulb(); break;
	case 0x34: des(); break;
	case 0x35: txs(); break;
	case 0x36: psha(); break;
	case 0x37: pshb(); break;
	case 0x38: illegal(); break;
	case 0x39: rts(); break;
	case 0x3a: illegal(); break;
	case 0x3b: rti(); break;
	case 0x3c: illegal(); break;
	case 0x3d: illegal(); break;
	case 0x3e: wai(); break;
	case 0x3f: swi(); break;
	case 0x40: nega(); break;
	case 0x41: illegal(); break;
	case 0x42: illegal(); break;
	case 0x43: coma(); break;
	case 0x44: lsra(); break;
	case 0x45: illegal(); break;
	case 0x46: rora(); break;
	case 0x47: asra(); break;
	case 0x48: asla(); break;
	case 0x49: rola(); break;
	case 0x4a: deca(); break;
	case 0x4b: illegal(); break;
	case 0x4c: inca(); break;
	case 0x4d: tsta(); break;
	case 0x4e: illegal(); break;
	case 0x4f: clra(); break;
	case 0x50: negb(); break;
	case 0x51: illegal(); break;
	case 0x52: illegal(); break;
	case 0x53: comb(); break;
	case 0x54: lsrb(); break;
	case 0x55: illegal(); break;
	case 0x56: rorb(); break;
	case 0x57: asrb(); break;
	case 0x58: aslb(); break;
	case 0x59: rolb(); break;
	case 0x5a: decb(); break;
	case 0x5b: illegal(); break;
	case 0x5c: incb(); break;
	case 0x5d: tstb(); break;
	case 0x5e: illegal(); break;
	case 0x5f: clrb(); break;
	case 0x60: neg_ix(); break;
	case 0x61: illegal(); break;
	case 0x62: illegal(); break;
	case 0x63: com_ix(); break;
	case 0x64: lsr_ix(); break;
	case 0x65: illegal(); break;

	case 0x66: ror_ix(); break;
	case 0x67: asr_ix(); break;
	case 0x68: asl_ix(); break;
	case 0x69: rol_ix(); break;
	case 0x6a: dec_ix(); break;
	case 0x6b: illegal(); break;

	case 0x6c: inc_ix(); break;
	case 0x6d: tst_ix(); break;
	case 0x6e: jmp_ix(); break;
	case 0x6f: clr_ix(); break;
	case 0x70: neg_ex(); break;
		
	case 0x71: nim_ix(); break;
	case 0x72: oim_ix_mb8861(); break;
		
	case 0x73: com_ex(); break;
	case 0x74: lsr_ex(); break;
		
	case 0x75: xim_ix(); break;
		
	case 0x76: ror_ex(); break;
	case 0x77: asr_ex(); break;
	case 0x78: asl_ex(); break;
	case 0x79: rol_ex(); break;
	case 0x7a: dec_ex(); break;
		
	case 0x7b: tmm_ix(); break;
		
	case 0x7c: inc_ex(); break;
	case 0x7d: tst_ex(); break;
	case 0x7e: jmp_ex(); break;
	case 0x7f: clr_ex(); break;
	case 0x80: suba_im(); break;
	case 0x81: cmpa_im(); break;
	case 0x82: sbca_im(); break;
	case 0x83: illegal(); break;
	case 0x84: anda_im(); break;
	case 0x85: bita_im(); break;
	case 0x86: lda_im(); break;
	case 0x87: sta_im(); break;
	case 0x88: eora_im(); break;
	case 0x89: adca_im(); break;
	case 0x8a: ora_im(); break;
	case 0x8b: adda_im(); break;
	case 0x8c: cmpx_im(); break;
	case 0x8d: bsr(); break;
	case 0x8e: lds_im(); break;
	case 0x8f: sts_im(); break;
	case 0x90: suba_di(); break;
	case 0x91: cmpa_di(); break;
	case 0x92: sbca_di(); break;
	case 0x93: illegal(); break;
	case 0x94: anda_di(); break;
	case 0x95: bita_di(); break;
	case 0x96: lda_di(); break;
	case 0x97: sta_di(); break;
	case 0x98: eora_di(); break;
	case 0x99: adca_di(); break;
	case 0x9a: ora_di(); break;
	case 0x9b: adda_di(); break;
	case 0x9c: cmpx_di(); break;
	case 0x9d: jsr_di(); break;
	case 0x9e: lds_di(); break;
	case 0x9f: sts_di(); break;
	case 0xa0: suba_ix(); break;
	case 0xa1: cmpa_ix(); break;
	case 0xa2: sbca_ix(); break;
	case 0xa3: illegal(); break;
	case 0xa4: anda_ix(); break;
	case 0xa5: bita_ix(); break;
	case 0xa6: lda_ix(); break;
	case 0xa7: sta_ix(); break;
	case 0xa8: eora_ix(); break;
	case 0xa9: adca_ix(); break;
	case 0xaa: ora_ix(); break;
	case 0xab: adda_ix(); break;
	case 0xac: cmpx_ix(); break;
	case 0xad: jsr_ix(); break;
	case 0xae: lds_ix(); break;
	case 0xaf: sts_ix(); break;
	case 0xb0: suba_ex(); break;
	case 0xb1: cmpa_ex(); break;
	case 0xb2: sbca_ex(); break;
	case 0xb3: illegal(); break;
	case 0xb4: anda_ex(); break;
	case 0xb5: bita_ex(); break;
	case 0xb6: lda_ex(); break;
	case 0xb7: sta_ex(); break;
	case 0xb8: eora_ex(); break;
	case 0xb9: adca_ex(); break;
	case 0xba: ora_ex(); break;
	case 0xbb: adda_ex(); break;
	case 0xbc: cmpx_ex(); break;
	case 0xbd: jsr_ex(); break;
	case 0xbe: lds_ex(); break;
	case 0xbf: sts_ex(); break;
	case 0xc0: subb_im(); break;
	case 0xc1: cmpb_im(); break;
	case 0xc2: sbcb_im(); break;
	case 0xc3: illegal(); break;
	case 0xc4: andb_im(); break;
	case 0xc5: bitb_im(); break;
	case 0xc6: ldb_im(); break;
	case 0xc7: stb_im(); break;
	case 0xc8: eorb_im(); break;
	case 0xc9: adcb_im(); break;
	case 0xca: orb_im(); break;
	case 0xcb: addb_im(); break;
	case 0xcc: illegal(); break;
	case 0xcd: illegal(); break;
	case 0xce: ldx_im(); break;
	case 0xcf: stx_im(); break;
	case 0xd0: subb_di(); break;
	case 0xd1: cmpb_di(); break;
	case 0xd2: sbcb_di(); break;
	case 0xd3: illegal(); break;
	case 0xd4: andb_di(); break;
	case 0xd5: bitb_di(); break;
	case 0xd6: ldb_di(); break;
	case 0xd7: stb_di(); break;
	case 0xd8: eorb_di(); break;
	case 0xd9: adcb_di(); break;
	case 0xda: orb_di(); break;
	case 0xdb: addb_di(); break;
	case 0xdc: illegal(); break;
	case 0xdd: illegal(); break;
	case 0xde: ldx_di(); break;
	case 0xdf: stx_di(); break;
	case 0xe0: subb_ix(); break;
	case 0xe1: cmpb_ix(); break;
	case 0xe2: sbcb_ix(); break;
	case 0xe3: illegal(); break;
	case 0xe4: andb_ix(); break;
	case 0xe5: bitb_ix(); break;
	case 0xe6: ldb_ix(); break;
	case 0xe7: stb_ix(); break;
	case 0xe8: eorb_ix(); break;
	case 0xe9: adcb_ix(); break;
	case 0xea: orb_ix(); break;
	case 0xeb: addb_ix(); break;
		
	case 0xec: adx_im(); break;
		
	case 0xed: illegal(); break;
	case 0xee: ldx_ix(); break;
	case 0xef: stx_ix(); break;
	case 0xf0: subb_ex(); break;
	case 0xf1: cmpb_ex(); break;
	case 0xf2: sbcb_ex(); break;
	case 0xf3: illegal(); break;
	case 0xf4: andb_ex(); break;
	case 0xf5: bitb_ex(); break;
	case 0xf6: ldb_ex(); break;
	case 0xf7: stb_ex(); break;
	case 0xf8: eorb_ex(); break;
	case 0xf9: adcb_ex(); break;
	case 0xfa: orb_ex(); break;
	case 0xfb: addb_ex(); break;
		
	case 0xfc: adx_ex(); break;
		
	case 0xfd: illegal(); break;
	case 0xfe: ldx_ex(); break;
	case 0xff: stx_ex(); break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}

/* operate one instruction for */
#define ONE_MORE_INSN() { \
	uint8_t ireg = M_RDOP(PCD); \
	prevpc = PC; \
	PC++; \
	insn(ireg); \
	increment_counter(cycles[ireg]); \
}

/* $71 NIM --**0- */ /* MB8861 only */
void MB8861::nim_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r &= t;
	CLR_NZV;
	if(!r) {
		SEZ;
	} else {
		SEN;
	}
	WM(EAD, r);
}


/* $72 OIM --**0- */ /* MB8861 only */
void MB8861::oim_ix_mb8861()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r |= t;
	CLR_NZV;
	if(!r) {
		SEZ;
	} else {
		SEN;
	}
	WM(EAD, r);
}

/* $75 XIM --**-- */ /* MB8861 only */
void MB8861::xim_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r ^= t;
	CLR_NZ;
	if(!r) {
		SEZ;
	} else {
		SEN;
	}
	WM(EAD, r);
}

/* $7b TMM --***- */ /* MB8861 only */
void MB8861::tmm_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r &= t;
	CLR_NZV;
	if(!t || !r) {
		SEZ;
	} else if(r == t) {
		SEV;
	} else {
		SEN;
	}
}

/* $ec ADX immediate -**** */ /* MB8861 only */
void MB8861::adx_im()
{
	uint32_t r, d, t;
	IMMBYTE(t);
	d = X;
	r = d + t;
	CLR_NZVC;
	SET_FLAGS16(d, t, r);
	X = r;
}

/* $fc ADX immediate -**** */ /* MB8861 only */
void MB8861::adx_ex()
{
	uint32_t r, d;
	pair32_t b;
	EXTWORD(b);
	d = X;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	X = r;
}

