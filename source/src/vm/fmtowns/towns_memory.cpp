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
	// ToDo: More smart.
	vram_size = 0x80000; // OK?
	
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
	uint32_t banknum = addr >> 12;
	maddr = read_bank_adrs_cx[banknum];
	if(maddr != NULL)  {
		if(wait != NULL) *wait = mem_wait_val;
		uint8_t *p = &maddr[addr & 0x00000fff];
		return (uint32_t)(*p);
	} else {
		daddr = device_bank_adrs_cx[banknum];
		if(daddr != NULL) {
			return daddr->read_data8w(addr, wait);
		} else {
			uint8_t __type = (uint8_t)(type_bank_adrs_cx[banknum] >> 24);
			uint32_t __offset = type_bank_adrs_cx[banknum] & 0x00ffffff;
			uint8_t *pp = NULL;
			switch(__type) {
			case TOWNS_MEMORY_TYPE_PAGE0F8:
				if(bankf8_ram) {
					if(wait != NULL) *wait = mem_wait_val;
					pp = &ram_0f8[addr & 0x7fff];
				} else {
					if(wait != NULL) *wait = mem_wait_val; // IS WAIT VALUE RIGHT?
					pp = &rom_system[0x38000 + (addr & 0x7fff)];
				}
					
				break;
			default:
				if(wait != NULL) *wait = 0;
				pp = NULL;
				break;
			}
			if(pp != NULL) {
				return (uint32_t)(*pp);
			} else {
				return 0xff;
			}
		}
	}
	return 0xff;
}

uint32_t TOWNS_MEMORY::read_data16w(uint32_t addr, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	uint32_t banknum = addr >> 12;
	maddr = read_bank_adrs_cx[banknum];
	if(maddr != NULL)  {
		if(wait != NULL) *wait = mem_wait_val;
		uint16_t *p = (uint16_t*)(&maddr[addr & 0x00000fff]);
		return (uint32_t)(*p);
	} else {
		daddr = device_bank_adrs_cx[banknum];
		if(daddr != NULL) {
			return daddr->read_data16w(addr, wait);
		} else {
			uint8_t __type = (uint8_t)(type_bank_adrs_cx[banknum] >> 24);
			uint32_t __offset = type_bank_adrs_cx[banknum] & 0x00ffffff;
			uint16_t *pp = NULL;
			switch(__type) {
			case TOWNS_MEMORY_TYPE_PAGE0F8:
				if(bankf8_ram) {
					if(wait != NULL) *wait = mem_wait_val;
					pp = (uint16_t*)(&ram_0f8[addr & 0x7fff]);
				} else {
					if(wait != NULL) *wait = mem_wait_val; // IS WAIT VALUE RIGHT?
					pp = (uint16_t*)(&rom_system[0x38000 + (addr & 0x7fff)]);
				}
					
				break;
			default:
				if(wait != NULL) *wait = 0;
				pp = NULL;
				break;
			}
			if(pp != NULL) {
				return (uint32_t)(*pp);
			} else {
				return 0xffff;
			}
		}
	}
	return 0xffff;
}

uint32_t TOWNS_MEMORY::read_data32w(uint32_t addr, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	uint32_t banknum = addr >> 12;
	maddr = read_bank_adrs_cx[banknum];
	if(maddr != NULL)  {
		if(wait != NULL) *wait = mem_wait_val;
		uint32_t *p = (uint32_t*)(&maddr[addr & 0x00000fff]);
		return (uint32_t)(*p);
	} else {
		daddr = device_bank_adrs_cx[banknum];
		if(daddr != NULL) {
			return daddr->read_data16w(addr, wait);
		} else {
			uint8_t __type = (uint8_t)(type_bank_adrs_cx[banknum] >> 24);
			uint32_t __offset = type_bank_adrs_cx[banknum] & 0x00ffffff;
			uint32_t *pp = NULL;
			switch(__type) {
			case TOWNS_MEMORY_TYPE_PAGE0F8:
				if(bankf8_ram) {
					if(wait != NULL) *wait = mem_wait_val;
					pp = (uint32_t*)(&ram_0f8[addr & 0x7fff]);
				} else {
					if(wait != NULL) *wait = mem_wait_val; // IS WAIT VALUE RIGHT?
					pp = (uint32_t*)(&rom_system[0x38000 + (addr & 0x7fff)]);
				}
					
				break;
			default:
				if(wait != NULL) *wait = 0;
				pp = NULL;
				break;
			}
			if(pp != NULL) {
				return (uint32_t)(*pp);
			} else {
				return 0xffffffff;
			}
		}
	}
	return 0xffffffff;
}


void TOWNS_MEMORY::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	uint32_t banknum = addr >> 12;
	maddr = write_bank_adrs_cx[banknum];
	if(maddr != NULL) {
		if(wait != NULL) *wait = mem_wait_val;
		uint8_t *p = &maddr[addr & 0x00000fff];
		*p = (uint8_t)(data & 0x000000ff);
		return;
	} else {
		daddr = device_bank_adrs_cx[banknum];
		if(daddr != NULL) {
			return daddr->write_data8w(addr, data, wait);
		} else {
			uint8_t __type = (uint8_t)(type_bank_adrs_cx[banknum] >> 24);
			uint32_t __offset = type_bank_adrs_cx[banknum] & 0x00ffffff;
			uint8_t *pp = NULL;
			switch(__type) {
			case TOWNS_MEMORY_TYPE_PAGE0F8:
				if(bankf8_ram) {
					if(wait != NULL) *wait = mem_wait_val;
					pp = &ram_0f8[addr & 0x7fff];
				}					
				break;
			default:
				if(wait != NULL) *wait = 0;
				pp = NULL;
				break;
			}
			if(pp != NULL) {
				*pp = (uint8_t)(data & 0x000000ff);
				return;
			} else {
				return;
			}
		}
	}
}

void TOWNS_MEMORY::write_data16w(uint32_t addr, uint32_t data, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	uint32_t banknum = addr >> 12;
	maddr = write_bank_adrs_cx[banknum];
	if(maddr != NULL) {
		if(wait != NULL) *wait = mem_wait_val;
		uint16_t *p = (uint16_t*)(&maddr[addr & 0x00000fff]);
		*p = (uint16_t)(data & 0x0000ffff);
		return;
	} else {
		daddr = device_bank_adrs_cx[banknum];
		if(daddr != NULL) {
			return daddr->write_data16w(addr, data, wait);
		} else {
			uint8_t __type = (uint8_t)(type_bank_adrs_cx[banknum] >> 24);
			uint32_t __offset = type_bank_adrs_cx[banknum] & 0x00ffffff;
			uint8_t *pp = NULL;
			switch(__type) {
			case TOWNS_MEMORY_TYPE_PAGE0F8:
				if(bankf8_ram) {
					if(wait != NULL) *wait = mem_wait_val;
					pp = &ram_0f8[addr & 0x7fff];
				}					
				break;
			default:
				if(wait != NULL) *wait = 0;
				pp = NULL;
				break;
			}
			if(pp != NULL) {
				uint16_t *ppp = (uint16_t*)pp;
				*ppp = (uint16_t)(data & 0x0000ffff);
				return;
			} else {
				return;
			}
		}
	}
}

void TOWNS_MEMORY::write_data32w(uint32_t addr, uint32_t data, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	uint32_t banknum = addr >> 12;
	maddr = write_bank_adrs_cx[banknum];
	if(maddr != NULL) {
		if(wait != NULL) *wait = mem_wait_val;
		uint32_t *p = (uint32_t*)(&maddr[addr & 0x00000fff]);
		*p = data;
		return;
	} else {
		daddr = device_bank_adrs_cx[banknum];
		if(daddr != NULL) {
			return daddr->write_data32w(addr, data, wait);
		} else {
			uint8_t __type = (uint8_t)(type_bank_adrs_cx[banknum] >> 24);
			uint32_t __offset = type_bank_adrs_cx[banknum] & 0x00ffffff;
			uint8_t *pp = NULL;
			switch(__type) {
			case TOWNS_MEMORY_TYPE_PAGE0F8:
				if(bankf8_ram) {
					if(wait != NULL) *wait = mem_wait_val;
					pp = &ram_0f8[addr & 0x7fff];
				}					
				break;
			default:
				if(wait != NULL) *wait = 0;
				pp = NULL;
				break;
			}
			if(pp != NULL) {
				uint32_t *ppp = (uint32_t*)pp;
				*ppp = data;
				return;
			} else {
				return;
			}
		}
	}
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
	// Address Cx000000
	memset(write_bank_adrs_cx, 0x00, sizeof(write_bank_adrs_cx));
	memset(read_bank_adrs_cx, 0x00, sizeof(read_bank_adrs_cx));
	memset(device_bank_adrs_cx, 0x00, sizeof(device_bank_adrs_cx));
	memset(type_bank_adrs_cx, 0x00, sizeof(type_bank_adrs_cx));

	// PAGE0
	for(uint32_t ui = 0x00000; ui < 0x000c0; ui++) {  // $00000000 - $000bffff
		read_bank_adrs_cx[ui] = &(ram_page0[ui << 12]);
		write_bank_adrs_cx[ui] = &(ram_page0[ui << 12]);
	}
	for(uint32_t ui = 0x000c0; ui < 0x000f0; ui++) { // $000c0000 - $000effff
		type_bank_adrs_cx[ui] = (TOWNS_MEMORY_FMR_VRAM << 24) | ((ui - 0xc0) << 12);
	}
	for(uint32_t ui = 0x000f0; ui < 0x000f8; ui++) { // $000f0000 - $000f7fff
		read_bank_adrs_cx[ui] = &(ram_0f0[(ui - 0xf0)  << 12]);
		write_bank_adrs_cx[ui] = &(ram_0f0[(ui - 0xf0) << 12]);
	}
	for(uint32_t ui = 0x000f8; ui < 0x00100; ui++) { // $000c0000 - $000effff
		type_bank_adrs_cx[ui] = (TOWNS_MEMORY_PAGE0F8 << 24) | ((ui - 0xf8) << 12);
	}
	// Extra RAM
	for(uint32_t ui = 0x00100; ui < (0x00100 + (extram_size >> 12)); ui++) {
		read_bank_adrs_cx[ui] = &(extram[(ui - 0x100)  << 12]);
		write_bank_adrs_cx[ui] = &(extram[(ui - 0x100) << 12]);
	}
	// ToDo: EXTRA IO(0x40000000 - 0x80000000)
	
	// VRAM
	for(uint32_t ui = 0x80000; ui < (0x80000 + (vram_size >> 12)); ui++) {
		device_bank_adrs_cx[ui] = d_vram;
	}
	for(uint32_t ui = 0x80100; ui < (0x80100 + (vram_size >> 12)); ui++) {
		device_bank_adrs_cx[ui] = d_vram;
	}
	for(uint32_t ui = 0x81000; ui < 0x81020; ui++) {
		device_bank_adrs_cx[ui] = d_sprite;
	}

	// ROM CARD
	for(uint32_t ui = 0xc0000; ui < 0xc1000; ui++) {
		device_bank_adrs_cx[ui] = d_romcard[0];
	}
	// ROM CARD2
	for(uint32_t ui = 0xc1000; ui < 0xc2000; ui++) {
		device_bank_adrs_cx[ui] = d_romcard[1];
	}
	// ROMs
	for(uint32_t ui = 0xc2000; ui < 0xc2080; ui++) {
		read_bank_adrs_cx[ui] = &(rom_msdos[(ui - 0xc2000)  << 12]);
	}
	for(uint32_t ui = 0xc2080; ui < 0xc2100; ui++) {
		read_bank_adrs_cx[ui] = &(rom_dict[(ui - 0xc2080)  << 12]);
	}
	for(uint32_t ui = 0xc2100; ui < 0xc2140; ui++) {
		read_bank_adrs_cx[ui] = &(rom_font1[(ui - 0xc2100)  << 12]);
	}
	for(uint32_t ui = 0xc2140; ui < 0xc2142; ui++) { 
		type_bank_adrs_cx[ui] = (TOWNS_MEMORY_LEARN_RAM << 24) | ((ui - 0xc2140) << 12);
	}
	for(uint32_t ui = 0xc2200; ui < 0xc2201; ui++) { 
		device_bank_adrs_cx[ui] = d_pcm;
	}

	// SYSTEM CODE ROM
	for(uint32_t ui = 0xfffc0; ui < 0x100000; ui++) { 
		read_bank_adrs_cx[ui] = &(rom_system[(ui - 0xfffc0)  << 12]);
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

#include "../../statesub.h"

void TOWNS_MEMORY::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_BOOL(bankf8_ram);
	DECL_STATE_ENTRY_BOOL(bankd0_dict);
	DECL_STATE_ENTRY_UINT32(dicrom_bank);
	DECL_STATE_ENTRY_UINT8(machine_id);
	
	DECL_STATE_ENTRY_UINT32(vram_wait_val);
	DECL_STATE_ENTRY_UINT32(mem_wait_val);
	
	DECL_STATE_ENTRY_UINT32(extram_size);
	DECL_STATE_ENTRY_1D_ARRAY(ram_page0, sizeof(ram_page0));
	DECL_STATE_ENTRY_1D_ARRAY(ram_0f0, sizeof(ram_0f0));
	DECL_STATE_ENTRY_1D_ARRAY(ram_0f8, sizeof(ram_0f8));
	DECL_STATE_ENTRY_VARARRAY_VAR(extram, extram_size);
	DECL_STATE_ENTRY_1D_ARRAY(ram_cmos, sizeof(ram_cmos));

	
	leave_decl_state();
	
}
void TOWNS_MEMORY::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
}

bool TOWNS_MEMORY::load_state(FILEIO* state_fio)
{
	
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
		this->out_debug_log(_T("Load State: MAINMEM: id=%d stat=%s\n"), this_device_id, (mb) ? _T("OK") : _T("NG"));
		if(!mb) return false;
	}
	
	// post process
	//update_bank();
	initialize_tables();
	
	return mb;
}

