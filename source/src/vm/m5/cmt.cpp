/*
	SORD m5 Emulator 'Emu5'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ cmt/printer ]
*/

#include "cmt.h"
#include "../datarec.h"
#include "../../fileio.h"

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

#define STATE_VERSION	1

void CMT::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputBool(in);
	state_fio->FputBool(out);
	state_fio->FputBool(remote);
	state_fio->FputBool(eot);
	state_fio->FputUint8(pout);
	state_fio->FputBool(strobe);
	state_fio->FputBool(busy);
}

bool CMT::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	in = state_fio->FgetBool();
	out = state_fio->FgetBool();
	remote = state_fio->FgetBool();
	eot = state_fio->FgetBool();
	pout = state_fio->FgetUint8();
	strobe = state_fio->FgetBool();
	busy = state_fio->FgetBool();
	return true;
}

