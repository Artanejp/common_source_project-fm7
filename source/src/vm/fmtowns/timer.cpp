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
	next_interval = 0;
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
	beepon_cff98h = false;
	beepon_60h = false;
	if(d_pcm != NULL) {
		d_pcm->write_signal(SIG_PCM1BIT_MUTE, 0, 1);
	}
	update_beep(false, false, true);

	clear_event(this, event_wait_1us);
	clear_event(this, event_interval_us);
	__LIKELY_IF(intr_target != nullptr) {
		intr_target->write_signal(intr_target_id, 0, intr_target_mask);
	}
	__LIKELY_IF(halt_target != nullptr) {
		halt_target->write_signal(halt_target_id, 0, halt_target_mask);
	}
	if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
		register_event(this, EVENT_INTERVAL_US, 1.0, true, &event_interval_us);
		do_interval();
	}
}

void TIMER::write_io16(uint32_t addr, uint32_t data)
{
	if(((addr & 0xfffe) == 0x006a) && (machine_id >= 0x0300)) { // After UX*/10F/20F/40H/80H
		interval_us.w = data;
		do_interval();
		return;
	}
	write_io8(addr + 0, data & 0xff);
	write_io8(addr + 1, (data >> 8) & 0xff);
}

uint32_t TIMER::read_io16(uint32_t addr)
{
	if(((addr & 0xfffe) == 0x006a) && (machine_id >= 0x0300)) { // After UX*/10F/20F/40H/80H
		return interval_us.w;;
	}
	pair16_t val;
	val.b.l = read_io8(addr + 0);
	val.b.h = read_io8(addr + 1);
	return val.w;
}
void TIMER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x0060:
		if(data & 0x80) {
			tmout0 = false;
		}
		intr_reg = data;
		update_beep(((data & 0x04) != 0) ? true : false, beepon_cff98h, false);
		update_intr();
		break;
	case 0x0068: // Interval control
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			bool _b = interval_enabled;
			interval_enabled = ((data & 0x80) == 0);
			intv_ov = false;
			intv_i = false;
			if(_b != interval_enabled) {
				do_interval();
			}
			update_interval_timer();
		}
		break;
	case 0x006a: // Interval control
	case 0x006b: // Interval control
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			// Q: Do reset intv_i and intv_ov? 20200124 K.O
			bool highaddress = (addr == 0x6b);
			if(highaddress) {
				interval_us.b.h = data;
				do_interval();
			} else {
				interval_us.b.l = data;
			}
		}
		break;
	case 0x006c: // Wait register.
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			__LIKELY_IF(event_wait_1us < 0) {
				register_event(this, EVENT_1US_WAIT, 1.0, false, &event_wait_1us);
				__LIKELY_IF(halt_target != nullptr) {
					halt_target->write_signal(halt_target_id, 0xffffffff, halt_target_mask);
				}
			}
		}
		break;
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

void TIMER::update_beep(bool on_60h, bool on_cff98h, bool force)
{
	__UNLIKELY_IF(d_pcm == NULL) return;
	if((on_cff98h == beepon_cff98h) && (on_60h == beepon_60h) && !(force)) {
		return;
	}
	beepon_60h = on_60h;
	beepon_cff98h = on_cff98h;
	if((beepon_60h) || (beepon_cff98h)) {
		d_pcm->write_signal(SIG_PCM1BIT_ON, 1, 1);
	} else {
		d_pcm->write_signal(SIG_PCM1BIT_ON, 0, 1);
	}
}

void TIMER::do_interval(void)
{
	if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
		if(interval_us.w == 0) {
			next_interval = free_run_counter + 65536;
		} else {
			next_interval = free_run_counter + interval_us.w;
		}
	}
}


uint32_t TIMER::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x0026:
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			return free_run_counter & 0xff;
		} else {
			return 0xff;
		}
		break;
	case 0x0027:
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			return (free_run_counter >> 8) & 0xff;
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
		// 20210227 K.O
		// At TSUGARU, written below:
		// Supposed to be 1us wait when written.
		// But, mouse BIOS is often reading from this register.
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			__LIKELY_IF(event_wait_1us < 0) {
				register_event(this, EVENT_1US_WAIT, 1.0, false, &event_wait_1us);
				__LIKELY_IF(halt_target != nullptr) {
					halt_target->write_signal(halt_target_id, 0xffffffff, halt_target_mask);
				}
			}
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
	}  else if(id == SIG_TIMER_BEEP_ON) {
		update_beep(beepon_60h, ((data & mask) != 0) ? true : false, false);
	}
}

void TIMER::update_intr()
{
	if((tmout0 && (intr_reg & 1)) || (tmout1 && (intr_reg & 2)) || (intv_i)) {
		__LIKELY_IF(intr_target != nullptr) {
			intr_target->write_signal(intr_target_id, 1, intr_target_mask);
		}
	} else {
		__LIKELY_IF(intr_target != nullptr) {
			intr_target->write_signal(intr_target_id, 0, intr_target_mask);
		}
	}
}

void TIMER::event_callback(int id, int err)
{
	switch(id) {
	case EVENT_1US_WAIT:
		event_wait_1us = -1;
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			__LIKELY_IF(halt_target != nullptr) {
				halt_target->write_signal(halt_target_id, 0, halt_target_mask);
			}
		}
		break;
	case EVENT_INTERVAL_US:
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			free_run_counter++;
			if(next_interval == free_run_counter) {
				do_interval();
				update_interval_timer();
			}
		}
		break;
	}
}

void TIMER::update_interval_timer()
{
	if(intv_i) intv_ov = true;
	intv_i = true;
	if(interval_enabled) {
		update_intr();
	}
}

#define STATE_VERSION	4

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
	state_fio->StateValue(next_interval);
	state_fio->StateValue(intr_reg);
	state_fio->StateValue(rtc_data);
	state_fio->StateValue(rtc_busy);

	state_fio->StateValue(tmout0);
	state_fio->StateValue(tmout1);

	state_fio->StateValue(interval_enabled);
	state_fio->StateValue(interval_us);
	state_fio->StateValue(intv_i);
	state_fio->StateValue(intv_ov);
	state_fio->StateValue(beepon_60h);
	state_fio->StateValue(beepon_cff98h);

	state_fio->StateValue(event_wait_1us);
	state_fio->StateValue(event_interval_us);

	return true;
}

}
