/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.17 -

	[ Z80PIO ]
*/

#include "z80pio.h"

#define MODE_OUTPUT	0x00
#define MODE_INPUT	0x40
#define MODE_BIDIRECT	0x80
#define MODE_CONTROL	0xc0

void Z80PIO::reset()
{
	for(int ch = 0; ch < 2; ch++) {
		port[ch].wreg = 0xffffff00;	// force output the first written data
		port[ch].mode = MODE_INPUT;
		port[ch].ctrl1 = 0;
		port[ch].ctrl2 = 0;
		port[ch].dir = 0xff;
		port[ch].mask = 0xff;
		port[ch].vector = 0;
		port[ch].set_dir = false;
		port[ch].set_mask = false;
		// status
		port[ch].ready_signal = -1;
		port[ch].input_empty = false;
		port[ch].output_ready = false;
		// interrupt
		port[ch].enb_intr = false;
		port[ch].req_intr = false;
		port[ch].in_service = false;
	}
	iei = oei = true;
	update_ready();
}

/*
	AD0 is to C/~D, AD1 is to B/~A:
	
	0	port a data
	1	port a control
	2	port b data
	3	port b control
*/

void Z80PIO::write_io8(uint32_t addr, uint32_t data)
{
	int ch = (addr >> 1) & 1;
	bool mode_changed = false;
	data &= 0xff;
	
	switch(addr & 3) {
	case 0:
	case 2:
		// data
		if(port[ch].wreg != data) {
			write_signals(&port[ch].outputs_data, data);
			port[ch].wreg = data;
		}
		if(port[ch].output_ready) {
			port[ch].output_ready = false;
			update_ready();
		}
		port[ch].output_ready = true;
		update_ready();
		if(port[ch].mode == MODE_OUTPUT || port[ch].mode == MODE_BIDIRECT) {
			if(!port[ch].hand_shake) {
				// the peripheral reads the data and sets the strobe signal immediately
				if(!(ch == 1 && port[0].mode == MODE_BIDIRECT)) {
					write_signal(SIG_Z80PIO_STROBE_A + ch, 1, 1);
				}
			}
		} else if(port[ch].mode == MODE_CONTROL) {
			check_mode3_intr(ch);
		}
		break;
	case 1:
	case 3:
		// control
		if(port[ch].set_dir) {
			port[ch].dir = data;
			port[ch].set_dir = false;
		} else if(port[ch].set_mask) {
			port[ch].mask = data;
			port[ch].set_mask = false;
			port[ch].enb_intr = port[ch].enb_intr_tmp;
			update_intr();
		} else if(!(data & 0x01)) {
			port[ch].vector = data;
		} else if((data & 0x0f) == 0x03) {
			port[ch].enb_intr = ((data & 0x80) != 0);
			port[ch].ctrl2 = data;
			update_intr();
		} else if((data & 0x0f) == 0x07) {
			port[ch].enb_intr = ((data & 0x80) != 0);
			port[ch].ctrl1 = data;
			if(data & 0x10) {
				port[ch].set_mask = true;
				// canel pending interrup ???
				port[ch].req_intr = false;
				// disable interrupt until the mask register is written
				port[ch].enb_intr_tmp = port[ch].enb_intr;
				port[ch].enb_intr = false;
			}
			update_intr();
		} else if((data & 0x0f) == 0x0f) {
			// port[].dir 0=output, 1=input
			if((data & 0xc0) == MODE_OUTPUT) {
				port[ch].dir = 0x00;
			} else if((data & 0xc0) == MODE_INPUT || (data & 0xc0) == MODE_BIDIRECT) {
				port[ch].dir = 0xff;
			} else if((data & 0xc0) == MODE_CONTROL) {
				port[ch].set_dir = true;
			}
			mode_changed = (port[ch].mode != (data & 0xc0));
			port[ch].mode = data & 0xc0;
		}
		if(port[ch].mode == MODE_BIDIRECT) {
			check_mode3_intr(ch);
		}
		if(mode_changed) {
			port[ch].input_empty = false;
			port[ch].output_ready = false;
			update_ready();
		}
		break;
	}
}

uint32_t Z80PIO::read_io8(uint32_t addr)
{
	int ch = (addr >> 1) & 1;
	
	switch(addr & 3) {
	case 0:
	case 2:
		// data
		if(!port[ch].input_empty) {
			port[ch].input_empty = true;
			update_ready();
		}
		return (port[ch].rreg & port[ch].dir) | (port[ch].wreg & ~port[ch].dir);
	case 1:
	case 3:
		// status (sharp z-80pio special function)
		return port[0].mode | (port[1].mode >> 4);
	}
	return 0xff;
}

void Z80PIO::write_signal(int id, uint32_t data, uint32_t mask)
{
	// port[].dir 0=output, 1=input
	int ch = 1;
	
	switch(id) {
	case SIG_Z80PIO_PORT_A:
		ch = 0;
	case SIG_Z80PIO_PORT_B:
		port[ch].rreg = (port[ch].rreg & ~mask) | (data & mask);
		if(port[ch].input_empty) {
			port[ch].input_empty = false;
			update_ready();
		}
		if(port[ch].mode == MODE_INPUT || port[ch].mode == MODE_BIDIRECT) {
			if(!port[ch].hand_shake) {
				// the peripheral sets the strobe signal immediately after it writes the data
				if(ch == 0 && port[0].mode == MODE_BIDIRECT) {
					write_signal(SIG_Z80PIO_STROBE_B, 1, 1);
				} else if(!(ch == 1 && port[0].mode == MODE_BIDIRECT)) {
					write_signal(SIG_Z80PIO_STROBE_A + ch, 1, 1);
				}
			}
		} else if(port[ch].mode == MODE_CONTROL) {
			check_mode3_intr(ch);
		}
		break;
	case SIG_Z80PIO_STROBE_A:
		ch = 0;
	case SIG_Z80PIO_STROBE_B:
		if(data & mask) {
			if(port[ch].output_ready) {
				port[ch].output_ready = false;
				update_ready();
			}
			port[ch].req_intr = true;
			update_intr();
		}
		break;
	}
}

void Z80PIO::update_ready()
{
	for(int ch = 0; ch < 2; ch++) {
		int next_signal = 0;
		if(ch == 1 && port[0].mode == MODE_BIDIRECT) {
			next_signal = (port[0].input_empty == true);
		} else {
			if(port[ch].mode == MODE_OUTPUT || port[ch].mode == MODE_BIDIRECT) {
				next_signal = (port[ch].output_ready == true);
			} else if(port[ch].mode == MODE_INPUT) {
				next_signal = (port[ch].input_empty == true);
			}
		}
		if(port[ch].ready_signal != next_signal) {
			write_signals(&port[ch].outputs_ready, (next_signal != 0) ? 0xffffffff : 0);
			port[ch].ready_signal = next_signal;
		}
	}
}

void Z80PIO::check_mode3_intr(int ch)
{
	// check mode3 interrupt status
	uint8_t mask = ~port[ch].mask;
	uint8_t val = (port[ch].rreg & port[ch].dir) | (port[ch].wreg & ~port[ch].dir);
	val &= mask;
	
	if((port[ch].ctrl1 & 0x60) == 0x00 && val != mask) {
		port[ch].req_intr = true;
	} else if((port[ch].ctrl1 & 0x60) == 0x20 && val != 0) {
		port[ch].req_intr = true;
	} else if((port[ch].ctrl1 & 0x60) == 0x40 && val == 0) {
		port[ch].req_intr = true;
	} else if((port[ch].ctrl1 & 0x60) == 0x60 && val == mask) {
		port[ch].req_intr = true;
	} else {
		port[ch].req_intr = false;
	}
	update_intr();
}

void Z80PIO::set_intr_iei(bool val)
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

void Z80PIO::update_intr()
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
	
	// set int signal
	if((next = iei) == true) {
		next = false;
		for(int ch = 0; ch < 2; ch++) {
			if(port[ch].in_service) {
				break;
			}
			if(port[ch].enb_intr && port[ch].req_intr) {
				next = true;
				break;
			}
		}
	}
	if(d_cpu) {
		d_cpu->set_intr_line(next, true, intr_bit);
	}
}

uint32_t Z80PIO::get_intr_ack()
{
	// ack (M1=IORQ=L)
	for(int ch = 0; ch < 2; ch++) {
		if(port[ch].in_service) {
			// invalid interrupt status
			return 0xff;
		}
		if(port[ch].enb_intr && port[ch].req_intr) {
			uint8_t vector = port[ch].vector;
			port[ch].req_intr = false;
			port[ch].in_service = true;
			update_intr();
			return vector;
		}
	}
	if(d_child) {
		return d_child->get_intr_ack();
	}
	return 0xff;
}

void Z80PIO::notify_intr_reti()
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

#define STATE_VERSION	1

bool Z80PIO::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int i = 0; i < 2; i++) {
		state_fio->StateValue(port[i].wreg);
		state_fio->StateValue(port[i].rreg);
		state_fio->StateValue(port[i].mode);
		state_fio->StateValue(port[i].ctrl1);
		state_fio->StateValue(port[i].ctrl2);
		state_fio->StateValue(port[i].dir);
		state_fio->StateValue(port[i].mask);
		state_fio->StateValue(port[i].vector);
		state_fio->StateValue(port[i].set_dir);
		state_fio->StateValue(port[i].set_mask);
		state_fio->StateValue(port[i].hand_shake);
		state_fio->StateValue(port[i].ready_signal);
		state_fio->StateValue(port[i].input_empty);
		state_fio->StateValue(port[i].output_ready);
		state_fio->StateValue(port[i].enb_intr);
		state_fio->StateValue(port[i].enb_intr_tmp);
		state_fio->StateValue(port[i].req_intr);
		state_fio->StateValue(port[i].in_service);
	}
	state_fio->StateValue(iei);
	state_fio->StateValue(oei);
	state_fio->StateValue(intr_bit);
	return true;
}

