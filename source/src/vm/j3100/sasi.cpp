/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.19-

	[ sasi hdd ]
*/

#include "sasi.h"
#include "../i8259.h"

#define PHASE_FREE	0
#define PHASE_SELECT	1
#define PHASE_COMMAND	2
#define PHASE_C2	3
#define PHASE_SENSE	4
#define PHASE_READ	5
#define PHASE_WRITE	6
#define PHASE_STATUS	7
//#define PHASE_MESSAGE	8

#define STATUS_REQ	0x01
#define STATUS_IXD	0x02
#define STATUS_CXD	0x04
#define STATUS_BSY	0x08
#define STATUS_DRQ	0x10
#define STATUS_IRQ	0x20
//#define STATUS_MSG	0

#define EVENT_COMMAND	0
#define EVENT_STATUS	1

void SASI::initialize()
{
	// open hard drive images
	for(int i = 0; i < 2; i++) {
		drive[i].fio = new FILEIO();
		if(!drive[i].fio->Fopen(create_local_path(_T("HDD%d.DAT"), i + 1), FILEIO_READ_WRITE_BINARY)) {
			delete drive[i].fio;
			drive[i].fio = NULL;
		}
		drive[i].access = false;
	}
}

void SASI::release()
{
	for(int i = 0; i < 2; i++) {
		if(drive[i].fio != NULL) {
			drive[i].fio->Fclose();
			delete drive[i].fio;
			drive[i].fio = NULL;
		}
	}
}

void SASI::reset()
{
	memset(buffer, 0, sizeof(buffer));
	memset(cmd, 0, sizeof(cmd));
	memset(status_buf, 0, sizeof(status_buf));
	
	phase = PHASE_FREE;
	sector = 0;
	blocks = 0;
	cmd_ptr = 0;
	unit = 0;
	buffer_ptr = 0;
	status = 0;
	status_irq_drq = 0;
	error = 0;
	status_ptr = 0;
	maskreg = 0;
}

void SASI::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x1f0:
		// data
		if(phase == PHASE_COMMAND) {
			cmd[cmd_ptr++] = data;
			if(cmd_ptr == 6) {
				check_cmd();
			}
		} else if(phase == PHASE_C2) {
			if(++status_ptr == 10) {
				set_status(0);
			}
		} else if(phase == PHASE_WRITE) {
			buffer[buffer_ptr++] = data;
			if(buffer_ptr == 256) {
				flush(unit);
				if(--blocks) {
					sector++;
					buffer_ptr = 0;
					if(!seek(unit)) {
						set_status(0x0f);
						set_drq(false);
					}
				} else {
					set_status(0);
					set_drq(false);
				}
			}
		}
		datareg = data;
		break;
	case 0x1f1:
		// reset
		reset();
		break;
	case 0x1f2:
		// select
		phase = PHASE_SELECT;
		register_event(this, EVENT_COMMAND, 10, false, NULL);
		break;
	case 0x1f3:
		// mask
		maskreg = data;
		break;
	}
}

uint32_t SASI::read_io8(uint32_t addr)
{
	uint32_t val = 0;
	
	switch(addr) {
	case 0x1f0:
		// data
		if(phase == PHASE_READ) {
			val = buffer[buffer_ptr++];
			if(buffer_ptr == 256) {
				if(--blocks) {
					sector++;
					buffer_ptr = 0;
					if(!seek(unit)) {
						set_status(0x0f);
						set_drq(false);
					}
				} else {
					set_status(0);
					set_drq(false);
				}
			}
		} else if(phase == PHASE_SENSE) {
			val = status_buf[status_ptr++];
			if(status_ptr == 4) {
				set_status(0);
			}
		} else if(phase == PHASE_STATUS) {
//			val = error ? 0x02 : status;
//			phase = PHASE_MESSAGE;
			val = (error ? 2 : 0) | (unit << 5);
			phase = PHASE_FREE;
//		} else if(phase == PHASE_MESSAGE) {
//			phase = PHASE_FREE;
		}
		return val;
	case 0x1f1:
		// status
		val = status_irq_drq;
		status_irq_drq &= ~STATUS_IRQ;
		if(phase != PHASE_FREE) {
			val |= STATUS_BSY;
		}
		if(phase > PHASE_SELECT) {
			val |= STATUS_REQ;
		}
		if(phase == PHASE_COMMAND) {
			val |= STATUS_CXD;
		}
		if(phase == PHASE_SENSE) {
			val |= STATUS_IXD;
		}
		if(phase == PHASE_READ) {
			val |= STATUS_IXD;
		}
		if(phase == PHASE_STATUS) {
			val |= STATUS_IXD | STATUS_CXD;
		}
//		if(phase == PHASE_MESSAGE) {
//			val |= STATUS_IXD | STATUS_CXD | STATUS_MSG;
//		}
		return val;
	}
	return 0xff;
}

void SASI::write_dma_io8(uint32_t addr, uint32_t data)
{
	write_io8(0x1f0, data);
}

uint32_t SASI::read_dma_io8(uint32_t addr)
{
	return read_io8(0x1f0);
}

uint32_t SASI::read_signal(int ch)
{
	// get access status
	uint32_t stat = (drive[0].access ? 0x10 : 0) | (drive[1].access ? 0x20 : 0);
	drive[0].access = drive[1].access = false;
	return stat;
}

void SASI::event_callback(int event_id, int err)
{
	if(event_id == EVENT_COMMAND) {
		phase = PHASE_COMMAND;
		cmd_ptr = 0;
	} else if(event_id == EVENT_STATUS) {
		phase = PHASE_STATUS;
		// raise irq
		if(maskreg & 2) {
#ifdef TYPE_SL
			d_pic->write_signal(SIG_I8259_IR5 | SIG_I8259_CHIP0, 1, 1);
#else
			d_pic->write_signal(SIG_I8259_IR6 | SIG_I8259_CHIP1, 1, 1);
#endif
		}
		status_irq_drq |= STATUS_IRQ;
	}
}

void SASI::check_cmd()
{
	unit = (cmd[1] >> 5) & 1;
	
	switch(cmd[0]) {
	case 0x00:
		// test drive ready
		if(drive[unit].fio != NULL) {
			status = 0x00;
			set_status(0x00);
		} else {
			status = 0x02;
			set_status(0x7f);
		}
		break;
	case 0x01:
		// recalib
		if(drive[unit].fio != NULL) {
			sector = 0;
			status = 0x00;
			set_status(0x00);
		} else {
			status = 0x02;
			set_status(0x7f);
		}
		break;
	case 0x03:
		// request sense status
		phase = PHASE_SENSE;
		status_buf[0] = error;
		status_buf[1] = (uint8_t)((unit << 5) | ((sector >> 16) & 0x1f));
		status_buf[2] = (uint8_t)(sector >> 8);
		status_buf[3] = (uint8_t)sector;
		error = 0;
		status = 0x00;
		status_ptr = 0;
		break;
	case 0x04:
		// format drive
		sector = 0;
		status = 0x00;
		set_status(0x0f);
		break;
	case 0x06:
		// format track
		sector = cmd[1] & 0x1f;
		sector = (sector << 8) | cmd[2];
		sector = (sector << 8) | cmd[3];
		blocks = cmd[4];
		status = 0;
		if(format(unit)) {
			set_status(0);
		} else {
			set_status(0x0f);
		}
		break;
	case 0x08:
		// read data
		sector = cmd[1] & 0x1f;
		sector = (sector << 8) | cmd[2];
		sector = (sector << 8) | cmd[3];
		blocks = cmd[4];
		status = 0;
		if(blocks != 0 && seek(unit)) {
			phase = PHASE_READ;
			buffer_ptr = 0;
			set_drq(true);
		} else {
			set_status(0x0f);
		}
		break;
	case 0x0a:
		sector = cmd[1] & 0x1f;
		sector = (sector << 8) | cmd[2];
		sector = (sector << 8) | cmd[3];
		blocks = cmd[4];
		status = 0;
		if(blocks != 0 && seek(unit)) {
			phase = PHASE_WRITE;
			buffer_ptr = 0;
			memset(buffer, 0, sizeof(buffer));
			set_drq(true);
		} else {
			set_status(0x0f);
		}
		break;
	case 0x0b:
		sector = cmd[1] & 0x1f;
		sector = (sector << 8) | cmd[2];
		sector = (sector << 8) | cmd[3];
		blocks = cmd[4];
		status = 0;
		set_status(0);
		break;
	case 0xc2:
		phase = PHASE_C2;
		status_ptr = 0;
		status = 0;
//		error = 0;
		break;
	default:
		// unknown
		set_status(0);
		break;
	}
}

void SASI::set_status(uint8_t err)
{
	error = err;
	register_event(this, EVENT_STATUS, 10, false, NULL);
}

void SASI::set_drq(bool flag)
{
	if(flag) {
		status_irq_drq |= STATUS_DRQ;
	} else {
		status_irq_drq &= ~STATUS_DRQ;
	}
}

bool SASI::seek(int drv)
{
	memset(buffer, 0, sizeof(buffer));
	
	if(drive[drv & 1].fio == NULL) {
		return false;
	}
	if(drive[drv & 1].fio->Fseek(sector * 256, FILEIO_SEEK_SET) != 0) {
		return false;
	}
	if(drive[drv & 1].fio->Fread(buffer, 256, 1) != 1) {
		return false;
	}
	drive[drv & 1].access = true;
	return true;
}

bool SASI::flush(int drv)
{
	if(drive[drv & 1].fio == NULL) {
		return false;
	}
	if(drive[drv & 1].fio->Fseek(sector * 256, FILEIO_SEEK_SET) != 0) {
		return false;
	}
	if(drive[drv & 1].fio->Fwrite(buffer, 256, 1) != 1) {
		return false;
	}
	drive[drv & 1].access = true;
	return true;
}

bool SASI::format(int drv)
{
	if(drive[drv & 1].fio == NULL) {
		return false;
	}
	if(drive[drv & 1].fio->Fseek(sector * 256, FILEIO_SEEK_SET) != 0) {
		return false;
	}
	// format 33 blocks
	memset(buffer, 0, sizeof(buffer));
	for(int i = 0; i < 33; i++) {
		if(drive[drv & 1].fio->Fwrite(buffer, 256, 1) != 1) {
			return false;
		}
		drive[drv & 1].access = true;
	}
	return true;
}

