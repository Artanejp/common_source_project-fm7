/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.30 -

	[ floppy ]
*/

#include "vm.h"
#include "floppy.h"
#include "../disk.h"
#include "../i8259.h"
#include "../mb8877.h"
namespace FMTOWNS {
void FLOPPY::initialize()
{
	drive_swapped = false; // ToDo: implement via config;
	for(int i = 0; i < 4; i++) {
		is_inserted[i] = false;
	}
}

void FLOPPY::reset()
{
	drvreg = 0;
	irq = irqmsk = false;
	drvsel = (drive_swapped) ? 2 : 0;

	for(int i = 0; i < MAX_DRIVE; i++) {
		d_fdc->set_drive_type(i, DRIVE_TYPE_2HD);
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
		if((drvsel < 2) || (machine_id < 0x0200)) {
			d_fdc->write_signal(SIG_MB8877_MOTOR, data, 0x10);
		} else {
			// 5 Inch
			d_fdc->write_signal(SIG_MB8877_MOTOR, data, 0x40);
		}
		d_fdc->write_signal(SIG_MB8877_SIDEREG, data, 4);
		 // ToDo: Single dencity.
		{
			DISK *_disk = d_fdc->get_disk_handler(drvsel);
			uint8_t _media = d_fdc->get_media_type(drvsel);
			if((data & 0x02) != 0) { // DDEN=2D/2DD/2HD
				if(_disk != NULL) {
					_disk->track_mfm = true;
				}
				if((data & 0x20) == 0) { // 2HD or 144
					switch(_media) {
					case MEDIA_TYPE_2HD:
						d_fdc->set_drive_type(drvsel, DRIVE_TYPE_2HD);
						break;
					case MEDIA_TYPE_144: // ToDo: Write failure, Read success. 20200925 K.O
 						d_fdc->set_drive_type(drvsel, DRIVE_TYPE_144);
						break;
					default:
						d_fdc->set_drive_type(drvsel, DRIVE_TYPE_UNK); // CLOCK IS WRONG.
						break;
					}
				} else {
					switch(_media) {
					case MEDIA_TYPE_2DD:
						d_fdc->set_drive_type(drvsel, DRIVE_TYPE_2D);
						break;
					case MEDIA_TYPE_2D:
 						d_fdc->set_drive_type(drvsel, DRIVE_TYPE_2D);
						break;
					default:
						d_fdc->set_drive_type(drvsel, DRIVE_TYPE_UNK); // CLOCK IS WRONG.
						break;
					}
				}
			} else { // 2DD or 2D or 1D
				if(_disk != NULL) {
					_disk->track_mfm = false;
				}
				if((data & 0x20) == 0) { // 2HD or 144
					d_fdc->set_drive_type(drvsel, DRIVE_TYPE_UNK); // CLOCK IS WRONG.
					// And maybe single density of 2HD/144HD don't exist.
				} else {
					// OK. Now use single density disk.
					switch(_media) {
					case MEDIA_TYPE_2DD:
						d_fdc->set_drive_type(drvsel, DRIVE_TYPE_2D);
						break;
					case MEDIA_TYPE_2D:
 						d_fdc->set_drive_type(drvsel, DRIVE_TYPE_2D);
						break;
					default:
						d_fdc->set_drive_type(drvsel, DRIVE_TYPE_UNK); // CLOCK IS WRONG.
						break;
					}
				}					
			}
		}
 		break;
	case 0x20c:
		// drive select register
		// ToDo: IN USE bit
		if(data & 1) {
			nextdrv = 0;
		} else if(data & 2) {
			nextdrv = 1;
		} else if(data & 4) {
			nextdrv = 2;
		} else if(data & 8) {
			nextdrv = 3;
		}
		if(drive_swapped) {
			nextdrv = (nextdrv + 2) & 3;
		}
		if(drvsel != nextdrv) {
			d_fdc->write_signal(SIG_MB8877_DRIVEREG, drvsel = nextdrv, 3);
		}
		d_fdc->set_drive_type(drvreg & 3, ((data & 0x40) != 0) ? DRIVE_TYPE_2HD : DRIVE_TYPE_2DD);
		drvreg = data;
		break;
	case 0x20e:
		drive_swapped = ((data & 0x01) != 0);
		break;
	}
}

uint32_t FLOPPY::read_io8(uint32_t addr)
{
	uint8_t val;
	switch(addr & 0xffff) {
	case 0x208:
		if(machine_id >= 0x0200) {
			if(is_inserted[drvsel]) { // OK?
				val = 0x00;
			} else {
				val = 0x01;
			}
		} else {
			val = 0x01;
		}
		is_inserted[drvsel] = false;
		if(d_fdc->is_disk_inserted(drvsel)) val |= 0x02;
		val |= 0x04; // ToDo 5.25 inch
		val |= 0x80; // 2 Drives (maybe default, will change) 20200925 K.O
		return val;
	case 0x20c:
		return drvreg;
	case 0x20e:
		val = (drive_swapped) ? 1 : 0;
		return val;
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
	write_signals(&output_intr_line, irq && irqmsk ? 1 : 0);
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
	
	state_fio->StateValue(drvreg);
	state_fio->StateValue(drvsel);
	state_fio->StateValue(irq);
	state_fio->StateValue(irqmsk);
	state_fio->StateValue(drive_swapped);
	
	state_fio->StateValue(cpu_id);
	state_fio->StateValue(machine_id);
	state_fio->StateBuffer(is_inserted, sizeof(is_inserted), 1);
	
	return true;
}

}
