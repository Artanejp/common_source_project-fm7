/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'

	Author : Takeda.Toshiya
	Date   : 2006.12.28 -

	[ memory ]
*/

#include "memory.h"
#include "../i8255.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x1000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x1000 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	// load ipl
	memset(rdmy, 0xff, sizeof(rdmy));
	load_ipl();
	
	// init memory map
	SET_BANK(0x0000, 0x7fff, ram + 0x0000, rom + 0x0000);
	SET_BANK(0x8000, 0xffff, ram + 0x8000, ram + 0x8000);
	vram_ptr = 0;
	vram_data = mem_map = 0;
}

void MEMORY::load_ipl()
{
	// load ipl
	memset(rom, 0xff, sizeof(rom));
	
	const _TCHAR *file_path = NULL;
	FILEIO* fio = new FILEIO();
	
	switch(config.boot_mode) {
	case MODE_TBASIC_V1_0:
		file_path = create_local_path(_T("TBASIC10.ROM"));
		break;
	case MODE_TBASIC_V1_1:
		file_path = create_local_path(_T("TBASIC11.ROM"));
		break;
	case MODE_OABASIC:
	case MODE_OABASIC_NO_DISK:
		file_path = create_local_path(_T("OABASIC.ROM"));
		break;
	case MODE_MINI_PASCAL:
		file_path = create_local_path(_T("PASCAL.ROM"));
		break;
	}
	if(file_path != NULL && fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	} else {
		// old bios file name
		if(fio->Fopen(create_local_path(_T("TBASIC.ROM")), FILEIO_READ_BINARY)) {
			fio->Fread(rom, sizeof(rom), 1);
			fio->Fclose();
		}
	}
	delete fio;
	
}

void MEMORY::reset()
{
	memset(vram, 0, sizeof(vram));
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

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	mem_map = data;
	
	if(mem_map & 4) {
		vm->reset();
	}
	if(mem_map & 2) {
		SET_BANK(0x0000, 0x7fff, ram, ram);
	} else {
		SET_BANK(0x0000, 0x7fff, ram, rom);
	}
	// to 8255-2 port-c, bit2
	d_pio2->write_signal(SIG_I8255_PORT_C, (mem_map & 2) ? 4 : 0, 4);
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	// vram control
	if(id == SIG_MEMORY_I8255_0_A) {
		// bit6 of high byte: 0=write, 1=read
		vram_ptr = (vram_ptr & 0xff00) | (data & 0xff);
		if(!(vram_ptr & 0x4000)) {
			vram[vram_ptr & 0x3fff] = vram_data;
			attr[vram_ptr & 0x3fff] = (vram_ptr & 0x8000) ? 1 : 0;
		}
		// to 8255-0 port-c
		d_pio0->write_signal(SIG_I8255_PORT_C, vram[vram_ptr & 0x3fff], 0xff);
		// to 8255-1 port-b, bit7
		d_pio1->write_signal(SIG_I8255_PORT_B, attr[vram_ptr & 0x3fff] ? 0x80 : 0, 0x80);
	} else if(id == SIG_MEMORY_I8255_0_B) {
		vram_data = data & 0xff;
	} else if(id == SIG_MEMORY_I8255_1_C) {
		// bit6 of high byte: 0=write, 1=read
		vram_ptr = (vram_ptr & 0x00ff) | ((data & 0xff) << 8);
		if(!(vram_ptr & 0x4000)) {
			vram[vram_ptr & 0x3fff] = vram_data;
			attr[vram_ptr & 0x3fff] = (vram_ptr & 0x8000) ? 1 : 0;
		}
		// to 8255-0 port-c
		d_pio0->write_signal(SIG_I8255_PORT_C, vram[vram_ptr & 0x3fff], 0xff);
		// to 8255-1 port-b, bit7
		d_pio1->write_signal(SIG_I8255_PORT_B, attr[vram_ptr & 0x3fff] ? 0x80 : 0, 0x80);
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
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateArray(attr, sizeof(attr), 1);
	state_fio->StateValue(vram_ptr);
	state_fio->StateValue(vram_data);
	state_fio->StateValue(mem_map);
	
	// post process
	if(loading) {
		if(mem_map & 2) {
			SET_BANK(0x0000, 0x7fff, ram, ram);
		} else {
			SET_BANK(0x0000, 0x7fff, ram, rom);
		}
	}
	return true;
}

