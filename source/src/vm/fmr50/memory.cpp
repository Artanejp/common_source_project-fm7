/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.29 -

	[ memory and crtc ]
*/

#include "memory.h"
#if defined(HAS_I286)
#include "../i286.h"
#else
#include "../i386.h"
#endif
#include "../../fileio.h"

static const uint8 bios1[] = {
	0xFA,				// cli
	0xDB,0xE3,			// fninit
	0xB8,0xA0,0xF7,			// mov	ax,F7A0
	0x8E,0xD0,			// mov	ss,ax
	0xBC,0x7E,0x05,			// mov	sp,057E
	// init i/o
	0xB4,0x80,			// mov	ah,80
	0x9A,0x14,0x00,0xFB,0xFF,	// call	far FFFB:0014
	// boot from fdd
	0xB4,0x81,			// mov	ah,81
	0x9A,0x14,0x00,0xFB,0xFF,	// call	far FFFB:0014
	0x73,0x0B,			// jnb	$+11
	0x74,0xF5,			// jz	$-11
	// boot from scsi-hdd
	0xB4,0x82,			// mov	ah,82
	0x9A,0x14,0x00,0xFB,0xFF,	// call	far FFFB:0014
	0x72,0xEC,			// jb	$-20
	// goto ipl
	0x9A,0x04,0x00,0x00,0xB0,	// call	far B000:0004
	0xEB,0xE7			// jmp $-25
};

static const uint8 bios2[] = {
	0xEA,0x00,0x00,0x00,0xFC,	// jmp	FC00:0000
	0x00,0x00,0x00,
	0xcf				// iret
};

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x800 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x800 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	// init memory
	memset(ram, 0, sizeof(ram));
	memset(vram, 0, sizeof(vram));
	memset(cvram, 0, sizeof(cvram));
#ifdef _FMR60
	memset(avram, 0, sizeof(avram));
#else
	memset(kvram, 0, sizeof(kvram));
	memset(dummy, 0, sizeof(dummy));
#endif
	memset(ipl, 0xff, sizeof(ipl));
#ifdef _FMR60
	memset(ank24, 0xff, sizeof(ank24));
	memset(kanji24, 0xff, sizeof(kanji24));
#else
	memset(ank8, 0xff, sizeof(ank8));
	memset(ank16, 0xff, sizeof(ank16));
	memset(kanji16, 0xff, sizeof(kanji16));
#endif
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	} else {
		// load pseudo ipl
		memcpy(ipl + 0x0000, bios1, sizeof(bios1));
		memcpy(ipl + 0x3ff0, bios2, sizeof(bios2));
	}
#ifdef _FMR60
	if(fio->Fopen(emu->bios_path(_T("ANK24.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ank24, sizeof(ank24), 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("KANJI24.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji24, sizeof(kanji24), 1);
		fio->Fclose();
	}
#else
	if(fio->Fopen(emu->bios_path(_T("ANK8.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ank8, sizeof(ank8), 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("ANK16.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ank16, sizeof(ank16), 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("KANJI16.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji16, sizeof(kanji16), 1);
		fio->Fclose();
	}
#endif
	delete fio;
	
	// set memory
	SET_BANK(0x000000, 0xffffff, wdmy, rdmy);
	SET_BANK(0x000000, sizeof(ram) - 1, ram, ram);
#ifdef _FMR60
	SET_BANK(0xff8000, 0xff9fff, cvram, cvram);
	SET_BANK(0xffa000, 0xffbfff, avram, avram);
#endif
	SET_BANK(0xffc000, 0xffffff, wdmy, ipl);
	
	// set palette
	for(int i = 0; i < 8; i++) {
		dpal[i] = i;
		apal[i][0] = (i & 1) ? 0xf0 : 0;
		apal[i][1] = (i & 2) ? 0xf0 : 0;
		apal[i][2] = (i & 4) ? 0xf0 : 0;
	}
	for(int i = 0; i < 16; i++) {
		if(i & 8) {
			palette_cg[i] = RGB_COLOR(i & 2 ? 255 : 0, i & 4 ? 255 : 0, i & 1 ? 255 : 0);
		} else {
			palette_cg[i] = RGB_COLOR(i & 2 ? 127 : 0, i & 4 ? 127 : 0, i & 1 ? 127 : 0);
		}
		palette_txt[i] = palette_cg[i];
	}
	palette_txt[0] = RGB_COLOR(63, 63, 63);
	apalsel = 0;
	
	// register event
	register_frame_event(this);
}

void MEMORY::reset()
{
	// reset memory
	protect = rst = 0;
	mainmem = rplane = wplane = 0;
#ifndef _FMR60
	pagesel = ankcg = 0;
#endif
	update_bank();
	
	// reset crtc
	blink = 0;
	apalsel = 0;
	outctrl = 0xf;
#ifndef _FMR60
	dispctrl = 0x47;
	mix = 8;
	accaddr = dispaddr = 0;
	kj_l = kj_h = kj_ofs = kj_row = 0;
	
	// reset logical operation
	cmdreg = maskreg = compbit = bankdis = 0;
	memset(compreg, sizeof(compreg), 0xff);
#endif
	dma_addr_reg = dma_wrap_reg = 0;
	dma_addr_mask = 0x00ffffff;
	d_cpu->set_address_mask(0x00ffffff);
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	if(addr & 0xff000000) {
		// > 16MB
		return;
	}
	if(!mainmem) {
#ifdef _FMR60
		if(0xc0000 <= addr && addr < 0xe0000) {
			addr &= 0x1ffff;
			for(int pl = 0; pl < 4; pl++) {
				if(wplane & (1 << pl)) {
					vram[addr + 0x20000 * pl] = data;
				}
			}
		}
#else
		if(0xc0000 <= addr && addr < 0xc8000) {
			// vram
			uint32 page;
			if(dispctrl & 0x40) {
				// 400 line
				addr = ((pagesel & 0x10) << 13) | (addr & 0x7fff);
				page = 0x8000;
			} else {
				// 200 line
				addr = ((pagesel & 0x18) << 13) | (addr & 0x3fff);
				page = 0x4000;
			}
			if(cmdreg & 0x80) {
				// logical operations
				if((cmdreg & 7) == 7) {
					// compare
					compbit = 0;
					for(uint8 bit = 1; bit <= 0x80; bit <<= 1) {
						uint8 val = 0;
						for(int pl = 0; pl < 4; pl++) {
							if(vram[addr + page * pl] & bit) {
								val |= 1 << pl;
							}
						}
						for(int i = 0; i < 8; i++) {
							if((compreg[i] & 0x80) && (compreg[i] & 0xf) == val) {
								compbit |= bit;
							}
						}
					}
				} else {
					uint8 mask = maskreg | ~data, val[4];
					for(int pl = 0; pl < 4; pl++) {
						val[pl] = (imgcol & (1 << pl)) ? 0xff : 0;
					}
					switch(cmdreg & 7) {
					case 2:	// or
						for(int pl = 0; pl < 4; pl++) {
							val[pl] |= vram[addr + page * pl];
						}
						break;
					case 3:	// and
						for(int pl = 0; pl < 4; pl++) {
							val[pl] &= vram[addr + page * pl];
						}
						break;
					case 4:	// xor
						for(int pl = 0; pl < 4; pl++) {
							val[pl] ^= vram[addr + page * pl];
						}
						break;
					case 5:	// not
						for(int pl = 0; pl < 4; pl++) {
							val[pl] = ~vram[addr + page * pl];
						}
						break;
					case 6:	// tile
						for(int pl = 0; pl < 4; pl++) {
							val[pl] = tilereg[pl];
						}
						break;
					}
					for(int pl = 0; pl < 4; pl++) {
						if(!(bankdis & (1 << pl))) {
							vram[addr + page * pl] &= mask;
							vram[addr + page * pl] |= val[pl] & ~mask;
						}
					}
				}
			} else {
				for(int pl = 0; pl < 4; pl++) {
					if(wplane & (1 << pl)) {
						vram[addr + page * pl] = data;
					}
				}
			}
			return;
		} else if(0xcff80 <= addr && addr < 0xcffe0) {
#ifdef _DEBUG_LOG
//			emu->out_debug_log(_T("MW\t%4x, %2x\n"), addr, data);
#endif
			// memory mapped i/o
			switch(addr & 0xffff) {
			case 0xff80:
				// mix register
				mix = data;
				break;
			case 0xff81:
				// update register
				wplane = data & 7;
				rplane = (data >> 6) & 3;
				update_bank();
				break;
			case 0xff82:
				// display ctrl register
				dispctrl = data;
				update_bank();
				break;
			case 0xff83:
				// page select register
				pagesel = data;
				update_bank();
				break;
			case 0xff88:
				// access start register
				accaddr = (accaddr & 0xff) | ((data & 0x7f) << 8);
				break;
			case 0xff89:
				// access start register
				accaddr = (accaddr & 0xff00) | (data & 0xfe);
				break;
			case 0xff8a:
				// display start register
				dispaddr = (dispaddr & 0xff) | ((data & 0x7f) << 8);
				break;
			case 0xff8b:
				// display start register
				dispaddr = (dispaddr & 0xff00) | (data & 0xfe);
				break;
			case 0xff8e:
				// crtc addr register
				d_crtc->write_io8(0, data);
				break;
			case 0xff8f:
				// crtc data register
				d_crtc->write_io8(1, data);
				break;
			case 0xff94:
				kj_h = data & 0x7f;
				break;
			case 0xff95:
				kj_l = data & 0x7f;
				kj_row = 0;
				if(kj_h < 0x30) {
					kj_ofs = (((kj_l - 0x00) & 0x1f) <<  5) | (((kj_l - 0x20) & 0x20) <<  9) | (((kj_l - 0x20) & 0x40) <<  7) | (((kj_h - 0x00) & 0x07) << 10);
				} else if(kj_h < 0x70) {
					kj_ofs = (((kj_l - 0x00) & 0x1f) <<  5) + (((kj_l - 0x20) & 0x60) <<  9) + (((kj_h - 0x00) & 0x0f) << 10) + (((kj_h - 0x30) & 0x70) * 0xc00) + 0x08000;
				} else {
					kj_ofs = (((kj_l - 0x00) & 0x1f) <<  5) | (((kj_l - 0x20) & 0x20) <<  9) | (((kj_l - 0x20) & 0x40) <<  7) | (((kj_h - 0x00) & 0x07) << 10) | 0x38000;
				}
				break;
			case 0xff96:
				kanji16[(kj_ofs | ((kj_row & 0xf) << 1)) & 0x3ffff] = data;
				break;
			case 0xff97:
				kanji16[(kj_ofs | ((kj_row++ & 0xf) << 1) | 1) & 0x3ffff] = data;
				break;
			case 0xff99:
				ankcg = data;
				update_bank();
				break;
			case 0xffa0:
				cmdreg = data;
				break;
			case 0xffa1:
				imgcol = data;
				break;
			case 0xffa2:
				maskreg = data;
				break;
			case 0xffa3:
			case 0xffa4:
			case 0xffa5:
			case 0xffa6:
			case 0xffa7:
			case 0xffa8:
			case 0xffa9:
			case 0xffaa:
				compreg[addr & 7] = data;
				break;
			case 0xffab:
				bankdis = data;
				break;
			case 0xffac:
			case 0xffad:
			case 0xffae:
			case 0xffaf:
				tilereg[addr & 3] = data;
				break;
			case 0xffb0:
				lofs = (lofs & 0xff) | (data << 8);
				break;
			case 0xffb1:
				lofs = (lofs & 0xff00) | data;
				break;
			case 0xffb2:
				lsty = (lsty & 0xff) | (data << 8);
				break;
			case 0xffb3:
				lsty = (lsty & 0xff00) | data;
				break;
			case 0xffb4:
				lsx = (lsx & 0xff) | (data << 8);
				break;
			case 0xffb5:
				lsx = (lsx & 0xff00) | data;
				break;
			case 0xffb6:
				lsy = (lsy & 0xff) | (data << 8);
				break;
			case 0xffb7:
				lsy = (lsy & 0xff00) | data;
				break;
			case 0xffb8:
				lex = (lex & 0xff) | (data << 8);
				break;
			case 0xffb9:
				lex = (lex & 0xff00) | data;
				break;
			case 0xffba:
				ley = (ley & 0xff) | (data << 8);
				break;
			case 0xffbb:
				ley = (ley & 0xff00) | data;
				// start drawing line
				line();
				break;
			}
			return;
		}
#endif
	}
	if((addr & ~3) == 8 && (protect & 0x80)) {
		return;
	}
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	if(addr & 0xff000000) {
		// > 16MB
		if(addr >= 0xffffc000) {
			return ipl[addr & 0x3fff];
		}
		return 0xff;
	}
#ifndef _FMR60
	if(!mainmem) {
		if(0xcff80 <= addr && addr < 0xcffe0) {
#ifdef _DEBUG_LOG
//			emu->out_debug_log(_T("MR\t%4x\n"), addr);
#endif
			// memory mapped i/o
			switch(addr & 0xffff) {
			case 0xff80:
				// mix register
				return mix;
			case 0xff81:
				// update register
				return wplane | (rplane << 6);
			case 0xff83:
				// page select register
				return pagesel;
			case 0xff86:
				// status register
				return (disp ? 0x80 : 0) | (vsync ? 4 : 0) | 0x10;
			case 0xff8e:
				// crtc addr register
				return d_crtc->read_io8(0);
			case 0xff8f:
				// crtc data register
				return d_crtc->read_io8(1);
			case 0xff94:
				return 0x80;	// ???
			case 0xff96:
				return kanji16[(kj_ofs | ((kj_row & 0xf) << 1)) & 0x3ffff];
			case 0xff97:
				return kanji16[(kj_ofs | ((kj_row++ & 0xf) << 1) | 1) & 0x3ffff];
			case 0xffa0:
				return cmdreg;
			case 0xffa1:
				return imgcol | 0xf0;
			case 0xffa2:
				return maskreg;
			case 0xffa3:
				return compbit;
			case 0xffab:
				return bankdis & 0xf;
			}
			return 0xff;
		}
	}
#endif
	return rbank[addr >> 11][addr & 0x7ff];
}

void MEMORY::write_dma_data8(uint32 addr, uint32 data)
{
	write_data8(addr & dma_addr_mask, data);
}

uint32 MEMORY::read_dma_data8(uint32 addr)
{
	return read_data8(addr & dma_addr_mask);
}

void MEMORY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xffff) {
	case 0x20:
		// protect and reset
		protect = data;
		update_bank();
		if(data & 0x40) {
			// power off
			emu->power_off();
		}
		if(data & 1) {
			// software reset
			rst |= 1;
			d_cpu->reset();
		}
		// protect mode
#if defined(HAS_I286)
		if(data & 0x20) {
			d_cpu->set_address_mask(0x00ffffff);
		} else {
			d_cpu->set_address_mask(0x000fffff);
		}
#else
		switch(data & 0x30) {
		case 0x00:	// 20bit
			d_cpu->set_address_mask(0x000fffff);
			break;
		case 0x20:	// 24bit
			d_cpu->set_address_mask(0x00ffffff);
			break;
		default:	// 32bit
			d_cpu->set_address_mask(0xffffffff);
			break;
		}
#endif
		update_dma_addr_mask();
		break;
	case 0x22:
		dma_addr_reg = data;
		update_dma_addr_mask();
		break;
	case 0x24:
		dma_wrap_reg = data;
		update_dma_addr_mask();
		break;
	case 0x400:
		// video output control
		break;
	case 0x402:
		// update register
		wplane = data & 0xf;
		break;
	case 0x404:
		// read out register
		mainmem = data & 0x80;
		rplane = data & 3;
		update_bank();
		break;
	case 0x408:
		// palette code register
		apalsel = data & 0xf;
		break;
	case 0x40a:
		// blue level register
		apal[apalsel][0] = data & 0xf0;
		palette_cg[apalsel] = RGB_COLOR(apal[apalsel][1], apal[apalsel][2], apal[apalsel][0]);
		break;
	case 0x40c:
		// red level register
		apal[apalsel][1] = data & 0xf0;
		palette_cg[apalsel] = RGB_COLOR(apal[apalsel][1], apal[apalsel][2], apal[apalsel][0]);
		break;
	case 0x40e:
		// green level register
		apal[apalsel][2] = data & 0xf0;
		palette_cg[apalsel] = RGB_COLOR(apal[apalsel][1], apal[apalsel][2], apal[apalsel][0]);
		break;
	case 0xfd98:
	case 0xfd99:
	case 0xfd9a:
	case 0xfd9b:
	case 0xfd9c:
	case 0xfd9d:
	case 0xfd9e:
	case 0xfd9f:
		// digital palette
		dpal[addr & 7] = data;
		if(data & 8) {
			palette_cg[addr & 7] = RGB_COLOR(data & 2 ? 255 : 0, data & 4 ? 255 : 0, data & 1 ? 255 : 0);
		} else {
			palette_cg[addr & 7] = RGB_COLOR(data & 2 ? 127 : 0, data & 4 ? 127 : 0, data & 1 ? 127 : 0);
		}
		break;
	case 0xfda0:
		// video output control
		outctrl = data;
		break;
	}
}

uint32 MEMORY::read_io8(uint32 addr)
{
	uint32 val = 0xff;
	
	switch(addr & 0xffff) {
	case 0x20:
		// reset cause register
		val = rst | (d_cpu->get_shutdown_flag() << 1);
		rst = 0;
		d_cpu->set_shutdown_flag(0);
		return val | 0x7c;
	case 0x21:
//		return 0x1f;
		return 0xdf;
	case 0x24:
		return dma_wrap_reg;
	case 0x30:
		// machine & cpu id
		return machine_id;
	case 0x400:
		// system status register
#ifdef _FMR60
		return 0xff;
#else
		return 0xfe;
//		return 0xf6;
#endif
	case 0x402:
		// update register
		return wplane | 0xf0;
	case 0x404:
		// read out register
		return mainmem | rplane | 0x7c;
	case 0x40a:
		// blue level register
		return apal[apalsel][0];
	case 0x40c:
		// red level register
		return apal[apalsel][1];
	case 0x40e:
		// green level register
		return apal[apalsel][2];
	case 0xfd98:
	case 0xfd99:
	case 0xfd9a:
	case 0xfd9b:
	case 0xfd9c:
	case 0xfd9d:
	case 0xfd9e:
	case 0xfd9f:
		// digital palette
		return dpal[addr & 7] | 0xf0;
	case 0xfda0:
		// status register
		return (disp ? 2 : 0) | (vsync ? 1 : 0) | 0xfc;
	}
	return 0xff;
}

void MEMORY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_MEMORY_DISP) {
		disp = ((data & mask) != 0);
	} else if(id == SIG_MEMORY_VSYNC) {
		vsync = ((data & mask) != 0);
	}
}

void MEMORY::event_frame()
{
	blink++;
}

void MEMORY::update_bank()
{
	if(!mainmem) {
#ifdef _FMR60
		int ofs = rplane * 0x20000;
		SET_BANK(0xc0000, 0xdffff, vram + ofs, vram + ofs);
		SET_BANK(0xe0000, 0xeffff, wdmy, rdmy);
#else
		if(dispctrl & 0x40) {
			// 400 line
			int ofs = (rplane | ((pagesel >> 2) & 4)) * 0x8000;
			SET_BANK(0xc0000, 0xc7fff, vram + ofs, vram + ofs);
		} else {
			// 200 line
			int ofs = (rplane | ((pagesel >> 1) & 0xc)) * 0x4000;
			SET_BANK(0xc0000, 0xc3fff, vram + ofs, vram + ofs);
			SET_BANK(0xc4000, 0xc7fff, vram + ofs, vram + ofs);
		}
		SET_BANK(0xc8000, 0xc8fff, cvram, cvram);
		SET_BANK(0xc9000, 0xc9fff, wdmy, rdmy);
		if(ankcg & 1) {
			SET_BANK(0xca000, 0xca7ff, wdmy, ank8);
			SET_BANK(0xca800, 0xcafff, wdmy, rdmy);
			SET_BANK(0xcb000, 0xcbfff, wdmy, ank16);
		} else {
			SET_BANK(0xca000, 0xcafff, kvram, kvram);
			SET_BANK(0xcb000, 0xcbfff, wdmy, rdmy);
		}
		SET_BANK(0xcc000, 0xeffff, wdmy, rdmy);
#endif
	} else {
		SET_BANK(0xc0000, 0xeffff, ram + 0xc0000, ram + 0xc0000);
	}
	if(!(protect & 0x20)) {
#ifdef _FMR60
		SET_BANK(0xf8000, 0xf9fff, cvram, cvram);
		SET_BANK(0xfa000, 0xfbfff, avram, avram);
#else
		SET_BANK(0xf8000, 0xfbfff, wdmy, rdmy);
#endif
		SET_BANK(0xfc000, 0xfffff, wdmy, ipl);
	} else {
		SET_BANK(0xf8000, 0xfffff, ram + 0xf8000, ram + 0xf8000);
	}
}

void MEMORY::update_dma_addr_mask()
{
	switch(dma_addr_reg & 3) {
	case 0:
		dma_addr_mask = d_cpu->get_address_mask();
		break;
	case 1:
		if(!(dma_wrap_reg & 1) && d_cpu->get_address_mask() == 0x000fffff) {
			dma_addr_mask = 0x000fffff;
		} else {
			dma_addr_mask = 0x00ffffff;
		}
		break;
	default:
		if(!(dma_wrap_reg & 1) && d_cpu->get_address_mask() == 0x000fffff) {
			dma_addr_mask = 0x000fffff;
		} else {
			dma_addr_mask = 0xffffffff;
		}
		break;
	}
}

#ifndef _FMR60
void MEMORY::point(int x, int y, int col)
{
	if(x < 640 && y < 400) {
		int ofs = ((lofs & 0x3fff) + (x >> 3) + y * 80) & 0x7fff;
		uint8 bit = 0x80 >> (x & 7);
		for(int pl = 0; pl < 4; pl++) {
			uint8 pbit = 1 << pl;
			if(!(bankdis & pbit)) {
				if(col & pbit) {
					vram[0x8000 * pl + ofs] |= bit;
				} else {
					vram[0x8000 * pl + ofs] &= ~bit;
				}
			}
		}
	}
}

void MEMORY::line()
{
	int nx = lsx, ny = lsy;
	int dx = abs(lex - lsx) * 2;
	int dy = abs(ley - lsy) * 2;
	int sx = (lex < lsx) ? -1 : 1;
	int sy = (ley < lsy) ? -1 : 1;
	int c = 0;
	
	point(lsx, lsy, (lsty & (0x8000 >> (c++ & 15))) ? imgcol : 0);
	if(dx > dy) {
		int frac = dy - dx / 2;
		while(nx != lex) {
			if(frac >= 0) {
				ny += sy;
				frac -= dx;
			}
			nx += sx;
			frac += dy;
			point(nx, ny, (lsty & (0x8000 >> (c++ & 15))) ? imgcol : 0);
		}
	} else {
		int frac = dx - dy / 2;
		while(ny != ley) {
			if(frac >= 0) {
				nx += sx;
				frac -= dy;
			}
			ny += sy;
			frac += dx;
			point(nx, ny, (lsty & (0x8000 >> (c++ & 15))) ? imgcol : 0);
		}
	}
//	point(lex, ley, (lsty & (0x8000 >> (c++ & 15))) ? imgcol : 0);
}
#endif

void MEMORY::draw_screen()
{
	// render screen
	memset(screen_txt, 0, sizeof(screen_txt));
	memset(screen_cg, 0, sizeof(screen_cg));
	if(outctrl & 1) {
#ifdef _FMR60
		draw_text();
#else
		if(mix & 8) {
			draw_text80();
		} else {
			draw_text40();
		}
#endif
	}
	if(outctrl & 4) {
		draw_cg();
	}
	
	for(int y = 0; y < SCREEN_HEIGHT; y++) {
		scrntype* dest = emu->screen_buffer(y);
		uint8* txt = screen_txt[y];
		uint8* cg = screen_cg[y];
		
		for(int x = 0; x < SCREEN_WIDTH; x++) {
			dest[x] = txt[x] ? palette_txt[txt[x] & 15] : palette_cg[cg[x]];
		}
	}
	emu->screen_skip_line = false;
}

#ifdef _FMR60
void MEMORY::draw_text()
{
	int src = ((chreg[12] << 9) | (chreg[13] << 1)) & 0x1fff;
	int caddr = ((chreg[8] & 0xc0) == 0xc0) ? -1 : (((chreg[14] << 9) | (chreg[15] << 1)) & 0x1fff);
	int ymax = (chreg[6] > 0) ? chreg[6] : 25;
	int yofs = 750 / ymax;
	
	for(int y = 0; y < 25; y++) {
		for(int x = 0; x < 80; x++) {
			bool cursor = ((src >> 1) == caddr);
			int cx = x;
			uint8 codel = cvram[src];
			uint8 attrl = avram[src];
			src = (src + 1) & 0x1ffff;
			uint8 codeh = cvram[src];
			uint8 attrh = avram[src];
			src = (src + 1) & 0x1ffff;
			uint8 col = (attrl & 15) | 16;
			bool blnk = (attrh & 0x40) || ((blink & 32) && (attrh & 0x10));
			bool rev = ((attrh & 8) != 0);
			uint8 xor_mask = (rev != blnk) ? 0xff : 0;
			
			if(codeh & 0x80) {
				// kanji
				int ofs = (codel | ((codeh & 0x7f) << 8)) * 72;
				for(int l = 3; l < 27 && l < yofs; l++) {
					uint8 pat0 = kanji24[ofs++] ^ xor_mask;
					uint8 pat1 = kanji24[ofs++] ^ xor_mask;
					uint8 pat2 = kanji24[ofs++] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 750) {
						break;
					}
					uint8* d = &screen_txt[yy][x * 14];
					
					d[ 2] = (pat0 & 0x80) ? col : 0;
					d[ 3] = (pat0 & 0x40) ? col : 0;
					d[ 4] = (pat0 & 0x20) ? col : 0;
					d[ 5] = (pat0 & 0x10) ? col : 0;
					d[ 6] = (pat0 & 0x08) ? col : 0;
					d[ 7] = (pat0 & 0x04) ? col : 0;
					d[ 8] = (pat0 & 0x02) ? col : 0;
					d[ 9] = (pat0 & 0x01) ? col : 0;
					d[10] = (pat1 & 0x80) ? col : 0;
					d[11] = (pat1 & 0x40) ? col : 0;
					d[12] = (pat1 & 0x20) ? col : 0;
					d[13] = (pat1 & 0x10) ? col : 0;
					d[14] = (pat1 & 0x08) ? col : 0;
					d[15] = (pat1 & 0x04) ? col : 0;
					d[16] = (pat1 & 0x02) ? col : 0;
					d[17] = (pat1 & 0x01) ? col : 0;
					d[18] = (pat2 & 0x80) ? col : 0;
					d[19] = (pat2 & 0x40) ? col : 0;
					d[20] = (pat2 & 0x20) ? col : 0;
					d[21] = (pat2 & 0x10) ? col : 0;
					d[22] = (pat2 & 0x08) ? col : 0;
					d[23] = (pat2 & 0x04) ? col : 0;
					d[24] = (pat2 & 0x02) ? col : 0;
					d[25] = (pat2 & 0x01) ? col : 0;
				}
				src = (src + 2) & 0x1fff;
				x++;
			} else if(codeh) {
			} else {
				// ank
				int ofs = codel * 48;
				for(int l = 3; l < 27 && l < yofs; l++) {
					uint8 pat0 = ank24[ofs++] ^ xor_mask;
					uint8 pat1 = ank24[ofs++] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 750) {
						break;
					}
					uint8* d = &screen_txt[yy][x * 14];
					
					d[ 0] = (pat0 & 0x80) ? col : 0;
					d[ 1] = (pat0 & 0x40) ? col : 0;
					d[ 2] = (pat0 & 0x20) ? col : 0;
					d[ 3] = (pat0 & 0x10) ? col : 0;
					d[ 4] = (pat0 & 0x08) ? col : 0;
					d[ 5] = (pat0 & 0x04) ? col : 0;
					d[ 6] = (pat0 & 0x02) ? col : 0;
					d[ 7] = (pat0 & 0x01) ? col : 0;
					d[ 8] = (pat1 & 0x80) ? col : 0;
					d[ 9] = (pat1 & 0x40) ? col : 0;
					d[10] = (pat1 & 0x20) ? col : 0;
					d[11] = (pat1 & 0x10) ? col : 0;
					d[12] = (pat1 & 0x08) ? col : 0;
					d[13] = (pat1 & 0x04) ? col : 0;
				}
			}
/*
			if(cursor) {
				int bp = chreg[10] & 0x60;
				if(bp == 0 || (bp == 0x40 && (blink & 8)) || (bp == 0x60 && (blink & 0x10))) {
					int st = chreg[10] & 15;
					int ed = chreg[11] & 15;
					for(int i = st; i < ed && i < yofs; i++) {
						memset(&screen_txt[y * yofs + i][cx << 3], 7, 8);
					}
				}
			}
*/
		}
	}
}
#else
void MEMORY::draw_text40()
{
	int src = ((chreg[12] << 9) | (chreg[13] << 1)) & 0xfff;
	int caddr = ((chreg[8] & 0xc0) == 0xc0) ? -1 : (((chreg[14] << 9) | (chreg[15] << 1) | (mix & 0x20 ? 1 : 0)) & 0x7ff);
	int ymax = (chreg[6] > 0) ? chreg[6] : 25;
	int yofs = 400 / ymax;
	
	for(int y = 0; y < ymax; y++) {
		for(int x = 0; x < 40; x++) {
			bool cursor = ((src >> 1) == caddr);
			int cx = x;
			uint8 code = cvram[src];
			uint8 h = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8 attr = cvram[src];
			uint8 l = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8 col = ((attr & 0x20) >> 2) | (attr & 7) | 16;
			bool blnk = (blink & 32) && (attr & 0x10);
			bool rev = ((attr & 8) != 0);
			uint8 xor_mask = (rev != blnk) ? 0xff : 0;
			
			if(attr & 0x40) {
				// kanji
				int ofs;
				if(h < 0x30) {
					ofs = (((l - 0x00) & 0x1f) <<  5) | (((l - 0x20) & 0x20) <<  9) | (((l - 0x20) & 0x40) <<  7) | (((h - 0x00) & 0x07) << 10);
				} else if(h < 0x70) {
					ofs = (((l - 0x00) & 0x1f) <<  5) + (((l - 0x20) & 0x60) <<  9) + (((h - 0x00) & 0x0f) << 10) + (((h - 0x30) & 0x70) * 0xc00) + 0x08000;
				} else {
					ofs = (((l - 0x00) & 0x1f) <<  5) | (((l - 0x20) & 0x20) <<  9) | (((l - 0x20) & 0x40) <<  7) | (((h - 0x00) & 0x07) << 10) | 0x38000;
				}
				
				for(int l = 0; l < 16 && l < yofs; l++) {
					uint8 pat0 = kanji16[ofs + (l << 1) + 0] ^ xor_mask;
					uint8 pat1 = kanji16[ofs + (l << 1) + 1] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 400) {
						break;
					}
					uint8* d = &screen_txt[yy][x << 4];
					
					d[ 0] = d[ 1] = (pat0 & 0x80) ? col : 0;
					d[ 2] = d[ 3] = (pat0 & 0x40) ? col : 0;
					d[ 4] = d[ 5] = (pat0 & 0x20) ? col : 0;
					d[ 6] = d[ 7] = (pat0 & 0x10) ? col : 0;
					d[ 8] = d[ 9] = (pat0 & 0x08) ? col : 0;
					d[10] = d[11] = (pat0 & 0x04) ? col : 0;
					d[12] = d[13] = (pat0 & 0x02) ? col : 0;
					d[14] = d[15] = (pat0 & 0x01) ? col : 0;
					d[16] = d[17] = (pat1 & 0x80) ? col : 0;
					d[18] = d[19] = (pat1 & 0x40) ? col : 0;
					d[20] = d[21] = (pat1 & 0x20) ? col : 0;
					d[22] = d[23] = (pat1 & 0x10) ? col : 0;
					d[24] = d[25] = (pat1 & 0x08) ? col : 0;
					d[26] = d[27] = (pat1 & 0x04) ? col : 0;
					d[28] = d[29] = (pat1 & 0x02) ? col : 0;
					d[30] = d[31] = (pat1 & 0x01) ? col : 0;
				}
				src = (src + 2) & 0xfff;
				x++;
			} else {
				for(int l = 0; l < 16 && l < yofs; l++) {
					uint8 pat = ank16[(code << 4) + l] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 400) {
						break;
					}
					uint8* d = &screen_txt[yy][x << 4];
					
					d[ 0] = d[ 1] = (pat & 0x80) ? col : 0;
					d[ 2] = d[ 3] = (pat & 0x40) ? col : 0;
					d[ 4] = d[ 5] = (pat & 0x20) ? col : 0;
					d[ 6] = d[ 7] = (pat & 0x10) ? col : 0;
					d[ 8] = d[ 9] = (pat & 0x08) ? col : 0;
					d[10] = d[11] = (pat & 0x04) ? col : 0;
					d[12] = d[13] = (pat & 0x02) ? col : 0;
					d[14] = d[15] = (pat & 0x01) ? col : 0;
				}
			}
			if(cursor) {
				int bp = chreg[10] & 0x60;
				if(bp == 0 || (bp == 0x40 && (blink & 8)) || (bp == 0x60 && (blink & 0x10))) {
					int st = chreg[10] & 15;
					int ed = chreg[11] & 15;
					for(int i = st; i < ed && i < yofs; i++) {
						memset(&screen_txt[y * yofs + i][cx << 3], 7, 8);
					}
				}
			}
		}
	}
}

void MEMORY::draw_text80()
{
	int src = ((chreg[12] << 9) | (chreg[13] << 1)) & 0xfff;
	int caddr = ((chreg[8] & 0xc0) == 0xc0) ? -1 : (((chreg[14] << 9) | (chreg[15] << 1) | (mix & 0x20 ? 1 : 0)) & 0x7ff);
	int ymax = (chreg[6] > 0) ? chreg[6] : 25;
	int yofs = 400 / ymax;
	
	for(int y = 0; y < 25; y++) {
		for(int x = 0; x < 80; x++) {
			bool cursor = ((src >> 1) == caddr);
			int cx = x;
			uint8 code = cvram[src];
			uint8 h = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8 attr = cvram[src];
			uint8 l = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8 col = ((attr & 0x20) >> 2) | (attr & 7) | 16;
			bool blnk = (blink & 32) && (attr & 0x10);
			bool rev = ((attr & 8) != 0);
			uint8 xor_mask = (rev != blnk) ? 0xff : 0;
			
			if(attr & 0x40) {
				// kanji
				int ofs;
				if(h < 0x30) {
					ofs = (((l - 0x00) & 0x1f) <<  5) | (((l - 0x20) & 0x20) <<  9) | (((l - 0x20) & 0x40) <<  7) | (((h - 0x00) & 0x07) << 10);
				} else if(h < 0x70) {
					ofs = (((l - 0x00) & 0x1f) <<  5) + (((l - 0x20) & 0x60) <<  9) + (((h - 0x00) & 0x0f) << 10) + (((h - 0x30) & 0x70) * 0xc00) + 0x08000;
				} else {
					ofs = (((l - 0x00) & 0x1f) <<  5) | (((l - 0x20) & 0x20) <<  9) | (((l - 0x20) & 0x40) <<  7) | (((h - 0x00) & 0x07) << 10) | 0x38000;
				}
				
				for(int l = 0; l < 16 && l < yofs; l++) {
					uint8 pat0 = kanji16[ofs + (l << 1) + 0] ^ xor_mask;
					uint8 pat1 = kanji16[ofs + (l << 1) + 1] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 400) {
						break;
					}
					uint8* d = &screen_txt[yy][x << 3];
					
					d[ 0] = (pat0 & 0x80) ? col : 0;
					d[ 1] = (pat0 & 0x40) ? col : 0;
					d[ 2] = (pat0 & 0x20) ? col : 0;
					d[ 3] = (pat0 & 0x10) ? col : 0;
					d[ 4] = (pat0 & 0x08) ? col : 0;
					d[ 5] = (pat0 & 0x04) ? col : 0;
					d[ 6] = (pat0 & 0x02) ? col : 0;
					d[ 7] = (pat0 & 0x01) ? col : 0;
					d[ 8] = (pat1 & 0x80) ? col : 0;
					d[ 9] = (pat1 & 0x40) ? col : 0;
					d[10] = (pat1 & 0x20) ? col : 0;
					d[11] = (pat1 & 0x10) ? col : 0;
					d[12] = (pat1 & 0x08) ? col : 0;
					d[13] = (pat1 & 0x04) ? col : 0;
					d[14] = (pat1 & 0x02) ? col : 0;
					d[15] = (pat1 & 0x01) ? col : 0;
				}
				src = (src + 2) & 0xfff;
				x++;
			} else {
				for(int l = 0; l < 16 && l < yofs; l++) {
					uint8 pat = ank16[(code << 4) + l] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 400) {
						break;
					}
					uint8* d = &screen_txt[yy][x << 3];
					
					d[0] = (pat & 0x80) ? col : 0;
					d[1] = (pat & 0x40) ? col : 0;
					d[2] = (pat & 0x20) ? col : 0;
					d[3] = (pat & 0x10) ? col : 0;
					d[4] = (pat & 0x08) ? col : 0;
					d[5] = (pat & 0x04) ? col : 0;
					d[6] = (pat & 0x02) ? col : 0;
					d[7] = (pat & 0x01) ? col : 0;
				}
			}
			if(cursor) {
				int bp = chreg[10] & 0x60;
				if(bp == 0 || (bp == 0x40 && (blink & 8)) || (bp == 0x60 && (blink & 0x10))) {
					int st = chreg[10] & 15;
					int ed = chreg[11] & 15;
					for(int i = st; i < ed && i < yofs; i++) {
						memset(&screen_txt[y * yofs + i][cx << 3], 7, 8);
					}
				}
			}
		}
	}
}
#endif

void MEMORY::draw_cg()
{
#ifdef _FMR60
	uint8* p0 = &vram[0x00000];
	uint8* p1 = &vram[0x20000];
	uint8* p2 = &vram[0x40000];
	uint8* p3 = &vram[0x60000];
	int ptr = 0;
	
	for(int y = 0; y < 750; y++) {
		for(int x = 0; x < 1120; x += 8) {
			uint8 r = p0[ptr];
			uint8 g = p1[ptr];
			uint8 b = p2[ptr];
			uint8 i = p3[ptr++];
			ptr &= 0x1ffff;
			uint8* d = &screen_cg[y][x];
			
			d[0] = ((r & 0x80) >> 7) | ((g & 0x80) >> 6) | ((b & 0x80) >> 5) | ((i & 0x80) >> 4);
			d[1] = ((r & 0x40) >> 6) | ((g & 0x40) >> 5) | ((b & 0x40) >> 4) | ((i & 0x40) >> 3);
			d[2] = ((r & 0x20) >> 5) | ((g & 0x20) >> 4) | ((b & 0x20) >> 3) | ((i & 0x20) >> 2);
			d[3] = ((r & 0x10) >> 4) | ((g & 0x10) >> 3) | ((b & 0x10) >> 2) | ((i & 0x10) >> 1);
			d[4] = ((r & 0x08) >> 3) | ((g & 0x08) >> 2) | ((b & 0x08) >> 1) | ((i & 0x08) >> 0);
			d[5] = ((r & 0x04) >> 2) | ((g & 0x04) >> 1) | ((b & 0x04) >> 0) | ((i & 0x04) << 1);
			d[6] = ((r & 0x02) >> 1) | ((g & 0x02) >> 0) | ((b & 0x02) << 1) | ((i & 0x02) << 2);
			d[7] = ((r & 0x01) >> 0) | ((g & 0x01) << 1) | ((b & 0x01) << 2) | ((i & 0x01) << 3);
		}
	}
#else
	if(dispctrl & 0x40) {
		// 400line
		int pofs = ((dispctrl >> 3) & 1) * 0x20000;
		uint8* p0 = (dispctrl & 0x01) ? &vram[pofs | 0x00000] : dummy;
		uint8* p1 = (dispctrl & 0x02) ? &vram[pofs | 0x08000] : dummy;
		uint8* p2 = (dispctrl & 0x04) ? &vram[pofs | 0x10000] : dummy;
		uint8* p3 = (dispctrl & 0x20) ? &vram[pofs | 0x18000] : dummy;	// ???
		int ptr = dispaddr & 0x7ffe;
		
		for(int y = 0; y < 400; y++) {
			for(int x = 0; x < 640; x += 8) {
				uint8 r = p0[ptr];
				uint8 g = p1[ptr];
				uint8 b = p2[ptr];
				uint8 i = p3[ptr++];
				ptr &= 0x7fff;
				uint8* d = &screen_cg[y][x];
				
				d[0] = ((r & 0x80) >> 7) | ((g & 0x80) >> 6) | ((b & 0x80) >> 5) | ((i & 0x80) >> 4);
				d[1] = ((r & 0x40) >> 6) | ((g & 0x40) >> 5) | ((b & 0x40) >> 4) | ((i & 0x40) >> 3);
				d[2] = ((r & 0x20) >> 5) | ((g & 0x20) >> 4) | ((b & 0x20) >> 3) | ((i & 0x20) >> 2);
				d[3] = ((r & 0x10) >> 4) | ((g & 0x10) >> 3) | ((b & 0x10) >> 2) | ((i & 0x10) >> 1);
				d[4] = ((r & 0x08) >> 3) | ((g & 0x08) >> 2) | ((b & 0x08) >> 1) | ((i & 0x08) >> 0);
				d[5] = ((r & 0x04) >> 2) | ((g & 0x04) >> 1) | ((b & 0x04) >> 0) | ((i & 0x04) << 1);
				d[6] = ((r & 0x02) >> 1) | ((g & 0x02) >> 0) | ((b & 0x02) << 1) | ((i & 0x02) << 2);
				d[7] = ((r & 0x01) >> 0) | ((g & 0x01) << 1) | ((b & 0x01) << 2) | ((i & 0x01) << 3);
			}
		}
	} else {
		// 200line
		int pofs = ((dispctrl >> 3) & 3) * 0x10000;
		uint8* p0 = (dispctrl & 0x01) ? &vram[pofs | 0x0000] : dummy;
		uint8* p1 = (dispctrl & 0x02) ? &vram[pofs | 0x4000] : dummy;
		uint8* p2 = (dispctrl & 0x04) ? &vram[pofs | 0x8000] : dummy;
		uint8* p3 = (dispctrl & 0x20) ? &vram[pofs | 0xc000] : dummy;	// ???
		int ptr = dispaddr & 0x3ffe;
		
		for(int y = 0; y < 400; y += 2) {
			for(int x = 0; x < 640; x += 8) {
				uint8 r = p0[ptr];
				uint8 g = p1[ptr];
				uint8 b = p2[ptr];
				uint8 i = p3[ptr++];
				ptr &= 0x3fff;
				uint8* d = &screen_cg[y][x];
				
				d[0] = ((r & 0x80) >> 7) | ((g & 0x80) >> 6) | ((b & 0x80) >> 5) | ((i & 0x80) >> 4);
				d[1] = ((r & 0x40) >> 6) | ((g & 0x40) >> 5) | ((b & 0x40) >> 4) | ((i & 0x40) >> 3);
				d[2] = ((r & 0x20) >> 5) | ((g & 0x20) >> 4) | ((b & 0x20) >> 3) | ((i & 0x20) >> 2);
				d[3] = ((r & 0x10) >> 4) | ((g & 0x10) >> 3) | ((b & 0x10) >> 2) | ((i & 0x10) >> 1);
				d[4] = ((r & 0x08) >> 3) | ((g & 0x08) >> 2) | ((b & 0x08) >> 1) | ((i & 0x08) >> 0);
				d[5] = ((r & 0x04) >> 2) | ((g & 0x04) >> 1) | ((b & 0x04) >> 0) | ((i & 0x04) << 1);
				d[6] = ((r & 0x02) >> 1) | ((g & 0x02) >> 0) | ((b & 0x02) << 1) | ((i & 0x02) << 2);
				d[7] = ((r & 0x01) >> 0) | ((g & 0x01) << 1) | ((b & 0x01) << 2) | ((i & 0x01) << 3);
			}
			memcpy(screen_cg[y + 1], screen_cg[y], 640);
		}
	}
#endif
}

