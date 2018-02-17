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
#if defined(HAS_DMA)
#include "hd6844.h"
#endif

#define FDC_CMD_TYPE1		1
#define FDC_CMD_RD_SEC		2
#define FDC_CMD_RD_MSEC		3
#define FDC_CMD_WR_SEC		4
#define FDC_CMD_WR_MSEC		5
#define FDC_CMD_RD_ADDR		6
#define FDC_CMD_RD_TRK		7
#define FDC_CMD_WR_TRK		8
#define FDC_CMD_TYPE4		0x80

void FM7_MAINIO::reset_fdc_2HD(void)
{
#if defined(HAS_2HD)
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return;
	fdc_2HD_cmdreg = 0;
	fdc_2HD_statreg = fdc_2HD->read_io8(0);
	fdc_2HD_trackreg = fdc_2HD->read_io8(1);
	fdc_2HD_sectreg = fdc_2HD->read_io8(2);
	fdc_2HD_datareg = fdc_2HD->read_io8(3);
	fdc_2HD_headreg = 0xfe | fdc_2HD->read_signal(SIG_MB8877_SIDEREG);
	fdc_2HD_drvsel = 0x7c | (fdc_2HD->read_signal(SIG_MB8877_DRIVEREG) & 0x03);
	irqreg_fdc_2HD = 0x00; //0b00000000;
	//fdc_2HD_motor = (fdc_2HD->read_signal(SIG_MB8877_MOTOR) != 0);
	fdc_2HD_motor = false;
		
	if(event_fdc_motor_2HD >= 0) cancel_event(this, event_fdc_motor_2HD);
	event_fdc_motor_2HD = -1;
	//irqstat_2HD_fdc = false;
	//irqmask_2HD_mfd = true;
	if(connect_fdc_2HD) {
		set_fdc_motor_2HD(fdc_2HD_motor);
	}
#endif
}

void FM7_MAINIO::set_fdc_cmd_2HD(uint8_t val)
{
#if defined(HAS_2HD)
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return;
	//irqreg_fdc = 0x00;
	fdc_2HD_cmdreg = val;
#if 0
#if defined(HAS_DMA)
	if(((fdc_2HD_cmdreg >= 0x80) && (fdc_2HD_cmdreg < 0xd0)) || (fdc_2HD_cmdreg >= 0xe0)) {
		uint32_t words = dmac->read_signal(HD6844_WORDS_REG_0);
		if((words != 0) && (words < 0xffff) && (dmac->read_signal(HD6844_IS_TRANSFER_0) == 0)) {
			dmac->write_signal(HD6844_SRC_FIXED_ADDR_CH0, 3, 0xffffffff);
			dmac->write_signal(HD6844_TRANSFER_START, 0, 0xffffffff);
			//this->out_debug_log(_T("FDC: Start DMA CMDREG=%02x CHRN=%02x %02x %02x * DRVSEL=%08x\n"),
			//					 fdc_cmdreg, fdc_trackreg, fdc_headreg & 0x01, fdc_sectreg, fdc_drvsel);
		}
	}
#endif
#endif
	fdc_2HD->write_io8(0, val & 0x00ff);
#ifdef _FM7_FDC_DEBUG	
	this->out_debug_log(_T("FDC(2HD): CMD: $%02x"), fdc_2HD_cmdreg);
#endif	
#endif
}

uint8_t FM7_MAINIO::get_fdc_stat_2HD(void)
{
#if defined(HAS_2HD)
	uint32_t stat_backup = fdc_2HD_statreg;
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return 0xff;
	fdc_2HD_statreg =  fdc_2HD->read_io8(0);
#ifdef _FM7_FDC_DEBUG	
	if(stat_backup != fdc_2HD_statreg) this->out_debug_log(_T("FDC(2HD): \nGet Stat(not busy): $%02x"), fdc_2HD_statreg);
#endif	
	return fdc_2HD_statreg;
#else
	return 0xff;
#endif
}

void FM7_MAINIO::set_fdc_track_2HD(uint8_t val)
{
#if defined(HAS_2HD)
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return;
	fdc_2HD_trackreg = val;
	fdc_2HD->write_io8(1, val);
#ifdef _FM7_FDC_DEBUG	
	this->out_debug_log(_T("FDC(2HD) : Set Track: %d"), val);
#endif
#endif
}

uint8_t FM7_MAINIO::get_fdc_track_2HD(void)
{
#if defined(HAS_2HD)
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return 0xff;
	fdc_2HD_trackreg = fdc_2HD->read_io8(1);
	return fdc_2HD_trackreg;
#else
	return 0xff;
#endif
}

void FM7_MAINIO::set_fdc_sector_2HD(uint8_t val)
{
#if defined(HAS_2HD)
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return;
	fdc_2HD_sectreg = val;
	fdc_2HD->write_io8(2, val);
#ifdef _FM7_FDC_DEBUG	
	this->out_debug_log(_T("FDC(2HD): Set Sector: $%02x"), val);
#endif
#endif
}

uint8_t FM7_MAINIO::get_fdc_sector_2HD(void)
{
#if defined(HAS_2HD)
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return 0xff;
	fdc_2HD_sectreg = fdc_2HD->read_io8(2);
	return fdc_2HD_sectreg;
#else
	return 0xff;
#endif
}
  
void FM7_MAINIO::set_fdc_data_2HD(uint8_t val)
{
#if defined(HAS_2HD)
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return;
	fdc_2HD_datareg = val;
	fdc_2HD->write_io8(3, val & 0x00ff);
#endif
}

uint8_t FM7_MAINIO::get_fdc_data_2HD(void)
{
#if defined(HAS_2HD)
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return 0xff;
	fdc_2HD_datareg = fdc_2HD->read_io8(3);
	
	return fdc_2HD_datareg;
#else
	return 0xff;
#endif
}

uint8_t FM7_MAINIO::get_fdc_motor_2HD(void)
{
#if defined(HAS_2HD)
	uint8_t val = 0x7c; //0b01111100;
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return 0xff;
	fdc_2HD_motor = (fdc_2HD->read_signal(SIG_MB8877_MOTOR) != 0) ? true : false;
	fdc_2HD_motor = fdc_2HD_motor & (fdc_2HD->get_drive_type(fdc_2HD_drvsel & 3) != DRIVE_TYPE_UNK);
	if(fdc_2HD_motor) val |= 0x80;
	val = val | (fdc_2HD_drvsel & 0x03);
	// OK?
#ifdef _FM7_FDC_DEBUG	
	this->out_debug_log(_T("FDC(2HD): Get motor/Drive: $%02x"), val);
#endif	
	return val;
#else
	return 0xff;
#endif
}
  
void FM7_MAINIO::set_fdc_fd1c_2HD(uint8_t val)
{
#if defined(HAS_2HD)
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return;
	fdc_2HD_headreg = (val & 0x01) | 0xfe;
	fdc_2HD->write_signal(SIG_MB8877_SIDEREG, val, 0x01);
#ifdef _FM7_FDC_DEBUG	
	this->out_debug_log(_T("FDC(2HD): Set side/head: $%02x"), val);
#endif
#endif
}

uint8_t FM7_MAINIO::get_fdc_fd1c_2HD(void)
{
#if defined(HAS_2HD)
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return 0xff;
	//fdc_2HD_headreg = fdc_2HD->read_signal(SIG_MB8877_SIDEREG);
	return fdc_2HD_headreg;
#else
	return 0xff;
#endif
}

void FM7_MAINIO::set_fdc_fd1d_2HD(uint8_t val)
{
#if defined(HAS_2HD)
	bool backup_motor = fdc_2HD_motor;
	bool f;
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return;
	if((val & 0x80) != 0) {
		f = true;
	} else {
		f = false;
	}

	fdc_2HD->write_signal(SIG_MB8877_DRIVEREG, val, 0x03);
	fdc_2HD_drvsel = val;

	if(f != backup_motor) {
		if(event_fdc_motor_2HD >= 0) cancel_event(this, event_fdc_motor_2HD);
		// OK?
		if(f) {
			register_event(this, EVENT_FD_MOTOR_ON_2HD, 1000.0 * 300.0, false, &event_fdc_motor_2HD); // Motor ON After 0.3Sec.
		} else {
			register_event(this, EVENT_FD_MOTOR_OFF_2HD, 1000.0 * 50.0, false, &event_fdc_motor_2HD); // Motor OFF After 0.05Sec.
		}
	}
#ifdef _FM7_FDC_DEBUG	
	this->out_debug_log(_T("FDC(2HD): Set Drive Select: $%02x"), val);
#endif
#endif
}

uint8_t FM7_MAINIO::get_fdc_fd1e_2HD(void)
{
	return 0xff;
}	

void FM7_MAINIO::set_fdc_fd1e_2HD(uint8_t val)
{
}

void FM7_MAINIO::set_irq_mfd_2HD(bool flag)
{
#if defined(HAS_2HD)
	bool backup = irqstat_fdc_2hd;
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return;
	if(flag) {
		irqreg_fdc_2HD |= 0x40; //0b01000000;
	} else {
		irqreg_fdc_2HD &= 0xbf; //0b10111111;
	}
	if(intmode_fdc) { // Temporally implement.
		if(flag) {
#if 0			
			double t;
			if(event_2hd_nmi >= 0) cancel_event(this, event_2hd_nmi);
			t = (double)nmi_delay;
			register_event(this, EVENT_FD_NMI_2HD, t, false, &event_2hd_nmi);
			irqstat_fdc_2hd = flag;
#else
			do_nmi(true);
#endif
		} else {
			//event_2hd_nmi = -1;
		}
	} else {
		irqstat_fdc_2hd = flag & !irqmask_mfd;
		if(backup != irqstat_fdc_2hd) do_irq();
	}
#endif
	return;
	
}

void FM7_MAINIO::set_drq_mfd_2HD(bool flag)
{
#if defined(HAS_2HD)
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return;
	if(flag) {
		irqreg_fdc_2HD |= 0x80;//0b10000000;
	} else {
		irqreg_fdc_2HD &= 0x7f; //0b01111111;
	}
	if(intmode_fdc) {
		//if(drqstat_fdc_2hd != flag) {
			drqstat_fdc_2hd = flag;
			do_firq();
			//}
	}
# if defined(HAS_DMA)
	if((dmac->read_signal(HD6844_IS_TRANSFER_0) != 0) && (flag)) {
		dmac->write_signal(HD6844_DO_TRANSFER, 0x0, 0xffffffff);
	}
# endif
#endif
	return;
}

uint8_t FM7_MAINIO::fdc_getdrqirq_2HD(void)
{
#if defined(HAS_2HD)
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return 0xff;
	uint8_t val = irqreg_fdc_2HD | 0x3f;
	return val;
#else
	return 0xff;
#endif
}

void FM7_MAINIO::set_fdc_motor_2HD(bool flag)
{
#if defined(HAS_2HD)
	if(!(connect_fdc_2HD) || (fdc_2HD == NULL)) return;
	fdc_2HD->write_signal(SIG_MB8877_MOTOR, flag ? 0x01 : 0x00, 0x01);
	fdc_2HD_motor = (fdc_2HD->read_signal(SIG_MB8877_MOTOR) != 0);
	fdc_2HD_motor = fdc_2HD_motor & (fdc_2HD->get_drive_type(fdc_2HD_drvsel & 3) != DRIVE_TYPE_UNK);
#endif
}
