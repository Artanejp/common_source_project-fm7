/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2007.02.11 -

	[ interrupt ]
*/

#include "interrupt.h"

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

void INTERRUPT::write_io8(uint32_t addr, uint32_t data)
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

void INTERRUPT::write_signal(int id, uint32_t data, uint32_t mask)
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

uint32_t INTERRUPT::get_intr_ack()
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
		return d_child->get_intr_ack();
	}
#endif
	return 0xff;
}

void INTERRUPT::notify_intr_reti()
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
		d_child->notify_intr_reti();
	}
#endif
}

#define STATE_VERSION	2

bool INTERRUPT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(select);
	for(int i = 0; i < array_length(irq); i++) {
		state_fio->StateValue(irq[i].vector);
		state_fio->StateValue(irq[i].enb_intr);
		state_fio->StateValue(irq[i].req_intr);
		state_fio->StateValue(irq[i].in_service);
	}
	state_fio->StateValue(req_intr_ch);
	state_fio->StateValue(iei);
	state_fio->StateValue(oei);
	state_fio->StateValue(intr_bit);
	return true;
}

