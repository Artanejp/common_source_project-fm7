/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.30 -

	[ floppy ]
*/

#include "floppy.h"
#include "../disk.h"
#include "../i8259.h"
#include "../mb8877.h"

void FLOPPY::initialize()
{
	drvreg = drvsel = 0;
	irq = irqmsk = false;
	changed[0] = changed[1] = changed[2] = changed[3] = false;
}

void FLOPPY::reset()
{
	for(int i = 0; i < MAX_DRIVE; i++) {
		d_fdc->set_drive_type(i, DRIVE_TYPE_2HD);
		d_fdc->set_drive_mfm (i, true);
	}
}

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	int nextdrv = drvsel;
	
	switch(addr & 0xffff) {
	case 0x208:
		// drive control register
		irqmsk = ((data & 1) != 0);
		update_intr();
		// note: bit5 is CLKSEL, but 0 is set while seeking
/*
		for(int i = 0; i < MAX_DRIVE; i++) {
			if(data & 0x20) {
				d_fdc->set_drive_type(i, DRIVE_TYPE_2DD);
			} else {
				d_fdc->set_drive_type(i, DRIVE_TYPE_2HD);
			}
		}
*/
		for(int i = 0; i < 4; i++) {
			d_fdc->set_drive_mfm(i, ((data & 2) != 0));
		}
		d_fdc->write_signal(SIG_MB8877_MOTOR, data, 0x10);
		d_fdc->write_signal(SIG_MB8877_SIDEREG, data, 4);
		break;
	case 0x20c:
		// drive select register
		if(data & 0x0f) {
			if(data & 1) {
				nextdrv = 0;
			} else if(data & 2) {
				nextdrv = 1;
			} else if(data & 4) {
				nextdrv = 2;
			} else if(data & 8) {
				nextdrv = 3;
			}
			if(drvsel != nextdrv) {
				d_fdc->write_signal(SIG_MB8877_DRIVEREG, drvsel = nextdrv, 3);
			}
			for(int i = 0; i < MAX_DRIVE; i++) {
				if((data & 0xc0) == 0x00) {
					d_fdc->set_drive_type(i, DRIVE_TYPE_2DD); // 300rpm
				} else if((data & 0xc0) == 0x40) {
					d_fdc->set_drive_type(i, DRIVE_TYPE_2HD); // 360rpm
				}
			}
		}
		drvreg = data;
		break;
	}
}

uint32_t FLOPPY::read_io8(uint32_t addr)
{
	switch(addr & 0xffff) {
	case 0x208:
		{
			int drvreg = d_fdc->read_signal(SIG_MB8877_DRIVEREG);
			uint32_t fdc_status = d_fdc->is_disk_inserted(drvreg) ? 2 : 0;
			
			if(changed[drvsel]) {
				changed[drvsel] = false;
				return fdc_status | 0xe1;	// fdd*2
			}
//			return fdc_status | 0x60;	// fdd*1
			return fdc_status | 0xe0;	// fdd*2
		}
	case 0x20c:
		return drvreg;
	}
	return 0xff;
}

void FLOPPY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_FLOPPY_IRQ) {
		irq = ((data & mask) != 0);
		update_intr();
	}
}

void FLOPPY::update_intr()
{
	d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR6, irq && irqmsk ? 1 : 0, 1);
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
	state_fio->StateValue(drvreg);
	state_fio->StateValue(drvsel);
	state_fio->StateValue(irq);
	state_fio->StateValue(irqmsk);
	state_fio->StateArray(changed, sizeof(changed), 1);
	return true;
}

