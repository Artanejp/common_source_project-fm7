/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ system port ]
*/

#include "sysport.h"

void SYSPORT::initialize()
{
	rst = 0;
	register_frame_event(this);
}

void SYSPORT::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0x3f0) {
	case 0x70:
		// port-c
		if(data & 8) {
			d_fdc->reset();
		}
#if defined(_MZ6500) || defined(_MZ6550)
		highden = data & 4;
#endif
		break;
	case 0x260:
		// z80ctc reti
		if(data == 0x4d) {
			d_ctc->notify_intr_reti();
		}
		break;
	}
}

uint32_t SYSPORT::read_io8(uint32_t addr)
{
	switch(addr & 0x3ff) {
	case 0x60:
		// port-a
#if defined(_MZ6500) || defined(_MZ6550)
		return 0xfc | (rst ? 0 : 2) | (highden ? 1 : 0);
#else
		return 0xfd | (rst ? 0 : 2);
#endif
	case 0x240:
		// z80ctc vector
		return d_ctc->get_intr_ack();
	case 0x250:
		// z80sio vector
		return d_sio->get_intr_ack();
	case 0x270:
		// port-b
		return 0xff;
	}
	return 0xff;
}

void SYSPORT::event_frame()
{
	if(rst) {
		rst--;
	}
}

#define STATE_VERSION	1

bool SYSPORT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(rst);
	state_fio->StateValue(highden);
	return true;
}

