/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.30 -

	[ timer ]
*/

#include "./timer.h"
#include "../i8259.h"
#include "../msm58321.h"
#include "../pcm1bit.h"

#define EVENT_1US_WAIT    1
#define EVENT_INTERVAL_US 2

namespace FMTOWNS {
	
void TIMER::initialize()
{
	free_run_counter = 0;
	intr_reg = rtc_data = 0;
	tmout0 = tmout1 = false;
	event_interval_us = -1;
	event_wait_1us = -1;
	rtc_busy = false;
}

void TIMER::reset()
{
	interval_enabled = false;
	interval_us.w = 0;
	intv_i = false;
	intv_ov = false;

	if(event_wait_1us >= 0) {
		cancel_event(this, event_wait_1us);
		event_wait_1us = -1;
	}
//	do_interval();
}
void TIMER::write_io16(uint32_t addr, uint32_t data)
{
	switch(addr & 0xfffe) {
	case 0x006a: // Interval control
		interval_us.w = data;
		do_interval();
		break;
	default:
		write_io8(addr, data);
		break;
	}
}

void TIMER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x0060:
		if(data & 0x80) {
			tmout0 = false;
		}
		intr_reg = data;
		d_pcm->write_signal(SIG_PCM1BIT_ON, data, 4);
		update_intr();
		break;
	case 0x0068: // Interval control
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			/*
			if(!(interval_enabled)) {// OK?
				intv_ov = false;
				intv_i = false;
			}*/
			interval_enabled = ((data & 0x80) == 0);
			do_interval();
		}
		break;
	case 0x006a: // Interval control
	case 0x006b: // Interval control
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			// Q: Do reset intv_i and intv_ov? 20200124 K.O
			bool highaddress = (addr == 0x6b);
			if(highaddress) {
				interval_us.b.h = data;
			} else {
				interval_us.b.l = data;
			}
			do_interval();
		}
	case 0x006c: // Wait register.
		/*if(machine_id >= 0x0300) */{ // After UX*/10F/20F/40H/80H
			if(event_wait_1us != -1) cancel_event(this, event_wait_1us);
			register_event(this, EVENT_1US_WAIT, 1.0, false, &event_wait_1us);
			write_signals(&outputs_halt_line, 0xffffffff);
		}
	case 0x0070:
		d_rtc->write_signal(SIG_MSM58321_DATA, data, 0x0f);
		break;
	case 0x0080:
		d_rtc->write_signal(SIG_MSM58321_CS, data, 0x80);
		d_rtc->write_signal(SIG_MSM58321_READ, data, 0x04);
		d_rtc->write_signal(SIG_MSM58321_WRITE, data, 0x02);
		d_rtc->write_signal(SIG_MSM58321_ADDR_WRITE, data, 0x01);
		break;
	}
}

void TIMER::do_interval(void)
{
	if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
		if(interval_enabled) {
			uint32_t interval = interval_us.w;
			if(interval == 0) interval = 65536;
			// Update interval
			if(event_interval_us >= 0) cancel_event(this, event_interval_us);
			register_event(this, EVENT_INTERVAL_US, 1.0 * (double)interval, true, &event_interval_us);
		} else {
			if(event_interval_us >= 0) cancel_event(this, event_interval_us);
			event_interval_us = -1;
		}
	}
}

uint32_t TIMER::read_io16(uint32_t addr)
{
	switch(addr & 0xfffe) {
	case 0x0026:
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			free_run_counter = (uint16_t)get_passed_usec(0);
			return free_run_counter & 0xffff;
		} else {
			return 0xffff;
		}
		break;
	case 0x006a: // Interval control
		return interval_us.w;
		break;
	}
	return read_io8(addr);
}
	
uint32_t TIMER::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x0026:
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			free_run_counter = (uint16_t)get_passed_usec(0);
			return free_run_counter & 0xff;
		} else {
			return 0xff;
		}
		break;
	case 0x0027:
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			return free_run_counter >> 8;
		} else {
			return 0xff;
		}
		break;
	case 0x0060:
		return (tmout0 ? 1 : 0) | (tmout1 ? 2 : 0) | ((intr_reg & 7) << 2) | 0x00;
	case 0x0068: //
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			uint8_t val = (interval_enabled) ? 0x1f : 0x9f;
			if(intv_i) val |= 0x40;
			if(intv_ov) val |= 0x20;
			intv_i = false;
			intv_ov = false;
			return val;
		}
		break;
	case 0x006a: // Interval control
	case 0x006b: // Interval control
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			bool highaddress = (addr == 0x6b);
			if(highaddress) {
				return interval_us.b.h;
			} else {
				return interval_us.b.l;
			}
		}
		break;
	case 0x006c: // Wait register.
		/*if(machine_id >= 0x0300) */{ // After UX*/10F/20F/40H/80H
			if(event_wait_1us != -1) cancel_event(this, event_wait_1us);
			register_event(this, EVENT_1US_WAIT, 1.0, false, &event_wait_1us);
			write_signals(&outputs_halt_line, 0xffffffff);
//			return 0x00;
		}
		break;
	case 0x0070:
		return (rtc_data & 0x7f) | ((rtc_busy) ? 0x80 : 0x00);
	}
	return 0xff;
}

void TIMER::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_TIMER_CH0) {
		if(data & mask) {
			tmout0 = true;
		}
		update_intr();
	} else if(id == SIG_TIMER_CH1) {
		tmout1 = ((data & mask) != 0);
		update_intr();
	} else if(id == SIG_TIMER_CH2) {
		//if((intr_reg & 4) != 0) {
		//	d_pcm->write_signal(SIG_PCM1BIT_SIGNAL, ((data & mask) != 0) ? 1 : 0, 1);
		//}
	} else if(id == SIG_TIMER_RTC) {
		rtc_data = data & mask;
	} else if(id == SIG_TIMER_RTC_BUSY) {
		rtc_busy = ((data & mask) == 0);
	}
}

void TIMER::update_intr()
{
	if((tmout0 && (intr_reg & 1)) || (tmout1 && (intr_reg & 2)) || (intv_i)) {
		write_signals(&outputs_intr_line, 1);
	} else {
		write_signals(&outputs_intr_line, 0);
	}
}

void TIMER::event_callback(int id, int err)
{
	switch(id) {
	case EVENT_1US_WAIT:
		event_wait_1us = -1;
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			write_signals(&outputs_halt_line, 0);
		}
		break;
	case EVENT_INTERVAL_US:
		if(interval_enabled) {
			if(intv_i) intv_ov = true;
			intv_i = true;
			update_intr();
		}
		break;
	}
}

#define STATE_VERSION	1

bool TIMER::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(machine_id);
	state_fio->StateValue(cpu_id);
	state_fio->StateValue(free_run_counter);
	state_fio->StateValue(intr_reg);
	state_fio->StateValue(rtc_data);
	state_fio->StateValue(rtc_busy);
	
	state_fio->StateValue(tmout0);
	state_fio->StateValue(tmout1);

	state_fio->StateValue(interval_enabled);
	state_fio->StateValue(interval_us);
	state_fio->StateValue(intv_i);
	state_fio->StateValue(intv_ov);

	state_fio->StateValue(event_wait_1us);
	state_fio->StateValue(event_interval_us);
	
	return true;
}

}

