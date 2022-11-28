/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2009.01.05-

	[ i8155 ]
*/

#include "i8155.h"

// status and portc
#define STA_INTR_A	1
#define STA_BF_A	2
#define STA_STB_A	4
#define STA_INTE_A	4
#define STA_INTR_B	8
#define STA_BF_B	0x10
#define STA_STB_B	0x20
#define STA_INTE_B	0x20
#define STA_INTR_T	0x40

// command
#define CMD_INTE_A	0x10
#define CMD_INTE_B	0x20

// mode
#define PIO_MODE_3	((cmdreg & 0xc) == 4)
#define PIO_MODE_4	((cmdreg & 0xc) == 8)

void I8155::initialize()
{
	// initialize timer
	count = countreg = 0x3fff;
	prev_out = true;
	prev_in = false;
	now_count = stop_tc = false;
	half = true;
	
	// clear ram
	memset(ram, 0, sizeof(ram));
}

void I8155::reset()
{
	// reset pio
	for(int i = 0; i < 3; i++) {
		pio[i].rmask = 0xff;
		pio[i].wreg = 0;
		pio[i].rreg = 0;
		pio[i].first = true;
	}
	statreg = cmdreg = 0;
	register_id = -1;
	
	// stop count but don't reset timer
	stop_count();
}

void I8155::write_data8(uint32_t addr, uint32_t data)
{
	ram[addr & 0xff] = data;
}

uint32_t I8155::read_data8(uint32_t addr)
{
	return ram[addr & 0xff];
}

void I8155::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 7) {
	case 0:
		pio[0].rmask = (data & 1) ? 0 : 0xff;
		pio[1].rmask = (data & 2) ? 0 : 0xff;
		pio[2].rmask = (data & 0xc) ? 0 : 0xff;
		statreg &= ~(STA_INTE_A | STA_INTE_B);
		if(data & CMD_INTE_A) {
			statreg |= STA_INTE_A;
		}
		if(data & CMD_INTE_B) {
			statreg |= STA_INTE_B;
		}
		// timer operation
		switch(data & 0xc0) {
		case 0x40:
			stop_count();
			break;
		case 0x80:
			stop_tc = true;
			break;
		case 0xc0:
			start_count();
			break;
		}
		cmdreg = data;
		break;
	case 1:
		set_pio(0, data);
		break;
	case 2:
		set_pio(1, data);
		break;
	case 3:
		if(PIO_MODE_3) {
			data = (data & ~7) | (pio[2].wreg & 7);
		}
		if(!PIO_MODE_4) {
			set_pio(2, data & 0x3f);
		}
		break;
	case 4:
		countreg = (countreg & 0xff00) | data;
		break;
	case 5:
		countreg = (countreg & 0xff) | (data << 8);
		break;
	}
}

uint32_t I8155::read_io8(uint32_t addr)
{
	switch(addr & 7) {
	case 0:
		if(statreg & STA_INTR_T) {
			statreg &= ~STA_INTR_T;
			return statreg | STA_INTR_T;
		}
		return statreg;
	case 1:
		if(PIO_MODE_3 || PIO_MODE_4) {
			statreg &= ~(STA_INTR_A | STA_BF_A);
			set_pio(2, pio[2].wreg & ~(STA_INTR_A | STA_BF_A));
		}
		return (pio[0].rreg & pio[0].rmask) | (pio[0].wreg & ~pio[0].rmask);
	case 2:
		if(PIO_MODE_4) {
			statreg &= ~(STA_INTR_B | STA_BF_B);
			set_pio(2, pio[2].wreg & ~(STA_INTR_B | STA_BF_B));
		}
		return (pio[1].rreg & pio[1].rmask) | (pio[1].wreg & ~pio[1].rmask);
	case 3:
		return (pio[2].rreg & pio[2].rmask) | (pio[2].wreg & ~pio[2].rmask);
	case 4:
		update_count();
		return count & 0xff;
	case 5:
		update_count();
		return ((count >> 8) & 0x3f) | ((countreg >> 8) & 0xc0);
	}
	return 0xff;
}

void I8155::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_I8155_PORT_A:
		if(PIO_MODE_3 || PIO_MODE_4) {
			// note: strobe signal must be checked
			uint32_t val = pio[2].wreg | STA_BF_A;
			statreg |= STA_BF_A;
			if(cmdreg & CMD_INTE_A) {
				val |= STA_INTR_A;
				statreg |= STA_INTR_A;
			}
			set_pio(2, val);
		}
		pio[0].rreg = (pio[0].rreg & ~mask) | (data & mask);
		break;
	case SIG_I8155_PORT_B:
		if(PIO_MODE_4) {
			// note: strobe signal must be checked
			uint32_t val = pio[2].wreg | STA_BF_B;
			statreg |= STA_BF_B;
			if(cmdreg & CMD_INTE_B) {
				val |= STA_INTR_B;
				statreg |= STA_INTR_B;
			}
			set_pio(2, val);
		}
		pio[1].rreg = (pio[1].rreg & ~mask) | (data & mask);
		break;
	case SIG_I8155_PORT_C:
		pio[2].rreg = (pio[2].rreg & ~mask) | (data & mask);
		break;
	case SIG_I8155_CLOCK:
		if(prev_in && !(data & mask)) {
			input_clock(1);
		}
		prev_in = ((data & mask) != 0);
		break;
	}
}

#define COUNT_VALUE ((countreg & 0x3fff) > 2 ? (countreg & 0x3fff) : 2)

void I8155::event_callback(int event_id, int err)
{
	register_id = -1;
	input_clock(input_clk);
	
	// register next event
	if(freq && now_count) {
		input_clk = get_next_clock();
		period = (int)(cpu_clocks * input_clk / freq) + err;
		prev_clk = get_current_clock() + err;
		register_event_by_clock(this, 0, period, false, &register_id);
	}
}

void I8155::input_clock(int clock)
{
	if(!(now_count && clock)) {
		return;
	}
	
	// update counter
	count -= clock;
	int32_t tmp = COUNT_VALUE;
loop:
	if(half) {
		set_signal(count > (tmp >> 1));
	} else {
		set_signal(count > 1);
	}
	if(count <= 0) {
		statreg |= STA_INTR_T;
		if(!stop_tc) {
			set_signal(true);
			count += tmp;
			goto loop;
		} else {
			now_count = false;
		}
	}
}

void I8155::start_count()
{
	// set timer mode
	stop_tc = ((countreg & 0x4000) == 0);
	half = ((countreg & 0x8000) == 0);
	
	if(!now_count) {
		count = COUNT_VALUE;
		now_count = true;
		
		// register event
		if(freq && register_id == -1) {
			input_clk = get_next_clock();
			period = (int)(cpu_clocks * input_clk / freq);
			prev_clk = get_current_clock();
			register_event_by_clock(this, 0, period, false, &register_id);
		}
	}
}

void I8155::stop_count()
{
	if(register_id != -1) {
		cancel_event(this, register_id);
	}
	register_id = -1;
	now_count = false;
}

void I8155::update_count()
{
	if(register_id != -1) {
		// update counter
		int passed = get_passed_clock(prev_clk);
		uint32_t input = (uint32_t)(freq * passed / cpu_clocks);
		if(input_clk <= input) {
			input = input_clk - 1;
		}
		if(input > 0) {
			input_clock(input);
			// cancel and re-register event
			cancel_event(this, register_id);
			input_clk -= input;
			period -= passed;
			prev_clk = get_current_clock();
			register_event_by_clock(this, 0, period, false, &register_id);
		}
	}
}

int I8155::get_next_clock()
{
	if(half) {
		int32_t tmp = COUNT_VALUE >> 1;
		return (count > tmp) ? count - tmp : count;
	}
	return (count > 1) ? count - 1 : 1;
}

void I8155::set_signal(bool signal)
{
	if(prev_out && !signal) {
		// H->L
		write_signals(&outputs_timer, 0);
	} else if(!prev_out && signal) {
		// L->H
		write_signals(&outputs_timer, 0xffffffff);
	}
	prev_out = signal;
}

void I8155::set_pio(int ch, uint8_t data)
{
	if(pio[ch].wreg != data || pio[ch].first) {
		write_signals(&pio[ch].outputs, data);
		pio[ch].wreg = data;
		pio[ch].first = false;
	}
}

#define STATE_VERSION	1

bool I8155::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(count);
	state_fio->StateValue(countreg);
	state_fio->StateValue(now_count);
	state_fio->StateValue(stop_tc);
	state_fio->StateValue(half);
	state_fio->StateValue(prev_out);
	state_fio->StateValue(prev_in);
	state_fio->StateValue(freq);
	state_fio->StateValue(register_id);
	state_fio->StateValue(input_clk);
	state_fio->StateValue(prev_clk);
	state_fio->StateValue(period);
	state_fio->StateValue(cpu_clocks);
	for(int i = 0; i < 3; i++) {
		state_fio->StateValue(pio[i].wreg);
		state_fio->StateValue(pio[i].rreg);
		state_fio->StateValue(pio[i].rmask);
		state_fio->StateValue(pio[i].mode);
		state_fio->StateValue(pio[i].first);
	}
	state_fio->StateValue(cmdreg);
	state_fio->StateValue(statreg);
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}

