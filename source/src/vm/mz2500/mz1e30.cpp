/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2004.09.10 -

	[ MZ-1E30 (SASI) ]
*/

#include "mz1e30.h"
#include "../../fileio.h"

#define PHASE_FREE	0
#define PHASE_SELECT	1
#define PHASE_COMMAND	2
#define PHASE_C2	3
#define PHASE_SENSE	4
#define PHASE_READ	5
#define PHASE_WRITE	6
#define PHASE_STATUS	7
#define PHASE_MESSAGE	8

#define STATUS_IXD	0x04
#define STATUS_CXD	0x08
#define STATUS_MSG	0x10
#define STATUS_BSY	0x20
#define STATUS_REQ	0x80

#define STATUS_IRQ	0
#define STATUS_DRQ	0

void MZ1E30::initialize()
{
	// rom file
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("MZ-1E30.ROM")), FILEIO_READ_BINARY) ||
	   fio->Fopen(emu->bios_path(_T("SASI.ROM")), FILEIO_READ_BINARY) ||
	   fio->Fopen(emu->bios_path(_T("FILE.ROM")), FILEIO_READ_BINARY)) {
		fio->Fseek(0, FILEIO_SEEK_END);
		if((rom_size = fio->Ftell()) > 0x1000000) {
			rom_size = 0x1000000;
		}
		rom_buffer = (uint8*)malloc(rom_size);
		
		fio->Fseek(0, FILEIO_SEEK_SET);
		fio->Fread(rom_buffer, rom_size, 1);
		fio->Fclose();
	} else {
		rom_size = 0;
		rom_buffer = (uint8*)malloc(1);
	}
	delete fio;
	rom_address = 0;
	
	// open hard drive images
	for(int i = 0; i < 2; i++) {
		_TCHAR file_name[_MAX_PATH];
		_stprintf_s(file_name, _MAX_PATH, _T("HDD%d.DAT"), i + 1);
		
		drive[i].fio = new FILEIO();
		if(!drive[i].fio->Fopen(emu->bios_path(file_name), FILEIO_READ_WRITE_BINARY)) {
			delete drive[i].fio;
			drive[i].fio = NULL;
		}
		drive[i].access = false;
	}
	
	// initialize sasi interface
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
}

void MZ1E30::release()
{
	for(int i = 0; i < 2; i++) {
		if(drive[i].fio != NULL) {
			drive[i].fio->Fclose();
			delete drive[i].fio;
		}
	}
	if(rom_buffer != NULL) {
		free(rom_buffer);
	}
}

void MZ1E30::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0xa4:
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
	case 0xa5:
		// command
		if(data == 0x00) {
			if(phase == PHASE_SELECT) {
				phase = PHASE_COMMAND;
				cmd_ptr = 0;
			}
		} else if(data == 0x20) {
			if(datareg & 1) {
				phase = PHASE_SELECT;
			} else {
				phase = PHASE_FREE;
			}
		}
		break;
	case 0xa8:
		// rom file
		rom_address = ((addr & 0xff00) << 8) | (data << 8) | (rom_address & 0x0000ff);
		break;
	}
}

uint32 MZ1E30::read_io8(uint32 addr)
{
	uint32 val = 0;
	
	switch(addr & 0xff) {
	case 0xa4:
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
			val = error ? 0x02 : status;
			phase = PHASE_MESSAGE;
		} else if(phase == PHASE_MESSAGE) {
			phase = PHASE_FREE;
		}
		return val;
	case 0xa5:
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
		if(phase == PHASE_MESSAGE) {
			val |= STATUS_IXD | STATUS_CXD | STATUS_MSG;
		}
		return val;
	case 0xa9:
		// rom file
		rom_address = (rom_address & 0xffff00) | ((addr & 0xff00) >> 8);
		if(rom_address < rom_size) {
			return rom_buffer[rom_address];
		}
		break;
	}
	return 0xff;
}

void MZ1E30::write_dma_io8(uint32 addr, uint32 data)
{
	write_io8(0xa4, data);
}

uint32 MZ1E30::read_dma_io8(uint32 addr)
{
	return read_io8(0xa4);
}

uint32 MZ1E30::read_signal(int ch)
{
	// get access status
	uint32 stat = (drive[0].access ? 0x10 : 0) | (drive[1].access ? 0x20 : 0);
	drive[0].access = drive[1].access = false;
	return stat;
}

void MZ1E30::check_cmd()
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
		status_buf[1] = (uint8)((unit << 5) | ((sector >> 16) & 0x1f));
		status_buf[2] = (uint8)(sector >> 8);
		status_buf[3] = (uint8)sector;
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

void MZ1E30::set_status(uint8 err)
{
	error = err;
#if 1
	phase = PHASE_STATUS;
	// raise irq
	status_irq_drq |= STATUS_IRQ;
#else
	vm->register_event(this, 0, 10, false, NULL);
#endif
}

void MZ1E30::event_callback(int event_id, int err)
{
#if 0
	phase = PHASE_STATUS;
	// raise irq
	status_irq_drq |= STATUS_IRQ;
#endif
}

void MZ1E30::set_drq(bool flag)
{
	if(flag) {
		status_irq_drq |= STATUS_DRQ;
	} else {
		status_irq_drq &= ~STATUS_DRQ;
	}
}

bool MZ1E30::seek(int drv)
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

bool MZ1E30::flush(int drv)
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

bool MZ1E30::format(int drv)
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

#define STATE_VERSION	1

void MZ1E30::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint32(rom_address);
	state_fio->Fwrite(buffer, sizeof(buffer), 1);
	state_fio->FputInt32(phase);
	state_fio->FputInt32(sector);
	state_fio->FputInt32(blocks);
	state_fio->Fwrite(cmd, sizeof(cmd), 1);
	state_fio->FputInt32(cmd_ptr);
	state_fio->FputInt32(unit);
	state_fio->FputInt32(buffer_ptr);
	state_fio->FputUint8(status);
	state_fio->FputUint8(status_irq_drq);
	state_fio->FputUint8(error);
	state_fio->Fwrite(status_buf, sizeof(status_buf), 1);
	state_fio->FputInt32(status_ptr);
	state_fio->FputUint8(datareg);
}

bool MZ1E30::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	rom_address = state_fio->FgetUint32();
	state_fio->Fread(buffer, sizeof(buffer), 1);
	phase = state_fio->FgetInt32();
	sector = state_fio->FgetInt32();
	blocks = state_fio->FgetInt32();
	state_fio->Fread(cmd, sizeof(cmd), 1);
	cmd_ptr = state_fio->FgetInt32();
	unit = state_fio->FgetInt32();
	buffer_ptr = state_fio->FgetInt32();
	status = state_fio->FgetUint8();
	status_irq_drq = state_fio->FgetUint8();
	error = state_fio->FgetUint8();
	state_fio->Fread(status_buf, sizeof(status_buf), 1);
	status_ptr = state_fio->FgetInt32();
	datareg = state_fio->FgetUint8();
	return true;
}

