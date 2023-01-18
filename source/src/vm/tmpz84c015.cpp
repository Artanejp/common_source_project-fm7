/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2022.07.02-

	[ TMPZ84C015/TMP84C013 ]
*/

#include "./tmpz84c015.h"

void TMPZ84C015::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_TMPZ84C015_PIO_PORT_A:
		d_pio->write_signal(SIG_Z80PIO_PORT_A, data, mask);
		break;
	case SIG_TMPZ84C015_PIO_PORT_B:
		d_pio->write_signal(SIG_Z80PIO_PORT_B, data, mask);
		break;
	case SIG_TMPZ84C015_PIO_STROBE_A:
		d_pio->write_signal(SIG_Z80PIO_STROBE_A, data, mask);
		break;
	case SIG_TMPZ84C015_PIO_STROBE_B:
		d_pio->write_signal(SIG_Z80PIO_STROBE_B, data, mask);
		break;
	default:
		TMPZ84C013::write_signal(id, data, mask);
		break;
	}
}

void TMPZ84C015::update_priority(uint8_t val)
{
	DEVICE *d_2nd;
	DEVICE *d_3rd;
	
	switch(val & 7) {
	case 0: d_1st = d_ctc; d_2nd = d_sio; d_3rd = d_pio; break; // CTC -> SIO -> PIO
	case 1: d_1st = d_sio; d_2nd = d_ctc; d_3rd = d_pio; break; // SIO -> CTC -> PIO
	case 2: d_1st = d_ctc; d_2nd = d_pio; d_3rd = d_sio; break; // CTC -> PIO -> SIO
	case 3: d_1st = d_pio; d_2nd = d_sio; d_3rd = d_ctc; break; // PIO -> SIO -> CTC
	case 4: d_1st = d_pio; d_2nd = d_ctc; d_3rd = d_sio; break; // PIO -> CTC -> SIO
	case 5: d_1st = d_sio; d_2nd = d_pio; d_3rd = d_ctc; break; // SIO -> PIO -> CTC
	default:
		// XXX: don't update the priority register
		priority = (val & ~7) & (priority & 7);
		return;
	}
	d_1st->set_context_child(d_2nd);
	d_2nd->set_context_child(d_3rd);
	d_3rd->set_context_child(d_child);

	priority = val;
}


