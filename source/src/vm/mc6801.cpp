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

#include "mc6801.h"
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
#include "../fifo.h"
//#endif
//#ifdef USE_DEBUGGER
#include "debugger.h"
#include "mc6800_consts.h"
#include "mc6801_consts.h"
//#endif
/****************************************************************************/
/* memory                                                                   */
/****************************************************************************/

uint32_t MC6801::RM(uint32_t Addr)
{
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	if(Addr < 0x20) {
		return mc6801_io_r(Addr);
	} else if(Addr >= 0x80 && Addr < 0x100 && (ram_ctrl & 0x40)) {
		return ram[Addr & 0x7f];
	}
//#endif
	return d_mem->read_data8(Addr);
}

void MC6801::WM(uint32_t Addr, uint32_t Value)
{
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	if(Addr < 0x20) {
		mc6801_io_w(Addr, Value);
	} else if(Addr >= 0x80 && Addr < 0x100 && (ram_ctrl & 0x40)) {
		ram[Addr & 0x7f] = Value;
	} else
//#endif
	d_mem->write_data8(Addr, Value);
}
//#if defined(HAS_MC6801) || defined(HAS_HD6301)

uint32_t MC6801::mc6801_io_r(uint32_t offset)
{
	switch (offset) {
	case 0x00:
		// port1 data direction register
		return port[0].ddr;
	case 0x01:
		// port2 data direction register
		return port[1].ddr;
	case 0x02:
		// port1 data register
		return (port[0].rreg & ~port[0].ddr) | (port[0].wreg & port[0].ddr);
	case 0x03:
		// port2 data register
		return (port[1].rreg & ~port[1].ddr) | (port[1].wreg & port[1].ddr);
	case 0x04:
		// port3 data direction register (write only???)
		return port[2].ddr;
	case 0x05:
		// port4 data direction register
		return port[3].ddr;
	case 0x06:
		// port3 data register
		if(p3csr_is3_flag_read) {
			p3csr_is3_flag_read = false;
			p3csr &= ~P3CSR_IS3_FLAG;
		}
		if(port[2].latched) {
			port[2].latched = false;
			return (port[2].latched_data & ~port[2].ddr) | (port[2].wreg & port[2].ddr);
		}
		return (port[2].rreg & ~port[2].ddr) | (port[2].wreg & port[2].ddr);
	case 0x07:
		// port4 data register
		return (port[3].rreg & ~port[3].ddr) | (port[3].wreg & port[3].ddr);
	case 0x08:
		// timer control register
		pending_tcsr = 0;
		return tcsr;
	case 0x09:
		// free running counter (msb)
		if(!(pending_tcsr & TCSR_TOF)) {
			tcsr &= ~TCSR_TOF;
		}
		return counter.b.h;
	case 0x0a:
		// free running counter (lsb)
		return counter.b.l;
	case 0x0b:
		// output compare register (msb)
		if(!(pending_tcsr & TCSR_OCF)) {
			tcsr &= ~TCSR_OCF;
		}
		return output_compare.b.h;
	case 0x0c:
		// output compare register (lsb)
		if(!(pending_tcsr & TCSR_OCF)) {
			tcsr &= ~TCSR_OCF;
		}
		return output_compare.b.l;
	case 0x0d:
		// input capture register (msb)
		if(!(pending_tcsr & TCSR_ICF)) {
			tcsr &= ~TCSR_ICF;
		}
		return (input_capture >> 0) & 0xff;
	case 0x0e:
		// input capture register (lsb)
		return (input_capture >> 8) & 0xff;
	case 0x0f:
		// port3 control/status register
		p3csr_is3_flag_read = true;
		return p3csr;
	case 0x10:
		// rate and mode control register
		return rmcr;
	case 0x11:
		if(trcsr & TRCSR_TDRE) {
			trcsr_read_tdre = true;
		}
		if(trcsr & TRCSR_ORFE) {
			trcsr_read_orfe = true;
		}
		if(trcsr & TRCSR_RDRF) {
			trcsr_read_rdrf = true;
		}
		return trcsr;
	case 0x12:
		// receive data register
		if(trcsr_read_orfe) {
			trcsr_read_orfe = false;
			trcsr &= ~TRCSR_ORFE;
		}
		if(trcsr_read_rdrf) {
			trcsr_read_rdrf = false;
			trcsr &= ~TRCSR_RDRF;
		}
		return rdr;
	case 0x13:
		// transmit data register
		return tdr;
	case 0x14:
		// ram control register
		return (ram_ctrl & 0x40) | 0x3f;
	}
	return 0;
}

void MC6801::mc6801_io_w(uint32_t offset, uint32_t data)
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
//		latch09 = data & 0xff;
//#endif
		CT = 0xfff8;
		TOH = CTH;
		MODIFIED_counters;
		break;
//#ifdef HAS_HD6301
//	case 0x0a:
//		// free running counter (lsb)
//		CT = (latch09 << 8) | (data & 0xff);
//		TOH = CTH;
//		MODIFIED_counters;
///		break;
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

void MC6801::increment_counter(int amount)
{
	total_icount += amount;
	icount -= amount;
	
	// timer
	if((CTD += amount) >= timer_next) {
		/* OCI */
		if( CTD >= OCD) {
			OCH++;	// next IRQ point
			tcsr |= TCSR_OCF;
			pending_tcsr |= TCSR_OCF;
		}
		/* TOI */
		if( CTD >= TOD) {
			TOH++;	// next IRQ point
			tcsr |= TCSR_TOF;
			pending_tcsr |= TCSR_TOF;
		}
		/* set next event */
		SET_TIMER_EVENT;
	}
	
	// serial i/o
	if((sio_counter -= amount) <= 0) {
		if((trcsr & TRCSR_TE) && !(trcsr & TRCSR_TDRE)) {
			write_signals(&outputs_sio, tdr);
			trcsr |= TRCSR_TDRE;
		}
		if((trcsr & TRCSR_RE) && !recv_buffer->empty()) {
			if(trcsr & TRCSR_WU) {
				// skip 10 bits
				trcsr &= ~TRCSR_WU;
				recv_buffer->read();
			} else if(!(trcsr & TRCSR_RDRF)) {
				// note: wait reveived data is read by cpu, so overrun framing error never occurs
				rdr = recv_buffer->read();
				trcsr |= TRCSR_RDRF;
			}
		}
		sio_counter += RMCR_SS[rmcr & 3];
	}
}
//#else 


void MC6801::initialize()
{
	MC6800::initialize();
	
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	recv_buffer = new FIFO(0x10000);
	ram_ctrl = 0xc0;
//#endif
//#ifdef USE_DEBUGGER
	if(__USE_DEBUGGER) {
		d_mem_stored = d_mem;
		d_debugger->set_context_mem(d_mem);
	}
//#endif
}
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
void MC6801::release()
{
	recv_buffer->release();
	delete recv_buffer;
}
//#endif

void MC6801::reset()
{
	CC = 0xc0;
	SEI; /* IRQ disabled */
	PCD = RM16(0xfffe);
	S = X = D = EA = 0;
	
	wai_state = 0;
	int_state = 0;
	
	icount = 0;
	
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	for(int i = 0; i < 4; i++) {
		port[i].ddr = 0x00;
		port[i].first_write = true;
		port[i].latched = false;
	}
	p3csr = 0x00;
	p3csr_is3_flag_read = false;
	sc1_state = sc2_state = false;
	
	tcsr = pending_tcsr = 0x00;
	CTD = 0x0000;
	OCD = 0xffff;
	TOD = 0xffff;
	
	recv_buffer->clear();
	trcsr = TRCSR_TDRE;
	trcsr_read_tdre = trcsr_read_orfe = trcsr_read_rdrf = false;
	rmcr = 0x00;
	sio_counter = RMCR_SS[rmcr & 3];
	
	ram_ctrl |= 0x40;
//#endif
}

void MC6801::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_CPU_IRQ:
		if(data & mask) {
			int_state |= INT_REQ_BIT;
		} else {
			int_state &= ~INT_REQ_BIT;
		}
		break;
	case SIG_CPU_NMI:
		if(data & mask) {
			int_state |= NMI_REQ_BIT;
		} else {
			int_state &= ~NMI_REQ_BIT;
		}
		break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case SIG_MC6801_PORT_1:
		port[0].rreg = (port[0].rreg & ~mask) | (data & mask);
		break;
	case SIG_MC6801_PORT_2:
		if((mask & 1) && (port[1].rreg & 1) != (data & 1) && (tcsr & 2) == ((data << 1) & 2)) {
			// active TIN edge in
			tcsr |= TCSR_ICF;
			pending_tcsr |= TCSR_ICF;
			input_capture = CT;
		}
		port[1].rreg = (port[1].rreg & ~mask) | (data & mask);
		break;
	case SIG_MC6801_PORT_3:
		port[2].rreg = (port[2].rreg & ~mask) | (data & mask);
		break;
	case SIG_MC6801_PORT_4:
		port[3].rreg = (port[3].rreg & ~mask) | (data & mask);
		break;
	case SIG_MC6801_PORT_3_SC1:
		if(sc1_state && !(data & mask)) {
			// SC1: H -> L
			if(!port[2].latched && (p3csr & P3CSR_LE)) {
				port[2].latched_data = port[2].rreg;
				port[2].latched = true;
				p3csr |= P3CSR_IS3_FLAG;
			}
		}
		sc1_state = ((data & mask) != 0);
		break;
	case SIG_MC6801_PORT_3_SC2:
		sc2_state = ((data & mask) != 0);
		break;
	case SIG_MC6801_SIO_RECV:
		recv_buffer->write(data & mask);
		break;
//#endif
	}
}

int MC6801::run(int clock)
{
        // run cpu
        if(clock == -1) {
                // run only one opcode

                CLEANUP_COUNTERS();

                icount = 0;
                run_one_opecode();
                return -icount;
        } else {
                /* run cpu while given clocks */

                CLEANUP_COUNTERS();

                icount += clock;
                int first_icount = icount;
                
                while(icount > 0) {
                        run_one_opecode();
                }
                return first_icount - icount;
        }
}

void MC6801::run_one_opecode()
{
	if(wai_state & (MC6800_WAI | HD6301_SLP)) {
		increment_counter(1);
	} else {
		do {
			one_more_insn = false;
			if(__USE_DEBUGGER) {
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
						d_mem = d_debugger;
					} else {
						now_debugging = false;
					}
					
					d_debugger->add_cpu_trace(PC);
					uint8_t ireg = M_RDOP(PCD);
					prevpc = PC;
					PC++;
					insn(ireg);
					increment_counter(cycles[ireg]);
					
					if(now_debugging) {
						if(!d_debugger->now_going) {
							d_debugger->now_suspended = true;
						}
						d_mem = d_mem_stored;
					}
				} else {
					if(__USE_DEBUGGER) d_debugger->add_cpu_trace(PC);
					uint8_t ireg = M_RDOP(PCD);
					prevpc = PC;
					PC++;
					insn(ireg);
					increment_counter(cycles[ireg]);
				}
			} else {
				uint8_t ireg = M_RDOP(PCD);
				prevpc = PC;
				PC++;
				insn(ireg);
				increment_counter(cycles[ireg]);
			}
		} while(one_more_insn);
	}
	
	// check interrupt
	if(int_state & NMI_REQ_BIT) {
		wai_state &= ~HD6301_SLP;
		int_state &= ~NMI_REQ_BIT;
		enter_interrupt(0xfffc);
	} else if(int_state & INT_REQ_BIT) {
		wai_state &= ~HD6301_SLP;
		if(!(CC & 0x10)) {
			int_state &= ~INT_REQ_BIT;
			enter_interrupt(0xfff8);
		}
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	} else if((tcsr & (TCSR_EICI | TCSR_ICF)) == (TCSR_EICI | TCSR_ICF)) {
		wai_state &= ~HD6301_SLP;
		if(!(CC & 0x10)) {
			TAKE_ICI;
		}
	} else if((tcsr & (TCSR_EOCI | TCSR_OCF)) == (TCSR_EOCI | TCSR_OCF)) {
		wai_state &= ~HD6301_SLP;
		if(!(CC & 0x10)) {
			TAKE_OCI;
		}
	} else if((tcsr & (TCSR_ETOI | TCSR_TOF)) == (TCSR_ETOI | TCSR_TOF)) {
		wai_state &= ~HD6301_SLP;
		if(!(CC & 0x10)) {
			TAKE_TOI;
		}
	} else if(((trcsr & (TRCSR_RIE | TRCSR_RDRF)) == (TRCSR_RIE | TRCSR_RDRF)) ||
	          ((trcsr & (TRCSR_RIE | TRCSR_ORFE)) == (TRCSR_RIE | TRCSR_ORFE)) ||
	          ((trcsr & (TRCSR_TIE | TRCSR_TDRE)) == (TRCSR_TIE | TRCSR_TDRE))) {
		wai_state &= ~HD6301_SLP;
		if(!(CC & 0x10)) {
			TAKE_SCI;
		}
//#endif
	}
}

int MC6801::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	uint8_t ops[4];
	for(int i = 0; i < 4; i++) {
		int wait;
		ops[i] = d_mem_stored->read_data8w(pc + i, &wait);
	}
	return Dasm680x(6801, buffer, pc, ops, ops, d_debugger->first_symbol);
	return 0;
}

void MC6801::insn(uint8_t code)
{
	switch(code) {
	case 0x00: illegal(); break;
	case 0x01: nop(); break;
	case 0x02: illegal(); break;
	case 0x03: illegal(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x04: lsrd(); break;
	case 0x05: asld(); break;
//#else
//	case 0x04: illegal(); break;
//	case 0x05: illegal(); break;
//#endif
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
//#if defined(HAS_HD6301)
//	case 0x12: undoc1(); break;
//	case 0x13: undoc2(); break;
//#else
	case 0x12: illegal(); break;
	case 0x13: illegal(); break;
//#endif
	case 0x14: illegal(); break;
	case 0x15: illegal(); break;
	case 0x16: tab(); break;
	case 0x17: tba(); break;
//#if defined(HAS_HD6301)
//	case 0x18: xgdx(); break;
//#else
	case 0x18: illegal(); break;
//#endif
	case 0x19: daa(); break;
//#if defined(HAS_HD6301)
//	case 0x1a: slp(); break;
//#else
	case 0x1a: illegal(); break;
//#endif
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
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x38: pulx(); break;
//#else
//	case 0x38: illegal(); break;
//#endif
	case 0x39: rts(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x3a: abx(); break;
//#else
//	case 0x3a: illegal(); break;
//#endif
	case 0x3b: rti(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x3c: pshx(); break;
	case 0x3d: mul(); break;
//#else
//	case 0x3c: illegal(); break;
//	case 0x3d: illegal(); break;
//#endif
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
//#if defined(HAS_HD6301)
//	case 0x61: aim_ix(); break;
//	case 0x62: oim_ix(); break;
//#else
	case 0x61: illegal(); break;
	case 0x62: illegal(); break;
//#endif
	case 0x63: com_ix(); break;
	case 0x64: lsr_ix(); break;
//#if defined(HAS_HD6301)
//	case 0x65: eim_ix(); break;
//#else
	case 0x65: illegal(); break;
//#endif
	case 0x66: ror_ix(); break;
	case 0x67: asr_ix(); break;
	case 0x68: asl_ix(); break;
	case 0x69: rol_ix(); break;
	case 0x6a: dec_ix(); break;
//#if defined(HAS_HD6301)
//	case 0x6b: tim_ix(); break;
//#else
	case 0x6b: illegal(); break;
//#endif
	case 0x6c: inc_ix(); break;
	case 0x6d: tst_ix(); break;
	case 0x6e: jmp_ix(); break;
	case 0x6f: clr_ix(); break;
	case 0x70: neg_ex(); break;
//#if defined(HAS_HD6301)
//	case 0x71: aim_di(); break;
//	case 0x72: oim_di(); break;
//#elif defined(HAS_MB8861)
//	case 0x71: nim_ix(); break;
//	case 0x72: oim_ix_mb8861(); break;
//#else
	case 0x71: illegal(); break;
	case 0x72: illegal(); break;
//#endif
	case 0x73: com_ex(); break;
	case 0x74: lsr_ex(); break;
//#if defined(HAS_HD6301)
//	case 0x75: eim_di(); break;
//#elif defined(HAS_MB8861)
//	case 0x75: xim_ix(); break;
//#else
	case 0x75: illegal(); break;
//#endif
	case 0x76: ror_ex(); break;
	case 0x77: asr_ex(); break;
	case 0x78: asl_ex(); break;
	case 0x79: rol_ex(); break;
	case 0x7a: dec_ex(); break;
//#if defined(HAS_HD6301)
//	case 0x7b: tim_di(); break;
//#elif defined(HAS_MB8861)
//	case 0x7b: tmm_ix(); break;
//#else
	case 0x7b: illegal(); break;
//#endif
	case 0x7c: inc_ex(); break;
	case 0x7d: tst_ex(); break;
	case 0x7e: jmp_ex(); break;
	case 0x7f: clr_ex(); break;
	case 0x80: suba_im(); break;
	case 0x81: cmpa_im(); break;
	case 0x82: sbca_im(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x83: subd_im(); break;
//#else
//	case 0x83: illegal(); break;
//#endif
	case 0x84: anda_im(); break;
	case 0x85: bita_im(); break;
	case 0x86: lda_im(); break;
	case 0x87: sta_im(); break;
	case 0x88: eora_im(); break;
	case 0x89: adca_im(); break;
	case 0x8a: ora_im(); break;
	case 0x8b: adda_im(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x8c: cpx_im (); break;
//#else
//	case 0x8c: cmpx_im(); break;
//#endif
	case 0x8d: bsr(); break;
	case 0x8e: lds_im(); break;
	case 0x8f: sts_im(); break;
	case 0x90: suba_di(); break;
	case 0x91: cmpa_di(); break;
	case 0x92: sbca_di(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x93: subd_di(); break;
//#else
//	case 0x93: illegal(); break;
//#endif
	case 0x94: anda_di(); break;
	case 0x95: bita_di(); break;
	case 0x96: lda_di(); break;
	case 0x97: sta_di(); break;
	case 0x98: eora_di(); break;
	case 0x99: adca_di(); break;
	case 0x9a: ora_di(); break;
	case 0x9b: adda_di(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0x9c: cpx_di (); break;
//#else
//	case 0x9c: cmpx_di(); break;
//#endif
	case 0x9d: jsr_di(); break;
	case 0x9e: lds_di(); break;
	case 0x9f: sts_di(); break;
	case 0xa0: suba_ix(); break;
	case 0xa1: cmpa_ix(); break;
	case 0xa2: sbca_ix(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xa3: subd_ix(); break;
//#else
//	case 0xa3: illegal(); break;
//#endif
	case 0xa4: anda_ix(); break;
	case 0xa5: bita_ix(); break;
	case 0xa6: lda_ix(); break;
	case 0xa7: sta_ix(); break;
	case 0xa8: eora_ix(); break;
	case 0xa9: adca_ix(); break;
	case 0xaa: ora_ix(); break;
	case 0xab: adda_ix(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xac: cpx_ix (); break;
//#else
//	case 0xac: cmpx_ix(); break;
//#endif
	case 0xad: jsr_ix(); break;
	case 0xae: lds_ix(); break;
	case 0xaf: sts_ix(); break;
	case 0xb0: suba_ex(); break;
	case 0xb1: cmpa_ex(); break;
	case 0xb2: sbca_ex(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xb3: subd_ex(); break;
//#else
//	case 0xb3: illegal(); break;
//#endif
	case 0xb4: anda_ex(); break;
	case 0xb5: bita_ex(); break;
	case 0xb6: lda_ex(); break;
	case 0xb7: sta_ex(); break;
	case 0xb8: eora_ex(); break;
	case 0xb9: adca_ex(); break;
	case 0xba: ora_ex(); break;
	case 0xbb: adda_ex(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xbc: cpx_ex (); break;
//#else
//	case 0xbc: cmpx_ex(); break;
//#endif
	case 0xbd: jsr_ex(); break;
	case 0xbe: lds_ex(); break;
	case 0xbf: sts_ex(); break;
	case 0xc0: subb_im(); break;
	case 0xc1: cmpb_im(); break;
	case 0xc2: sbcb_im(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xc3: addd_im(); break;
//#else
//	case 0xc3: illegal(); break;
//#endif
	case 0xc4: andb_im(); break;
	case 0xc5: bitb_im(); break;
	case 0xc6: ldb_im(); break;
	case 0xc7: stb_im(); break;
	case 0xc8: eorb_im(); break;
	case 0xc9: adcb_im(); break;
	case 0xca: orb_im(); break;
	case 0xcb: addb_im(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xcc: ldd_im(); break;
	case 0xcd: std_im(); break;
//#else
//	case 0xcc: illegal(); break;
//	case 0xcd: illegal(); break;
//#endif
	case 0xce: ldx_im(); break;
	case 0xcf: stx_im(); break;
	case 0xd0: subb_di(); break;
	case 0xd1: cmpb_di(); break;
	case 0xd2: sbcb_di(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xd3: addd_di(); break;
//#else
//	case 0xd3: illegal(); break;
//#endif
	case 0xd4: andb_di(); break;
	case 0xd5: bitb_di(); break;
	case 0xd6: ldb_di(); break;
	case 0xd7: stb_di(); break;
	case 0xd8: eorb_di(); break;
	case 0xd9: adcb_di(); break;
	case 0xda: orb_di(); break;
	case 0xdb: addb_di(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xdc: ldd_di(); break;
	case 0xdd: std_di(); break;
//#else
//	case 0xdc: illegal(); break;
//	case 0xdd: illegal(); break;
//#endif
	case 0xde: ldx_di(); break;
	case 0xdf: stx_di(); break;
	case 0xe0: subb_ix(); break;
	case 0xe1: cmpb_ix(); break;
	case 0xe2: sbcb_ix(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xe3: addd_ix(); break;
//#else
//	case 0xe3: illegal(); break;
//#endif
	case 0xe4: andb_ix(); break;
	case 0xe5: bitb_ix(); break;
	case 0xe6: ldb_ix(); break;
	case 0xe7: stb_ix(); break;
	case 0xe8: eorb_ix(); break;
	case 0xe9: adcb_ix(); break;
	case 0xea: orb_ix(); break;
	case 0xeb: addb_ix(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xec: ldd_ix(); break;
	case 0xed: std_ix(); break;
//#elif defined(HAS_MB8861)
//	case 0xec: adx_im(); break;
//	case 0xed: illegal(); break;
//#else
//	case 0xec: illegal(); break;
//	case 0xed: illegal(); break;
//#endif
	case 0xee: ldx_ix(); break;
	case 0xef: stx_ix(); break;
	case 0xf0: subb_ex(); break;
	case 0xf1: cmpb_ex(); break;
	case 0xf2: sbcb_ex(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xf3: addd_ex(); break;
//#else
//	case 0xf3: illegal(); break;
//#endif
	case 0xf4: andb_ex(); break;
	case 0xf5: bitb_ex(); break;
	case 0xf6: ldb_ex(); break;
	case 0xf7: stb_ex(); break;
	case 0xf8: eorb_ex(); break;
	case 0xf9: adcb_ex(); break;
	case 0xfa: orb_ex(); break;
	case 0xfb: addb_ex(); break;
//#if defined(HAS_MC6801) || defined(HAS_HD6301)
	case 0xfc: ldd_ex(); break;
	case 0xfd: std_ex(); break;
//#elif defined(HAS_MB8861)
//	case 0xfc: adx_ex(); break;
//	case 0xfd: illegal(); break;
//#else
//	case 0xfc: illegal(); break;
//	case 0xfd: illegal(); break;
//#endif
	case 0xfe: ldx_ex(); break;
	case 0xff: stx_ex(); break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}

/* $04 LSRD inherent -0*-* */
void MC6801::lsrd()
{
	uint16_t t;
	CLR_NZC;
	t = D;
	CC |= (t & 0x0001);
	t >>= 1;
	SET_Z16(t);
	D = t;
}

/* $05 ASLD inherent ?**** */
void MC6801::asld()
{
	int r;
	uint16_t t;
	t = D;
	r = t << 1;
	CLR_NZVC;
	SET_FLAGS16(t, t, r);
	D = r;
}
/* $38 PULX inherent ----- */
void MC6801::pulx()
{
	PULLWORD(pX);
}

/* $3a ABX inherent ----- */
void MC6801::abx()
{
	X += B;
}
/* $3c PSHX inherent ----- */
void MC6801::pshx()
{
	PUSHWORD(pX);
}

/* $3d MUL inherent --*-@ */
void MC6801::mul()
{
	uint16_t t;
	t = A*B;
	CLR_C;
	if(t & 0x80) SEC;
	D = t;
}
/* $83 SUBD immediate -**** */
void MC6801::subd_im()
{
	uint32_t r, d;
	pair_t b;
	IMMWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $8c CPX immediate -**** (6801) */
void MC6801::cpx_im()
{
	uint32_t r, d;
	pair_t b;
	IMMWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $93 SUBD direct -**** */
void MC6801::subd_di()
{
	uint32_t r, d;
	pair_t b;
	DIRWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $9c CPX direct -**** (6801) */
void MC6801::cpx_di()
{
	uint32_t r, d;
	pair_t b;
	DIRWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}
/* $a3 SUBD indexed -**** */
void MC6801::subd_ix()
{
	uint32_t r, d;
	pair_t b;
	IDXWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $ac CPX indexed -**** (6801)*/
void MC6801::cpx_ix()
{
	uint32_t r, d;
	pair_t b;
	IDXWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $b3 SUBD extended -**** */
void MC6801::subd_ex()
{
	uint32_t r, d;
	pair_t b;
	EXTWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $bc CPX extended -**** (6801) */
void MC6801::cpx_ex()
{
	uint32_t r, d;
	pair_t b;
	EXTWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $c3 ADDD immediate -**** */
void MC6801::addd_im()
{
	uint32_t r, d;
	pair_t b;
	IMMWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $CC LDD immediate -**0- */
void MC6801::ldd_im()
{
	IMMWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* is this a legal instruction? */
/* $cd STD immediate -**0- */
void MC6801::std_im()
{
	IMM16;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD, &pD);
}

/* $d3 ADDD direct -**** */
void MC6801::addd_di()
{
	uint32_t r, d;
	pair_t b;
	DIRWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $dc LDD direct -**0- */
void MC6801::ldd_di()
{
	DIRWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $dd STD direct -**0- */
void MC6801::std_di()
{
	DIRECT;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD, &pD);
}

/* $e3 ADDD indexed -**** */
void MC6801::addd_ix()
{
	uint32_t r, d;
	pair_t b;
	IDXWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $ec LDD indexed -**0- */
void MC6801::ldd_ix()
{
	IDXWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $ed STD indexed -**0- */
void MC6801::std_ix()
{
	INDEXED;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD, &pD);
}

/* $f3 ADDD extended -**** */
void MC6801::addd_ex()
{
	uint32_t r, d;
	pair_t b;
	EXTWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $fc LDD extended -**0- */
void MC6801::ldd_ex()
{
	EXTWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $fd STD extended -**0- */
void MC6801::std_ex()
{
	EXTENDED;
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD, &pD);
}

#define STATE_VERSION	2

bool MC6801::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateUint32(pc.d);
	state_fio->StateUint16(prevpc);
	state_fio->StateUint32(sp.d);
	state_fio->StateUint32(ix.d);
	state_fio->StateUint32(acc_d.d);
	state_fio->StateUint32(ea.d);
	state_fio->StateUint8(cc);
	state_fio->StateInt32(wai_state);
	state_fio->StateInt32(int_state);
	if(__USE_DEBUGGER) {
		state_fio->StateUint64(total_icount);
	}
	state_fio->StateInt32(icount);
// #if defined(HAS_MC6801) || defined(HAS_HD6301)
 	for(int i = 0; i < 4; i++) {
		state_fio->StateUint8(port[i].wreg);
		state_fio->StateUint8(port[i].rreg);
		state_fio->StateUint8(port[i].ddr);
		state_fio->StateUint8(port[i].latched_data);
		state_fio->StateBool(port[i].latched);
		state_fio->StateBool(port[i].first_write);
 	}
	state_fio->StateUint8(p3csr);
	state_fio->StateBool(p3csr_is3_flag_read);
	state_fio->StateBool(sc1_state);
	state_fio->StateBool(sc2_state);
	state_fio->StateUint32(counter.d);
	state_fio->StateUint32(output_compare.d);
	state_fio->StateUint32(timer_over.d);
	state_fio->StateUint8(tcsr);
	state_fio->StateUint8(pending_tcsr);
	state_fio->StateUint16(input_capture);
	state_fio->StateUint32(timer_next);
	if(!recv_buffer->process_state((void *)state_fio, loading)) {
 		return false;
 	}
	state_fio->StateUint8(trcsr);
	state_fio->StateUint8(rdr);
	state_fio->StateUint8(tdr);
	state_fio->StateBool(trcsr_read_tdre);
	state_fio->StateBool(trcsr_read_orfe);
	state_fio->StateBool(trcsr_read_rdrf);
	state_fio->StateUint8(rmcr);
	state_fio->StateInt32(sio_counter);
	state_fio->StateUint8(ram_ctrl);
	state_fio->StateBuffer(ram, sizeof(ram), 1);
//#endif
	
	// post process
	if(__USE_DEBUGGER) {
		if(loading) {
			prev_total_icount = total_icount;
		}
	}
 	return true;
}

