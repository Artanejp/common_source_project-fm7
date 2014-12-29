/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2007.02.11 -

	[ interrupt ]
*/

#include "interrupt.h"
#include "../../fileio.h"

//#define SUPPURT_CHILD_DEVICE

void INTERRUPT::reset()
{
	for(int ch = 0; ch < 4; ch++) {
		irq[ch].enb_intr = false;
		irq[ch].req_intr = false;
		irq[ch].in_service = false;
	}
	iei = oei = true;
	req_intr_ch = -1;
	select = 0;
}

void INTERRUPT::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0xc6:
		irq[0].enb_intr = ((data & 8) != 0);
		irq[1].enb_intr = ((data & 4) != 0);
		irq[2].enb_intr = ((data & 2) != 0);
		irq[3].enb_intr = ((data & 1) != 0);
		select = data;
		update_intr();
		break;
	case 0xc7:
		if(select & 0x80) {
			irq[0].vector = data;	// crtc
		}
		if(select & 0x40) {
			irq[1].vector = data;	// i8253
		}
		if(select & 0x20) {
			irq[2].vector = data;	// printer
		}
		if(select & 0x10) {
			irq[3].vector = data;	// rp5c15
		}
		break;
	}
}

void INTERRUPT::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_INTERRUPT_CRTC) {
		bool next = ((data & mask) != 0);
		if(next != irq[0].req_intr) {
			irq[0].req_intr = next;
			update_intr();
		}
	} else if(id == SIG_INTERRUPT_I8253) {
		bool next = ((data & mask) != 0);
		if(next != irq[1].req_intr) {
			irq[1].req_intr = next;
			update_intr();
		}
	} else if(id == SIG_INTERRUPT_PRINTER) {
		bool next = ((data & mask) != 0);
		if(next != irq[2].req_intr) {
			irq[2].req_intr = next;
			update_intr();
		}
	} else if(id == SIG_INTERRUPT_RP5C15) {
		bool next = ((data & mask) != 0);
		if(next != irq[3].req_intr) {
			irq[3].req_intr = next;
			update_intr();
		}
	}
}

void INTERRUPT::set_intr_iei(bool val)
{
	if(iei != val) {
		iei = val;
		update_intr();
	}
}

#define set_intr_oei(val) { \
	if(oei != val) { \
		oei = val; \
		if(d_child) { \
			d_child->set_intr_iei(oei); \
		} \
	} \
}

void INTERRUPT::update_intr()
{
#ifdef SUPPURT_CHILD_DEVICE
	// set oei signal
	bool next = false;
	if(iei) {
		next = true;
		for(int ch = 0; ch < 4; ch++) {
			if(irq[ch].in_service) {
				next = false;
				break;
			}
		}
	}
	set_intr_oei(next);
#endif
	// set int signal
	req_intr_ch = -1;
	if(iei) {
		for(int ch = 0; ch < 4; ch++) {
			if(irq[ch].in_service) {
				break;
			}
			if(irq[ch].enb_intr && irq[ch].req_intr) {
				req_intr_ch = ch;
				break;
			}
		}
	}
	if(req_intr_ch != -1) {
		d_cpu->set_intr_line(true, true, intr_bit);
	} else {
		d_cpu->set_intr_line(false, true, intr_bit);
	}
}

uint32 INTERRUPT::intr_ack()
{
	// ack (M1=IORQ=L)
	if(req_intr_ch != -1) {
		int ch = req_intr_ch;
		irq[ch].req_intr = false;
		irq[ch].in_service = true;
#ifdef SUPPURT_CHILD_DEVICE
		update_intr();
#endif
		return irq[ch].vector;
	}
#ifdef SUPPURT_CHILD_DEVICE
	if(d_child) {
		return d_child->intr_ack();
	}
#endif
	return 0xff;
}

void INTERRUPT::intr_reti()
{
	// detect RETI
	for(int ch = 0; ch < 4; ch++) {
		if(irq[ch].in_service) {
			irq[ch].in_service = false;
			update_intr();
			return;
		}
	}
#ifdef SUPPURT_CHILD_DEVICE
	if(d_child) {
		d_child->intr_reti();
	}
#endif
}

#define STATE_VERSION	1

void INTERRUPT::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(select);
	state_fio->Fwrite(irq, sizeof(irq), 1);
	state_fio->FputInt32(req_intr_ch);
	state_fio->FputBool(iei);
	state_fio->FputBool(oei);
	state_fio->FputUint32(intr_bit);
}

bool INTERRUPT::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	select = state_fio->FgetUint8();
	state_fio->Fread(irq, sizeof(irq), 1);
	req_intr_ch = state_fio->FgetInt32();
	iei = state_fio->FgetBool();
	oei = state_fio->FgetBool();
	intr_bit = state_fio->FgetUint32();
	return true;
}

