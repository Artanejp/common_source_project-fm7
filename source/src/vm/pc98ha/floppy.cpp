/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.06.11 -

	[ floppy ]
*/

#include "floppy.h"
#include "../disk.h"
#include "../i8259.h"
#include "../upd71071.h"
#include "../upd765a.h"

#define EVENT_TIMER	0

void FLOPPY::reset()
{
	for(int i = 0; i < 4; i++) {
		d_fdc->set_drive_type(i, DRIVE_TYPE_2DD);
	}
	ctrlreg = 0x80;
	modereg = 0x00;
	timer_id = -1;
}

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0x00ca:
		d_fdc->write_io8(1, data);
		break;
	case 0x00cc:
	case 0x00ce:
		if(!(ctrlreg & 0x80) && (data & 0x80)) {
			d_fdc->reset();
		}
		d_fdc->write_signal(SIG_UPD765A_FREADY, data, 0x40);
		d_fdc->write_signal(SIG_UPD765A_MOTOR, data, 0x08);
		if(data & 0x01) {
			if(timer_id != -1) {
				cancel_event(this, timer_id);
			}
			register_event(this, EVENT_TIMER, 100000, false, &timer_id);
		}
		ctrlreg = data;
		break;
	case 0x00be:
		if(!(modereg & 2) && (data & 2)) {
			d_fdc->set_drive_type(0, DRIVE_TYPE_2HD);
			d_fdc->set_drive_type(1, DRIVE_TYPE_2HD);
		} else if((modereg & 2) && !(data & 2)) {
			d_fdc->set_drive_type(0, DRIVE_TYPE_2DD);
			d_fdc->set_drive_type(1, DRIVE_TYPE_2DD);
		}
		modereg = data;
		break;
	}
}

uint32_t FLOPPY::read_io8(uint32_t addr)
{
	switch(addr & 0xffff) {
	case 0x00c8:
		return d_fdc->read_io8(0);
	case 0x00ca:
		return d_fdc->read_io8(1);
	case 0x00cc:
	case 0x00ce:
		return (d_fdc->is_disk_inserted() ? 0x10 : 0) | 0x6c;
	case 0x00be:
		return (modereg & 0x03) | 0xe4;
	}
	return addr & 0xff;
}

void FLOPPY::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_FLOPPY_IRQ:
		d_pic->write_signal(SIG_I8259_IR6, data, mask);
		break;
	case SIG_FLOPPY_DRQ:
		d_dma->write_signal(SIG_UPD71071_CH3, data, mask);
		break;
	}
}

void FLOPPY::event_callback(int event_id, int err)
{
	if(ctrlreg & 4) {
		write_signal(SIG_FLOPPY_IRQ, 1, 1);
	}
	timer_id = -1;
}

#define STATE_VERSION	2

bool FLOPPY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(ctrlreg);
	state_fio->StateValue(modereg);
	state_fio->StateValue(timer_id);
	return true;
}

