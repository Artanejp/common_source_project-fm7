/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2011.12.18-

	[ PC-80S31K ]
*/

#include "pc80s31k.h"
#include "disk.h"
#include "upd765a.h"

//#define _DEBUG_PC80S31K

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 13, eb = (e) >> 13; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x2000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x2000 * (i - sb); \
		} \
	} \
}

void PC80S31K::initialize()
{
	// init memory
	memset(rom, 0xff, sizeof(rom));
	memset(ram, 0, sizeof(ram));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("PC88.ROM")), FILEIO_READ_BINARY)) {
		fio->Fseek(0x14000, FILEIO_SEEK_CUR);
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("DISK.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	} else {
		// stop cpu
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
	}
	delete fio;
	
	// init memory map
	SET_BANK(0x0000, 0x1fff, wdmy, rom);
	SET_BANK(0x2000, 0x3fff, wdmy, rdmy);
	SET_BANK(0x4000, 0x7fff, ram, ram);
	SET_BANK(0x8000, 0xffff, wdmy, rdmy);
	
	// XM8 version 1.20
	// both drives always set force ready signal
	d_fdc->write_signal(SIG_UPD765A_FREADY, 1, 1);
}

void PC80S31K::reset()
{
	d_fdc->set_drive_type(0, DRIVE_TYPE_2D);
	d_fdc->set_drive_type(1, DRIVE_TYPE_2D);
	
	// clear output
	d_pio->write_io8(1, 0);
	d_pio->write_io8(2, 0);
}

uint32_t PC80S31K::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return rbank[addr >> 13][addr & 0x1fff];
}

uint32_t PC80S31K::fetch_op(uint32_t addr, int *wait)
{
	addr &= 0xffff;
#ifdef PC80S31K_NO_WAIT
	// XM8 version 1.20
	// no access wait (both ROM and RAM)
	*wait = 0;
#else
	*wait = (addr < 0x2000) ? 1 : 0;
#endif
	return rbank[addr >> 13][addr & 0x1fff];
}

void PC80S31K::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	if(addr == 0x7f15 && data == 0x1f && d_cpu->get_pc() < 0x2000) {
		// ugly patch to enable both #1 and #2 drives
		data = 0x3f;
	}
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32_t PC80S31K::read_io8(uint32_t addr)
{
	uint32_t val;
	
	switch(addr & 0xff) {
	case 0xf8:
		d_fdc->write_signal(SIG_UPD765A_TC, 1, 1);
		break;
	case 0xfa:
	case 0xfb:
		return d_fdc->read_io8(addr & 1);
	case 0xfc:
	case 0xfd:
		val = d_pio->read_io8(addr & 3);
#ifdef _DEBUG_PC80S31K
		this->out_debug_log(_T("SUB\tIN RECV(%d)=%2x\n"), addr & 3, val);
#endif
		return val;
	case 0xfe:
		val = d_pio->read_io8(addr & 3);
#ifdef _DEBUG_PC80S31K
		{
			static uint32_t prev = -1;
			if(prev != val) {
//				this->out_debug_log(_T("SUB\tIN DAV=%d,RFD=%d,DAC=%d,ATN=%d\n"), val&1, (val>>1)&1, (val>>2)&1, (val>>3)&1);
				prev = val;
			}
		}
#endif
		return val;
	}
	return 0xff;
}

void PC80S31K::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xf4:
		// XM8 version 1.20
		// MR/MH/MA/MA2/MA... type ROM only
		if(rom[0x7ee] != 0xfe) {
			break;
		}
		for(int drv = 0; drv < 2; drv++) {
			uint32_t mode = data >> drv;
			if(mode & 1) {
				d_fdc->set_drive_type(drv, DRIVE_TYPE_2HD);
			} else if(mode & 4) {
				d_fdc->set_drive_type(drv, DRIVE_TYPE_2DD);
			} else {
				d_fdc->set_drive_type(drv, DRIVE_TYPE_2D);
			}
		}
		break;
	case 0xf7:
		// external printer port
		break;
	case 0xf8:
		// TODO: we need to update uPD765A to let the motor of each drive on/off
		break;
	case 0xfb:
		d_fdc->write_io8(addr & 1, data);
		break;
	case 0xfc:
	case 0xfd:
#ifdef _DEBUG_PC80S31K
		this->out_debug_log(_T("SUB\tOUT SEND(%d)=%2x\n"), addr & 3, data);
#endif
		d_pio->write_io8(addr & 3, data);
		break;
	case 0xfe:
//		this->out_debug_log(_T("SUB\tOUT DAV=%d,RFD=%d,DAC=%d,ATN=%d\n"), (data>>4)&1, (data>>5)&1, (data>>6)&1, (data>>7)&1);
		d_pio->write_io8(addr & 3, data);
		break;
	case 0xff:
		if(!(data & 0x80)) {
			int bit = (data >> 1) & 7;
			if(bit == 4) {
//				this->out_debug_log(_T("SUB\tOUT DAV=%d\n"), data & 1);
			} else if(bit == 5) {
//				this->out_debug_log(_T("SUB\tOUT RFD=%d\n"), data & 1);
			} else if(bit == 6) {
//				this->out_debug_log(_T("SUB\tOUT DAC=%d\n"), data & 1);
			} else if(bit == 7) {
//				this->out_debug_log(_T("SUB\tOUT ATN=%d\n"), data & 1);
			}
		}
		d_pio->write_io8(addr & 3, data);
		break;
	}
}

uint32_t PC80S31K::get_intr_ack()
{
	return 0;	// NOP
}

#define STATE_VERSION	1

bool PC80S31K::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}

