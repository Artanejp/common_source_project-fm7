/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2010.09.15-

	[ floppy ]
*/

#include "floppy.h"
#include "../disk.h"
#include "../i8237.h"
#include "../i8259.h"
#include "../upd765a.h"
#include "../../fileio.h"

#define EVENT_TIMER	0

void FLOPPY::reset()
{
#if defined(SUPPORT_2HD_FDD_IF)
	for(int i = 0; i < MAX_DRIVE; i++) {
		d_fdc_2hd->set_drive_type(i, DRIVE_TYPE_2HD);
	}
	ctrlreg_2hd = 0x80;
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	for(int i = 0; i < MAX_DRIVE; i++) {
		d_fdc_2dd->set_drive_type(i, DRIVE_TYPE_2DD);
	}
	ctrlreg_2dd = 0x80;
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	for(int i = 0; i < 4; i++) {
		d_fdc->set_drive_type(i, DRIVE_TYPE_2HD);
	}
	ctrlreg = 0x80;
	modereg = 0x03;
#endif
	timer_id = -1;
}

void FLOPPY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xffff) {
#if defined(SUPPORT_2HD_FDD_IF)
	case 0x90:
		d_fdc_2hd->write_io8(0, data);
		break;
	case 0x92:
		d_fdc_2hd->write_io8(1, data);
		break;
	case 0x94:
		if(!(ctrlreg_2hd & 0x80) && (data & 0x80)) {
			d_fdc_2hd->reset();
		}
		d_fdc_2hd->write_signal(SIG_UPD765A_FREADY, data, 0x40);
		ctrlreg_2hd = data;
		break;
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	case 0xc8:
		d_fdc_2dd->write_io8(0, data);
		break;
	case 0xca:
		d_fdc_2dd->write_io8(1, data);
		break;
	case 0xcc:
		if(!(ctrlreg_2dd & 0x80) && (data & 0x80)) {
			d_fdc_2dd->reset();
		}
		if(data & 1) {
			if(timer_id != -1) {
				cancel_event(this, timer_id);
			}
			register_event(this, EVENT_TIMER, 100000, false, &timer_id);
		}
		d_fdc_2dd->write_signal(SIG_UPD765A_MOTOR, data, 0x08);
		ctrlreg_2dd = data;
		break;
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	case 0x90:
	case 0xc8:
		if(((addr >> 4) & 1) == (modereg & 1)) {
			d_fdc->write_io8(0, data);
		}
		break;
	case 0x92:
	case 0xca:
		if(((addr >> 4) & 1) == (modereg & 1)) {
			d_fdc->write_io8(1, data);
		}
		break;
	case 0x94:
	case 0xcc:
		if(((addr >> 4) & 1) == (modereg & 1)) {
			if(!(ctrlreg & 0x80) && (data & 0x80)) {
				d_fdc->reset();
			}
			if(!(addr == 0xcc && !(data & 0x20))) {
				d_fdc->write_signal(SIG_UPD765A_FREADY, data, 0x40);
			}
			if(data & 1) {
				if(timer_id != -1) {
					cancel_event(this, timer_id);
				}
				register_event(this, EVENT_TIMER, 100000, false, &timer_id);
			}
//			if(modereg & 4) {
//				d_fdc->write_signal(SIG_UPD765A_MOTOR, data, 0x08);
//			}
			ctrlreg = data;
		}
		break;
	case 0xbe:
		if(!(modereg & 2) && (data & 2)) {
			d_fdc->set_drive_type(0, DRIVE_TYPE_2HD);
			d_fdc->set_drive_type(1, DRIVE_TYPE_2HD);
		} else if((modereg & 2) && !(data & 2)) {
			d_fdc->set_drive_type(0, DRIVE_TYPE_2DD);
			d_fdc->set_drive_type(1, DRIVE_TYPE_2DD);
		}
		modereg = data;
		break;
#endif
	}
}

uint32 FLOPPY::read_io8(uint32 addr)
{
	switch(addr & 0xffff) {
#if defined(SUPPORT_2HD_FDD_IF)
	case 0x90:
		return d_fdc_2hd->read_io8(0);
	case 0x92:
		return d_fdc_2hd->read_io8(1);
#if !defined(_PC9801)
	case 0x94:
		return 0x5f;	// 0x40 ???
#endif
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	case 0xc8:
		return d_fdc_2dd->read_io8(0);
	case 0xca:
		return d_fdc_2dd->read_io8(1);
	case 0xcc:
		return (d_fdc_2dd->disk_inserted() ? 0x10 : 0) | 0x6f;	// 0x60 ???
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	case 0x90:
	case 0xc8:
		if(((addr >> 4) & 1) == (modereg & 1)) {
			return d_fdc->read_io8(0);
		}
		break;
	case 0x92:
	case 0xca:
		if(((addr >> 4) & 1) == (modereg & 1)) {
			return d_fdc->read_io8(1);
		}
		break;
	case 0x94:
		if(modereg & 1) {
			return 0x44;
		}
		break;
	case 0xbe:
		return 0xf8 | (modereg & 3);
	case 0xcc:
		if(!(modereg & 1)) {
			return (d_fdc->disk_inserted() ? 0x10 : 0) | 0x64;	// 0x60 ???
		}
		break;
#endif
	}
	return 0xff;//addr & 0xff;
}

void FLOPPY::write_signal(int id, uint32 data, uint32 mask)
{
	switch(id) {
#if defined(SUPPORT_2HD_FDD_IF)
	case SIG_FLOPPY_2HD_IRQ:
		d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR3, data, mask);
		break;
	case SIG_FLOPPY_2HD_DRQ:
		d_dma->write_signal(SIG_I8237_CH2, data, mask);
		break;
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	case SIG_FLOPPY_2DD_IRQ:
		d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR2, data, mask);
		break;
	case SIG_FLOPPY_2DD_DRQ:
		d_dma->write_signal(SIG_I8237_CH3, data, mask);
		break;
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	case SIG_FLOPPY_IRQ:
		if(modereg & 1) {
			d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR3, data, mask);
		} else {
			d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR2, data, mask);
		}
		break;
	case SIG_FLOPPY_DRQ:
		if(modereg & 1) {
			d_dma->write_signal(SIG_I8237_CH2, data, mask);
		} else {
			d_dma->write_signal(SIG_I8237_CH3, data, mask);
		}
		break;
#endif
	}
}

void FLOPPY::event_callback(int event_id, int err)
{
#if defined(SUPPORT_2DD_FDD_IF)
	if(ctrlreg_2dd & 4) {
		d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR2, 1, 1);
	}
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	if(ctrlreg & 4) {
		d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR2, 1, 1);
	}
#endif
	timer_id = -1;
}

#define STATE_VERSION	1

void FLOPPY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
#if defined(SUPPORT_2HD_FDD_IF)
	state_fio->FputUint8(ctrlreg_2hd);
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	state_fio->FputUint8(ctrlreg_2dd);
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	state_fio->FputUint8(ctrlreg);
	state_fio->FputUint8(modereg);
#endif
	state_fio->FputInt32(timer_id);
}

bool FLOPPY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
#if defined(SUPPORT_2HD_FDD_IF)
	ctrlreg_2hd = state_fio->FgetUint8();
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	ctrlreg_2dd = state_fio->FgetUint8();
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	ctrlreg = state_fio->FgetUint8();
	modereg = state_fio->FgetUint8();
#endif
	timer_id = state_fio->FgetInt32();
	return true;
}

