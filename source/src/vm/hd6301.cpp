
/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date  : 2011.04.23-

	[ MC6801 ]
*/
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4996 )
#endif

#include "hd6301.h"
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
#include "../fifo.h"
//#endif
//#ifdef USE_DEBUGGER
#include "debugger.h"
#include "mc6800_consts.h"
#include "mc6801_consts.h"
//#endif

void HD6301::mc6801_io_w(uint32_t offset, uint32_t data)
{
	switch(offset) {
	case 0x00:
		// port1 data direction register
		port[0].ddr = data;
		break;
	case 0x01:
		// port2 data direction register
		port[1].ddr = data;
		break;
	case 0x02:
		// port1 data register
		if(port[0].wreg != data || port[0].first_write) {
			write_signals(&port[0].outputs, data);
			port[0].wreg = data;
			port[0].first_write = false;
		}
		break;
	case 0x03:
		// port2 data register
		if(port[1].wreg != data || port[1].first_write) {
			write_signals(&port[1].outputs, data);
			port[1].wreg = data;
			port[1].first_write = false;
		}
		break;
	case 0x04:
		// port3 data direction register
		port[2].ddr = data;
		break;
	case 0x05:
		// port4 data direction register
		port[3].ddr = data;
		break;
	case 0x06:
		// port3 data register
		if(p3csr_is3_flag_read) {
			p3csr_is3_flag_read = false;
			p3csr &= ~P3CSR_IS3_FLAG;
		}
		if(port[2].wreg != data || port[2].first_write) {
			write_signals(&port[2].outputs, data);
			port[2].wreg = data;
			port[2].first_write = false;
		}
		break;
	case 0x07:
		// port4 data register
		if(port[3].wreg != data || port[3].first_write) {
			write_signals(&port[3].outputs, data);
			port[3].wreg = data;
			port[3].first_write = false;
		}
		break;
	case 0x08:
		// timer control/status register
		tcsr = data;
		pending_tcsr &= tcsr;
		break;
	case 0x09:
		// free running counter (msb)
//#ifdef HAS_HD6301
		latch09 = data & 0xff;
//#endif
		CT = 0xfff8;
		TOH = CTH;
		MODIFIED_counters;
		break;
//#ifdef HAS_HD6301
	case 0x0a:
		// free running counter (lsb)
		CT = (latch09 << 8) | (data & 0xff);
		TOH = CTH;
		MODIFIED_counters;
		break;
//#endif
	case 0x0b:
		// output compare register (msb)
		if(output_compare.b.h != data) {
			output_compare.b.h = data;
			MODIFIED_counters;
		}
        tcsr &=~TCSR_OCF;
		break;
	case 0x0c:
		// output compare register (lsb)
		if(output_compare.b.l != data) {
			output_compare.b.l = data;
			MODIFIED_counters;
		}
        tcsr &=~TCSR_OCF;
		break;
	case 0x0f:
		// port3 control/status register
		p3csr = (p3csr & P3CSR_IS3_FLAG) | (data & ~P3CSR_IS3_FLAG);
		break;
	case 0x10:
		// rate and mode control register
		rmcr = data;
		break;
	case 0x11:
		// transmit/receive control/status register
		trcsr = (trcsr & 0xe0) | (data & 0x1f);
		break;
	case 0x13:
		// transmit data register
		if(trcsr_read_tdre) {
			trcsr_read_tdre = false;
			trcsr &= ~TRCSR_TDRE;
		}
		tdr = data;
		break;
	case 0x14:
		// ram control register
		ram_ctrl = data;
		break;
	}
}

void HD6301::insn(uint8_t code)
{
	switch(code) {
	case 0x00: illegal(); break;
	case 0x01: nop(); break;
	case 0x02: illegal(); break;
	case 0x03: illegal(); break;
	case 0x04: lsrd(); break;
	case 0x05: asld(); break;
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

	case 0x12: undoc1(); break;
	case 0x13: undoc2(); break;
		
	case 0x14: illegal(); break;
	case 0x15: illegal(); break;
	case 0x16: tab(); break;
	case 0x17: tba(); break;

	case 0x18: xgdx(); break;

	case 0x19: daa(); break;

	case 0x1a: slp(); break;

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

	case 0x38: pulx(); break;
	
	case 0x39: rts(); break;

	case 0x3a: abx(); break;

	case 0x3b: rti(); break;

	case 0x3c: pshx(); break;
	case 0x3d: mul(); break;
		
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

	case 0x61: aim_ix(); break;
	case 0x62: oim_ix(); break;

	case 0x63: com_ix(); break;
	case 0x64: lsr_ix(); break;

	case 0x65: eim_ix(); break;

	case 0x66: ror_ix(); break;
	case 0x67: asr_ix(); break;
	case 0x68: asl_ix(); break;
	case 0x69: rol_ix(); break;
	case 0x6a: dec_ix(); break;

	case 0x6b: tim_ix(); break;
		
	case 0x6c: inc_ix(); break;
	case 0x6d: tst_ix(); break;
	case 0x6e: jmp_ix(); break;
	case 0x6f: clr_ix(); break;
	case 0x70: neg_ex(); break;

	case 0x71: aim_di(); break;
	case 0x72: oim_di(); break;
		
	case 0x73: com_ex(); break;
	case 0x74: lsr_ex(); break;

	case 0x75: eim_di(); break;

	case 0x76: ror_ex(); break;
	case 0x77: asr_ex(); break;
	case 0x78: asl_ex(); break;
	case 0x79: rol_ex(); break;
	case 0x7a: dec_ex(); break;

	case 0x7b: tim_di(); break;
		
	case 0x7c: inc_ex(); break;
	case 0x7d: tst_ex(); break;
	case 0x7e: jmp_ex(); break;
	case 0x7f: clr_ex(); break;
	case 0x80: suba_im(); break;
	case 0x81: cmpa_im(); break;
	case 0x82: sbca_im(); break;

	case 0x83: subd_im(); break;

	case 0x84: anda_im(); break;
	case 0x85: bita_im(); break;
	case 0x86: lda_im(); break;
	case 0x87: sta_im(); break;
	case 0x88: eora_im(); break;
	case 0x89: adca_im(); break;
	case 0x8a: ora_im(); break;
	case 0x8b: adda_im(); break;
		
	case 0x8c: cpx_im (); break;
	case 0x8d: bsr(); break;
	case 0x8e: lds_im(); break;
	case 0x8f: sts_im(); break;
	case 0x90: suba_di(); break;
	case 0x91: cmpa_di(); break;
	case 0x92: sbca_di(); break;

	case 0x93: subd_di(); break;

	case 0x94: anda_di(); break;
	case 0x95: bita_di(); break;
	case 0x96: lda_di(); break;
	case 0x97: sta_di(); break;
	case 0x98: eora_di(); break;
	case 0x99: adca_di(); break;
	case 0x9a: ora_di(); break;
	case 0x9b: adda_di(); break;

	case 0x9c: cpx_di (); break;
	case 0x9d: jsr_di(); break;
	case 0x9e: lds_di(); break;
	case 0x9f: sts_di(); break;
	case 0xa0: suba_ix(); break;
	case 0xa1: cmpa_ix(); break;
	case 0xa2: sbca_ix(); break;

	case 0xa3: subd_ix(); break;
	case 0xa4: anda_ix(); break;
	case 0xa5: bita_ix(); break;
	case 0xa6: lda_ix(); break;
	case 0xa7: sta_ix(); break;
	case 0xa8: eora_ix(); break;
	case 0xa9: adca_ix(); break;
	case 0xaa: ora_ix(); break;
	case 0xab: adda_ix(); break;

	case 0xac: cpx_ix (); break;
		
	case 0xad: jsr_ix(); break;
	case 0xae: lds_ix(); break;
	case 0xaf: sts_ix(); break;
	case 0xb0: suba_ex(); break;
	case 0xb1: cmpa_ex(); break;
	case 0xb2: sbca_ex(); break;

	case 0xb3: subd_ex(); break;
	case 0xb4: anda_ex(); break;
	case 0xb5: bita_ex(); break;
	case 0xb6: lda_ex(); break;
	case 0xb7: sta_ex(); break;
	case 0xb8: eora_ex(); break;
	case 0xb9: adca_ex(); break;
	case 0xba: ora_ex(); break;
	case 0xbb: adda_ex(); break;

	case 0xbc: cpx_ex (); break;
	case 0xbd: jsr_ex(); break;
	case 0xbe: lds_ex(); break;
	case 0xbf: sts_ex(); break;
	case 0xc0: subb_im(); break;
	case 0xc1: cmpb_im(); break;
	case 0xc2: sbcb_im(); break;

	case 0xc3: addd_im(); break;

	case 0xc4: andb_im(); break;
	case 0xc5: bitb_im(); break;
	case 0xc6: ldb_im(); break;
	case 0xc7: stb_im(); break;
	case 0xc8: eorb_im(); break;
	case 0xc9: adcb_im(); break;
	case 0xca: orb_im(); break;
	case 0xcb: addb_im(); break;

	case 0xcc: ldd_im(); break;
	case 0xcd: std_im(); break;


	case 0xce: ldx_im(); break;
	case 0xcf: stx_im(); break;
	case 0xd0: subb_di(); break;
	case 0xd1: cmpb_di(); break;
	case 0xd2: sbcb_di(); break;

	case 0xd3: addd_di(); break;

	case 0xd4: andb_di(); break;
	case 0xd5: bitb_di(); break;
	case 0xd6: ldb_di(); break;
	case 0xd7: stb_di(); break;
	case 0xd8: eorb_di(); break;
	case 0xd9: adcb_di(); break;
	case 0xda: orb_di(); break;
	case 0xdb: addb_di(); break;

	case 0xdc: ldd_di(); break;
	case 0xdd: std_di(); break;

	case 0xde: ldx_di(); break;
	case 0xdf: stx_di(); break;
	case 0xe0: subb_ix(); break;
	case 0xe1: cmpb_ix(); break;
	case 0xe2: sbcb_ix(); break;

	case 0xe3: addd_ix(); break;

	case 0xe4: andb_ix(); break;
	case 0xe5: bitb_ix(); break;
	case 0xe6: ldb_ix(); break;
	case 0xe7: stb_ix(); break;
	case 0xe8: eorb_ix(); break;
	case 0xe9: adcb_ix(); break;
	case 0xea: orb_ix(); break;
	case 0xeb: addb_ix(); break;

	case 0xec: ldd_ix(); break;
	case 0xed: std_ix(); break;
	
	case 0xee: ldx_ix(); break;
	case 0xef: stx_ix(); break;
	case 0xf0: subb_ex(); break;
	case 0xf1: cmpb_ex(); break;
	case 0xf2: sbcb_ex(); break;

	case 0xf3: addd_ex(); break;

	case 0xf4: andb_ex(); break;
	case 0xf5: bitb_ex(); break;
	case 0xf6: ldb_ex(); break;
	case 0xf7: stb_ex(); break;
	case 0xf8: eorb_ex(); break;
	case 0xf9: adcb_ex(); break;
	case 0xfa: orb_ex(); break;
	case 0xfb: addb_ex(); break;

	case 0xfc: ldd_ex(); break;
	case 0xfd: std_ex(); break;

	case 0xfe: ldx_ex(); break;
	case 0xff: stx_ex(); break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}

/* $00 ILLEGAL */
void HD6301::illegal()
{
//#ifdef HAS_HD6301
	TAKE_TRAP;
//#endif
}

/* $12 ILLEGAL */
void HD6301::undoc1()
{
	X += RM(S + 1);
}

/* $13 ILLEGAL */
void HD6301::undoc2()
{
	X += RM(S + 1);
}

/* $18 XGDX inherent ----- */ /* HD6301 only */
void HD6301::xgdx()
{
	uint16_t t = X;
	X = D;
	D = t;
}

/* $1a SLP */ /* HD6301 only */
void HD6301::slp()
{
	/* wait for next IRQ (same as waiting of wai) */
	wai_state |= HD6301_SLP;
}
/* $61 AIM --**0- */ /* HD6301 only */
void HD6301::aim_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r &= t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD, r);
}

/* $62 OIM --**0- */ /* HD6301 only */
void HD6301::oim_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r |= t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD, r);
}
/* $65 EIM --**0- */ /* HD6301 only */
void HD6301::eim_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r ^= t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD, r);
}
/* $6b TIM --**0- */ /* HD6301 only */
void HD6301::tim_ix()
{
	uint8_t t, r;
	IMMBYTE(t);
	IDXBYTE(r);
	r &= t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $71 AIM --**0- */ /* HD6301 only */
void HD6301::aim_di()
{
	uint8_t t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r &= t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD, r);
}
/* $72 OIM --**0- */ /* HD6301 only */
void HD6301::oim_di()
{
	uint8_t t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r |= t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD, r);
}
/* $75 EIM --**0- */ /* HD6301 only */
void HD6301::eim_di()
{
	uint8_t t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r ^= t;
	CLR_NZV;
	SET_NZ8(r);
	WM(EAD, r);
}

/* $7b TIM --**0- */ /* HD6301 only */
void HD6301::tim_di()
{
	uint8_t t, r;
	IMMBYTE(t);
	DIRBYTE(r);
	r &= t;
	CLR_NZV;
	SET_NZ8(r);
}

int HD6301::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	uint8_t ops[4];
	for(int i = 0; i < 4; i++) {
		int wait;
		ops[i] = d_mem_stored->read_data8w(pc + i, &wait);
	}
	return Dasm680x(6301, buffer, pc, ops, ops, d_debugger->first_symbol);
	return 0;
}

#define STATE_VERSION	2

#include "../statesub.h"

void HD6301::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_PAIR(pc);
	DECL_STATE_ENTRY_UINT16(prevpc);
	DECL_STATE_ENTRY_PAIR(sp);
	DECL_STATE_ENTRY_PAIR(ix);
	DECL_STATE_ENTRY_PAIR(acc_d);
	DECL_STATE_ENTRY_PAIR(ea);
	DECL_STATE_ENTRY_UINT8(cc);
	DECL_STATE_ENTRY_INT32(wai_state);
	DECL_STATE_ENTRY_INT32(int_state);
	if(__USE_DEBUGGER) {
		DECL_STATE_ENTRY_UINT64(total_icount);
	}
	DECL_STATE_ENTRY_INT32(icount);
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	for(int i = 0; i < 4; i++) {
		DECL_STATE_ENTRY_UINT8_MEMBER(port[i].wreg, i);
		DECL_STATE_ENTRY_UINT8_MEMBER(port[i].rreg, i);
		DECL_STATE_ENTRY_UINT8_MEMBER(port[i].ddr, i);
		DECL_STATE_ENTRY_UINT8_MEMBER(port[i].latched_data, i);
		DECL_STATE_ENTRY_BOOL_MEMBER(port[i].latched, i);
		DECL_STATE_ENTRY_BOOL_MEMBER(port[i].first_write, i);
	}
	DECL_STATE_ENTRY_UINT8(p3csr);
	DECL_STATE_ENTRY_BOOL(p3csr_is3_flag_read);
	DECL_STATE_ENTRY_BOOL(sc1_state);
	DECL_STATE_ENTRY_BOOL(sc2_state);
	DECL_STATE_ENTRY_PAIR(counter);
	DECL_STATE_ENTRY_PAIR(output_compare);
	DECL_STATE_ENTRY_PAIR(timer_over);
	DECL_STATE_ENTRY_UINT8(tcsr);
	DECL_STATE_ENTRY_UINT8(pending_tcsr);
	DECL_STATE_ENTRY_UINT16(input_capture);
//#ifdef HAS_HD6301
	DECL_STATE_ENTRY_UINT16(latch09);
//#endif
	DECL_STATE_ENTRY_UINT32(timer_next);
	//recv_buffer->save_state((void *)state_fio);
	DECL_STATE_ENTRY_UINT8(trcsr);
	DECL_STATE_ENTRY_UINT8(rdr);
	DECL_STATE_ENTRY_UINT8(tdr);
	DECL_STATE_ENTRY_BOOL(trcsr_read_tdre);
	DECL_STATE_ENTRY_BOOL(trcsr_read_orfe);
	DECL_STATE_ENTRY_BOOL(trcsr_read_rdrf);
	DECL_STATE_ENTRY_UINT8(rmcr);
	DECL_STATE_ENTRY_INT32(sio_counter);
	DECL_STATE_ENTRY_UINT8(ram_ctrl);
	DECL_STATE_ENTRY_1D_ARRAY(ram, sizeof(ram));
//#endif

	leave_decl_state();
}

void HD6301::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
	//state_fio->FputUint32(STATE_VERSION);
	//state_fio->FputInt32(this_device_id);
	
	//state_fio->FputUint32(pc.d);
	//state_fio->FputUint16(prevpc);
	//state_fio->FputUint32(sp.d);
	//state_fio->FputUint32(ix.d);
	//state_fio->FputUint32(acc_d.d);
	//state_fio->FputUint32(ea.d);
	//state_fio->FputUint8(cc);
	//state_fio->FputInt32(wai_state);
	//state_fio->FputInt32(int_state);
	//if(__USE_DEBUGGER) state_fio->FputUint64(total_icount);
	//state_fio->FputInt32(icount);
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	//for(int i = 0; i < 4; i++) {
	//	state_fio->FputUint8(port[i].wreg);
	//	state_fio->FputUint8(port[i].rreg);
	//	state_fio->FputUint8(port[i].ddr);
	//	state_fio->FputUint8(port[i].latched_data);
	//	state_fio->FputBool(port[i].latched);
	//	state_fio->FputBool(port[i].first_write);
	//}
	//state_fio->FputUint8(p3csr);
	//state_fio->FputBool(p3csr_is3_flag_read);
	//state_fio->FputBool(sc1_state);
	//state_fio->FputBool(sc2_state);
	//state_fio->FputUint32(counter.d);
	//state_fio->FputUint32(output_compare.d);
	//state_fio->FputUint32(timer_over.d);
	//state_fio->FputUint8(tcsr);
	//state_fio->FputUint8(pending_tcsr);
	//state_fio->FputUint16(input_capture);
//#ifdef HAS_HD6301
	//state_fio->FputUint16(latch09);
//#endif
	//state_fio->FputUint32(timer_next);
	recv_buffer->save_state((void *)state_fio);
	//state_fio->FputUint8(trcsr);
	//state_fio->FputUint8(rdr);
	//state_fio->FputUint8(tdr);
	//state_fio->FputBool(trcsr_read_tdre);
	//state_fio->FputBool(trcsr_read_orfe);
	//state_fio->FputBool(trcsr_read_rdrf);
	//state_fio->FputUint8(rmcr);
	//state_fio->FputInt32(sio_counter);
	//state_fio->FputUint8(ram_ctrl);
	//state_fio->Fwrite(ram, sizeof(ram), 1);
//#endif
}

bool HD6301::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) return false;
	//if(state_fio->FgetUint32() != STATE_VERSION) {
	//	return false;
	//}
	//if(state_fio->FgetInt32() != this_device_id) {
	//	return false;
	//}
	//pc.d = state_fio->FgetUint32();
	//prevpc = state_fio->FgetUint16();
	//sp.d = state_fio->FgetUint32();
	//ix.d = state_fio->FgetUint32();
	//acc_d.d = state_fio->FgetUint32();
	//ea.d = state_fio->FgetUint32();
	//cc = state_fio->FgetUint8();
	//wai_state = state_fio->FgetInt32();
	//int_state = state_fio->FgetInt32();
	//if(__USE_DEBUGGER) { total_icount = prev_total_icount = state_fio->FgetUint64(); }
	//icount = state_fio->FgetInt32();
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	//for(int i = 0; i < 4; i++) {
	//	port[i].wreg = state_fio->FgetUint8();
	//	port[i].rreg = state_fio->FgetUint8();
	//	port[i].ddr = state_fio->FgetUint8();
	//	port[i].latched_data = state_fio->FgetUint8();
	//	port[i].latched = state_fio->FgetBool();
	//	port[i].first_write = state_fio->FgetBool();
	//}
	//p3csr = state_fio->FgetUint8();
	//p3csr_is3_flag_read = state_fio->FgetBool();
	//sc1_state = state_fio->FgetBool();
	//sc2_state = state_fio->FgetBool();
	//counter.d = state_fio->FgetUint32();
	//output_compare.d = state_fio->FgetUint32();
	//timer_over.d = state_fio->FgetUint32();
	//tcsr = state_fio->FgetUint8();
	//pending_tcsr = state_fio->FgetUint8();
	//input_capture = state_fio->FgetUint16();
//#ifdef HAS_HD6301
	//latch09 = state_fio->FgetUint16();
//#endif
	//timer_next = state_fio->FgetUint32();
	if(__USE_DEBUGGER) {
		prev_total_icount = total_icount;
	}
	if(!recv_buffer->load_state((void *)state_fio)) {
		return false;
	}
	//trcsr = state_fio->FgetUint8();
	//rdr = state_fio->FgetUint8();
	//tdr = state_fio->FgetUint8();
	//trcsr_read_tdre = state_fio->FgetBool();
	//trcsr_read_orfe = state_fio->FgetBool();
	//trcsr_read_rdrf = state_fio->FgetBool();
	//rmcr = state_fio->FgetUint8();
	//sio_counter = state_fio->FgetInt32();
	//ram_ctrl = state_fio->FgetUint8();
	//state_fio->Fread(ram, sizeof(ram), 1);
//#endif
	return true;
}
