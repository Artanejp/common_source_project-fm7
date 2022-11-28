/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2004.09.10 -

	[ MZ-1E30 (SASI I/F) ]
*/

#include "mz1e30.h"

#define STATUS_INT	0x00	// unknown
#define STATUS_IXO	0x04
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
	if(fio->Fopen(create_local_path(_T("MZ-1E30.ROM")), FILEIO_READ_BINARY) ||
	   fio->Fopen(create_local_path(_T("SASI.ROM")), FILEIO_READ_BINARY) ||
	   fio->Fopen(create_local_path(_T("FILE.ROM")), FILEIO_READ_BINARY)) {
		fio->Fseek(0, FILEIO_SEEK_END);
		if((rom_size = fio->Ftell()) > 0x1000000) {
			rom_size = 0x1000000;
		}
		rom_buffer = (uint8_t*)malloc(rom_size);
		
		fio->Fseek(0, FILEIO_SEEK_SET);
		fio->Fread(rom_buffer, rom_size, 1);
		fio->Fclose();
	} else {
		rom_size = 0;
		rom_buffer = (uint8_t*)malloc(1);
	}
	delete fio;
	rom_address = 0;
}

void MZ1E30::release()
{
	if(rom_buffer != NULL) {
		free(rom_buffer);
	}
}

void MZ1E30::reset()
{
	irq_status = drq_status = false;
}

void MZ1E30::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xa4:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[MZ1E30] out %04X %02X\n"), addr, data);
		#endif
		d_host->write_dma_io8(addr, data);
		break;
	case 0xa5:
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[MZ1E30] out %04X %02X\n"), addr, data);
		#endif
		d_host->write_signal(SIG_SCSI_RST, data, 0x40);
		d_host->write_signal(SIG_SCSI_SEL, data, 0x20);
		break;
	case 0xa8:
		// rom file
		rom_address = ((addr & 0xff00) << 8) | (data << 8) | (rom_address & 0x0000ff);
		break;
	}
}

uint32_t MZ1E30::read_io8(uint32_t addr)
{
	uint32_t val = 0;
	
	switch(addr & 0xff) {
	case 0xa4:
		val = d_host->read_dma_io8(addr);
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[MZ1E30] in  %04X %02X\n"), addr, value);
		#endif
		return val;
	case 0xa5:
		val = (d_host->read_signal(SIG_SCSI_REQ) ? STATUS_REQ : 0) |
//		      (d_host->read_signal(SIG_SCSI_ACK) ? STATUS_ACK : 0) |
		      (d_host->read_signal(SIG_SCSI_BSY) ? STATUS_BSY : 0) |
		      (d_host->read_signal(SIG_SCSI_MSG) ? STATUS_MSG : 0) |
		      (d_host->read_signal(SIG_SCSI_CD ) ? STATUS_CXD : 0) |
		      (d_host->read_signal(SIG_SCSI_IO ) ? STATUS_IXO : 0) |
		      (irq_status                        ? STATUS_INT : 0);
		irq_status = false;
		#ifdef _SCSI_DEBUG_LOG
			this->out_debug_log(_T("[MZ1E30] in  %04X %02X (REQ=%d,BSY=%d,MSG=%d,CxD=%d,IxO=%d)\n"), addr, val,
				(val & STATUS_REQ) ? 1 : 0,
				(val & STATUS_BSY) ? 1 : 0,
				(val & STATUS_MSG) ? 1 : 0,
				(val & STATUS_CXD) ? 1 : 0,
				(val & STATUS_IXO) ? 1 : 0);
		#endif
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

void MZ1E30::write_dma_io8(uint32_t addr, uint32_t data)
{
	write_io8(0xa4, data);
}

uint32_t MZ1E30::read_dma_io8(uint32_t addr)
{
	return read_io8(0xa4);
}

void MZ1E30::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_MZ1E30_IRQ:
		irq_status = ((data & mask) != 0);
		break;
	case SIG_MZ1E30_DRQ:
		drq_status = ((data & mask) != 0);
		break;
	}
}

#define STATE_VERSION	2

bool MZ1E30::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(rom_address);
	state_fio->StateValue(irq_status);
	state_fio->StateValue(drq_status);
	return true;
}

