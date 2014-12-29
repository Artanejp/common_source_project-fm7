/*
	SORD m5 Emulator 'Emu5'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ cmt/printer ]
*/

#include "cmt.h"
#include "../datarec.h"

void CMT::initialize()
{
	// data recorder
	in = out = remote = false;
	
	// printer
	strobe = busy = false;
	
	// reset/halt key
	key_stat = emu->key_buffer();
}

void CMT::write_io8(uint32 addr, uint32 data)
{
	bool signal, motor;
	
	switch(addr & 0xff) {
	case 0x40:
		// printer
		pout = data;
		break;
	case 0x50:
		// data recorder
		if((signal = ((data & 1) == 0)) != out) {
			d_drec->write_signal(SIG_DATAREC_OUT, signal ? 1 : 0, 1);
			out = signal;
		}
		if((motor = ((data & 2) != 0)) != remote) {
			d_drec->write_signal(SIG_DATAREC_REMOTE, motor ? 1 : 0, 1);
			remote = motor;
		}
		// printer
		strobe = ((data & 1) != 0);
		break;
	}
}

uint32 CMT::read_io8(uint32 addr)
{
	// back-space (0x08): reset/halt key
	uint32 status = (in ? 1 : 0) | (busy ? 2 : 0) | ((key_stat[0x08] || eot) ? 0x80 : 0);
	eot = false;
	return status;
}

void CMT::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_CMT_IN) {
		in = ((data & mask) != 0);
	} else if(id == SIG_CMT_EOT) {
		if((data & mask) != 0 && vm->tape_inserted()) {
			eot = true;
		}
	} else if(id == SIG_PRINTER_BUSY) {
		busy = ((data & mask) != 0);
	}
}

