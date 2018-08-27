/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.01 -

	[ memory]
*/

#include "./towns_memory.h"
#include "i386.h"

void TOWNS_MEMORY::initialize()
{

	bankf8_ram = false;
	bankd0_dict = false;
	dict_bank = 0;
	
	vram_wait_val = 6;
	mem_wait_val = 3;

	memset(ram_page0, 0x00, sizeof(ram_page0));
	memset(ram_0f0, 0x00, sizeof(ram_0f0));
	memset(ram_0f8, 0x00, sizeof(ram_0f8));
	
	memset(rom_msdos, 0xff, sizeof(rom_msdos));
	memset(rom_font1, 0xff, sizeof(rom_font1));
#if 0
	memset(rom_font20, 0xff, sizeof(rom_font20));
#endif
	memset(rom_dict, 0xff, sizeof(rom_dict));
	memset(rom_system, 0xff, sizeof(rom_system));

	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_SYS.ROM")), FILEIO_READ_BINARY)) { // SYSTEM
		fio->Fread(rom_system, sizeof(rom_system), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("FMT_FNT.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(rom_font1, sizeof(rom_font1), 1);
		fio->Fclose();
	}
#if 0
	if(fio->Fopen(create_local_path(_T("FMT_F20.ROM")), FILEIO_READ_BINARY)) { // 20 pixels FONT : Optional
		fio->Fread(rom_font20, sizeof(rom_font20), 1);
		fio->Fclose();
	}
#endif
	if(fio->Fopen(create_local_path(_T("FMT_DOS.ROM")), FILEIO_READ_BINARY)) { // MSDOS
		fio->Fread(msdos_rom, sizeof(msdos_rom), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("FMT_DIC.ROM")), FILEIO_READ_BINARY)) { // DICTIONARIES
		fio->Fread(dict_rom, sizeof(dict_rom), 1);
		fio->Fclose();
	}
	// ToDo: Will move to config.
	extram_size = TOWNS_EXTRAM_PAGES * 0x100000;
	// ToDo: Limit extram_size per VM.
	extram = (uint8_t *)malloc(extram_size);

	initialize_tables();
}

void TOWNS_MEMORY::reset()
{
	// reset memory
	protect = rst = 0;
	// ToDo
	dma_addr_reg = dma_wrap_reg = 0;
	dma_addr_mask = 0x00ffffff;
	d_cpu->set_address_mask(0xffffffff);
}

uint32_t TOWNS_MEMORY::read_data8w(uint32_t addr, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	int udata;
	uint32_t mask;
	int wval;
	if(set_mapaddress(addr, false, &maddr, &daddr, &udata, &mask, &wval)) {
		if(maddr != NULL) {
			if(wait != NULL) *wait = wval;
			return (uint32_t)(*maddr);
		} else if(daddr != NULL) {
			return (uint32_t)(daddr->read_data8w((addr & mask) + (uint32_t)udata, wait));
		}
	}
	return 0xff;
}

uint32_t TOWNS_MEMORY::read_data16w(uint32_t addr, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	int udata;
	uint32_t mask;
	int wval;
	if(set_mapaddress(addr, false, &maddr, &daddr, &udata, &mask, &wval)) {
		if(maddr != NULL) {
			if(wait != NULL) *wait = wval;
			uiint16_t *p = (uint16_t *)maddr;
			return (uint32_t)(*p);
		} else if(daddr != NULL) {
			return (uint32_t)(daddr->read_data16w((addr & mask) + (uint32_t)udata, wait));
		}
	}
	return 0xffff;
}

uint32_t TOWNS_MEMORY::read_data32w(uint32_t addr, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	int udata;
	uint32_t mask;
	int wval;
	if(set_mapaddress(addr, false, &maddr, &daddr, &udata, &mask, &wval)) {
		if(maddr != NULL) {
			if(wait != NULL) *wait = wval;
			uiint32_t *p = (uint32_t *)maddr;
			return *p;
		} else if(daddr != NULL) {
			return (uint32_t)(daddr->read_data32w((addr & mask) + (uint32_t)udata, wait));
		}
	}
	return 0xffffffff;
}

void TOWNS_MEMORY::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	int udata;
	uint32_t mask;
	int wval;
	if(set_mapaddress(addr, true, &maddr, &daddr, &udata, &mask, &wval)) {
		if(maddr != NULL) {
			if(wait != NULL) *wait = wval;
			*maddr = (uint8_t)data;
		} else if(daddr != NULL) {
			daddr->write_data8w((addr & mask) + (uint32_t)udata, data, wait);
		}
	}
}


void TOWNS_MEMORY::write_data16w(uint32_t addr, uint32_t data, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	int udata;
	uint32_t mask;
	int wval;
	if(set_mapaddress(addr, true, &maddr, &daddr, &udata, &mask, &wval)) {
		if(maddr != NULL) {
			uint16_t *p = (uint16_t *)maddr;
			if(wait != NULL) *wait = wval;
			*p = (uint16_t)data;
		} else if(daddr != NULL) {
			daddr->write_data16w((addr & mask) + (uint32_t)udata, data, wait);
		}
	}
}

void TOWNS_MEMORY::write_data32w(uint32_t addr, uint32_t data, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	int udata;
	uint32_t mask;
	int wval
	if(set_mapaddress(addr, true, &maddr, &daddr, &udata, &mask, &wval)) {
		if(maddr != NULL) {
			uint32_t *p = (uint32_t *)maddr;
			if(wait != NULL) *wait = wval;
			*p = data;
		} else if(daddr != NULL) {
			daddr->write_data32w((addr & mask) + (uint32_t)udata, data, wait);
		}
	}
}


bool TOWNS_MEMORY::set_mapaddress(uint32_t addr, bool is_write, uint32_t **memory_address, DEVICE **device_address, int *userdata, uint32_t *maskdata, int *waitval)
{
	uint8_t *retaddr = NULL;
	DEVICE *devaddr = NULL;
	uint32_t mask = 0xffffffff;
	bool stat = false;
	int udata = 0;
	int wait = 0;
	uint8_t upper_bank = (addr & 0xf0000000) >> 28;
	uint8_t mid_bank   = (addr & 0x0f000000) >> 24;
	uint8_t low_bank =   (addr & 0x00ff0000) >> 16;
	
	switch(upper_bank) {
	case 0:
		{
			if(addr < 0x100000) {
				uint8_t naddr = (uint8_t)((addr >> 12) & 0xff);
				switch(naddr >> 4) {
				case 0x0:
				case 0x1:
				case 0x2:
				case 0x3:
				case 0x4:
				case 0x5:
				case 0x6:
				case 0x7:
				case 0x8:
				case 0x9:
				case 0xa:
				case 0xb:
					wait = mem_wait_val;
					retaddr = (uint8_t *)(&(ram_page0[addr & 0xfffff]));
					stat = true;
					break;
				case 0xc:
					if((addr & 0x0f000) < 0x08000) {
						mask = 0x00007fff;
						devaddr = d_vram;
						udata = TOWNS_VRAM_OFFSET_PLANE_BANKED;
					} else {
						mask = 0x00007fff;
						devaddr = d_mmio;
					}
					stat = true;
					break;
				case 0xd:
				case 0xe:
					{
						uint32_t maddr = addr & 0x1f000;
						if(maddr < 0x08000) {
							// Dictionary ROM.
							if(!is_write) {
								wait = vram_wait_val; // OK?
								retaddr = &(rom_dict[(addr & 0x07fff) + (dicrom_bank << 15)]);
								stat = true;
							}
						} else if(maddr < 0x0a000) {
							if((addr & 0x7fff) < 0x0800) {
								wait = mem_wait_val; // OK?
								if((addr & 0x01) == 0) {
									mask = 0x000007ff;
									retaddr = &(ram_cmos[addr & 0x07ff]);
									stat = true;
								}
							} else {
								wait = vram_wait_val; // OK?
								if((addr & 0x01) == 0) {
									devaddr = d_vram;
									udata = TOWNS_VRAM_OFFSET_GAIJI;
									mask = 0x0000ffff;
									stat = true;
								}
							}
						
						}
					}
					break;
				case 0xf:
					if(naddr < 0xf8) {
						wait = mem_wait_val; // OK?
						retaddr = &(ram_0f0[addr & 0x7fff]);
						stat = true;
					} else {
						if(!bootrom_selected) {
							wait = mem_wait_val; // OK?
							retaddr = &(ram_0f8[addr & 0x7fff]);
							stat = true;
						} else {
							wait = mem_wait_val; // OK?
							if(!is_write) {
								retaddr = &(rom_system[(addr & 0x7fff) + 0x38000]);
								stat = true;
							}
						}				
					}
					break;
				}
			} else {
				wait = mem_wait_val; // OK?
				if(extram_size >= (addr - 0x100000)) {
					retaddr = &(extram[addr - 0x100000]);
					stat = true;
				}
			}
		}
		break;
	case 1:
	case 2:
	case 3:
		if(extram_size >= (addr - 0x100000)) {
			wait = mem_wait_val; // OK?
			retaddr = &(extram[addr - 0x100000]);
			stat = true;
		}
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		// I/O extension slot
		udata = FMTOWNS_IO_EXPAND_SLOT;
		mask = 0x3fffffff;
		wait = mem_wait_val; // OK?
		retaddr = NULL;
		stat = true;
		break;
	case 8:
		// VRAM
		if(addr < 0x81000000) {
			if(d_vram != NULL) {
				devaddr = d_vram;
				mask = 0x000fffff; // OK?
				if(addr < 0x80100000) {
					udata = TOWNS_VRAM_OFFSET_SPLIT;
				} else if(addr < 0x80200000) {
					udata = TOWNS_VRAM_OFFSET_SINGLE;
				} else {
					udata = TOWNS_VRAM_OFFSET_UNDEFINED;
				}
				wait = vram_wait_val; // OK?
				stat = true;
			}
		} else if(addr < 0x81020000) {
			mask = 0x0001ffff; // OK?
			devaddr = d_sprite;
			stat = true;
		}
		break;
	case 9:
	case a:
	case b:
		// VRAM: Reserved

		break;
	case 0xc:
		if(addr < 0xc2000000) {
			uint32_t cardbank = ((addr & 0x01000000) >> 24);
			if(d_romcard[cardbank] != NULL) {
				wait = mem_wait_val; // OK?
				devaddr = d_romcard[cardbank];
				stat = true;
			}
		} else if(addr < 0xc2080000) {
			if(!is_write) {
				wait = mem_wait_val; // OK?
				retaddr = &(rom_msdos[addr & 0x7ffff]);
				stat = true;
			}
		} else if(addr < 0xc2100000) {
			if(!is_write) {
				wait = mem_wait_val; // OK?
				retaddr = &(rom_dict[addr & 0x7ffff]);
				stat = true;
			}
		} else if(addr < 0xc2140000) {
			if(!is_write) {
				wait = mem_wait_val; // OK?
				retaddr = &(rom_font[addr & 0x4ffff]);
				stat = true;
			}
		} else if(addr < 0xc2142000) {
			wait = mem_wait_val; // OK?
			retaddr = &(ram_cmos[addr & 0x1fff]);
			stat = true;
		} else if(addr < 0xc2200000) {
			// Reserve
		} else if(addr < 0xc2201000) {
			if(d_pcm != NULL) {
				devarrd = d_pcm;
				stat = true;
			}
		}
		break;
	case 0xc:
	case 0xd:
	case 0xe:
		break;
	case 0xf:
		if(addr >= 0xfffc0000) {
			if(!is_write) {
				wait = mem_wait_val; // OK?
				retaddr = &(rom_system[addr & 0x3ffff]);
				stat = true;
			}
		}
		break;
	}
	if(memory_address != NULL) *memory_address = retaddr;
	if(device_address != NULL) *device_address = devaddr;
	if(userdata != NULL) *userdata = udata;
	if(maskdata != NULL) *maskdata = mask;
	if(waitval != NULL) *waitval = wait;
	return stat;
}


uint32_t TOWNS_MEMORY::read_data32(uint32_t addr)
{
	return read_data32w(addr, NULL);
}

uint32_t TOWNS_MEMORY::read_data16(uint32_t addr)
{
	return read_data16w(addr, NULL);
}

uint32_t TOWNS_MEMORY::read_data8(uint32_t addr)
{
	return read_data8w(addr, NULL);
}

void TOWNS_MEMORY::initialize_tables(void)
{
	memset(extram_adrs, 0x00, sizeof(extram_adrs));
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

void TOWNS_MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_data8w(addr, data, &wait);
}

void TOWNS_MEMORY::write_data16(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_data16w(addr, data, &wait);
}

void TOWNS_MEMORY::write_data32(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_data32w(addr, data, &wait);
}

#if 0

// ToDo: Inter operability
uint32_t TOWNS_MEMORY::read_page0_8(uint32_t addr, int *wait)
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
#endif

uint32_t TOWNS_MEMORY::read_data8(uint32_t addr)
{
	int wait;
	return read_data8w(addr, &wait);
}

uint32_t TOWNS_MEMORY::read_data16(uint32_t addr)
{
	int wait;
	return read_data16w(addr, &wait);
}

uint32_t TOWNS_MEMORY::read_data32(uint32_t addr)
{
	int wait;
	return read_data32w(addr, &wait);
}

void TOWNS_MEMORY::write_dma_data8(uint32_t addr, uint32_t data)
{
	int wait;
	write_data8w(addr & dma_addr_mask, data, &wait);
}

uint32_t TOWNS_MEMORY::read_dma_data8(uint32_t addr)
{
	int wait;
	return read_data8w(addr & dma_addr_mask, &wait);
}
void TOWNS_MEMORY::write_dma_data16(uint32_t addr, uint32_t data)
{
	int wait;
	write_data16w(addr & dma_addr_mask, data, &wait);
}

uint32_t TOWNS_MEMORY::read_dma_data16(uint32_t addr)
{
	int wait;
	return read_data16w(addr & dma_addr_mask, &wait);
}

void TOWNS_MEMORY::write_io8(uint32_t addr, uint32_t data)
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

uint32_t TOWNS_MEMORY::read_io8(uint32_t addr)
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

void TOWNS_MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_DISP) {
		disp = ((data & mask) != 0);
	} else if(id == SIG_MEMORY_VSYNC) {
		vsync = ((data & mask) != 0);
	}
}

void TOWNS_MEMORY::update_dma_addr_mask()
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

#define STATE_VERSION	2

void TOWNS_MEMORY::decl_state()
{
}
void TOWNS_MEMORY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);

	state_fio->FputBool(bank8_ram);
	state_fio->FputBool(bankd0_dict);
	state_fio->FputUint8(dict_bank);
	state_fio->FputUint32_LE(extram_pages);

	state_fio->FputInt8((int8_t)vram_wait_val);
	state_fio->FputInt8((int8_t)mem_wait_val);
	state_fio->FputInt8((int8_t)extio_wait_val);
	
	// Save rom?
	state_fio->Fwrite(page0, sizeof(page0));
	state_fio->Fwrite(ram_0d0, sizeof(ram_0d0));
	state_fio->Fwrite(ram_0f0, sizeof(ram_0f0));
	state_fio->Fwrite(ram_0f8, sizeof(ram_0f8));
	
	// ROM?
	state_fio->Fwrite(msdos_rom, sizeof(msdos_rom));
	state_fio->Fwrite(dict_rom, sizeof(dict_rom));
	state_fio->Fwrite(font_rom, sizeof(font_rom));
	//state_fio->Fwrite(font_20_rom, sizeof(font_20_rom));
	state_fio->Fwrite(system_rom, sizeof(system_rom));
	
	state_fio->Fwrite(extram_base, extram_pages * 0x100000);
	
// ToDo
	state_fio->FputUint8(protect);
	state_fio->FputUint8(rst);
}

bool TOWNS_MEMORY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}

	bank8_tam = state_fio->FgetBool();
	bank0_dict = state_fio->FgetBool();
	dict_bank = state_fio->FgetUint8();
	extram_pages = state_fio->FgetUint32_LE();
	
	vram_wait_val = (int)state_fio->FgetInt8();
	mem_wait_val  = (int)state_fio->FgetInt8();
	extio_wait_val  = (int)state_fio->FgetInt8();

	// Save rom?
	state_fio->Fread(page0, sizeof(page0));
	state_fio->Fread(ram_0d0, sizeof(ram_0d0));
	state_fio->Fread(ram_0f0, sizeof(ram_0f0));
	state_fio->Fread(ram_0f8, sizeof(ram_0f8));
	
	// ROM?
	state_fio->Fread(msdos_rom, sizeof(msdos_rom));
	state_fio->Fread(dict_rom, sizeof(dict_rom));
	state_fio->Fread(font_rom, sizeof(font_rom));
	//state_fio->Fwrite(font_20_rom, sizeof(font_20_rom));
	state_fio->Fread(system_rom, sizeof(system_rom));
	
	uint8_t *pp;
	pp = malloc(extram_pages * 0x100000);
	if(pp == NULL) {
		return false;
	} else {
		state_fio->Fread(pp, extram_pages * 0x100000);
	}
	//ToDo
	protect = state_fio->FgetUint8();
	rst = state_fio->FgetUint8();
	
	
	// post process
	//update_bank();
	extram_base = pp;
	initialize_tables();
	
	return true;
}

