/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.01 -

	[ memory]
*/

#include "./towns_memory.h"
#include "../i386.h"

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
	if(fio->Fopen(create_local_path(_T("FMT_DIC.ROM")), FILEIO_READ_BINARY)) { // MSDOS
		fio->Fread(dict_rom, sizeof(dict_rom), 1);
		fio->Fclose();
	}
	// ToDo: Will move to config.
	extram_size = TOWNS_EXTRAM_PAGES * 0x100000;
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

void TOWNS_MEMORY::write_data32w_page0(uint32_t addr, uint32_t data, int *wait)
{
	pair_t nd;

	nd.d = data;
	if((addr & 0xc8000) == 0xc8000) {
		if(d_mmio != NULL) d_mmio->write_data32(addr & 0x7ffc, data, wait);
	} else {	
		write_data8w_page0((addr & 0xffffc) + 0, nd.b.l, NULL);
		write_data8w_page0((addr & 0xffffc) + 1, nd.b.h, wait);
		write_data8w_page0((addr & 0xffffc) + 2, nd.b.h2, NULL);
		write_data8w_page0((addr & 0xffffc) + 3, nd.b.h3, NULL);
	}
}

void TOWNS_MEMORY::write_data16w_page0(uint32_t addr, uint32_t data, int *wait)
{
	pair_t nd;
	nd.d = data;
	if((addr & 0xc8000) == 0xc8000) {
		if(d_mmio != NULL) d_mmio->write_data32(addr & 0x7ffe, data, wait);
	} else {	
		write_data8w_page0((addr & 0xffffe) + 0, nd.b.l, NULL);
		write_data8w_page0((addr & 0xffffe) + 1, nd.b.h, wait);
	}
}

void TOWNS_MEMORY::write_data8w_page0(uint32_t addr, uint32_t data, int *wait)
{
	uint8_t naddr;
	naddr = (uint8_t)((addr >> 12) & 0xff);
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
	case 0xb: // FALLBACK
		{
			uint8_t *p = (uint8_t *)(&(ram_page0[addr & 0xfffff]));
			*p = data;
			if(wait != NULL) *wait = mem_wait_val;
		}
		break;
	case 0xc:
		if((addr & 0x0f000) < 0x08000) {
			d_vram->write_data8w((addr & 0x7fff) + TOWNS_VRAM_OFFSET_PLANE, data, wait);
		} else {
			d_mmio->write_data8w((addr & 0x7fff), data, wait);
		}
		break;
	case 0xd:
	case 0xe:
		{
			uint32_t maddr = addr & 0x1f000;
			if(maddr < 0x08000) {
				// NOOP.Dictionary ROM.
			} else if(maddr < 0x0a000) {
				if((addr & 0x7fff) < 0x0800) {
					uint8_t *p = (uint8_t *)(&(ram_cmos[addr & 0x07ff]));
					*p = data;
					if(wait != NULL) *wait = mem_wait_val; // OK?
				} else {
					d_vram->write_data8w((addr & 0x1fff) + TOWNS_VRAM_OFFSET_GAIJI, data, wait);
				}
						
			}
		}
		break;
	case 0xf:
		if(naddr < 0xf8) {
			uint8_t *p = (uint8_t *)(&(ram_0f0[addr & 0x7ff]));
			*p = data;
			if(wait != NULL) *wait = mem_wait_val; // OK?
		} else {
			if(!bootrom_selected) {
				uint8_t *p = (uint8_t *)(&(ram_0f8[addr & 0x7ff]));
				*p = data;
				if(wait != NULL) *wait = mem_wait_val; // OK?
			}			
		}
		break;
	}
}


uint32_t TOWNS_MEMORY::read_data32w_page0(uint32_t addr, int *wait)
{
	pair_t nd;

	nd.d = 0x00000000;
	
	if((addr & 0xc8000) == 0xc8000) {
		if(d_mmio != NULL) nd.d = d_mmio->read_data32(addr & 0x7ffc, wait);
	} else {	
		nd.b.l  = read_data8w_page0((addr & 0xffffc) + 0, NULL);
		nd.b.h  = read_data8w_page0((addr & 0xffffc) + 1,  wait);
		nd.b.h2 = read_data8w_page0((addr & 0xffffc) + 2, NULL);
		nd.b.h3 = read_data8w_page0((addr & 0xffffc) + 3, NULL);
	}
}

uint32_t TOWNS_MEMORY::read_data16w_page0(uint32_t addr, int *wait)
{
	pair_t nd;
	nd.d = 0x00000000;

	if((addr & 0xc8000) == 0xc8000) {
		if(d_mmio != NULL) nd.w.l = d_mmio->read_data16(addr & 0x7ffe, wait);
	} else {	
		nd.b.l = read_data8w_page0((addr & 0xffffe) + 0, NULL);
		nd.b.h = read_data8w_page0((addr & 0xffffe) + 1, wait);
	}
	return nd.d;
}

uint32_t TOWNS_MEMORY::read_data8w_page0(uint32_t addr, int *wait)
{
	uint8_t naddr;
	uint8_t retdata = 0xff;
	naddr = (uint8_t)((addr >> 12) & 0xff);
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
	case 0xb: // FALLBACK
		{
			uint8_t *p = (uint8_t *)(&(ram_page0[addr & 0xfffff]));
			retdata = *p;
			if(wait != NULL) *wait = mem_wait_val;
		}
		break;
	case 0xc:
		if((addr & 0x0f000) < 0x08000) {
			retdata = d_vram->read_data8w((addr & 0x7fff) + TOWNS_VRAM_OFFSET_PLANE, wait);
		} else {
			retdata = d_mmio->read_data8w((addr & 0x7fff), wait);
		}
		break;
	case 0xd:
	case 0xe:
		{
			uint32_t maddr = addr & 0x1f000;
			if(maddr < 0x08000) {
				// NOOP.Dictionary ROM.
				retdata = rom_dict[(addr & 0x07fff) + (dicrom_bank << 15)];
			} else if(maddr < 0x0a000) {
				if((addr & 0x7fff) < 0x0800) {
					uint32_t *p = (uint32_t *)(&(ram_cmos[addr & 0x07ff]));
					retdata = *p;
					if(wait != NULL) *wait = mem_wait_val; // OK?
				} else {
					retdata = d_vram->read_data32w((addr & 0x1fff) + TOWNS_VRAM_OFFSET_GAIJI, wait);
				}
						
			}
		}
		break;
	case 0xf:
		if(naddr < 0xf8) {
			uint32_t *p = (uint32_t *)(&(ram_0f0[addr & 0x7fff]));
			retdata = *p;
			if(wait != NULL) *wait = mem_wait_val; // OK?
		} else {
			if(!bootrom_selected) {
				uint32_t *p = (uint32_t *)(&(ram_0f8[addr & 0x7fff]));
				retdata = *p;
				if(wait != NULL) *wait = mem_wait_val; // OK?
			} else {
				uint32_t *p = (uint32_t *)(&(rom_system[(addr & 0x7fff) + 0x38000]));
				retdata = *p;
				if(wait != NULL) *wait = mem_wait_val; // OK?
			}				
		}
		break;
	}
	return retdata;
}

uint32_t TOWNS_MEMORY::read_extram_data8w(uint32_t addr, int*wait)
{
	uint8_t retval;
	if(extram == NULL) return 0xffffffff;
	if(addr >= (0x00100000 + (extram_size & 0x3ff00000))) {
		return 0xff;
	} else { // EXTRAM_OK
		retval = extram[(addr & 0xffffffff) - 0x00100000];
		if(wait != NULL) *wait = mem_wait_val;
		return (uint32_t)retval;
	}
}

uint32_t TOWNS_MEMORY::read_extram_data16w(uint32_t addr, int*wait)
{
	uint16_t retval;
	if(extram == NULL) return 0xffffffff;
	if(addr >= (0x00100000 + (extram_size & 0x3ff00000))) {
		return 0xffff;
	} else { // EXTRAM_OK
		uint16_t *p = (uint16_t *)(&(extram[(addr & 0xfffffffe) - 0x00100000]));
		retval = *p;
		if(wait != NULL) *wait = mem_wait_val;
		return (uint32_t)retval;
	}
}

uint32_t TOWNS_MEMORY::read_extram_data32w(uint32_t addr, int*wait)
{
	uint32_t retval;
	if(extram == NULL) return 0xffffffff;
	if(addr >= (0x00100000 + (extram_size & 0x3ff00000))) {
		return 0xffffffff;
	} else { // EXTRAM_OK
		uint32_t *p = (uint32_t *)(&(extram[(addr & 0xfffffffc) - 0x00100000]));
		retval = *p;
		if(wait != NULL) *wait = mem_wait_val;
		return (uint32_t)retval;
	}
}

uint32_t TOWNS_MEMORY::read_data32w(uint32_t addr, int* wait)
{
	uint8_t upper_bank = (addr & 0xf0000000) >> 28;
	uint8_t mid_bank   = (addr & 0x0f000000) >> 24;
	uint8_t low_bank =   (addr & 0x00ff0000) >> 16;
	addr = addr & 0xfffffffc;
	
	switch(upper_bank) {
	case 0:
		{
			if(mid_bank != 0) {
				return read_extram_data32w(addr, wait);
			} else {
				{
					if(low_bank < 0x10) {
						return read_data32w_page0(addr, wait);
					} else {
						return read_extram_data32w(addr, wait);
					}
				}
			}
		}
		break;
	case 1:
	case 2:
	case 3:
		return read_extram_data32w(addr, wait);
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		// I/O extension slot
		return 0xffffffff;
	case 8:
		// VRAM
		if(addr < 0x81000000) {
			if(d_vram != NULL) {
				return d_vram->read_data32w(addr, data, wait);
			}
			return 0xffffffff;
		} else {
			if(addr < 0x81020000) {
				if(d_sprite != NULL) {
					return d_sprite->read_data32w(addr, data, wait);
				}
				return 0xffffffff;
			} else {
				// Reserve
				return 0xffffffff;
			}
		}
		break;
	case 9:
	case a:
	case b:
		// VRAM: Reserved
		return 0xffffffff;
	case 0xc:
		if(addr < 0xc2000000) {
			uint32_t cardbank = ((addr & 0x01000000) >> 24);
			if(d_romcard[cardbank] != NULL) {
				d_romcard[cardbank]->read_data32w((addr & 0xffffff), wait);
			}
			return 0xffffffff;
		} else if(addr < 0xc2080000) {
			uint32_t *p = (uint32_t *)(&(rom_msdos[addr & 0x7ffff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return *p;
		} else if(addr < 0xc2100000) {
			uint32_t *p = (uint32_t *)(&(rom_dict[addr & 0x7ffff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return *p;
		} else if(addr < 0xc2140000) {
			uint32_t *p = (uint32_t *)(&(rom_font[addr & 0x4ffff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return *p;
		} else if(addr < 0xc2142000) {
			uint32_t *p = (uint32_t *)(&(ram_cmos[addr & 0x1fff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return *p;
		} else if(addr < 0xc2200000) {
			// Reserve
			return 0xffffffff;
		} else if(addr < 0xc2201000) {
			if(d_pcm != NULL) {
				return d_pcm->read_data32w((addr & 0x0fff), wait);
			}
			return 0xffffffff;
		} else {
			return 0xffffffff;
		}
		break;
	case 0xc:
	case 0xd:
	case 0xe:
		return 0xffffffff;
		break;
	case 0xf:
		if(addr < 0xfffc0000) {
			return 0xffffffff;
		} else {
			uint32_t *p = (uint32_t *)(&(rom_system[addr & 0x3ffff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return *p;
		}
		break;
	}
	return 0xffffffff;
}

uint32_t TOWNS_MEMORY::read_data16w(uint32_t addr, int* wait)
{
	uint8_t upper_bank = (addr & 0xf0000000) >> 28;
	uint8_t mid_bank   = (addr & 0x0f000000) >> 24;
	uint8_t low_bank =   (addr & 0x00ff0000) >> 16;
	addr = addr & 0xfffffffe;
	
	switch(upper_bank) {
	case 0:
		{
			if(mid_bank != 0) {
				return read_extram_data16w(addr, wait);
			} else {
				{
					if(low_bank < 0x10) {
						return read_data16w_page0(addr, wait);
					} else {
						return read_extram_data16w(addr, wait);
					}
				}
			}
		}
		return 0xffff;
		break;
	case 1:
	case 2:
	case 3:
		return read_extram_data16w(addr, wait);
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		// I/O extension slot
		return 0xffff;
	case 8:
		// VRAM
		if(addr < 0x81000000) {
			if(d_vram != NULL) {
				return d_vram->read_data16w(addr, data, wait);
			}
			return 0xffff;
		} else {
			if(addr < 0x81020000) {
				if(d_sprite != NULL) {
					return d_sprite->read_data16w(addr, data, wait);
				}
				return 0xffff;
			} else {
				// Reserve
				return 0xffff;
			}
		}
		break;
	case 9:
	case a:
	case b:
		// VRAM: Reserved
		return 0xffff;
	case 0xc:
		if(addr < 0xc2000000) {
			uint32_t cardbank = ((addr & 0x01000000) >> 24);
			if(d_romcard[cardbank] != NULL) {
				d_romcard[cardbank]->read_data16w((addr & 0xffffff), wait);
			}
			return 0xffff;
		} else if(addr < 0xc2080000) {
			uint16_t *p = (uint16_t *)(&(rom_msdos[addr & 0x7ffff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return *p;
		} else if(addr < 0xc2100000) {
			uint16_t *p = (uint16_t *)(&(rom_dict[addr & 0x7ffff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return (uint32_t)(*p);
		} else if(addr < 0xc2140000) {
			uint16_t *p = (uint16_t *)(&(rom_font[addr & 0x4ffff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return (uint32_t)(*p);
		} else if(addr < 0xc2142000) {
			uint16_t *p = (uint16_t *)(&(ram_cmos[addr & 0x1fff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return (uint32_t)(*p);
		} else if(addr < 0xc2200000) {
			// Reserve
			return 0xffff;
		} else if(addr < 0xc2201000) {
			if(d_pcm != NULL) {
				return d_pcm->read_data16w((addr & 0x0fff), wait);
			}
			return 0ffff;
		} else {
			return 0xffff;
		}
		break;
	case 0xc:
	case 0xd:
	case 0xe:
		return 0xffff;
		break;
	case 0xf:
		if(addr < 0xfffc0000) {
			return 0xffff;
		} else {
			uint16_t *p = (uint16_t *)(&(rom_system[addr & 0x3ffff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return (uint32_t)(*p);
		}
		break;
	}
	return 0xffff;
}

uint32_t TOWNS_MEMORY::read_data8w(uint32_t addr, int* wait)
{
	uint8_t upper_bank = (addr & 0xf0000000) >> 28;
	uint8_t mid_bank   = (addr & 0x0f000000) >> 24;
	uint8_t low_bank =   (addr & 0x00ff0000) >> 16;
	switch(upper_bank) {
	case 0:
		{
			if(mid_bank != 0) {
				return read_extram_data8w(addr, wait);
			} else {
				{
					if(low_bank < 0x10) {
						return read_data8w_page0(addr, wait);
					} else {
						return read_extram_data8w(addr, wait);
					}
				}
			}
		}
		return 0xff;
		break;
	case 1:
	case 2:
	case 3:
		return read_extram_data8w(addr, wait);
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		// I/O extension slot
		return 0xff;
	case 8:
		// VRAM
		if(addr < 0x81000000) {
			if(d_vram != NULL) {
				return d_vram->read_data8w(addr, data, wait);
			}
			return 0xff;
		} else {
			if(addr < 0x81020000) {
				if(d_sprite != NULL) {
					return d_sprite->read_data8w(addr, data, wait);
				}
				return 0xff;
			} else {
				// Reserve
				return 0xff;
			}
		}
		break;
	case 9:
	case a:
	case b:
		// VRAM: Reserved
		return 0xff;
	case 0xc:
		if(addr < 0xc2000000) {
			uint32_t cardbank = ((addr & 0x01000000) >> 24);
			if(d_romcard[cardbank] != NULL) {
				d_romcard[cardbank]->read_data8w((addr & 0xffffff), wait);
			}
			return 0xff;
		} else if(addr < 0xc2080000) {
			uint8_t *p = (uint8_t *)(&(rom_msdos[addr & 0x7ffff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return *p;
		} else if(addr < 0xc2100000) {
			uint8_t *p = (uint8_t *)(&(rom_dict[addr & 0x7ffff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return (uint32_t)(*p);
		} else if(addr < 0xc2140000) {
			uint8_t *p = (uint8_t *)(&(rom_font[addr & 0x4ffff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return (uint32_t)(*p);
		} else if(addr < 0xc2142000) {
			uint8_t *p = (uint8_t *)(&(ram_cmos[addr & 0x1fff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return (uint32_t)(*p);
		} else if(addr < 0xc2200000) {
			// Reserve
			return 0xff;
		} else if(addr < 0xc2201000) {
			if(d_pcm != NULL) {
				return d_pcm->read_data8w((addr & 0x0fff), wait);
			}
			return 0xff;
		} else {
			return 0xff;
		}
		break;
	case 0xc:
	case 0xd:
	case 0xe:
		return 0xff;
		break;
	case 0xf:
		if(addr < 0xfffc0000) {
			return 0xff;
		} else {
			uint8_t *p = (uint8_t *)(&(rom_system[addr & 0x3ffff]));
			if(wait != NULL) *wait = mem_wait_val; // OK?
			return (uint32_t)(*p);
		}
		break;
	}
	return 0xff;
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

void TOWNS_MEMORY::write_data8w(uint32_t addr, uint32_t data, int *wait)
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

void TOWNS_MEMORY::write_data16w(uint32_t addr, uint32_t data, int *wait)
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

void TOWNS_MEMORY::write_data32w(uint32_t addr, uint32_t data, int *wait)
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

uint32_t TOWNS_MEMORY::read_data8w(uint32_t addr, int *wait)
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

uint32_t TOWNS_MEMORY::read_data16w(uint32_t addr, int *wait)
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

uint32_t TOWNS_MEMORY::read_data32w(uint32_t addr, int *wait)
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

#define STATE_VERSION	1

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

