/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2022.07.02-

	[ TMP84C013 ]
*/

#include "tmpz84c013.h"

void TMPZ84C013::reset()
{
	iei = true;
	update_priority(0);
}

void TMPZ84C013::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0xf0:
	case 0xf1:
		// watch dog timer
		break;
	case 0xf4:
		update_priority(data);
		d_1st->set_intr_iei(iei);
		break;
	}
}

uint32_t TMPZ84C013::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0xf4:
		return priority;
	}
	return 0xff;
}

void TMPZ84C013::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_TMPZ84C015_CTC_TRIG_0:
		d_ctc->write_signal(SIG_Z80CTC_TRIG_0, data, mask);
		break;
	case SIG_TMPZ84C015_CTC_TRIG_1:
		d_ctc->write_signal(SIG_Z80CTC_TRIG_1, data, mask);
		break;
	case SIG_TMPZ84C015_CTC_TRIG_2:
		d_ctc->write_signal(SIG_Z80CTC_TRIG_2, data, mask);
		break;
	case SIG_TMPZ84C015_CTC_TRIG_3:
		d_ctc->write_signal(SIG_Z80CTC_TRIG_3, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_RECV_CH0:
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_RECV_CH1:
		d_sio->write_signal(SIG_Z80SIO_RECV_CH1, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_BREAK_CH0:
		d_sio->write_signal(SIG_Z80SIO_BREAK_CH0, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_BREAK_CH1:
		d_sio->write_signal(SIG_Z80SIO_BREAK_CH1, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_DCD_CH0:
		d_sio->write_signal(SIG_Z80SIO_DCD_CH0, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_DCD_CH1:
		d_sio->write_signal(SIG_Z80SIO_DCD_CH1, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_CTS_CH0:
		d_sio->write_signal(SIG_Z80SIO_CTS_CH0, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_CTS_CH1:
		d_sio->write_signal(SIG_Z80SIO_CTS_CH1, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_SYNC_CH0:
		d_sio->write_signal(SIG_Z80SIO_SYNC_CH0, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_SYNC_CH1:
		d_sio->write_signal(SIG_Z80SIO_SYNC_CH1, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_TX_CLK_CH0:
		d_sio->write_signal(SIG_Z80SIO_TX_CLK_CH0, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_TX_CLK_CH1:
		d_sio->write_signal(SIG_Z80SIO_TX_CLK_CH1, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_RX_CLK_CH0:
		d_sio->write_signal(SIG_Z80SIO_RX_CLK_CH0, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_RX_CLK_CH1:
		d_sio->write_signal(SIG_Z80SIO_RX_CLK_CH1, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_CLEAR_CH0:
		d_sio->write_signal(SIG_Z80SIO_CLEAR_CH0, data, mask);
		break;
	case SIG_TMPZ84C015_SIO_CLEAR_CH1:
		d_sio->write_signal(SIG_Z80SIO_CLEAR_CH1, data, mask);
		break;
	}
}

void TMPZ84C013::update_priority(uint8_t val)
{
	DEVICE *d_2nd;
	
	switch(val & 1) {
	case 0: d_1st = d_ctc; d_2nd = d_sio; break; // CTC -> SIO
	case 1: d_1st = d_sio; d_2nd = d_ctc; break; // SIO -> CTC
	}
	d_1st->set_context_child(d_2nd);
	d_2nd->set_context_child(d_child);
	
	priority = val;
}

#define STATE_VERSION	1

bool TMPZ84C013::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(iei);
	state_fio->StateValue(priority);
	
	if(loading) {
		update_priority(priority);
	}
	return true;
}
