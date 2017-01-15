/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.29 -

	[ memory and crtc ]
*/

#include "memory.h"
#include "../i386.h"


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

	bankf8_ram = false;
	bankd0_dict = false;
	dict_bank = 0;
	
	vram_wait_val = 6;
	mem_wait_val = 3;
	
	memset(page0, 0x00, sizeof(page0));
	memset(ram_0d0, 0x00, sizeof(ram_0d0));
	memset(ram_0f0, 0x00, sizeof(ram_0f0));
	memset(ram_0f8, 0x00, sizeof(ram_0f8));
	
	memset(system_rom, 0xff, sizeof(system_rom));
	memset(font_rom, 0xff, sizeof(font_rom));
#if 0
	memset(font_20_rom, 0xff, sizeof(font_20_rom));
#endif
	memset(msdos_rom, 0xff, sizeof(msdos_rom));
	memset(dict_rom, 0xff, sizeof(dict_rom));

	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_SYS.ROM")), FILEIO_READ_BINARY)) { // SYSTEM
		fio->Fread(system_rom, sizeof(system_rom), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("FMT_FNT.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(font_rom, sizeof(font_rom), 1);
		fio->Fclose();
	}
#if 0
	if(fio->Fopen(create_local_path(_T("FMT_F20.ROM")), FILEIO_READ_BINARY)) { // 20 pixels FONT : Optional
		fio->Fread(font_20_rom, sizeof(font_20_rom), 1);
		fio->Fclose();
	}
#endif
	if(fio->Fopen(create_local_path(_T("FMT_DOS.ROM")), FILEIO_READ_BINARY)) { // MSDOS
		fio->Fread(msdos_rom, sizeof(msdos_rom), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("FMT_DIC.ROM")), FILEIO_READ_BINARY)) { // MSDOS
		fio->Fread(dict_rom, sizeof(dict_rom), 1);
		fio->Fclose();
	}
	
	memset(extram_adrs, 0x00, sizeof(extram_adrs));
	extram_pages = TOWNS_EXTRAM_PAGES;
	extram_base = (uint8_t *)malloc(extram_pages * 0x100000);
	if(extram_base == NULL) {
		extram_pages = 0;
	} else {
		for(uint32_t ui = 0; ui < extram_pages; ui++) {
			uint8_t *p;
			p = &(extram_base[ui << 20]);
			extram_adrs[ui] = p;
		}
	}
	// Address Cx000000
	memset(write_bank_adrs_cx, 0x00, sizeof(write_bank_adrs_cx));
	memset(read_bank_adrs_cx, 0x00, sizeof(read_bank_adrs_cx));
	memset(device_bank_adrs_cx, 0x00, sizeof(device_bank_adrs_cx));
	
	for(uint32_t ui = 0x0000; ui < 0x4000; ui++) {
		if(ui < 0x2000) {
			// ROM CARD?
		} else if(ui < 0x2080) {
			read_bank_adrs_cx[ui] = &(msdos_rom[(ui - 0x2000) << 12]);
		} else if(ui < 0x2100) {
			read_bank_adrs_cx[ui] = &(dict_rom[(ui - 0x2080) << 12]);
		} else if(ui < 0x2140) {
			read_bank_adrs_cx[ui] = &(font_rom[(ui - 0x2100) << 12]);
		} else if(ui < 0x2142) {
			devicetype_adrs_cx[ui] = TOWNS_MEMORY_TYPE_DICTLEARN;
		} eise if(ui < 0x2200) {
			// Reserved.
		} else if(ui < 0x2201) {
			devicetype_adrs_cx[ui] = TOWNS_MEMORY_TYPE_WAVERAM;
		} else {
			// Reserved.
		}
	}
			
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
	d_cpu->set_address_mask(0xffffffff);
}

void MEMORY::write_page0_8(uint32_t addr, uint32_t data, int *wait)
{
	addr = addr & 0x000fffff;
	if(wait != NULL) *wait = mem_wait_val;

	if(addr < 0xc0000) {
		page0[addr] = (uint8_t)data;
	} else if(addr < 0xc8000) {
		if(d_vram != NULL) {
			d_vram->write_plane_data8(addr & 0x7fff, data);
			if(wait != NULL) *wait = vram_wait_val;
	} else if(addr < 0xd0000) {
		// MMIO, VRAM and ram.
		if(0xcff80 <= addr && addr < 0xcffe0) {
#ifdef _DEBUG_LOG
//			this->out_debug_log(_T("MW\t%4x, %2x\n"), addr, data);
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
	} else if(addr < 0xd8000) {
		if(!bankd0_dict) {
			ram_0d0[addr - 0x0d0000] = (uint8_t)data;
			// RAM? DICT?
		} else {
			// DICT
			//dict_rom[addr - 0xd0000 + (((uint32_t)dict_bank) << 15))
		}
	} else if(addr < 0xda000) {
		if(!bankd0_dict) {
			// RAM? DICT?
			ram_0d0[addr - 0x0d0000] = (uint8_t)data;
		} else {
			// DICT
			if(d_cmos != NULL) d_cmos->write_data8(addr, data);
			return;
		}
	} else if(addr < 0xf0000) {
		if(!bankd0_dict) {
			ram_0d0[addr - 0x0d0000] = (uint8_t)data;
		}
	} else if(addr < 0xf8000) {
		ram_0f0[addr - 0xf0000] = (uint8_t)data;
	} else if(addr < 0x100000) {
		if(bankf8_ram) {
			// RAM
			ram_0f8[addr - 0xf8000] = (uint8_t)data;
		} else {
			// BOOT ROM(ro)
			//system_rom[addr - 0xf8000 + 0x18000];
		}
	}
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_data8w(addr, data, &wait);
}

void MEMORY::write_data16(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_data16w(addr, data, &wait);
}

void MEMORY::write_data32(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_data32w(addr, data, &wait);
}

void MEMORY::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	uint32_t addr_head = (addr & 0xf0000000) >> 28;
	uint32_t addr_mid;
	uint32_t addr_low;
	uint32_t ui;
	uint8_t *pp;
	
	if(wait != NULL) *wait = mem_wait_val;
	switch(addr_head) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
		if(addr < 0x00100000) {
			write_page0_8(addr, data, wait);
		} else {
			ui = (((addr - 0x00100000) & 0x3ff00000) >> 20);
			uint8_t *p = extram_adrs[ui];
			if(p != NULL) {
				addr_low = addr & 0x000fffff;
				p[addr_low] = (uint8_t)data;
			}
		}
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		if(extio != NULL) extio->write_data8(addr, data);
		break;
	case 8:
		if(d_vram != NULL) {
			d_vram->write_data8(addr, data);
			if(wait != NULL) *wait = vram_wait_val;
		}
		break;
	case 9:
	case 0x0a:
	case 0x0b:
		// ??
		break;
	case 0x0c:
		addr_mid = (addr & 0x03fff000) >> 16 ;
		addr_low = addr & 0x00000fff;
		pp = write_bank_adrs_cx[addr_mid];
		if(pp != NULL) {
			pp[addr_low] = (uint8_t)data;
		} else if(device_type_adrs_cx[addr_mid] != 0) {
			switch(device_type_adrs_cx[addr_mid]) {
			case TOWNS_MEMORY_TYPE_WAVERAM:
				if(d_pcm != NULL) {
					d_pcm->write_data8(addr, data);
				}
				break;
			case TOWNS_MEMORY_TYPE_DICTLEARN:
				if(d_cmos != NULL) {
					d_cmos->write_data8(addr, data);
				}
				break;
			case TOWNS_MEMORY_TYPE_FORBID:
			default:
				break;
			}
		}
		break;
	case 0x0d:
	case 0x0e:
		// ??
		break;
	case 0x0f:
		// ROM, maybe unable to write.
		break;
	}
}

void MEMORY::write_data16w(uint32_t addr, uint32_t data, int *wait)
{
	uint32_t addr_head = (addr & 0xf0000000) >> 28;
	uint32_t addr_low;
	uint32_t addr_mid;
	uint16_t *pp;
	if(wait != NULL) *wait = mem_wait_val;
	switch(addr_head) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
		if(addr < 0x00100000) {
			write_page0_16(addr, data, wait);
		} else {
			ui = (((addr - 0x00100000) & 0x3ff00000) >> 20);
			uint16_t *p = (uint16_t *)extram_adrs[ui];
			if(p != NULL) {
				addr_low = (addr & 0x000fffff) >> 1;
#if __LITTLE_ENDIAN__
				p[addr_low] = (uint16_t)data;
#else
				uint8_t *p8 = (uint8_*)(&(p[addr_low]));
				pair_t d;
				d.d = data;
				d.write_2bytes_le_to(p8);
#endif
			}
		}
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		if(extio != NULL) extio->write_data16(addr, data);
		break;
	case 8:
		if(d_vram != NULL) {
			d_vram->write_data16(addr, data);
			if(wait != NULL) *wait = mem_wait_val;
		}
		break;
	case 9:
	case 0x0a:
	case 0x0b:
		// ??
		break;
	case 0x0c:
		addr_mid = (addr & 0x03fff000) >> 16 ;
		addr_low = addr & 0x00000fff;
		pp = (uint16_t *)write_bank_adrs_cx[addr_mid];
		if(pp != NULL) {
#if __LITTLE_ENDIAN__
			pp[addr_low >> 1] = (uint16_t)data;
#else
			uint8_t *p8 = (uint8_*)(&(pp[addr_low]));
			pair_t d;
			d.d = data;
			d.write_2bytes_le_to(p8);
#endif
		} else if(device_type_adrs_cx[addr_mid] != 0) {
			switch(device_type_adrs_cx[addr_mid]) {
			case TOWNS_MEMORY_TYPE_WAVERAM:
				if(d_pcm != NULL) {
					d_pcm->write_data8(addr, data);
				}
				break;
			case TOWNS_MEMORY_TYPE_DICTLEARN:
				if(d_cmos != NULL) {
					d_cmos->write_data8(addr, data);
				}
				break;
			case TOWNS_MEMORY_TYPE_FORBID:
			default:
				break;
			}
		}
		break;
	case 0x0d:
	case 0x0e:
		// ??
		break;
	case 0x0f:
		// ROM, maybe unable to write.
		break;
	}
}

void MEMORY::write_data32w(uint32_t addr, uint32_t data, int *wait)
{
	uint32_t addr_head = (addr & 0xf0000000) >> 28;
	uint32_t addr_low;
	uint32_t addr_mid;
	uint32_t *pp;
	if(wait != NULL) *wait = mem_wait_val;
	switch(addr_head) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
		if(addr < 0x00100000) {
			write_page0_32(addr, data, wait);
		} else {
			ui = (((addr - 0x00100000) & 0x3ff00000) >> 20);
			uint32_t *p = (uint32_t *)extram_adrs[ui];
			if(p != NULL) {
				addr_low = (addr & 0x000fffff) >> 2;
#if __LITTLE_ENDIAN__
				p[addr_low] = data;
#else
				uint8_t *p8 = (uint8_*)(&(p[addr_low]));
				pair_t d;
				d.d = data;
				d.write_4bytes_le_to(p8);
#endif
			}
		}
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		if(extio != NULL) extio->write_data32(addr, data);
		break;
	case 8:
		if(d_vram != NULL) {
			d_vram->write_data32(addr, data);
			if(wait != NULL) *wait = vram_wait_val;
		}
		break;
	case 9:
	case 0x0a:
	case 0x0b:
		// ??
		break;
	case 0x0c:
		addr_mid = (addr & 0x03fff000) >> 16 ;
		addr_low = (addr & 0x00000fff >> 2);
		pp = (uint32_t *)write_bank_adrs_cx[addr_mid];
		if(pp != NULL) {
#if __LITTLE_ENDIAN__
			pp[addr_low >> 2] = data;
#else
			uint8_t *p8 = (uint8_*)(&(pp[addr_low]));
			pair_t d;
			d.d = data;
			d.write_4bytes_le_to(p8);
#endif
		} else if(device_type_adrs_cx[addr_mid] != 0) {
			switch(device_type_adrs_cx[addr_mid]) {
			case TOWNS_MEMORY_TYPE_WAVERAM:
				if(d_pcm != NULL) {
					d_pcm->write_data8(addr, data);
				}
				break;
			case TOWNS_MEMORY_TYPE_DICTLEARN:
				if(d_cmos != NULL) {
					d_cmos->write_data8(addr, data);
				}
				break;
			case TOWNS_MEMORY_TYPE_FORBID:
			default:
				break;
			}
		}
		break;
	case 0x0d:
	case 0x0e:
		// ??
		break;
	case 0x0f:
		// ROM, maybe unable to write.
		break;
	}
}

uint32_t MEMORY::read_page0_8(uint32_t addr, int *wait)
{

	addr = addr & 0x000fffff;
	if(wait != NULL) *wait = mem_wait_val;
	if(addr < 0xc0000) {
		return page0[addr];
	} else if(addr < 0xc8000) {
		if(d_vram != NULL) {
			if(wait != NULL) *wait = vram_wait_val;
			return d_vram->read_plane_data8(addr 0x7fff);
		}
		return 0xff;
	} else if(addr < 0xd0000) {
		// MMIO, VRAM and ram.
		if(0xcff80 <= addr && addr < 0xcffe0) {
#ifdef _DEBUG_LOG
//			this->out_debug_log(_T("MR\t%4x\n"), addr);
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
	} else if(addr < 0xd8000) {
		if(!bankd0_dict) {
			return ram_0d0[addr - 0x0d0000];
			// RAM? DICT?
		} else {
			// DICT
			return dict_rom[addr - 0xd0000 + (((uint32_t)dict_bank) << 15))];
		}
	} else if(addr < 0xda000) {
		if(!bankd0_dict) {
			// RAM? DICT?
			return ram_0d0[addr - 0x0d0000];
		} else {
			// DICT
			if(d_cmos != NULL) return d_cmos->read_data8(addr);
			return;
		}
	} else if(addr < 0xf0000) {
		if(!bankd0_dict) {
			return ram_0d0[addr - 0x0d0000];
		}
		return 0xff;
	} else if(addr < 0xf8000) {
		return ram_0f0[addr - 0xf0000];
	} else if(addr < 0x100000) {
		if(bankf8_ram) {
			// RAM
			return ram_0f8[addr - 0xf8000];
		} else {
			// BOOT ROM(ro)
			return system_rom[addr - 0xf8000 + 0x18000];
		}
	}
	return 0xff;
}


uint32_t MEMORY::read_data8(uint32_t addr)
{
	int wait;
	return read_data8w(addr, &wait);
}

uint32_t MEMORY::read_data16(uint32_t addr)
{
	int wait;
	return read_data16w(addr, &wait);
}

uint32_t MEMORY::read_data32(uint32_t addr)
{
	int wait;
	return read_data32w(addr, &wait);
}

uint32_t MEMORY::read_data8w(uint32_t addr, int *wait)
{
	uint32_t addr_head = (addr & 0xf0000000) >> 28;
	uint32_t addr_mid;
	uint32_t addr_low;
	uint32_t ui;
	uint8_t *pp;
	
	if(wait != NULL) *wait = mem_wait_val;
	switch(addr_head) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		if(addr < 0x00100000) {
			return read_page0_8(addr, wait);
		} else {
			ui = (((addr - 0x00100000) & 0x3ff00000) >> 20);
			pp = extram_adrs[ui];
			if(pp != NULL) {
				addr_low = addr & 0x000fffff;
				return pp[addr_low];
			}
		}
		return 0xff;
		break;
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
		if(extio != NULL) return extio->read_data8(addr);
		break;
	case 0x08:
		if(d_vram != NULL) {
			if(wait != NULL) *wait = vram_wait_val;
			return d_vram->read_data8(addr);
		}
		break;
	case 0x09:
	case 0x0a:
	case 0x0b:
		return 0xff;
		// ??
		break;
	case 0x0c:
		addr_mid = (addr & 0x03fff000) >> 12 ;
		pp = read_bank_adrs_cx[addr_mid];
		if(pp != NULL) {
			addr_low = addr & 0x00000fff;
			return pp[addr_low];
		} else if(device_type_adrs_cx[addr_mid] != 0) {
			switch(device_type_adrs_cx[addr_mid]) {
			case TOWNS_MEMORY_TYPE_WAVERAM:
				if(d_pcm != NULL) {
					return d_pcm->read_data8((addr & 0x0ffe));
				}
				break;
			case TOWNS_MEMORY_TYPE_DICTLEARN:
				if(d_cmos != NULL) {
					return d_cmos->read_data8((addr & 0x0ffe));
				}
				break;
			case TOWNS_MEMORY_TYPE_FORBID:
			default:
				return 0xff;
				break;
			}
			return 0xff;
		} else {
			return 0xff;
		}
		break;
	case 0x0d:
	case 0x0e:
		// ??
		return 0xff;
		break;
	case 0x0f:
		// ROM, maybe unable to write.
		if(addr < 0xfffc0000) {
			return 0xff;
		} else {
			pp = system_rom;
			addr_low = addr - 0xfffc0000;
			return (uint32_t)pp[addr_low];
		}
		break;
	}
	return 0xff;
}

uint32_t MEMORY::read_data16w(uint32_t addr, int *wait)
{
	uint32_t addr_head = (addr & 0xf0000000) >> 28;
	uint32_t addr_mid;
	uint32_t addr_low;
	uint32_t ui;
	uint16_t *pp;

	
	if(wait != NULL) *wait = mem_wait_val;
	switch(addr_head) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		if(addr < 0x00100000) {
			return read_page0_16(addr, wait);
		} else {
			ui = (((addr - 0x00100000) & 0x3ff00000) >> 20);
			pp = (uint16_t *)extram_adrs[ui];
			if(pp != NULL) {
#ifdef __LITTLE_ENDIAN__
				addr_low = (addr & 0x000fffff) >> 1;
				return pp[addr_low];
#else
				pair_t d;
				uint8 *p8 = (uint8 *)pp;
				addr_low = addr & 0x000ffffe;
				d.read_2bytes_le_from(&(p8[addr_low]));
				return d.d;
#endif
			}
		}
		return 0xffffffff;
		break;
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
		if(extio != NULL) return extio->read_data16(addr);
		break;
	case 0x08:
		if(d_vram != NULL) {
			if(wait != NULL) *wait = vram_wait_val;
			return d_vram->read_data16(addr);
		}
		break;
	case 0x09:
	case 0x0a:
	case 0x0b:
		return 0xffffffff;
		// ??
		break;
	case 0x0c:
		addr_mid = (addr & 0x03fff000) >> 12 ;
		pp = (uint16_t *)read_bank_adrs_cx[addr_mid];
		if(pp != NULL) {
#ifdef __LITTLE_ENDIAN__
				addr_low = (addr & 0x00000fff) >> 1;
				return pp[addr_low];
#else
				pair_t d;
				uint8 *p8 = (uint8 *)pp;
				addr_low = addr & 0x00000ffe;
				d.read_2bytes_le_from(&(p8[addr_low]));
				return d.d;
#endif
		} else if(device_type_adrs_cx[addr_mid] != 0) {
			switch(device_type_adrs_cx[addr_mid]) {
			case TOWNS_MEMORY_TYPE_WAVERAM:
				if(d_pcm != NULL) {
					return d_pcm->read_data8((addr & 0x0ffe));
				}
				break;
			case TOWNS_MEMORY_TYPE_DICTLEARN:
				if(d_cmos != NULL) {
					return d_cmos->read_data8((addr & 0x0ffe));
				}
				break;
			case TOWNS_MEMORY_TYPE_FORBID:
			default:
				return 0xffff;
				break;
			}
			return 0xffff;
		} else {
			return 0xffff;
		}
		break;
	case 0x0d:
	case 0x0e:
		// ??
		return 0xffff;
		break;
	case 0x0f:
		// ROM, maybe unable to write.
		if(addr < 0xfffc0000) {
			return 0xffff;
		} else {
#ifdef __LITTLE_ENDIAN__
			pp = (uint16_t *)system_rom;
			addr_low = (addr - 0xfffc0000) >> 1;
			return (uint32_t)pp[addr_low];
#else
			pair_t d;
			uint8_t *p8;
			addr_low = (addr - 0xfffc0000) & 0x3fffe;
			p8 = &(system_rom[addr_low]);
			d.read_2bytes_le_from(p8);
			return d.d;
#endif
		}
		break;
	}
	return 0xffff;
}

uint32_t MEMORY::read_data32w(uint32_t addr, int *wait)
{
	uint32_t addr_head = (addr & 0xf0000000) >> 28;
	uint32_t addr_mid;
	uint32_t addr_low;
	uint32_t ui;
	uint32_t *pp;

	if(wait != NULL) *wait = mem_wait_val;
	switch(addr_head) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		if(addr < 0x00100000) {
			return read_page0_32(addr);
		} else {
			ui = (((addr - 0x00100000) & 0x3ff00000) >> 20);
			pp = (uint32_t *)extram_adrs[ui];
			if(pp != NULL) {
#ifdef __LITTLE_ENDIAN__
				addr_low = (addr & 0x000fffff) >> 2;
				return pp[addr_low];
#else
				pair_t d;
				uint8 *p8 = (uint8 *)pp;
				addr_low = addr & 0x000ffffc;
				d.read_4bytes_le_from(&(p8[addr_low]));
				return d.d;
#endif
			}
		}
		return 0xffffffff;
		break;
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
		if(extio != NULL) return extio->read_data32(addr);
		break;
	case 0x08:
		if(d_vram != NULL) {
			if(wait != NULL) *wait = vram_wait_val;
			return d_vram->read_data32(addr);
		}
		break;
	case 0x09:
	case 0x0a:
	case 0x0b:
		return 0xffffffff;
		// ??
		break;
	case 0x0c:
		addr_mid = (addr & 0x03fff000) >> 12 ;
		pp = (uint32_t *)read_bank_adrs_cx[addr_mid];
		if(pp != NULL) {
#ifdef __LITTLE_ENDIAN__
				addr_low = (addr & 0x00000fff) >> 2;
				return pp[addr_low];
#else
				pair_t d;
				uint8 *p8 = (uint8 *)pp;
				addr_low = addr & 0x00000ffc;
				d.read_4bytes_le_from(&(p8[addr_low]));
				return d.d;
#endif
		} else if(device_type_adrs_cx[addr_mid] != 0) {
			switch(device_type_adrs_cx[addr_mid]) {
			case TOWNS_MEMORY_TYPE_WAVERAM:
				if(d_pcm != NULL) {
					return d_pcm->read_data8((addr & 0x0ffc));
				}
				break;
			case TOWNS_MEMORY_TYPE_DICTLEARN:
				if(d_cmos != NULL) {
					return d_cmos->read_data8((addr & 0x0ffc));
				}
				break;
			case TOWNS_MEMORY_TYPE_FORBID:
			default:
				return 0xffffffff;
				break;
			}
			return 0xffffffff;
		} else {
			return 0xffffffff;
		}
		break;
	case 0x0d:
	case 0x0e:
		// ??
		return 0xffffffff;
		break;
	case 0x0f:
		// ROM, maybe unable to write.
		if(addr < 0xfffc0000) {
			return 0xffffffff;
		} else {
#ifdef __LITTLE_ENDIAN__
			pp = (uint32_t *)system_rom;
			addr_low = (addr - 0xfffc0000) >> 2;
			return pp[addr_low];
#else
			pair_t d;
			uint8_t *p8;
			addr_low = (addr - 0xfffc0000) & 0x3fffc;
			p8 = &(system_rom[addr_low]);
			d.read_4bytes_le_from(p8);
			return d.d;
#endif
		}
		break;
	}
	return 0xffffffff;
}

void MEMORY::write_dma_data8(uint32_t addr, uint32_t data)
{
	int wait;
	write_data8w(addr & dma_addr_mask, data, &wait);
}

uint32_t MEMORY::read_dma_data8(uint32_t addr)
{
	int wait;
	return read_data8w(addr & dma_addr_mask, &wait);
}
void MEMORY::write_dma_data16(uint32_t addr, uint32_t data)
{
	int wait;
	write_data16w(addr & dma_addr_mask, data, &wait);
}

uint32_t MEMORY::read_dma_data16(uint32_t addr)
{
	int wait;
	return read_data16w(addr & dma_addr_mask, &wait);
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
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
		if(data & 0x80) {
			// NMI Vector protect
		}
		break;
	case 0x22:
		// Power off
		if(data & 0x40) {
			// power off
			emu->power_off();
		}
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
	case 0x480:
		bankf8_ram = false;
		if((data & 0x02) != 0) {
			bankf8_ram = true;
		}
		bankf8_dic = false;
		if((data & 0x01) != 0) {
			bankf8_dic = true;
		}
		break;
	case 0x484:
		dict_bank = data & 0x0f;
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

uint32_t MEMORY::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	
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
		// Towns
	case 0x480:
		return (bankf8_ram ? 0x02 : 0x00) | (bankf8_dic ? 0x01 : 0x00);
		break;
	case 0x484:
		return dict_bank & 0x0f;
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
		return dpal[addr & 7] | 0xf0;
	case 0xfda0:
		// status register
		return (disp ? 2 : 0) | (vsync ? 1 : 0) | 0xfc;
	}
	return 0xff;
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
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
		uint8_t bit = 0x80 >> (x & 7);
		for(int pl = 0; pl < 4; pl++) {
			uint8_t pbit = 1 << pl;
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
		scrntype_t* dest = emu->get_screen_buffer(y);
		uint8_t* txt = screen_txt[y];
		uint8_t* cg = screen_cg[y];
		
		for(int x = 0; x < SCREEN_WIDTH; x++) {
			dest[x] = txt[x] ? palette_txt[txt[x] & 15] : palette_cg[cg[x]];
		}
	}
	emu->screen_skip_line(false);
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
			uint8_t codel = cvram[src];
			uint8_t attrl = avram[src];
			src = (src + 1) & 0x1ffff;
			uint8_t codeh = cvram[src];
			uint8_t attrh = avram[src];
			src = (src + 1) & 0x1ffff;
			uint8_t col = (attrl & 15) | 16;
			bool blnk = (attrh & 0x40) || ((blink & 32) && (attrh & 0x10));
			bool rev = ((attrh & 8) != 0);
			uint8_t xor_mask = (rev != blnk) ? 0xff : 0;
			
			if(codeh & 0x80) {
				// kanji
				int ofs = (codel | ((codeh & 0x7f) << 8)) * 72;
				for(int l = 3; l < 27 && l < yofs; l++) {
					uint8_t pat0 = kanji24[ofs++] ^ xor_mask;
					uint8_t pat1 = kanji24[ofs++] ^ xor_mask;
					uint8_t pat2 = kanji24[ofs++] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 750) {
						break;
					}
					uint8_t* d = &screen_txt[yy][x * 14];
					
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
					uint8_t pat0 = ank24[ofs++] ^ xor_mask;
					uint8_t pat1 = ank24[ofs++] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 750) {
						break;
					}
					uint8_t* d = &screen_txt[yy][x * 14];
					
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
			uint8_t code = cvram[src];
			uint8_t h = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8_t attr = cvram[src];
			uint8_t l = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8_t col = ((attr & 0x20) >> 2) | (attr & 7) | 16;
			bool blnk = (blink & 32) && (attr & 0x10);
			bool rev = ((attr & 8) != 0);
			uint8_t xor_mask = (rev != blnk) ? 0xff : 0;
			
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
					uint8_t pat0 = kanji16[ofs + (l << 1) + 0] ^ xor_mask;
					uint8_t pat1 = kanji16[ofs + (l << 1) + 1] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 400) {
						break;
					}
					uint8_t* d = &screen_txt[yy][x << 4];
					
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
					uint8_t pat = ank16[(code << 4) + l] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 400) {
						break;
					}
					uint8_t* d = &screen_txt[yy][x << 4];
					
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
			uint8_t code = cvram[src];
			uint8_t h = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8_t attr = cvram[src];
			uint8_t l = kvram[src] & 0x7f;
			src = (src + 1) & 0xfff;
			uint8_t col = ((attr & 0x20) >> 2) | (attr & 7) | 16;
			bool blnk = (blink & 32) && (attr & 0x10);
			bool rev = ((attr & 8) != 0);
			uint8_t xor_mask = (rev != blnk) ? 0xff : 0;
			
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
					uint8_t pat0 = kanji16[ofs + (l << 1) + 0] ^ xor_mask;
					uint8_t pat1 = kanji16[ofs + (l << 1) + 1] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 400) {
						break;
					}
					uint8_t* d = &screen_txt[yy][x << 3];
					
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
					uint8_t pat = ank16[(code << 4) + l] ^ xor_mask;
					int yy = y * yofs + l;
					if(yy >= 400) {
						break;
					}
					uint8_t* d = &screen_txt[yy][x << 3];
					
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
	uint8_t* p0 = &vram[0x00000];
	uint8_t* p1 = &vram[0x20000];
	uint8_t* p2 = &vram[0x40000];
	uint8_t* p3 = &vram[0x60000];
	int ptr = 0;
	
	for(int y = 0; y < 750; y++) {
		for(int x = 0; x < 1120; x += 8) {
			uint8_t r = p0[ptr];
			uint8_t g = p1[ptr];
			uint8_t b = p2[ptr];
			uint8_t i = p3[ptr++];
			ptr &= 0x1ffff;
			uint8_t* d = &screen_cg[y][x];
			
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
		uint8_t* p0 = (dispctrl & 0x01) ? &vram[pofs | 0x00000] : dummy;
		uint8_t* p1 = (dispctrl & 0x02) ? &vram[pofs | 0x08000] : dummy;
		uint8_t* p2 = (dispctrl & 0x04) ? &vram[pofs | 0x10000] : dummy;
		uint8_t* p3 = (dispctrl & 0x20) ? &vram[pofs | 0x18000] : dummy;	// ???
		int ptr = dispaddr & 0x7ffe;
		
		for(int y = 0; y < 400; y++) {
			for(int x = 0; x < 640; x += 8) {
				uint8_t r = p0[ptr];
				uint8_t g = p1[ptr];
				uint8_t b = p2[ptr];
				uint8_t i = p3[ptr++];
				ptr &= 0x7fff;
				uint8_t* d = &screen_cg[y][x];
				
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
		uint8_t* p0 = (dispctrl & 0x01) ? &vram[pofs | 0x0000] : dummy;
		uint8_t* p1 = (dispctrl & 0x02) ? &vram[pofs | 0x4000] : dummy;
		uint8_t* p2 = (dispctrl & 0x04) ? &vram[pofs | 0x8000] : dummy;
		uint8_t* p3 = (dispctrl & 0x20) ? &vram[pofs | 0xc000] : dummy;	// ???
		int ptr = dispaddr & 0x3ffe;
		
		for(int y = 0; y < 400; y += 2) {
			for(int x = 0; x < 640; x += 8) {
				uint8_t r = p0[ptr];
				uint8_t g = p1[ptr];
				uint8_t b = p2[ptr];
				uint8_t i = p3[ptr++];
				ptr &= 0x3fff;
				uint8_t* d = &screen_cg[y][x];
				
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

#define STATE_VERSION	1

void MEMORY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
	state_fio->Fwrite(vram, sizeof(vram), 1);
	state_fio->Fwrite(cvram, sizeof(cvram), 1);
#ifdef _FMR60
	state_fio->Fwrite(avram, sizeof(avram), 1);
#else
	state_fio->Fwrite(kvram, sizeof(kvram), 1);
#endif
	state_fio->FputUint8(machine_id);
	state_fio->FputUint8(protect);
	state_fio->FputUint8(rst);
	state_fio->FputUint8(mainmem);
	state_fio->FputUint8(rplane);
	state_fio->FputUint8(wplane);
	state_fio->FputUint8(dma_addr_reg);
	state_fio->FputUint8(dma_wrap_reg);
	state_fio->FputUint32(dma_addr_mask);
	state_fio->FputBool(disp);
	state_fio->FputBool(vsync);
	state_fio->FputInt32(blink);
	state_fio->Fwrite(apal, sizeof(apal), 1);
	state_fio->FputUint8(apalsel);
	state_fio->Fwrite(dpal, sizeof(dpal), 1);
	state_fio->FputUint8(outctrl);
#ifndef _FMR60
	state_fio->FputUint8(pagesel);
	state_fio->FputUint8(ankcg);
	state_fio->FputUint8(dispctrl);
	state_fio->FputUint8(mix);
	state_fio->FputUint16(accaddr);
	state_fio->FputUint16(dispaddr);
	state_fio->FputInt32(kj_h);
	state_fio->FputInt32(kj_l);
	state_fio->FputInt32(kj_ofs);
	state_fio->FputInt32(kj_row);
	state_fio->FputUint8(cmdreg);
	state_fio->FputUint8(imgcol);
	state_fio->FputUint8(maskreg);
	state_fio->Fwrite(compreg, sizeof(compreg), 1);
	state_fio->FputUint8(compbit);
	state_fio->FputUint8(bankdis);
	state_fio->Fwrite(tilereg, sizeof(tilereg), 1);
	state_fio->FputUint16(lofs);
	state_fio->FputUint16(lsty);
	state_fio->FputUint16(lsx);
	state_fio->FputUint16(lsy);
	state_fio->FputUint16(lex);
	state_fio->FputUint16(ley);
#endif
	state_fio->Fwrite(palette_cg, sizeof(palette_cg), 1);
}

bool MEMORY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(ram, sizeof(ram), 1);
	state_fio->Fread(vram, sizeof(vram), 1);
	state_fio->Fread(cvram, sizeof(cvram), 1);
#ifdef _FMR60
	state_fio->Fread(avram, sizeof(avram), 1);
#else
	state_fio->Fread(kvram, sizeof(kvram), 1);
#endif
	machine_id = state_fio->FgetUint8();
	protect = state_fio->FgetUint8();
	rst = state_fio->FgetUint8();
	mainmem = state_fio->FgetUint8();
	rplane = state_fio->FgetUint8();
	wplane = state_fio->FgetUint8();
	dma_addr_reg = state_fio->FgetUint8();
	dma_wrap_reg = state_fio->FgetUint8();
	dma_addr_mask = state_fio->FgetUint32();
	disp = state_fio->FgetBool();
	vsync = state_fio->FgetBool();
	blink = state_fio->FgetInt32();
	state_fio->Fread(apal, sizeof(apal), 1);
	apalsel = state_fio->FgetUint8();
	state_fio->Fread(dpal, sizeof(dpal), 1);
	outctrl = state_fio->FgetUint8();
#ifndef _FMR60
	pagesel = state_fio->FgetUint8();
	ankcg = state_fio->FgetUint8();
	dispctrl = state_fio->FgetUint8();
	mix = state_fio->FgetUint8();
	accaddr = state_fio->FgetUint16();
	dispaddr = state_fio->FgetUint16();
	kj_h = state_fio->FgetInt32();
	kj_l = state_fio->FgetInt32();
	kj_ofs = state_fio->FgetInt32();
	kj_row = state_fio->FgetInt32();
	cmdreg = state_fio->FgetUint8();
	imgcol = state_fio->FgetUint8();
	maskreg = state_fio->FgetUint8();
	state_fio->Fread(compreg, sizeof(compreg), 1);
	compbit = state_fio->FgetUint8();
	bankdis = state_fio->FgetUint8();
	state_fio->Fread(tilereg, sizeof(tilereg), 1);
	lofs = state_fio->FgetUint16();
	lsty = state_fio->FgetUint16();
	lsx = state_fio->FgetUint16();
	lsy = state_fio->FgetUint16();
	lex = state_fio->FgetUint16();
	ley = state_fio->FgetUint16();
#endif
	state_fio->Fread(palette_cg, sizeof(palette_cg), 1);
	
	// post process
	update_bank();
	return true;
}

