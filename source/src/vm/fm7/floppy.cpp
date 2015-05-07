/*
 * FM-7 Main I/O -> FDC [floppy.cpp]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Mar 19, 2015 : Initial, split from fm7_mainio.cpp .
 *
 */

#include "fm7_mainio.h"

#include "../mc6809.h"
#include "../z80.h"

#include "../mb8877.h"
#include "../disk.h"

void FM7_MAINIO::reset_fdc(void)
{
  
  	fdc_statreg = 0x00;
	fdc_cmdreg = 0x00;
	fdc_trackreg = 0x00;
	fdc_sectreg = 0x00;
	fdc_datareg = 0x00;
	fdc_headreg = 0xfe;
	fdc_drvsel = 0x00;
	fdc_motor = false;
	irqreg_fdc = 0xff; //0b11111111;
	if(connect_fdc) {
		extdet_neg = true;
		irqreg_fdc = 0x1f; //0b00011111;
	}
	irqstat_fdc = false;
	irqmask_mfd = true;
}

/* FDD */

void FM7_MAINIO::set_fdc_cmd(uint8 val)
{
	if(!connect_fdc) return;
	irqreg_fdc = 0x00;
	fdc_cmdreg = val;
	fdc->write_io8(0, val & 0x00ff);
}

uint8 FM7_MAINIO::get_fdc_stat(void)
{
	if(!connect_fdc) return 0xff;
	fdc_statreg =  fdc->read_io8(0);
	return fdc_statreg;
}

void FM7_MAINIO::set_fdc_track(uint8 val)
{
	if(!connect_fdc) return;
	// if mode is 2DD and type-of-image = 2D then val >>= 1;
	fdc_trackreg = val;
	fdc->write_io8(1, val & 0x00ff);
}

uint8 FM7_MAINIO::get_fdc_track(void)
{
	if(!connect_fdc) return 0xff;
	fdc_trackreg = fdc->read_io8(1);
	return fdc_trackreg;
}

void FM7_MAINIO::set_fdc_sector(uint8 val)
{
	if(!connect_fdc) return;
	fdc_sectreg = val;
	fdc->write_io8(2, val & 0x00ff);
}

uint8 FM7_MAINIO::get_fdc_sector(void)
{
	if(!connect_fdc) return 0xff;
	fdc_sectreg = fdc->read_io8(2);
	return fdc_sectreg;
}
  
void FM7_MAINIO::set_fdc_data(uint8 val)
{
	if(!connect_fdc) return;
	fdc_datareg = val;
	fdc->write_io8(3, val & 0x00ff);
}

uint8 FM7_MAINIO::get_fdc_data(void)
{
	if(!connect_fdc) return 0xff;
	fdc_datareg = fdc->read_io8(3);
	return fdc_datareg;
}

uint8 FM7_MAINIO::get_fdc_motor(void)
{
	uint8 val = 0x3c; //0b00111100;
	if(!connect_fdc) return 0xff;
	if(fdc_motor) val |= 0x80;
	val = val | (fdc_drvsel & 0x03);
	return val;
}
  
void FM7_MAINIO::set_fdc_fd1c(uint8 val)
{
	if(!connect_fdc) return;
	fdc_headreg = (val & 0x01) | 0xfe;
	fdc->write_signal(SIG_MB8877_SIDEREG, val, 0x01);
}

uint8 FM7_MAINIO::get_fdc_fd1c(void)
{
	if(!connect_fdc) return 0xff;
	return fdc_headreg;
}

void FM7_MAINIO::set_fdc_fd1d(uint8 val)
{
	bool backup_motor = fdc_motor;
	if(!connect_fdc) return;
	if((val & 0x80) != 0) {
		fdc_motor = true;
	} else {
		fdc_motor = false;
	}
	//	fdc->write_signal(SIG_MB8877_DRIVEREG, val, 0x07);
	fdc->write_signal(SIG_MB8877_DRIVEREG, val, 0x03);
	if(fdc_motor != backup_motor) {
		if(fdc_motor) {
			register_event(this, EVENT_FD_MOTOR_ON, 1000.0 * 300.0, false, NULL); // Motor ON After 0.3Sec.
		} else {
			register_event(this, EVENT_FD_MOTOR_OFF, 1000.0 * 300.0, false, NULL); // Motor OFF After 0.3Sec.
		}
	}
	fdc_drvsel = val;
}

void FM7_MAINIO::set_irq_mfd(bool flag)
{
	uint8 backup = irqreg_fdc;

	if(!connect_fdc) return;
	if(flag) {
		irqreg_fdc |= 0x40; //0b01000000;
		if(!(irqmask_mfd)) irqstat_fdc = true;
	} else {
		irqreg_fdc &= 0xbf; //0b10111111;
		irqstat_fdc = false;
	}
	if(backup != irqreg_fdc) do_irq();
	return;
}

void FM7_MAINIO::set_drq_mfd(bool flag)
{
	if(!connect_fdc) return;
	if(flag) {
		irqreg_fdc |= 0x80;//0b10000000;
	} else {
	  irqreg_fdc &= 0x7f; //0b01111111;
	}
	return;
}

uint8 FM7_MAINIO::fdc_getdrqirq(void)
{
	uint8 val = irqreg_fdc | 0x3f;
	if((fdc->read_io8(0) & 0x01) == 0) val |= 0x40; // Workaround of 太陽の神殿
	irqreg_fdc |= 0x20; //0b00100000;
	return val;
}

void FM7_MAINIO::set_fdc_motor(bool flag)
{
	fdc->write_signal(SIG_MB8877_MOTOR, flag ? 0x01 : 0x00, 0x01);
}
