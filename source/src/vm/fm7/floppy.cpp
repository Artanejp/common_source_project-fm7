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

void FM7_MAINIO::reset_fdc(void)
{
	if(connect_fdc) {
		fdc_cmdreg = 0;
		fdc_statreg = fdc->read_io8(0);
		fdc_trackreg = fdc->read_io8(1);
		fdc_sectreg = fdc->read_io8(2);
		fdc_datareg = fdc->read_io8(3);
		fdc_headreg = 0xfe | fdc->read_signal(SIG_MB8877_SIDEREG);
		fdc_drvsel = 0x7c | fdc->read_signal(SIG_MB8877_DRIVEREG);
		irqreg_fdc = 0x00; //0b00000000;
	}
	//fdc_motor = (fdc->read_signal(SIG_MB8877_MOTOR) != 0);
	fdc_motor = false;
	
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20EX)
	if(connect_fdc) {
		fdc_reg_fd1e = 0x80;
		for(int i = 0; i < 4; i++) {
			fdc_drive_table[i] = (uint8_t)i;
			fdc->set_drive_type(i, DRIVE_TYPE_2D);
		}
	}
#else
	if(connect_fdc) {
		for(int i = 0; i < 4; i++) {
			fdc->set_drive_type(i, DRIVE_TYPE_2D);
		}
	}
#endif	
	if(event_fdc_motor >= 0) cancel_event(this, event_fdc_motor);
	event_fdc_motor = -1;
	irqstat_fdc = false;
	irqmask_mfd = true;
	if(connect_fdc) {
		set_fdc_motor(fdc_motor);
	}
}
	

/* FDD */

void FM7_MAINIO::set_fdc_cmd(uint8_t val)
{
	if(!connect_fdc) return;
	//irqreg_fdc = 0x00;
	fdc_cmdreg = val;
#if defined(HAS_DMA)
	if(((fdc_cmdreg >= 0x80) && (fdc_cmdreg < 0xd0)) || (fdc_cmdreg >= 0xe0)) {
		uint32_t words = dmac->read_signal(HD6844_WORDS_REG_0);
		if((words != 0) && (words < 0xffff) && (dmac->read_signal(HD6844_IS_TRANSFER_0) == 0)) {
			dmac->write_signal(HD6844_SRC_FIXED_ADDR_CH0, 3, 0xffffffff);
			dmac->write_signal(HD6844_TRANSFER_START, 0, 0xffffffff);
			//this->out_debug_log(_T("FDC: Start DMA CMDREG=%02x CHRN=%02x %02x %02x * DRVSEL=%08x\n"),
			//					 fdc_cmdreg, fdc_trackreg, fdc_headreg & 0x01, fdc_sectreg, fdc_drvsel);
		}
	}
#endif	
	fdc->write_io8(0, val & 0x00ff);
//#ifdef _FM7_FDC_DEBUG	
	if(config.special_debug_fdc) out_debug_log(_T("FDC: CMD: $%02x"), fdc_cmdreg);
//#endif	
}

uint8_t FM7_MAINIO::get_fdc_stat(void)
{
	//uint32_t stat_backup = fdc_statreg;
	if(!connect_fdc) return 0xff;
	fdc_statreg =  fdc->read_io8(0);
#ifdef _FM7_FDC_DEBUG	
	if(stat_backup != fdc_statreg) this->out_debug_log(_T("FDC: \nGet Stat(not busy): $%02x"), fdc_statreg);
#endif	
	return fdc_statreg;
}

void FM7_MAINIO::set_fdc_track(uint8_t val)
{
	if(!connect_fdc) return;
	// if mode is 2DD and type-of-image = 2D then val >>= 1;
	
	fdc_trackreg = val;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20EX)
	uint32_t d;
	if((fdc_drvsel & 0x40) == 0) {
		d = fdc_drive_table[fdc_drvsel & 0x03] & 0x03;
	} else {
		d = fdc_drvsel & 0x03;
	}
#if 1
	DISK *disk = fdc->get_disk_handler(d);
   	if(disk->media_type != MEDIA_TYPE_2D){
		if(config.special_debug_fdc)  out_debug_log(_T("NOTE: MEDIA TYPE IS 2DD"));
		//if(disk->drive_type == DRIVE_TYPE_2D) val <<= 1;
	} else { 
		if(config.special_debug_fdc)  out_debug_log(_T("NOTE: MEDIA TYPE IS 2D"));
		//if(disk->drive_type != DRIVE_TYPE_2D) val >>= 1;
	}
#endif
#endif	
	fdc->write_io8(1, val);
//#ifdef _FM7_FDC_DEBUG	
	if(config.special_debug_fdc)  out_debug_log(_T("FDC : Set Track: %d"), val);
//#endif	
}

uint8_t FM7_MAINIO::get_fdc_track(void)
{
	if(!connect_fdc) return 0xff;
	fdc_trackreg = fdc->read_io8(1);
	return fdc_trackreg;
}

void FM7_MAINIO::set_fdc_sector(uint8_t val)
{
	if(!connect_fdc) return;
	fdc_sectreg = val;
	fdc->write_io8(2, val);
//#ifdef _FM7_FDC_DEBUG	
	if(config.special_debug_fdc)  out_debug_log(_T("FDC: Set Sector: $%02x"), val);
//#endif	
}

uint8_t FM7_MAINIO::get_fdc_sector(void)
{
	if(!connect_fdc) return 0xff;
	fdc_sectreg = fdc->read_io8(2);
	return fdc_sectreg;
}
  
void FM7_MAINIO::set_fdc_data(uint8_t val)
{
	if(!connect_fdc) return;
	fdc_datareg = val;
	fdc->write_io8(3, val & 0x00ff);
}

uint8_t FM7_MAINIO::get_fdc_data(void)
{
	if(!connect_fdc) return 0xff;
	fdc_datareg = fdc->read_io8(3);
	
	return fdc_datareg;
}

uint8_t FM7_MAINIO::get_fdc_motor(void)
{
	uint8_t val = 0x3c; //0b01111100;
	uint8_t drv;
	bool bv;
	
	if(!connect_fdc) return 0xff;
	fdc_motor = (fdc->read_signal(SIG_MB8877_MOTOR) != 0) ? true : false;
	//fdc_drvsel = fdc->read_signal(SIG_MB8877_READ_DRIVE_REG);
	drv = fdc_drvsel & 0x03;
	val = val | (fdc_drvsel & 0x03);
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20EX)
	val = val | (fdc_drvsel & 0x40);
	if((fdc_drvsel & 0x40) != 0) {
		drv = fdc_drive_table[drv & 0x03];
	}
#endif	
	fdc_motor = fdc_motor & (fdc->get_drive_type(drv) != DRIVE_TYPE_UNK);
	if(fdc_motor) val |= 0x80;
//#ifdef _FM7_FDC_DEBUG	
	if(config.special_debug_fdc)  out_debug_log(_T("FDC: Get motor/Drive: $%02x"), val);
//#endif	
	return val;
}
  
void FM7_MAINIO::set_fdc_fd1c(uint8_t val)
{
	if(!connect_fdc) return;
	fdc_headreg = (val & 0x01) | 0xfe;
	fdc->write_signal(SIG_MB8877_SIDEREG, val, 0x01);
//#ifdef _FM7_FDC_DEBUG	
	if(config.special_debug_fdc)  out_debug_log(_T("FDC: Set side/head: $%02x"), val);
//#endif	
}

uint8_t FM7_MAINIO::get_fdc_fd1c(void)
{
	if(!connect_fdc) return 0xff;
	//fdc_headreg = fdc->read_signal(SIG_MB8877_SIDEREG);
	return fdc_headreg;
}

void FM7_MAINIO::set_fdc_fd1d(uint8_t val)
{
	bool backup_motor = fdc_motor;
	bool f;
	if(!connect_fdc) return;
	if((val & 0x80) != 0) {
		f = true;
	} else {
		f = false;
	}

#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20EX)
	if((val & 0x40) == 0) {
		fdc->write_signal(SIG_MB8877_DRIVEREG, fdc_drive_table[val & 0x03], 0x03);
	} else {
		fdc->write_signal(SIG_MB8877_DRIVEREG, val, 0x03);
	}
	fdc_drvsel = val;
#else
	fdc->write_signal(SIG_MB8877_DRIVEREG, val, 0x03);
	fdc_drvsel = val;
#endif	

	if(f != backup_motor) {
		if(event_fdc_motor >= 0) cancel_event(this, event_fdc_motor);
		if(f) {
			register_event(this, EVENT_FD_MOTOR_ON, 1000.0 * 300.0, false, &event_fdc_motor); // Motor ON After 0.3Sec.
		} else {
			register_event(this, EVENT_FD_MOTOR_OFF, 1000.0 * 50.0, false, &event_fdc_motor); // Motor OFF After 0.05Sec.
		}
	}
//#ifdef _FM7_FDC_DEBUG	
	if(config.special_debug_fdc)  out_debug_log(_T("FDC: Set Drive Select: $%02x"), val);
//#endif	
}

uint8_t FM7_MAINIO::get_fdc_fd1e(void)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20EX)
	uint8_t val = 0xa0;
	val |= (fdc_reg_fd1e & 0x5f);
	if(config.special_debug_fdc) out_debug_log(_T("FDC: GET $FD1E value=$%02x"), val);
	return val;
#else
	//if(config.special_debug_fdc) out_debug_log(_T("FDC: GET $FD1E value=$%02x"), val);
	return 0xff;
#endif
}	

void FM7_MAINIO::set_fdc_fd1e(uint8_t val)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20EX)
	uint8_t drive;
	if(!connect_fdc) return;
	
	fdc_reg_fd1e = val;
	
	if((val & 0x10) != 0) {
		fdc_drive_table[(val & 0x0c) >> 2] = val & 0x03;
		if(((val & 0x0c) >> 2) == (fdc_drvsel & 0x03)) {
			fdc->write_signal(SIG_MB8877_DRIVEREG, fdc_drive_table[(val & 0x0c) >> 2], 0x03);
		}
	}
	if((val & 0x40) != 0) {
		for(drive = 0; drive < MAX_DRIVE; drive++) fdc->set_drive_type(drive, DRIVE_TYPE_2D);
		if(config.special_debug_fdc) out_debug_log(_T("SET DRIVE to 2D"));
	} else {
		for(drive = 0; drive < MAX_DRIVE; drive++) fdc->set_drive_type(drive, DRIVE_TYPE_2DD);
		if(config.special_debug_fdc) out_debug_log(_T("SET DRIVE to 2DD"));
	}
	if(config.special_debug_fdc) out_debug_log(_T("FDC: SET $FD1E value=$%02x"), val);
#endif	
}

void FM7_MAINIO::set_irq_mfd(bool flag)
{
	bool backup = irqstat_fdc;
	if(!connect_fdc) return;
	if(flag) {
		irqreg_fdc |= 0x40; //0b01000000;
	} else {
		irqreg_fdc &= 0xbf; //0b10111111;
	}
#if defined(_FM77_VARIANTS)
	flag = flag & !(intmode_fdc);
#endif
//#if !defined(_FM8) // With FM8, $FD1F is alive and not do_irq(), Thanks to Anna_Wu.
	irqstat_fdc = flag & !irqmask_mfd;
	if(backup != irqstat_fdc) do_irq();
//#endif
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
#if defined(HAS_DMA)
	if((dmac->read_signal(HD6844_IS_TRANSFER_0) != 0) && (flag)) {
		dmac->write_signal(HD6844_DO_TRANSFER, 0x0, 0xffffffff);
	}
#endif	
	return;
}

uint8_t FM7_MAINIO::fdc_getdrqirq(void)
{
	uint8_t val = irqreg_fdc | 0x3f;
	if(!connect_fdc) return 0xff;
	return val;
}

void FM7_MAINIO::set_fdc_motor(bool flag)
{
	if(!connect_fdc) return;
	uint8_t val;
	fdc->write_signal(SIG_MB8877_MOTOR, flag ? 0x01 : 0x00, 0x01);
	val = fdc_drvsel & 0x03;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20EX)
	if((fdc_drvsel & 0x40) == 0) {
		val = fdc_drive_table[val & 0x03];
	}
#endif
	fdc_motor = (fdc->read_signal(SIG_MB8877_MOTOR) != 0);
	fdc_motor = fdc_motor & (fdc->get_drive_type(val) != DRIVE_TYPE_UNK);
	if(config.special_debug_fdc) out_debug_log(_T("FDC: MOTOR=%d VAL=$%02x"), flag, val);
}

