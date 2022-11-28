/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2009.03.14-

	[ memory ]
*/

#include "memory.h"
#ifdef _X1TURBO_FEATURE
#include "../i8255.h"
#else
#include "../z80.h"
#endif

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		wbank[i] = (w) + 0x1000 * (i - sb); \
		rbank[i] = (r) + 0x1000 * (i - sb); \
	} \
}

void MEMORY::initialize()
{
	// init memory
	memset(rom, 0xff, sizeof(rom));
	memset(ram, 0, sizeof(ram));
	
	// load ipl
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(IPL_ROM_FILE_NAME), FILEIO_READ_BINARY)) {
		// xmillenium rom
		fio->Fread(rom, IPL_ROM_FILE_SIZE, 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, IPL_ROM_FILE_SIZE, 1);
		fio->Fclose();
	}
	delete fio;
#ifndef _X1TURBO_FEATURE
	for(int ofs = 0x1000; ofs < 0x8000; ofs += 0x1000) {
		memcpy(rom + ofs, rom, 0x1000);
	}
#endif
}

void MEMORY::reset()
{
	SET_BANK(0x0000, 0x7fff, ram, rom);
	SET_BANK(0x8000, 0xffff, ram + 0x8000, ram + 0x8000);
	romsel = 1;
#ifdef _X1TURBO_FEATURE
	bank = 0x10;
	d_pio->write_signal(SIG_I8255_PORT_B, 0x00, 0x10);
#else
	m1_cycle = 1;
#endif
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return rbank[addr >> 12][addr & 0xfff];
}

#ifndef _X1TURBO_FEATURE
uint32_t MEMORY::fetch_op(uint32_t addr, int *wait)
{
	*wait = m1_cycle;
	return read_data8(addr);
}
#endif

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	bool update_map_required = false;
	
	switch(addr & 0xff00) {
#ifdef _X1TURBO_FEATURE
	case 0xb00:
		if((bank & 0x1f) != (data & 0x1f)) {
			update_map_required = true;
		}
		bank = data;
		break;
#endif
	case 0x1d00:
		if(!romsel) {
			romsel = 1;
			update_map_required = true;
#ifdef _X1TURBO_FEATURE
			d_pio->write_signal(SIG_I8255_PORT_B, 0x00, 0x10);
#else
			m1_cycle = 1;
#endif
		}
		break;
	case 0x1e00:
		if(romsel) {
			romsel = 0;
			update_map_required = true;
#ifdef _X1TURBO_FEATURE
			d_pio->write_signal(SIG_I8255_PORT_B, 0x10, 0x10);
#else
			m1_cycle = 0;
#endif
		}
		break;
	}
	if(update_map_required) {
		update_map();
	}
}

uint32_t MEMORY::read_io8(uint32_t addr)
{
	switch(addr & 0xff00) {
	case 0x1e00: // thanks Mr.Sato
		if(romsel) {
			romsel = 0;
#ifdef _X1TURBO_FEATURE
			d_pio->write_signal(SIG_I8255_PORT_B, 0x10, 0x10);
#else
			m1_cycle = 0;
#endif
			update_map();
		}
		break;
#ifdef _X1TURBO_FEATURE
	case 0xb00:
		return bank;
#endif
	}
	return 0xff;
}

void MEMORY::update_map()
{
#ifdef _X1TURBO_FEATURE
	if(!(bank & 0x10)) {
		uint8_t *ptr = extram + 0x8000 * (bank & 0x0f);
		SET_BANK(0x0000, 0x7fff, ptr, ptr);
	} else
#endif
	if(romsel) {
		SET_BANK(0x0000, 0x7fff, ram, rom);
	} else {
		SET_BANK(0x0000, 0x7fff, ram, ram);
	}
}

#define STATE_VERSION	1

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateValue(romsel);
#ifdef _X1TURBO_FEATURE
	state_fio->StateArray(extram, sizeof(extram), 1);
	state_fio->StateValue(bank);
#else
	state_fio->StateValue(m1_cycle);
#endif
	
	// post process
	if(loading) {
		update_map();
	}
	return true;
}

