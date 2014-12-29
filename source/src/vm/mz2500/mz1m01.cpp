/*
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2013.03.30-

	[ MZ-1M01 (16bit Board) ]
*/

#include "mz1m01.h"
#include "../z80pio.h"
#include "../../fileio.h"

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

void MZ1M01::initialize()
{
	memset(ram, 0, sizeof(ram));
	memset(ipl, 0xff, sizeof(ipl));
	memset(kanji, 0xff, sizeof(kanji));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("MZ-1M01.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	} else {
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
	}
	if(fio->Fopen(emu->bios_path(_T("MZ-1R08.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, sizeof(kanji), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK(0x00000, 0x1ffff, ram, ram);
	SET_BANK(0x20000, 0x3ffff, wdmy, rdmy);
	SET_BANK(0x40000, 0x5ffff, wdmy, kanji);
	SET_BANK(0x60000, 0xfdfff, wdmy, rdmy);
	SET_BANK(0xfe000, 0xfffff, wdmy, ipl);
}

void MZ1M01::reset()
{
	port[0] = port[1] = 0xff;
}

void MZ1M01::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xfffff;
	wbank[addr >> 13][addr & 0x1fff] = data;
}

uint32 MZ1M01::read_data8(uint32 addr)
{
	addr &= 0xfffff;
	return rbank[addr >> 13][addr & 0x1fff];
}

void MZ1M01::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0x7c:
	case 0x7d:
		d_pic->write_io8(addr, data);
		break;
	case 0x7e:
		d_pio->write_signal(SIG_Z80PIO_PORT_A, data, 0xff);
		d_pio->write_signal(SIG_Z80PIO_STROBE_B, 1, 1);
		break;
	case 0x7f:
		d_pio->write_signal(SIG_Z80PIO_PORT_B, data, 0x7f);
		port[1] = (port[1] & 0x80) | (data & 0x7f);
		break;
	}
}

uint32 MZ1M01::read_io8(uint32 addr)
{
	switch(addr & 0xff) {
	case 0x7c:
	case 0x7d:
		return d_pic->read_io8(addr);
	case 0x7e:
		d_pio->write_signal(SIG_Z80PIO_STROBE_A, 1, 1);
	case 0x7f:
		return port[addr & 1];
	}
	return 0xff;
}

void MZ1M01::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_MZ1M01_PORT_A) {
		port[0] = (port[0] & ~mask) | (data & mask);
	} else if(id == SIG_MZ1M01_PORT_B) {
		port[1] = (port[1] & ~mask) | (data & mask);
	}
}

#define STATE_VERSION	1

void MZ1M01::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
	state_fio->Fwrite(port, sizeof(port), 1);
}

bool MZ1M01::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(ram, sizeof(ram), 1);
	state_fio->Fread(port, sizeof(port), 1);
	return true;
}

