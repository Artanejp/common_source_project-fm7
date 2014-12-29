/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ memory ]
*/

#include "memory.h"
#include "io.h"
#include "../i8255.h"
#include "../../fileio.h"

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
	memset(bios, 0xff, sizeof(bios));
	memset(basic, 0xff, sizeof(basic));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("BIOS.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(bios, sizeof(bios), 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(basic, sizeof(basic), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK(0x0000, 0x3fff, wdmy, bios);
	SET_BANK(0x4000, 0x7fff, wdmy, bios);
	SET_BANK(0x8000, 0xbfff, wdmy, bios);
	SET_BANK(0xc000, 0xffff, wdmy, bios);
	
	plane = 0;
	vram_sel = pal_sel = attr_wrap = false;
}

void MEMORY::reset()
{
	memset(vram, 0, sizeof(vram));
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	if(vram_sel && (addr & 0xc000) == 0x8000) {
		if(pal_sel && !(plane & 0x70)) {
			pal[addr & 0x0f] = data & 0x0f;
			return;
		}
		uint32 laddr = addr & 0x3fff;
		if(plane & 0x10) {
			vram[0x0000 | laddr] = (plane & 0x1) ? data : 0xff;
		}
		if(plane & 0x20) {
			vram[0x4000 | laddr] = (plane & 0x2) ? data : 0xff;
		}
		if(plane & 0x40) {
			vram[0x8000 | laddr] = (plane & 0x4) ? data : 0xff;
			attr_latch = attr_wrap ? attr_latch : attr_data;
			vram[0xc000 | laddr] = attr_latch;
			// 8255-0, Port B
			d_pio0->write_signal(SIG_I8255_PORT_B, (attr_latch << 4) | (attr_latch & 7), 0x87);
		}
		return;
	}
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	addr &= 0xffff;
	if(vram_sel && (addr & 0xc000) == 0x8000) {
		if(pal_sel && !(plane & 0x70)) {
			return pal[addr & 0x0f];
		}
		uint32 laddr = addr & 0x3fff, val = 0xff;
		if((plane & 0x11) == 0x11) {
			val &= vram[0x0000 | laddr];
		}
		if((plane & 0x22) == 0x22) {
			val &= vram[0x4000 | laddr];
		}
		if((plane & 0x44) == 0x44) {
			attr_latch = vram[0xc000 | laddr];
			val &= vram[0x8000 | laddr];
			// 8255-0, Port B
			d_pio0->write_signal(SIG_I8255_PORT_B, (attr_latch << 4) | (attr_latch & 7), 0x87);
		}
		return val;
	}
	return rbank[addr >> 12][addr & 0xfff];
}

void MEMORY::write_io8(uint32 addr, uint32 data)
{
	if(data & 2) {
		SET_BANK(0x0000, 0x3fff, ram + 0x0000, ram + 0x0000);
	} else {
		SET_BANK(0x0000, 0x3fff, ram + 0x0000, basic + 0x0000);
	}
	if(data & 1) {
		SET_BANK(0x4000, 0x7fff, ram + 0x4000, bios + 0x0000);
	} else if(data & 2) {
		SET_BANK(0x4000, 0x7fff, ram + 0x4000, ram + 0x4000);
	} else {
		SET_BANK(0x4000, 0x7fff, ram + 0x4000, basic + 0x4000);
	}
	if(data & 4) {
		SET_BANK(0x8000, 0xbfff, wdmy, rdmy);
	} else {
		SET_BANK(0x8000, 0xbfff, ram + 0x8000, ram + 0x8000);
	}
	SET_BANK(0xc000, 0xffff, ram + 0xc000, ram + 0xc000);
	
	vram_sel = ((data & 4) != 0);
	// I/O memory access
	d_io->write_signal(SIG_IO_MIO, data, 8);
	// 8255-2, Port C
	d_pio2->write_signal(SIG_I8255_PORT_C, data, 3);
}

void MEMORY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_MEMORY_I8255_1_A) {
		plane = data;
	} else if(id == SIG_MEMORY_I8255_1_B) {
		attr_data = data & 0x0f;
	} else if(id == SIG_MEMORY_I8255_1_C) {
		attr_wrap = ((data & 0x10) != 0);
		pal_sel = ((data & 0x0c) != 0);
	}
}

