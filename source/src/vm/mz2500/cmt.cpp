/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.04 -

	[ cmt ]
*/

#include "cmt.h"
#include "../datarec.h"
#include "../i8255.h"
#include "../../fileio.h"

#define EVENT_FREW	0
#define EVENT_FFWD	1
#define EVENT_FWD	2
#define EVENT_STOP	3
#define EVENT_EJECT	4
#define EVENT_APSS	5
#define EVENT_IPL	6

#define PERIOD_IPL_SIGNAL	100
//#define PERIOD_CMT_SIGNAL	300000
#define PERIOD_CMT_SIGNAL	1000

void CMT::initialize()
{
	pa = pc = 0xff;
	play = rec = false;
	now_play = now_rewind = false;
}

void CMT::reset()
{
	register_id_frew = -1;
	register_id_ffwd = -1;
	register_id_fwd = -1;
	register_id_stop = -1;
	register_id_eject = -1;
#ifndef _MZ80B
	register_id_apss = -1;
	now_apss = false;
#endif
	register_id_ipl = -1;
	close_tape();
}

void CMT::fast_forward()
{
	if(play) {
		d_drec->set_ff_rew(1);
		d_drec->set_remote(true);
	}
	now_play = now_rewind = false;
}

void CMT::fast_rewind()
{
	if(play) {
		d_drec->set_ff_rew(-1);
		d_drec->set_remote(true);
	}
	now_rewind = play;
	now_play = false;
}

void CMT::forward()
{
	if(play || rec) {
		d_drec->set_ff_rew(0);
		d_drec->set_remote(true);
	}
	now_play = (play || rec);
	now_rewind = false;
}

void CMT::stop()
{
	if(play || rec) {
		d_drec->set_remote(false);
	}
	now_play = now_rewind = false;
	d_pio->write_signal(SIG_I8255_PORT_B, 0, 0x40);
}

void CMT::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_CMT_PIO_PA) {
#ifndef _MZ80B
		if((pa & 1) && !(data & 1)) {
			fast_rewind();
			now_apss = ((pa & 0x80) == 0);
//			register_event(this, EVENT_FREW, PERIOD_CMT_SIGNAL, false, &register_id_frew);
//			now_apss_tmp = ((pa & 0x80) == 0);
//		} else if(!(pa & 1) && (data & 1)) {
//			if(register_id_frew != -1) {
//				cancel_event(this, register_id_frew);
//				register_id_frew = -1;
//			}
		}
		if((pa & 2) && !(data & 2)) {
			fast_forward();
			now_apss = ((pa & 0x80) == 0);
//			register_event(this, EVENT_FFWD, PERIOD_CMT_SIGNAL, false, &register_id_ffwd);
//			now_apss_tmp = ((pa & 0x80) == 0);
//		} else if(!(pa & 2) && (data & 2)) {
//			if(register_id_ffwd != -1) {
//				cancel_event(this, register_id_ffwd);
//				register_id_ffwd = -1;
//			}
		}
		if((pa & 4) && !(data & 4)) {
			forward();
			now_apss = false;
//			register_event(this, EVENT_FWD, PERIOD_CMT_SIGNAL, false, &register_id_fwd);
//		} else if(!(pa & 4) && (data & 4)) {
//			if(register_id_fwd != -1) {
//				cancel_event(this, register_id_fwd);
//				register_id_fwd = -1;
//			}
		}
		if((pa & 8) && !(data & 8)) {
			stop();
			// stop apss
			if(register_id_apss != -1) {
				cancel_event(this, register_id_apss);
				register_id_apss = -1;
			}
			now_apss = false;
//			register_event(this, EVENT_STOP, PERIOD_CMT_SIGNAL, false, &register_id_stop);
//		} else if(!(pa & 8) && (data & 8)) {
//			if(register_id_stop != -1) {
//				cancel_event(this, register_id_stop);
//				register_id_stop = -1;
//			}
		}
#else
		if(!(pa & 1) && (data & 1)) {
			if(data & 2) {
				fast_forward();
//				register_event(this, EVENT_FFWD, PERIOD_CMT_SIGNAL, false, &register_id_ffwd);
			} else {
				fast_rewind();
//				register_event(this, EVENT_FREW, PERIOD_CMT_SIGNAL, false, &register_id_frew);
			}
//		} else if((pa & 1) && !(data & 1)) {
//			if(register_id_ffwd != -1) {
//				cancel_event(this, register_id_ffwd);
//				register_id_ffwd = -1;
//			}
//			if(register_id_frew != -1) {
//				cancel_event(this, register_id_frew);
//				register_id_frew = -1;
//			}
		}
		if(!(pa & 4) && (data & 4)) {
			forward();
//			register_event(this, EVENT_FWD, PERIOD_CMT_SIGNAL, false, &register_id_fwd);
//		} else if((pa & 4) && !(data & 4)) {
//			if(register_id_fwd != -1) {
//				cancel_event(this, register_id_fwd);
//				register_id_fwd = -1;
//			}
		}
		if(!(pa & 8) && (data & 8)) {
			stop();
//			register_event(this, EVENT_STOP, PERIOD_CMT_SIGNAL, false, &register_id_stop);
//		} else if((pa & 8) && !(data & 8)) {
//			if(register_id_stop != -1) {
//				cancel_event(this, register_id_stop);
//				register_id_stop = -1;
//			}
		}
#endif
		pa = data;
	} else if(id == SIG_CMT_PIO_PC) {
		if(!(pc & 2) && (data & 2)) {
			vm->special_reset();
		}
#ifdef _MZ2500
		if(!(pc & 8) && (data & 8)) {
			vm->reset();
		}
#else
		if((pc & 8) && !(data & 8)) {
//			if(register_id_ipl == -1) {
				register_event(this, EVENT_IPL, PERIOD_IPL_SIGNAL, false, &register_id_ipl);
//			}
		} else if(!(pc & 8) && (data & 8)) {
			if(register_id_ipl != -1) {
				cancel_event(this, register_id_ipl);
				register_id_ipl = -1;
			}
		}
#endif
		if((pc & 0x10) && !(data & 0x10)) {
			register_event(this, EVENT_EJECT, PERIOD_CMT_SIGNAL, false, &register_id_eject);
		} else if(!(pc & 0x10) && (data & 0x10)) {
			if(register_id_eject != -1) {
				cancel_event(this, register_id_eject);
				register_id_eject = -1;
			}
		}
		d_drec->write_signal(SIG_DATAREC_OUT, data, 0x80);
		pc = data;
	} else if(id == SIG_CMT_OUT) {
#ifndef _MZ80B
		if(now_apss) {
			if((data & mask) && register_id_apss == -1) {
				register_event(this, EVENT_APSS, 350000, false, &register_id_apss);
				d_pio->write_signal(SIG_I8255_PORT_B, 0x40, 0x40);
			}
		} else
#endif
		if(now_play) {
			d_pio->write_signal(SIG_I8255_PORT_B, (data & mask) ? 0x40 : 0, 0x40);
		}
	} else if(id == SIG_CMT_REMOTE) {
		d_pio->write_signal(SIG_I8255_PORT_B, (data & mask) ? 0 : 8, 8);
	} else if(id == SIG_CMT_END) {
		if((data & mask) && now_play) {
#ifndef _MZ80B
			if(!(pa & 0x20)) {
				fast_rewind();
			}
#endif
			now_play = false;
		}
	} else if(id == SIG_CMT_TOP) {
		if((data & mask) && now_rewind) {
#ifndef _MZ80B
			if(!(pa & 0x40)) {
				forward();
			}
#endif
			now_rewind = false;
		}
	}
}

void CMT::event_callback(int event_id, int err)
{
	if(event_id == EVENT_FREW) {
		fast_rewind();
#ifndef _MZ80B
		now_apss = now_apss_tmp;
#endif
		register_id_frew = -1;
	} else if(event_id == EVENT_FFWD) {
		fast_forward();
#ifndef _MZ80B
		now_apss = now_apss_tmp;
#endif
		register_id_ffwd = -1;
	} else if(event_id == EVENT_FWD) {
		forward();
#ifndef _MZ80B
//		if(register_id_apss != -1) {
//			cancel_event(this, register_id_apss);
//			register_id_apss = -1;
//		}
		now_apss = false;
#endif
		register_id_fwd = -1;
	} else if(event_id == EVENT_STOP) {
		stop();
#ifndef _MZ80B
		if(register_id_apss != -1) {
			cancel_event(this, register_id_apss);
			register_id_apss = -1;
		}
		now_apss = false;
#endif
		register_id_stop = -1;
	} else if(event_id == EVENT_EJECT) {
		emu->close_tape();
		register_id_eject = -1;
#ifndef _MZ80B
	} else if(event_id == EVENT_APSS) {
		d_pio->write_signal(SIG_I8255_PORT_B, 0, 0x40);
		register_id_apss = -1;
#endif
	} else if(event_id == EVENT_IPL) {
		vm->reset();
		register_id_ipl = -1;
	}
}

void CMT::play_tape(bool value)
{
	play = value;
	rec = false;
	d_pio->write_signal(SIG_I8255_PORT_B, play ? 0x10 : 0x30, 0x30);
}

void CMT::rec_tape(bool value)
{
	play = false;
	rec = value;
	d_pio->write_signal(SIG_I8255_PORT_B, rec ? 0 : 0x30, 0x30);
}

void CMT::close_tape()
{
	play = rec = false;
	now_play = now_rewind = false;
	d_pio->write_signal(SIG_I8255_PORT_B, 0x30, 0x30);
	d_pio->write_signal(SIG_I8255_PORT_B, 0, 0x40);
	
#ifndef _MZ80B
	if(register_id_apss != -1) {
		cancel_event(this, register_id_apss);
		register_id_apss = -1;
	}
#endif
}

#define STATE_VERSION	1

void CMT::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(pa);
	state_fio->FputUint8(pc);
	state_fio->FputBool(play);
	state_fio->FputBool(rec);
	state_fio->FputBool(now_play);
	state_fio->FputBool(now_rewind);
	state_fio->FputInt32(register_id_frew);
	state_fio->FputInt32(register_id_ffwd);
	state_fio->FputInt32(register_id_fwd);
	state_fio->FputInt32(register_id_stop);
	state_fio->FputInt32(register_id_eject);
#ifndef _MZ80B
	state_fio->FputInt32(register_id_apss);
	state_fio->FputBool(now_apss);
	state_fio->FputBool(now_apss_tmp);
#endif
	state_fio->FputInt32(register_id_ipl);
}

bool CMT::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	pa = state_fio->FgetUint8();
	pc = state_fio->FgetUint8();
	play = state_fio->FgetBool();
	rec = state_fio->FgetBool();
	now_play = state_fio->FgetBool();
	now_rewind = state_fio->FgetBool();
	register_id_frew = state_fio->FgetInt32();
	register_id_ffwd = state_fio->FgetInt32();
	register_id_fwd = state_fio->FgetInt32();
	register_id_stop = state_fio->FgetInt32();
	register_id_eject = state_fio->FgetInt32();
#ifndef _MZ80B
	register_id_apss = state_fio->FgetInt32();
	now_apss = state_fio->FgetBool();
	now_apss_tmp = state_fio->FgetBool();
#endif
	register_id_ipl = state_fio->FgetInt32();
	return true;
}

