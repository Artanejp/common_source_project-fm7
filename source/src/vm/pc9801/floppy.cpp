/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
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

#define EVENT_TIMER	0

void FLOPPY::reset()
{
#if defined(SUPPORT_2HD_FDD_IF)
	if(d_fdc_2hd) {
		for(int i = 0; i < MAX_DRIVE; i++) {
			d_fdc_2hd->set_drive_type(i, DRIVE_TYPE_2HD);
		}
	}
	ctrlreg_2hd = 0x80;
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	if(d_fdc_2dd) {
		for(int i = 0; i < MAX_DRIVE; i++) {
			d_fdc_2dd->set_drive_type(i, DRIVE_TYPE_2DD);
		}
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

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
#if defined(SUPPORT_2HD_FDD_IF)
	case 0x0090:
		if(d_fdc_2hd) {
			d_fdc_2hd->write_io8(0, data);
		}
		break;
	case 0x0092:
		if(d_fdc_2hd) {
			d_fdc_2hd->write_io8(1, data);
		}
		break;
	case 0x0094:
	case 0x0096:
		if(d_fdc_2hd) {
			if(!(ctrlreg_2hd & 0x80) && (data & 0x80)) {
				d_fdc_2hd->reset();
			}
			d_fdc_2hd->write_signal(SIG_UPD765A_FREADY, data, 0x40);
		}
		ctrlreg_2hd = data;
		break;
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	case 0x00c8:
		if(d_fdc_2dd) {
			d_fdc_2dd->write_io8(0, data);
		}
		break;
	case 0x00ca:
		if(d_fdc_2dd) {
			d_fdc_2dd->write_io8(1, data);
		}
		break;
	case 0x00cc:
	case 0x00ce:
		if(d_fdc_2dd) {
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
		}
		ctrlreg_2dd = data;
		break;
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	case 0x0090:
#if !defined(SUPPORT_HIRESO)
	case 0x00c8:
		if(((addr >> 4) & 1) == (modereg & 1))
#endif
		{
			d_fdc->write_io8(0, data);
		}
		break;
	case 0x0092:
#if !defined(SUPPORT_HIRESO)
	case 0x00ca:
		if(((addr >> 4) & 1) == (modereg & 1))
#endif
		{
			d_fdc->write_io8(1, data);
		}
		break;
	case 0x0094:
	case 0x0096:
#if !defined(SUPPORT_HIRESO)
	case 0x00cc:
	case 0x00ce:
		if(((addr >> 4) & 1) == (modereg & 1))
#endif
		{
			if(!(ctrlreg & 0x80) && (data & 0x80)) {
				d_fdc->reset();
			}
#if !defined(SUPPORT_HIRESO)
			if(!(addr == 0xcc && !(data & 0x20)))
#endif
			{
				d_fdc->write_signal(SIG_UPD765A_FREADY, data, 0x40);
			}
#if defined(SUPPORT_HIRESO)
			if((ctrlreg & 0x20) && !(data & 0x20)) {
				d_fdc->set_drive_type(0, DRIVE_TYPE_2HD);
				d_fdc->set_drive_type(1, DRIVE_TYPE_2HD);
			} else if(!(ctrlreg & 0x20) && (data & 0x20)) {
				d_fdc->set_drive_type(0, DRIVE_TYPE_2DD);
				d_fdc->set_drive_type(1, DRIVE_TYPE_2DD);
			}
#endif
//#if !defined(_PC98XA) && !defined(_PC98XL)
//			if(modereg & 0x04) {
//				d_fdc->write_signal(SIG_UPD765A_MOTOR, data, 0x08);
//			}
			if(data & 0x01) {
				if(timer_id != -1) {
					cancel_event(this, timer_id);
				}
				register_event(this, EVENT_TIMER, 100000, false, &timer_id);
			}
			ctrlreg = data;
//#endif
		}
		break;
	case 0x00be:
#if !defined(SUPPORT_HIRESO)
		if(!(modereg & 2) && (data & 2)) {
			d_fdc->set_drive_type(0, DRIVE_TYPE_2HD);
			d_fdc->set_drive_type(1, DRIVE_TYPE_2HD);
		} else if((modereg & 2) && !(data & 2)) {
			d_fdc->set_drive_type(0, DRIVE_TYPE_2DD);
			d_fdc->set_drive_type(1, DRIVE_TYPE_2DD);
		}
#endif
		modereg = data;
		break;
#endif
	}
}

uint32_t FLOPPY::read_io8(uint32_t addr)
{
	uint32_t value = 0;
	
	switch(addr) {
#if defined(SUPPORT_2HD_FDD_IF)
	case 0x0090:
		if(d_fdc_2hd) {
			return d_fdc_2hd->read_io8(0);
		}
		break;
	case 0x0092:
		if(d_fdc_2hd) {
			return d_fdc_2hd->read_io8(1);
		}
		break;
	case 0x0094:
	case 0x0096:
		if(d_fdc_2hd) {
//			value |= 0x80; // FINT1 (DIP SW 1-7), 1 = OFF, 0 = ON
			value |= 0x40; // FINT0 (DIP SW 1-6), 1 = OFF, 0 = ON
//			value |= 0x20; // DMACH (DIP SW 1-3), 1 = OFF, 0 = ON
			return value;
		}
		break;
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	case 0x00c8:
		if(d_fdc_2dd) {
			return d_fdc_2dd->read_io8(0);
		}
		break;
	case 0x00ca:
		if(d_fdc_2dd) {
			return d_fdc_2dd->read_io8(1);
		}
		break;
	case 0x00cc:
	case 0x00ce:
		if(d_fdc_2dd) {
//			value |= 0x80; // FINT1 (DIP SW 1-7), 1 = OFF, 0 = ON
			value |= 0x40; // FINT0 (DIP SW 1-6), 1 = OFF, 0 = ON
			value |= 0x20; // DMACH (DIP SW 1-3), 1 = OFF, 0 = ON
			if(d_fdc_2dd->is_disk_inserted()) {
				value |= 0x10; // RDY
			}
			return value;
		}
		break;
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	case 0x0090:
#if !defined(SUPPORT_HIRESO)
	case 0x00c8:
		if(((addr >> 4) & 1) == (modereg & 1))
#endif
		{
			return d_fdc->read_io8(0);
		}
		break;
	case 0x0092:
#if !defined(SUPPORT_HIRESO)
	case 0x00ca:
		if(((addr >> 4) & 1) == (modereg & 1))
#endif
		{
			return d_fdc->read_io8(1);
		}
		break;
	case 0x0094:
	case 0x0096:
#if !defined(SUPPORT_HIRESO)
		if(modereg & 1) {
//			value |= 0x80; // FINT1 (DIP SW 1-7), 1 = OFF, 0 = ON
			value |= 0x40; // FINT0 (DIP SW 1-6), 1 = OFF, 0 = ON
//			value |= 0x20; // DMACH (DIP SW 1-3), 1 = OFF, 0 = ON
//			value |= 0x08; // TYP1,0 (DIP SW 1-4), 1,0 = ON  Internal FDD: #3,#4, External FDD: #1,#2
			value |= 0x04; // TYP1,0 (DIP SW 1-4), 0,1 = OFF Internal FDD: #1,#2, External FDD: #3,#4
			return value;
		}
#else
//		value |= 0x80; // MODE, 0 = Internal FDD existing
		value |= ctrlreg & 0x20; // High Density, 1 = 640KB, 0 = 1MB
		return value;
#endif
		break;
#if !defined(SUPPORT_HIRESO)
	case 0x00be:
		return 0xf8 | (modereg & 3);
	case 0x00cc:
	case 0x00ce:
		if(!(modereg & 1)) {
//			value |= 0x80; // FINT1 (DIP SW 1-7), 1 = OFF, 0 = ON
			value |= 0x40; // FINT0 (DIP SW 1-6), 1 = OFF, 0 = ON
			value |= 0x20; // DMACH (DIP SW 1-3), 1 = OFF, 0 = ON
//			value |= 0x08; // TYP1,0 (DIP SW 1-4), 1,0 = ON  Internal FDD: #3,#4, External FDD: #1,#2
			value |= 0x04; // TYP1,0 (DIP SW 1-4), 0,1 = OFF Internal FDD: #1,#2, External FDD: #3,#4
			if(d_fdc->is_disk_inserted()) {
				value |= 0x10; // RDY
			}
			return value;
		}
		break;
#endif
#endif
	}
	return 0xff;//addr & 0xff;
}

void FLOPPY::write_signal(int id, uint32_t data, uint32_t mask)
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
#if !defined(SUPPORT_HIRESO)
		if(modereg & 1) {
			d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR3, data, mask);
		} else {
			d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR2, data, mask);
		}
#else
		d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR3, data, mask);
#endif
		break;
	case SIG_FLOPPY_DRQ:
#if !defined(SUPPORT_HIRESO)
		if(modereg & 1) {
			d_dma->write_signal(SIG_I8237_CH2, data, mask);
		} else {
			d_dma->write_signal(SIG_I8237_CH3, data, mask);
		}
#else
		d_dma->write_signal(SIG_I8237_CH1, data, mask);
#endif
		break;
#endif
	}
}

void FLOPPY::event_callback(int event_id, int err)
{
#if defined(SUPPORT_2DD_FDD_IF)
	if(ctrlreg_2dd & 4) {
		write_signal(SIG_FLOPPY_2DD_IRQ, 1, 1);
	}
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	if(ctrlreg & 4) {
		write_signal(SIG_FLOPPY_IRQ, 1, 1);
	}
#endif
	timer_id = -1;
}

#define STATE_VERSION	1

bool FLOPPY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
#if defined(SUPPORT_2HD_FDD_IF)
	state_fio->StateValue(ctrlreg_2hd);
#endif
#if defined(SUPPORT_2DD_FDD_IF)
	state_fio->StateValue(ctrlreg_2dd);
#endif
#if defined(SUPPORT_2HD_2DD_FDD_IF)
	state_fio->StateValue(ctrlreg);
	state_fio->StateValue(modereg);
#endif
	state_fio->StateValue(timer_id);
	return true;
}

