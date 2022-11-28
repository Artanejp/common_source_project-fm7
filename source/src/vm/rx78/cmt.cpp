/*
	BANDAI RX-78 Emulator 'eRX-78'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ cmt ]
*/

#include "cmt.h"
#include "../datarec.h"

void CMT::initialize()
{
	// data recorder
	in = out = remote = now_acc = false;
	framecnt = 0;
	
	// register event to detect the end of access
	register_frame_event(this);
}

void CMT::write_io8(uint32_t addr, uint32_t data)
{
	// data recorder
	if(!remote) {
		// motor on
		d_drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
		remote = true;
	}
	bool signal = ((data & 1) != 0);
	if(signal != out) {
		d_drec->write_signal(SIG_DATAREC_MIC, signal ? 1 : 0, 1);
		out = signal;
	}
	now_acc = true;
}

uint32_t CMT::read_io8(uint32_t addr)
{
	if(!remote) {
		// motor on
		d_drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
		remote = true;
	}
	now_acc = true;
	return in ? 1 : 0;
}

void CMT::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CMT_IN) {
		in = ((data & mask) != 0);
	}
}

void CMT::event_frame()
{
	if(remote) {
		if(now_acc) {
			framecnt = 0;
		} else if(++framecnt >= FRAMES_PER_SEC) {
			// motor off if not accessed for past 1 sec
			d_drec->write_signal(SIG_DATAREC_REMOTE, 0, 1);
			remote = false;
		}
		now_acc = false;
	}
}

#define STATE_VERSION	1

bool CMT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(in);
	state_fio->StateValue(out);
	state_fio->StateValue(remote);
	state_fio->StateValue(now_acc);
	state_fio->StateValue(framecnt);
	return true;
}

