/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.14 -

	[ i8251 ]
*/

#include "i8251.h"
#include "../fifo.h"

// max 256kbytes
#define BUFFER_SIZE	0x40000
// 100usec/byte
#define RECV_DELAY	100
#define SEND_DELAY	100

#define EVENT_RECV	0
#define EVENT_SEND	1

#define TXRDY		0x01
#define RXRDY		0x02
#define TXE		0x04
#define PE		0x08
#define OE		0x10
#define FE		0x20
#define SYNDET		0x40
#define DSR		0x80

#define MODE_CLEAR	0
#define MODE_SYNC	1
#define MODE_ASYNC	2
#define MODE_SYNC1	3
#define MODE_SYNC2	4

#define RECV_BREAK	-1

void I8251::initialize()
{
	recv_buffer = new FIFO(BUFFER_SIZE);
	send_buffer = new FIFO(4);
	status = TXRDY | TXE;
}

void I8251::release()
{
	recv_buffer->release();
	delete recv_buffer;
	send_buffer->release();
	delete send_buffer;
}

void I8251::reset()
{
	mode = MODE_CLEAR;
	recv = 0x00;	// XM8 version 1.10
//	recv = 0xff;
	
	// dont reset dsr
	status &= DSR;
	status |= TXRDY | TXE;
	txen = rxen = loopback = false;
	
	recv_buffer->clear();
	send_buffer->clear();
	recv_id = send_id = -1;
}

void I8251::write_io8(uint32_t addr, uint32_t data)
{
	if(addr & 1) {
		switch(mode) {
		case MODE_CLEAR:
			if(data & 3) {
				mode = MODE_ASYNC;
			} else if(data & 0x80) {
				mode = MODE_SYNC2;	// 1char
			} else {
				mode = MODE_SYNC1;	// 2chars
			}
			break;
		case MODE_SYNC1:
			mode = MODE_SYNC2;
			break;
		case MODE_SYNC2:
			mode = MODE_SYNC;
			break;
		case MODE_ASYNC:
		case MODE_SYNC:
			if(data & 0x40) {
				mode = MODE_CLEAR;
				break;
			}
			if(data & 0x10) {
				status &= ~(PE | OE | FE);
			}
			// dtr
			write_signals(&outputs_dtr, (data & 0x02) ? 0xffffffff : 0);
			// break
			write_signals(&outputs_brk, (data & 0x08) ? 0xffffffff : 0);
			// rts
			write_signals(&outputs_rts, (data & 0x20) ? 0xffffffff : 0);
			// rxen
			rxen = ((data & 0x04) != 0);
			if(rxen && !recv_buffer->empty() && recv_id == -1) {
				register_event(this, EVENT_RECV, RECV_DELAY, false, &recv_id);
			}
			// txen
			txen = ((data & 0x01) != 0);
			if(txen && !send_buffer->empty() && send_id == -1) {
				register_event(this, EVENT_SEND, SEND_DELAY, false, &send_id);
			}
			// note: when txen=false, txrdy signal must be low
			break;
		}
	} else {
		if(status & TXRDY) {
			send_buffer->write(data);
			// txrdy
			if(send_buffer->full()) {
				status &= ~TXRDY;
				write_signals(&outputs_txrdy, 0);
			}
			// txempty
			status &= ~TXE;
			write_signals(&outputs_txe, 0);
			// register event
			if(txen && send_id == -1) {
				register_event(this, EVENT_SEND, SEND_DELAY, false, &send_id);
			}
		}
	}
}

uint32_t I8251::read_io8(uint32_t addr)
{
	if(addr & 1) {
		// XM8 version 1.10
		if(!txen) {
			return status & ~(TXRDY | TXE);
		}
		return status;
	} else {
		if(status & RXRDY) {
			status &= ~RXRDY;
			write_signals(&outputs_rxrdy, 0);
		}
		return recv;
	}
}

void I8251::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_I8251_RECV) {
		recv_buffer->write(data & mask);
		// register event
		if(rxen && !recv_buffer->empty() && recv_id == -1) {
			register_event(this, EVENT_RECV, RECV_DELAY, false, &recv_id);
		}
	} else if(id == SIG_I8251_BREAK) {
		if(data & mask) {
			recv_buffer->write(RECV_BREAK);
			// register event
			if(rxen && !recv_buffer->empty() && recv_id == -1) {
				register_event(this, EVENT_RECV, RECV_DELAY, false, &recv_id);
			}
		}
	} else if(id == SIG_I8251_DSR) {
		if(data & mask) {
			status |= DSR;
		} else {
			status &= ~DSR;
		}
	} else if(id == SIG_I8251_CLEAR) {
		recv_buffer->clear();
	} else if(id == SIG_I8251_LOOPBACK) {
		loopback = ((data & mask) != 0);
	}
}

void I8251::event_callback(int event_id, int err)
{
	if(event_id == EVENT_RECV) {
		if(rxen && !(status & RXRDY)) {
			if(!recv_buffer->empty()) {
				int val = recv_buffer->read();
				if(val == RECV_BREAK) {
					// break
					status |= SYNDET;
					write_signals(&outputs_syndet, 0xffffffff);
				} else {
					recv = (uint8_t)val;
					status |= RXRDY;
					write_signals(&outputs_rxrdy, 0xffffffff);
				}
			}
		}
		// if data is still left in buffer, register event for next data
		if(rxen && !recv_buffer->empty()) {
			register_event(this, EVENT_RECV, RECV_DELAY, false, &recv_id);
		} else {
			recv_id = -1;
		}
	} else if(event_id == EVENT_SEND) {
		if(txen && !send_buffer->empty()) {
			uint8_t send = send_buffer->read();
			if(loopback) {
				// send to this device
				write_signal(SIG_I8251_RECV, send, 0xff);
			} else {
				// send to external devices
				write_signals(&outputs_out, send);
			}
			// txrdy
			status |= TXRDY;
			write_signals(&outputs_txrdy, 0xffffffff);
			// txe
			if(send_buffer->empty()) {
				status |= TXE;
				write_signals(&outputs_txe, 0xffffffff);
			}
		}
		// if data is still left in buffer, register event for next data
		if(txen && !send_buffer->empty()) {
			register_event(this, EVENT_SEND, SEND_DELAY, false, &send_id);
		} else {
			send_id = -1;
		}
	}
}

#define STATE_VERSION	1

bool I8251::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(recv);
	state_fio->StateValue(status);
	state_fio->StateValue(mode);
	state_fio->StateValue(txen);
	state_fio->StateValue(rxen);
	state_fio->StateValue(loopback);
	if(!recv_buffer->process_state((void *)state_fio, loading)) {
		return false;
	}
	if(!send_buffer->process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(recv_id);
	state_fio->StateValue(send_id);
	return true;
}

