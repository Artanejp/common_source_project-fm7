/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.12.31 -

	[ Z80SIO ]
*/

#include "z80sio.h"
#include "../fifo.h"

#define EVENT_SEND	2
#define EVENT_RECV	4

//#define SIO_DEBUG

#define MONOSYNC(ch)	((port[ch].wr[4] & 0x3c) == 0x00)
#define BISYNC(ch)	((port[ch].wr[4] & 0x3c) == 0x10)
//#define SDLC(ch)	((port[ch].wr[4] & 0x3c) == 0x20)
//#define EXTSYNC(ch)	((port[ch].wr[4] & 0x3c) == 0x30)
#define SYNC_MODE(ch)	(MONOSYNC(ch) || BISYNC(ch))

#define BIT_SYNC1	1
#define BIT_SYNC2	2

#define REGISTER_FIRST_SEND_EVENT(ch) { \
	if(port[ch].tx_clock != 0) { \
		if(port[ch].send_id == -1) { \
			register_event(this, EVENT_SEND + ch, 1000000.0 / port[ch].tx_clock / 2.0, false, &port[ch].send_id); \
		} \
	} else { \
		if(port[ch].tx_bits_x2_remain == 0) { \
			port[ch].tx_bits_x2_remain = 1; \
		} \
	} \
}

#define REGISTER_SEND_EVENT(ch) { \
	if(port[ch].tx_clock != 0) { \
		if(port[ch].send_id == -1) { \
			register_event(this, EVENT_SEND + ch, port[ch].tx_interval, false, &port[ch].send_id); \
		} \
	} else { \
		if(port[ch].tx_bits_x2_remain == 0) { \
			port[ch].tx_bits_x2_remain = port[ch].tx_bits_x2; \
		} \
	} \
}

#define CANCEL_SEND_EVENT(ch) { \
	if(port[ch].tx_clock != 0) { \
		if(port[ch].send_id != -1) { \
			cancel_event(this, port[ch].send_id); \
			port[ch].send_id = -1; \
		} \
	} else { \
		port[ch].tx_bits_x2_remain = 0; \
	} \
}

#define REGISTER_RECV_EVENT(ch) { \
	if(port[ch].rx_clock != 0) { \
		if(port[ch].recv_id == -1) { \
			register_event(this, EVENT_RECV + ch, port[ch].rx_interval, false, &port[ch].recv_id); \
		} \
	} else { \
		if(port[ch].rx_bits_x2_remain == 0) { \
			port[ch].rx_bits_x2_remain = port[ch].rx_bits_x2; \
		} \
	} \
}

#define CANCEL_RECV_EVENT(ch) { \
	if(port[ch].rx_clock != 0) { \
		if(port[ch].recv_id != -1) { \
			cancel_event(this, port[ch].recv_id); \
			port[ch].recv_id = -1; \
		} \
	} else { \
		port[ch].rx_bits_x2_remain = 0; \
	} \
}

void Z80SIO::initialize()
{
	for(int ch = 0; ch < 2; ch++) {
#ifdef HAS_UPD7201
		port[ch].send = new FIFO(16);
		port[ch].recv = new FIFO(16);
		port[ch].rtmp = new FIFO(16);
#else
		port[ch].send = new FIFO(1);
		port[ch].recv = new FIFO(4);
		port[ch].rtmp = new FIFO(8);
#endif
		// input signals
		port[ch].dcd = true;
		port[ch].cts = true;
		port[ch].sync = true;
		port[ch].sync_bit = 0;
	}
}

void Z80SIO::reset()
{
	for(int ch = 0; ch < 2; ch++) {
		port[ch].pointer = 0;
		port[ch].nextrecv_intr = false;
		port[ch].first_data = false;
		port[ch].over_flow = false;
		port[ch].under_run = false;
		port[ch].abort = false;
#ifdef HAS_UPD7201
		port[ch].tx_count = 0;
#endif
		port[ch].send->clear();
		port[ch].recv->clear();
		port[ch].rtmp->clear();
		port[ch].shift_reg = -1;
		port[ch].send_id = -1;
		port[ch].recv_id = -1;
		memset(port[ch].wr, 0, sizeof(port[ch].wr));
		// interrupt
		port[ch].err_intr = false;
		port[ch].recv_intr = 0;
		port[ch].stat_intr = false;
		port[ch].send_intr = false;
		port[ch].req_intr = false;
		port[ch].in_service = false;
	}
	iei = oei = true;
}

void Z80SIO::release()
{
	for(int ch = 0; ch < 2; ch++) {
		if(port[ch].send) {
			port[ch].send->release();
			delete port[ch].send;
		}
		if(port[ch].recv) {
			port[ch].recv->release();
			delete port[ch].recv;
		}
		if(port[ch].rtmp) {
			port[ch].rtmp->release();
			delete port[ch].rtmp;
		}
	}
}

/*
	0	ch.a data
	1	ch.a control
	2	ch.b data
	3	ch.b control
*/

void Z80SIO::write_io8(uint32_t addr, uint32_t data)
{
	int ch = (addr >> 1) & 1;
	bool update_intr_required = false;
	bool update_tx_timing_required = false;
	bool update_rx_timing_required = false;
	
	switch(addr & 3) {
	case 0:
	case 2:
		// send data
		if(port[ch].send_intr) {
			port[ch].send_intr = false;
			update_intr();
		}
		// register next event
		if(port[ch].wr[5] & 8) {
			int tx_data_bits = 5;
			if((data & 0xe0) == 0x00) {
				tx_data_bits = 5;
			} else if((data & 0xf0) == 0x80) {
				tx_data_bits = 4;
			} else if((data & 0xf8) == 0xc0) {
				tx_data_bits = 3;
			} else if((data & 0xfc) == 0xe0) {
				tx_data_bits = 2;
			} else if((data & 0xfe) == 0xf0) {
				tx_data_bits = 1;
			}
			if(port[ch].tx_data_bits != tx_data_bits) {
				port[ch].tx_data_bits = tx_data_bits;
				update_tx_timing(ch);
			}
			if((port[ch].wr[4] & 0x0c) != 0 && port[ch].shift_reg == -1 && port[ch].send->empty()) {
				// this is the first data
				CANCEL_SEND_EVENT(ch);
				REGISTER_FIRST_SEND_EVENT(ch);
			} else {
				REGISTER_SEND_EVENT(ch);
			}
		} else {
			CANCEL_SEND_EVENT(ch);
		}
#ifndef HAS_UPD7201
		port[ch].send->clear();
#endif
		port[ch].send->write(data);
#ifdef HAS_UPD7201
		port[ch].tx_count++;
#endif
		break;
	case 1:
	case 3:
		// control
#ifdef SIO_DEBUG
//		this->out_debug_log(_T("Z80SIO: ch=%d WR[%d]=%2x\n"), ch, port[ch].pointer, data);
#endif
		switch(port[ch].pointer) {
		case 0:
			switch(data & 0x38) {
			case 0x10:
				if(port[ch].stat_intr) {
					port[ch].stat_intr = false;
					update_intr_required = true;
				}
				break;
			case 0x18:
				// channel reset
				CANCEL_SEND_EVENT(ch);
				CANCEL_RECV_EVENT(ch);
				port[ch].nextrecv_intr = false;
				port[ch].first_data = false;
				port[ch].over_flow = false;
#ifdef HAS_UPD7201
				port[ch].tx_count = 0;	// is this correct ???
#endif
				port[ch].send->clear();
				port[ch].recv->clear();
				port[ch].rtmp->clear();
				port[ch].shift_reg = -1;
				memset(port[ch].wr, 0, sizeof(port[ch].wr));
				// interrupt
				if(port[ch].err_intr) {
					port[ch].err_intr = false;
					update_intr_required = true;
				}
				if(port[ch].recv_intr) {
					port[ch].recv_intr = 0;
					update_intr_required = true;
				}
				if(port[ch].stat_intr) {
					port[ch].stat_intr = false;
					update_intr_required = true;
				}
				if(port[ch].send_intr) {
					port[ch].send_intr = false;
					update_intr_required = true;
				}
				port[ch].req_intr = false;
				break;
			case 0x20:
				port[ch].nextrecv_intr = true;
				break;
			case 0x28:
				if(port[ch].send_intr) {
					port[ch].send_intr = false;
					update_intr_required = true;
				}
				break;
			case 0x30:
				port[ch].over_flow = false;
				if(port[ch].err_intr) {
					port[ch].err_intr = false;
					update_intr_required = true;
				}
				break;
			case 0x38:
				// end of interrupt
				if(ch == 0) {
					for(int c = 0; c < 2; c++) {
						if(port[c].in_service) {
							port[c].in_service = false;
							update_intr_required = true;
							break;
						}
					}
				}
				break;
			}
			switch(data & 0xc0) {
			case 0x40:
				// reset receive crc checker
				break;
			case 0x80:
				// reset transmit crc generator
				break;
			case 0xc0:
				// reset transmit underrun
				if(port[ch].under_run) {
					port[ch].under_run = false;
					if(port[ch].stat_intr) {
						port[ch].stat_intr = false;
						update_intr_required = true;
					}
				}
				break;
			}
			break;
		case 1:
		case 2:
			if(port[ch].wr[port[ch].pointer] != data) {
				update_intr_required = true;
			}
			break;
		case 3:
			if((data & 0x11) == 0x11) {
				// enter hunt/sync phase
				if(MONOSYNC(ch)) {
#ifdef SIO_DEBUG
					this->out_debug_log(_T("Z80SIO: ch=%d enter hunt/sync phase (monosync)\n"), ch);
#endif
					port[ch].sync_bit = BIT_SYNC1;
				} else if(BISYNC(ch)) {
#ifdef SIO_DEBUG
					this->out_debug_log(_T("Z80SIO: ch=%d enter hunt/sync phase (bisync)\n"), ch);
#endif
					port[ch].sync_bit = BIT_SYNC1 | BIT_SYNC2;
				}
				port[ch].sync = false;
				write_signals(&port[ch].outputs_sync, 0xffffffff);
			}
			if((port[ch].wr[3] & 0xc0) != (data & 0xc0)) {
				update_rx_timing_required = true;
			}
			break;
		case 4:
			if((port[ch].wr[4] & 0xcd) != (data & 0xcd)) {
				update_tx_timing_required = update_rx_timing_required = true;
			}
			break;
		case 5:
			if((uint32_t)(port[ch].wr[5] & 2) != (data & 2)) {
				// rts
				write_signals(&port[ch].outputs_rts, (data & 2) ? 0 : 0xffffffff);
			}
			if((uint32_t)(port[ch].wr[5] & 0x80) != (data & 0x80)) {
				// dtr
				write_signals(&port[ch].outputs_dtr, (data & 0x80) ? 0 : 0xffffffff);
			}
			if(data & 8) {
				if((port[ch].wr[4] & 0x0c) != 0 && port[ch].shift_reg == -1 && !port[ch].send->empty()) {
//					CANCEL_SEND_EVENT(ch);
					REGISTER_FIRST_SEND_EVENT(ch);
				} else {
					REGISTER_SEND_EVENT(ch);
				}
			} else {
				CANCEL_SEND_EVENT(ch);
			}
			if(data & 0x10) {
				// send break
				write_signals(&port[ch].outputs_break, 0xffffffff);
			}
			if((port[ch].wr[5] & 0x60) != (data & 0x60)) {
				update_tx_timing_required = true;
			}
			break;
		}
		port[ch].wr[port[ch].pointer] = data;
		if(update_intr_required) {
			update_intr();
		}
		if(update_tx_timing_required) {
			update_tx_timing(ch);
		}
		if(update_rx_timing_required) {
			update_rx_timing(ch);
		}
		port[ch].pointer = (port[ch].pointer == 0) ? (data & 7) : 0;
		break;
	}
}

uint32_t Z80SIO::read_io8(uint32_t addr)
{
	int ch = (addr >> 1) & 1;
	uint32_t val = 0;
	
	switch(addr & 3) {
	case 0:
	case 2:
		// recv data;
		if(port[ch].recv_intr) {
			// cancel pending interrupt
			if(--port[ch].recv_intr == 0) {
				update_intr();
			}
		}
		// for polling case (thanks YAT)
		if(!SYNC_MODE(ch)) {
			if(port[ch].recv->empty()) {
				int data = port[ch].rtmp->read();
				port[ch].recv->write(data);
			}
		}
		return port[ch].recv->read();
	case 1:
	case 3:
		if(port[ch].pointer == 0) {
			if(!port[ch].recv->empty()) {
				val |= 1;
			}
			if(ch == 0 && (port[0].req_intr || port[1].req_intr)) {
				val |= 2;
			}
			if(!port[ch].send->full()) {
				val |= 4;	// ???
			}
			if(!port[ch].dcd) {
				val |= 8;
			}
			if(!port[ch].sync) {
				val |= 0x10;
			}
			if(!port[ch].cts) {
				val |= 0x20;
			}
			if(port[ch].under_run) {
				val |= 0x40;
			}
			if(port[ch].abort) {
				val |= 0x80;
			}
		} else if(port[ch].pointer == 1) {
			val = 0x8e;	// TODO
			if(port[ch].send->empty()) {
				val |= 1;
			}
			if(port[ch].over_flow) {
				val |= 0x20;
			}
		} else if(port[ch].pointer == 2) {
			val = port[ch].vector;
#ifdef HAS_UPD7201
		} else if(port[ch].pointer == 3) {
			val = port[ch].tx_count & 0xff;
			port[ch].tx_count_hi = port[ch].tx_count >> 8;
		} else if(port[ch].pointer == 4) {
//			val = (port[ch].tx_count >> 8) & 0xff;
			val = port[ch].tx_count_hi;
#endif
		}
		port[ch].pointer = 0;
		return val;
	}
	return 0xff;
}

void Z80SIO::write_signal(int id, uint32_t data, uint32_t mask)
{
	// recv data
	int ch = id & 1;
	bool signal = ((data & mask) != 0);
	
	switch(id) {
	case SIG_Z80SIO_RECV_CH0:
	case SIG_Z80SIO_RECV_CH1:
		// recv data
		REGISTER_RECV_EVENT(ch);
		if(port[ch].rtmp->empty()) {
			port[ch].first_data = true;
		}
		port[ch].rtmp->write(data & mask);
		break;
	case SIG_Z80SIO_BREAK_CH0:
	case SIG_Z80SIO_BREAK_CH1:
		// recv break
		if((data & mask) && !port[ch].abort) {
			port[ch].abort = true;
			if(!port[ch].stat_intr) {
				port[ch].stat_intr = true;
				update_intr();
			}
		}
		break;
	case SIG_Z80SIO_DCD_CH0:
	case SIG_Z80SIO_DCD_CH1:
		if(port[ch].dcd != signal) {
			port[ch].dcd = signal;
			if(!signal && (port[ch].wr[3] & 0x20)) {
				// auto enables
				port[ch].wr[3] |= 1;
			}
			if(!port[ch].stat_intr) {
				port[ch].stat_intr = true;
				update_intr();
			}
		}
		break;
	case SIG_Z80SIO_CTS_CH0:
	case SIG_Z80SIO_CTS_CH1:
		if(port[ch].cts != signal) {
			port[ch].cts = signal;
			if(!signal && (port[ch].wr[3] & 0x20)) {
				// auto enables
				if((port[ch].wr[4] & 0x0c) != 0 && port[ch].shift_reg == -1 && !port[ch].send->empty()) {
//					CANCEL_SEND_EVENT(ch);
					REGISTER_FIRST_SEND_EVENT(ch);
				} else {
					REGISTER_SEND_EVENT(ch);
				}
				port[ch].wr[5] |= 8;
			}
			if(!port[ch].stat_intr) {
				port[ch].stat_intr = true;
				update_intr();
			}
		}
	case SIG_Z80SIO_SYNC_CH0:
	case SIG_Z80SIO_SYNC_CH1:
		if(port[ch].sync != signal) {
			port[ch].sync = signal;
			if(!port[ch].stat_intr) {
				port[ch].stat_intr = true;
				update_intr();
			}
		}
		break;
	case SIG_Z80SIO_TX_CLK_CH0:
	case SIG_Z80SIO_TX_CLK_CH1:
		if(port[ch].prev_tx_clock_signal != signal) {
			if(port[ch].tx_bits_x2_remain > 0 && --port[ch].tx_bits_x2_remain == 0) {
				event_callback(EVENT_SEND + ch, 0);
			}
			port[ch].prev_tx_clock_signal = signal;
		}
		break;
	case SIG_Z80SIO_RX_CLK_CH0:
	case SIG_Z80SIO_RX_CLK_CH1:
		if(port[ch].prev_rx_clock_signal != signal) {
			if(port[ch].rx_bits_x2_remain > 0 && --port[ch].rx_bits_x2_remain == 0) {
				event_callback(EVENT_RECV + ch, 0);
			}
			port[ch].prev_rx_clock_signal = signal;
		}
		break;
	case SIG_Z80SIO_CLEAR_CH0:
	case SIG_Z80SIO_CLEAR_CH1:
		// hack: clear recv buffer
		if(data & mask) {
			CANCEL_RECV_EVENT(ch);
			port[ch].rtmp->clear();
			port[ch].recv->clear();
			if(port[ch].recv_intr) {
				port[ch].recv_intr = 0;
				update_intr();
			}
		}
		break;
	}
}

void Z80SIO::event_callback(int event_id, int err)
{
	int ch = event_id & 1;
	
	if(event_id & EVENT_SEND) {
		// send
		port[ch].send_id = -1;
		port[ch].tx_bits_x2_remain = 0;
		
		bool under_run = true;
		
		if(port[ch].shift_reg != -1) {
			// send data in shift register
			write_signals(&port[ch].outputs_send, port[ch].shift_reg);
			port[ch].shift_reg = -1;
			under_run = false;
		}
		if(!port[ch].send->empty()) {
			// load data in send buffer to shift register
			port[ch].shift_reg = port[ch].send->read();
			under_run = false;
		}
		if(under_run) {
			// underrun interrupt
			if(!port[ch].under_run) {
				port[ch].under_run = true;
				if(!port[ch].stat_intr) {
					port[ch].stat_intr = true;
					update_intr();
				}
			}
		}
		if(port[ch].send->empty()) {
			// transmitter interrupt
			if(!port[ch].send_intr) {
				port[ch].send_intr = true;
				update_intr();
			}
			write_signals(&port[ch].outputs_txdone, 0xffffffff);
		}
		REGISTER_SEND_EVENT(ch);
	} else if(event_id & EVENT_RECV) {
		// recv
		port[ch].recv_id = -1;
		port[ch].rx_bits_x2_remain = 0;
		
		if(!(port[ch].wr[3] & 1)) {
			REGISTER_RECV_EVENT(ch);
			return;
		}
		bool update_intr_required = false;
		
		if(port[ch].recv->full()) {
			// overflow
			if(!port[ch].over_flow) {
				port[ch].over_flow = true;
				if(!port[ch].err_intr) {
					port[ch].err_intr = true;
					update_intr_required = true;
				}
			}
		} else {
			// no error
			int data = port[ch].rtmp->read();
			
			if(SYNC_MODE(ch) && port[ch].sync_bit != 0) {
				// receive sync data in monosync/bisync mode ?
				if(port[ch].sync_bit & BIT_SYNC1) {
					if(data != port[ch].wr[6]) {
						goto request_next_data;
					}
#ifdef SIO_DEBUG
					this->out_debug_log(_T("Z80SIO: ch=%d recv sync1\n"), ch);
#endif
					port[ch].sync_bit &= ~BIT_SYNC1;
				} else if(port[ch].sync_bit & BIT_SYNC2) {
					if(data != port[ch].wr[7]) {
						port[ch].sync_bit |= BIT_SYNC1;
						goto request_next_data;
					}
#ifdef SIO_DEBUG
					this->out_debug_log(_T("Z80SIO: ch=%d recv sync2\n"), ch);
#endif
					port[ch].sync_bit &= ~BIT_SYNC2;
				}
				if(port[ch].sync_bit == 0) {
#ifdef SIO_DEBUG
					this->out_debug_log(_T("Z80SIO: ch=%d leave hunt/sync phase\n"), ch);
#endif
					if(!port[ch].stat_intr) {
						port[ch].stat_intr = true;
						update_intr_required = true;
					}
					port[ch].sync = true;
					write_signals(&port[ch].outputs_sync, 0);
				}
				if(port[ch].wr[3] & 2) {
					// sync char is not loaded into buffer
					goto request_next_data;
				}
			}
			// load received data into buffer
#ifdef SIO_DEBUG
			this->out_debug_log(_T("Z80SIO: ch=%d recv %2x\n"), ch, data);
#endif
			port[ch].recv->write(data);
			
			// quit abort
			if(port[ch].abort) {
				port[ch].abort = false;
				if(!port[ch].stat_intr) {
					port[ch].stat_intr = true;
					update_intr_required = true;
				}
			}
			
			// check receive interrupt
			bool req = false;
			if((port[ch].wr[1] & 0x18) == 8 && (port[ch].first_data || port[ch].nextrecv_intr)) {
				req = true;
			} else if(port[ch].wr[1] & 0x10) {
				req = true;
			}
			if(req) {
				if(port[ch].recv_intr++ == 0) {
					update_intr_required = true;
				}
			}
			port[ch].first_data = port[ch].nextrecv_intr = false;
		}
request_next_data:
		bool first_data = port[ch].first_data;
		if(port[ch].rtmp->empty()) {
			// request data in this message
			write_signals(&port[ch].outputs_rxdone, 0xffffffff);
		}
		if(port[ch].rtmp->empty()) {
			// no data received
#ifdef SIO_DEBUG
			this->out_debug_log(_T("Z80SIO: ch=%d end of block\n"), ch);
#endif
			port[ch].recv_id = -1;
		} else {
			REGISTER_RECV_EVENT(ch);
			port[ch].first_data = first_data;
		}
		if(update_intr_required) {
			update_intr();
		}
	}
}

void Z80SIO::update_tx_timing(int ch)
{
	port[ch].tx_bits_x2 = (port[ch].wr[4] & 1) * 2;
	switch(port[ch].wr[5] & 0x60) {
	case 0x00: port[ch].tx_bits_x2 += 2 * port[ch].tx_data_bits; break;
	case 0x20: port[ch].tx_bits_x2 += 2 * 7; break;
	case 0x40: port[ch].tx_bits_x2 += 2 * 6; break;
	case 0x60: port[ch].tx_bits_x2 += 2 * 8; break;
	}
	switch(port[ch].wr[4] & 0x0c) {
	case 0x00: port[ch].tx_bits_x2 += 0; break;	// sync mode
	case 0x04: port[ch].tx_bits_x2 += 4; break;	// 2 * (1 + 1)
	case 0x08: port[ch].tx_bits_x2 += 5; break;	// 2 * (1 + 1.5)
	case 0x0c: port[ch].tx_bits_x2 += 6; break;	// 2 * (1 + 2)
	}
	switch(port[ch].wr[4] & 0xc0) {
	case 0x40: port[ch].tx_bits_x2 *= 16; break;
	case 0x80: port[ch].tx_bits_x2 *= 32; break;
	case 0xc0: port[ch].tx_bits_x2 *= 64; break;
	}
	if(port[ch].tx_clock != 0) {
		port[ch].tx_interval = 1000000.0 / port[ch].tx_clock * (double)port[ch].tx_bits_x2 / 2.0;
	}
}

void Z80SIO::update_rx_timing(int ch)
{
	port[ch].rx_bits_x2 = (port[ch].wr[4] & 1) * 2;
	switch(port[ch].wr[3] & 0xc0) {
	case 0x00: port[ch].rx_bits_x2 += 2 * 5; break;
	case 0x40: port[ch].rx_bits_x2 += 2 * 7; break;
	case 0x80: port[ch].rx_bits_x2 += 2 * 6; break;
	case 0xc0: port[ch].rx_bits_x2 += 2 * 8; break;
	}
	switch(port[ch].wr[4] & 0x0c) {
	case 0x00: port[ch].rx_bits_x2 += 0; break;	// sync mode
	case 0x04: port[ch].rx_bits_x2 += 4; break;	// 2 * (1 + 1)
	case 0x08: port[ch].rx_bits_x2 += 5; break;	// 2 * (1 + 1.5)
	case 0x0c: port[ch].rx_bits_x2 += 6; break;	// 2 * (1 + 2)
	}
	switch(port[ch].wr[4] & 0xc0) {
	case 0x40: port[ch].rx_bits_x2 *= 16; break;
	case 0x80: port[ch].rx_bits_x2 *= 32; break;
	case 0xc0: port[ch].rx_bits_x2 *= 64; break;
	}
	if(port[ch].rx_clock != 0) {
		port[ch].rx_interval = 1000000.0 / port[ch].rx_clock * (double)port[ch].rx_bits_x2 / 2.0;
	}
}

void Z80SIO::set_intr_iei(bool val)
{
	if(iei != val) {
		iei = val;
		update_intr();
	}
}

#define set_intr_oei(val) { \
	if(oei != val) { \
		oei = val; \
		if(d_child) { \
			d_child->set_intr_iei(oei); \
		} \
	} \
}

void Z80SIO::update_intr()
{
	bool next;
	
	// set oei signal
	if((next = iei) == true) {
		for(int ch = 0; ch < 2; ch++) {
			if(port[ch].in_service) {
				next = false;
				break;
			}
		}
	}
	set_intr_oei(next);
	
	// check interrupt status
	for(int ch = 0; ch < 2; ch++) {
		if(port[ch].err_intr) {
			port[ch].req_intr = true;
			port[ch].affect = (ch ? 0 : 4) | 3;
		} else if(port[ch].recv_intr && (port[ch].wr[1] & 0x18)) {
			port[ch].req_intr = true;
			port[ch].affect = (ch ? 0 : 4) | 2;
		} else if(port[ch].stat_intr && (port[ch].wr[1] & 1)) {
			port[ch].req_intr = true;
			port[ch].affect = (ch ? 0 : 4) | 1;
		} else if(port[ch].send_intr && (port[ch].wr[1] & 2)) {
			port[ch].req_intr = true;
			port[ch].affect = (ch ? 0 : 4) | 0;
		} else {
			port[ch].req_intr = false;
		}
	}
	
	// create vector
	if(port[1].wr[1] & 4) {
#ifdef HAS_UPD7201
		uint8_t affect = 7;	// no interrupt pending
		for(int ch = 0; ch < 2; ch++) {
			if(port[ch].in_service) {
				break;
			}
			if(port[ch].req_intr) {
				affect = port[ch].affect;
				break;
			}
		}
		uint8_t mode = port[0].wr[2] & 0x38;
		if(mode == 0 || mode == 8 || mode == 0x20 || mode == 0x28 || mode == 0x38) {
			port[1].vector = (port[1].wr[2] & 0xe3) | (affect << 2);	// 8085
		} else {
			port[1].vector = (port[1].wr[2] & 0xf8) | (affect << 0);	// 8086
		}
#else
		uint8_t affect = 3;	// no interrupt pending
		for(int ch = 0; ch < 2; ch++) {
			if(port[ch].in_service) {
				break;
			}
			if(port[ch].req_intr) {
				affect = port[ch].affect;
				break;
			}
		}
		port[1].vector = (port[1].wr[2] & 0xf1) | (affect << 1);
#endif
	} else {
		port[1].vector = port[1].wr[2];
	}
	
	// set int signal
	if((next = iei) == true) {
		next = false;
		for(int ch = 0; ch < 2; ch++) {
			if(port[ch].in_service) {
				break;
			}
			if(port[ch].req_intr) {
				next = true;
				break;
			}
		}
	}
	if(d_cpu) {
		d_cpu->set_intr_line(next, true, intr_bit);
	}
}

uint32_t Z80SIO::get_intr_ack()
{
	// ack (M1=IORQ=L)
	for(int ch = 0; ch < 2; ch++) {
		if(port[ch].in_service) {
			// invalid interrupt status
			return 0xff;
		}
		// priority is error > receive > status > send ???
		if(port[ch].err_intr) {
			port[ch].err_intr = false;
			port[ch].in_service = true;
		} else if(port[ch].recv_intr && (port[ch].wr[1] & 0x18)) {
//			port[ch].recv_intr = 0;	// thanks YAT
			port[ch].in_service = true;
		} else if(port[ch].stat_intr && (port[ch].wr[1] & 1)) {
			port[ch].stat_intr = false;
			port[ch].in_service = true;
		} else if(port[ch].send_intr && (port[ch].wr[1] & 2)) {
			port[ch].send_intr = false;
			port[ch].in_service = true;
		}
		if(port[ch].in_service) {
			uint8_t vector = port[1].vector;
			update_intr();
			return vector;
		}
	}
	if(d_child) {
		return d_child->get_intr_ack();
	}
	return 0xff;
}

void Z80SIO::notify_intr_reti()
{
	// detect RETI
	for(int ch = 0; ch < 2; ch++) {
		if(port[ch].in_service) {
			port[ch].in_service = false;
			update_intr();
			return;
		}
	}
	if(d_child) {
		d_child->notify_intr_reti();
	}
	update_intr();
}

#define STATE_VERSION	3

bool Z80SIO::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int i = 0; i < 2; i++) {
		state_fio->StateValue(port[i].pointer);
		state_fio->StateArray(port[i].wr, sizeof(port[i].wr), 1);
		state_fio->StateValue(port[i].vector);
		state_fio->StateValue(port[i].affect);
		state_fio->StateValue(port[i].nextrecv_intr);
		state_fio->StateValue(port[i].first_data);
		state_fio->StateValue(port[i].over_flow);
		state_fio->StateValue(port[i].under_run);
		state_fio->StateValue(port[i].abort);
		state_fio->StateValue(port[i].sync);
		state_fio->StateValue(port[i].sync_bit);
#ifdef HAS_UPD7201
		state_fio->StateValue(port[i].tx_count);
		state_fio->StateValue(port[i].tx_count_hi);
#endif
		state_fio->StateValue(port[i].tx_clock);
		state_fio->StateValue(port[i].tx_interval);
		state_fio->StateValue(port[i].rx_clock);
		state_fio->StateValue(port[i].rx_interval);
		state_fio->StateValue(port[i].tx_data_bits);
		state_fio->StateValue(port[i].tx_bits_x2);
		state_fio->StateValue(port[i].tx_bits_x2_remain);
		state_fio->StateValue(port[i].rx_bits_x2);
		state_fio->StateValue(port[i].rx_bits_x2_remain);
		state_fio->StateValue(port[i].prev_tx_clock_signal);
		state_fio->StateValue(port[i].prev_rx_clock_signal);
		if(!port[i].send->process_state((void *)state_fio, loading)) {
			return false;
		}
		if(!port[i].recv->process_state((void *)state_fio, loading)) {
			return false;
		}
		if(!port[i].rtmp->process_state((void *)state_fio, loading)) {
			return false;
		}
		state_fio->StateValue(port[i].shift_reg);
		state_fio->StateValue(port[i].send_id);
		state_fio->StateValue(port[i].recv_id);
		state_fio->StateValue(port[i].err_intr);
		state_fio->StateValue(port[i].recv_intr);
		state_fio->StateValue(port[i].stat_intr);
		state_fio->StateValue(port[i].send_intr);
		state_fio->StateValue(port[i].req_intr);
		state_fio->StateValue(port[i].in_service);
		state_fio->StateValue(port[i].dcd);
		state_fio->StateValue(port[i].cts);
	}
	state_fio->StateValue(iei);
	state_fio->StateValue(oei);
	state_fio->StateValue(intr_bit);
	return true;
}

