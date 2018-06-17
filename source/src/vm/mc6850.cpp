/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2017.05.09-

	[ MC6850 ]
*/

#include "mc6850.h"
#include "../fifo.h"

// max 256kbytes
#define BUFFER_SIZE	0x40000
// 100usec/byte
#define RECV_DELAY	100
#define SEND_DELAY	100

#define EVENT_RECV	0
#define EVENT_SEND	1

#define STAT_RDRF	0x01
#define STAT_TDRE	0x02
#define STAT_DCD	0x04
#define STAT_CTS	0x08
#define STAT_FE		0x10
#define STAT_OVRN	0x20
#define STAT_PE		0x40
#define STAT_IRQ	0x80

#define RECV_DCS	-1

void MC6850::initialize()
{
	recv_buffer = new FIFO(BUFFER_SIZE);
	send_buffer = new FIFO(4);
	
	recv = ctrl = 0x00;
}

void MC6850::release()
{
	recv_buffer->release();
	delete recv_buffer;
	send_buffer->release();
	delete send_buffer;
}

void MC6850::reset()
{
	recv_buffer->clear();
	send_buffer->clear();
	recv_id = send_id = -1;
	
	status = STAT_TDRE;
	update_irq();
}

void MC6850::write_io8(uint32_t addr, uint32_t data)
{
	if(addr & 1) {
		if(status & STAT_TDRE) {
			send_buffer->write(data);
			// tdre
			if(send_buffer->full()) {
				status &= ~STAT_TDRE;
				update_irq();
			}
			// register event
			if(send_id == -1) {
				register_event(this, EVENT_SEND, SEND_DELAY, false, &send_id);
			}
		}
	} else {
		ctrl = data;
		update_irq();
		write_signals(&outputs_rts, (data & 0x60) == 0x40 ? 0xffffffff : 0);
	}
}

uint32_t MC6850::read_io8(uint32_t addr)
{
	if(addr & 1) {
		status &= ~STAT_RDRF;
		update_irq();
		return recv;
	} else {
		return status;
	}
}

void MC6850::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MC6850_RECV) {
		recv_buffer->write(data & mask);
		// register event
		if(!recv_buffer->empty() && recv_id == -1) {
			register_event(this, EVENT_RECV, RECV_DELAY, false, &recv_id);
		}
	} else if(id == SIG_MC6850_DCD) {
		if(data & mask) {
			recv_buffer->write(RECV_DCS);
			// register event
			if(!recv_buffer->empty() && recv_id == -1) {
				register_event(this, EVENT_RECV, RECV_DELAY, false, &recv_id);
			}
		}
	} else if(id == SIG_MC6850_CTS) {
		if(data & mask) {
			status |= STAT_CTS;
		} else {
			status &= ~STAT_CTS;
		}
		update_irq();
	} else if(id == SIG_MC6850_CLEAR) {
		recv_buffer->clear();
	}
}

void MC6850::event_callback(int event_id, int err)
{
	if(event_id == EVENT_RECV) {
		if(!(status & STAT_RDRF)) {
			if(!recv_buffer->empty()) {
				int val = recv_buffer->read();
				if(val == RECV_DCS) {
					status |= STAT_DCD;
				} else {
					recv = (uint8_t)val;
					status &= ~STAT_DCD;
					status |= STAT_RDRF;
				}
				update_irq();
			}
		}
		// if data is still left in buffer, register event for next data
		if(!recv_buffer->empty()) {
			register_event(this, EVENT_RECV, RECV_DELAY, false, &recv_id);
		} else {
			recv_id = -1;
		}
	} else if(event_id == EVENT_SEND) {
		if(!(status & STAT_CTS)) {
			if(!send_buffer->empty()) {
				uint8_t send = send_buffer->read();
				write_signals(&outputs_out, send);
				// tdre
				status |= STAT_TDRE;
				update_irq();
			}
		}
		// if data is still left in buffer, register event for next data
		if(!send_buffer->empty()) {
			register_event(this, EVENT_SEND, SEND_DELAY, false, &send_id);
		} else {
			send_id = -1;
		}
	}
}

void MC6850::update_irq()
{
	if(((ctrl & 0x60) == 0x20 && (status & STAT_TDRE) && !(status & STAT_CTS)) || ((ctrl & 0x80) && (status & STAT_RDRF))) {
		status |= STAT_IRQ;
	} else {
		status &= ~STAT_IRQ;
	}
	write_signals(&outputs_irq, (status & STAT_IRQ) ? 0xffffffff : 0);
}

#define STATE_VERSION	1

#include "../statesub.h"

void MC6850::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_INT32(this_device_id);
	
	DECL_STATE_ENTRY_UINT8(recv);
	DECL_STATE_ENTRY_UINT8(status);
	DECL_STATE_ENTRY_UINT8(ctrl);
	DECL_STATE_ENTRY_FIFO(recv_buffer);
	DECL_STATE_ENTRY_FIFO(send_buffer);
	DECL_STATE_ENTRY_INT32(recv_id);
	DECL_STATE_ENTRY_INT32(send_id);

	leave_decl_state();
}
void MC6850::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}

//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
//	
//	state_fio->FputUint8(recv);
//	state_fio->FputUint8(status);
//	state_fio->FputUint8(ctrl);
//	recv_buffer->save_state((void *)state_fio);
//	send_buffer->save_state((void *)state_fio);
//	state_fio->FputInt32(recv_id);
//	state_fio->FputInt32(send_id);
}

bool MC6850::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) return false;
	
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	recv = state_fio->FgetUint8();
//	status = state_fio->FgetUint8();
//	ctrl = state_fio->FgetUint8();
//	if(!recv_buffer->load_state((void *)state_fio)) {
//		return false;
//	}
//	if(!send_buffer->load_state((void *)state_fio)) {
//		return false;
//	}
//	recv_id = state_fio->FgetInt32();
//	send_id = state_fio->FgetInt32();
	return true;
}

