/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.30 -

	[ timer ]
*/

#include "timer.h"
#include "../i8259.h"
#include "../msm58321.h"
#include "../pcm1bit.h"

void TIMER::initialize()
{
	free_run_counter = 0;
	intr_reg = rtc_data = 0;
	tmout0 = tmout1 = false;
}

void TIMER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x60:
		if(data & 0x80) {
			tmout0 = false;
		}
		intr_reg = data;
		update_intr();
		d_pcm->write_signal(SIG_PCM1BIT_ON, data, 4);
		break;
	case 0x70:
		d_rtc->write_signal(SIG_MSM58321_DATA, data, 0x0f);
		break;
	case 0x80:
		d_rtc->write_signal(SIG_MSM58321_CS, data, 0x80);
		d_rtc->write_signal(SIG_MSM58321_READ, data, 0x04);
		d_rtc->write_signal(SIG_MSM58321_WRITE, data, 0x02);
		d_rtc->write_signal(SIG_MSM58321_ADDR_WRITE, data, 0x01);
		break;
	}
}

uint32_t TIMER::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x26:
		free_run_counter = (uint16_t)get_passed_usec(0);
		return free_run_counter & 0xff;
	case 0x27:
		return free_run_counter >> 8;
	case 0x60:
		return (tmout0 ? 1 : 0) | (tmout1 ? 2 : 0) | ((intr_reg & 7) << 2) | 0xe0;
	case 0x70:
		return rtc_data;
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
	} else if(id == SIG_TIMER_RTC) {
		rtc_data = (data & mask) | (rtc_data & ~mask);
	}
}

void TIMER::update_intr()
{
	if((tmout0 && (intr_reg & 1)) || (tmout1 && (intr_reg & 2))) {
		d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR0, 1, 1);
	} else {
		d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR0, 0, 1);
	}
}

#define STATE_VERSION	1

void TIMER::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint16(free_run_counter);
	state_fio->FputUint8(intr_reg);
	state_fio->FputUint8(rtc_data);
	state_fio->FputBool(tmout0);
	state_fio->FputBool(tmout1);
}

bool TIMER::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	free_run_counter = state_fio->FgetUint16();
	intr_reg = state_fio->FgetUint8();
	rtc_data = state_fio->FgetUint8();
	tmout0 = state_fio->FgetBool();
	tmout1 = state_fio->FgetBool();
	return true;
}

