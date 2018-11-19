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
	extra_nmi_mask = true;
	extra_nmi_val = false;
	
	vram_wait_val = 6;
	mem_wait_val = 3;
	machine_id = 0x0100;   // FM-Towns 1,2
	// machine_id = 0x0200 // FM-Towns  1F/2F/1H/2H
	// machine_id = 0x0300 // FM-Towns  UX10/UX20/UX40
	// machine_id = 0x0400 // FM-Towns  10F/20F/40H/80H
	// machine_id = 0x0500 // FM-Towns2 CX10/CX20/CX40/CX100
	// machine_id = 0x0600 // FM-Towns2 UG10/UG20/UG40/UG80
	// machine_id = 0x0700 // FM-Towns2 HR20/HR100/HR200
	// machine_id = 0x0800 // FM-Towns2 HG20/HG40/HG100
	// machine_id = 0x0900 // FM-Towns2 UR20/UR40/UR80
	// machine_id = 0x0b00 // FM-Towns2 MA20/MA170/MA340
	// machine_id = 0x0c00 // FM-Towns2 MX20/MX170/MX340
	// machine_id = 0x0d00 // FM-Towns2 ME20/ME170
	// machine_id = 0x0f00 // FM-Towns2 MF20/MF170/Fresh

	//cpu_id = 0x00; // 80286. 
	cpu_id = 0x01; // 80386DX. 
	//cpu_id = 0x02; // 80486SX/DX. 
	//cpu_id = 0x03; // 80386SX. 

		
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

bool TOWNS_MEMORY::bank_check(uint32_t addr, uint32_t *mask, uint32_t *offset, void** readfn, void** writefn, void** readp, void** writep)
{
	uint8_t __type = (uint8_t)(type_bank_adrs_cx[banknum] >> 24);
	uint32_t __offset = type_bank_adrs_cx[banknum] & 0x00ffffff;
	if(offset == NULL) return false;
	if(mask == NULL) return false;
	if(readfn == NULL) return false;
	if(writefn == NULL) return false;
	if(readp == NULL) return false;
	if(writep == NULL) return false;

	*readfn = NULL;
	*writefn = NULL;
	*readp = NULL;
	*writep = NULL;
	*mask = 0x00;
	*offset = 0;
	switch(__type) {
	case TOWNS_MEMORY_FMR_VRAM:
		if(!mainmem_enabled) {
			*readfn = (void *)d_vram;
			*writefn = (void *)d_vram;
			*offset = FMTOWNS_VRAM_PLANE_ACCESS;
			*mask = 0x7fff;
		} else {
			*readp = (void *)(&(ram_0c0[addr & 0x7fff]));
			*writep = (void *)(&(ram_0c0[addr & 0x7fff]));
			*mask = 0x7fff;
		}
		break;
	case TOWNS_MEMORY_FMR_TEXT:
		if(!mainmem_enabled) {
			if((addr & 0x1000) == 0) {
				*readfn = (void *)d_vram;
				*writefn = (void *)d_vram;
				*offset = FMTOWNS_VRAM_TEXT_VRAM;
				*mask = 0x0fff;
			} else {
				*mask = 0x1fff;
				*readp = (void*)(&(ram_0c8[addr & 0x1fff]));
				*writedp = (void*)(&(ram_0c8[addr & 0x1fff]));
			}						
		} else {
			*mask = 0x1fff;
			*readp = (void*)(&(ram_0c8[addr & 0x1fff]));
			*writedp = (void*)(&(ram_0c8[addr & 0x1fff]));
		}
		break;
	case TOWNS_MEMORY_MMIO_0CC:
		if(!mainmem_enabled) {
			if((addr & 0xfffff) < 0xcff80) {
				*mask = 0x3fff;
				*readp = (void*)(&(ram_0cc[addr & 0x3fff]));
				*writedp = (void*)(&(ram_0cc[addr & 0x3fff]));
			}
		} else {
			*mask = 0x3fff;
			*readp = (void*)(&(ram_0cc[addr & 0x3fff]));
			*writedp = (void*)(&(ram_0cc[addr & 0x3fff]));
		}
		break;
	case TOWNS_MEMORY_SPRITE_ANKCG1:
		if(!mainmem_enabled) {
			if((ankcg_enabled) && ((addr & 0xfff) < 0x800)) {
				*mask = 0x7ff;
				*readp = (void*)(&(rom_font1[(addr & 0x7ff) + 0x3d000]));
				*writep = (void*)(&(ram_0ca[addr & 0x7ff]));
			} else {
				*offset = 0x2000 + FMTOWNS_VRAM_TEXT_VRAM;
				*mask = 0xfff;
				*readfn = (void *)d_vram;
				*writefn = (void *)d_vram;
			}
		} else {
			*readp = (void*)(&(ram_0ca[addr & 0xfff]));
			*writep = (void*)(&(ram_0ca[addr & 0xfff]));
		}
		break;
	case TOWNS_MEMORY_SPRITE_ANKCG2:
		if(!(mainmem_enabled) && (ankcg_enabled)) {
			*readp = (void*)(&(rom_font1[(addr & 0x0fff) + 0x3d800]));
			*writep = (void*)(&(ram_0cb[addr & 0xfff]));
			*mask = 0x0fff;
		} else {
			*readp = (void*)(&(ram_0cb[addr & 0xfff]));
			*writep = (void*)(&(ram_0cb[addr & 0xfff]));
			*mask = 0xfff;
			*offset = 0;
		}
		break;
	case TOWNS_MEMORY_DICT_0D0:
		if(bank0_dict) {
			*readp = (void*)(&(dict_rom[(addr & 0x7fff) + (((uint32_t)(dict_bank & 0x0f)) << 15))]);
			*writep = (void*)(&(ram_0d0[addr & 0x7fff]));
		} else {
			*readp = (void*)(&(ram_0d0[addr & 0x7fff]));
			*writep = (void*)(&(ram_0d0[addr & 0x7fff]));
		}
		break;
	case TOWNS_MEMORY_CMOS_0D8:
		if(bank0_dict) {
			*offset = 0;
			*mask = 0x1fff;
			*readfn = (void *)d_cmos;
			*writefn = (void *)d_cmos;
		} else {
			*offset = 0;
			*mask = 0x1fff;
			*readp = (void*)(&(ram_0d8[addr & 0x1fff]));
			*writep = (void*)(&(ram_0d8[addr & 0x1fff]));
		}
		break;
				
	case TOWNS_MEMORY_PAGE0F8:
		if(bankf8_ram) {
			*offset = 0;
			*mask = 0x7fff;
			*readp = (void*)(&(ram_0f8[addr & 0x7fff]));
			*writep = (void*)(&(ram_0f8[addr & 0x7fff]));
		} else {
			*offset = 0;
			*mask = 0x7fff;
			*readp = (void*)(&rom_system[0x38000 + (addr & 0x7fff)]);
			*writep = (void*)(&(ram_0f8[addr & 0x7fff]));
		}
		break;
	default:
		return false;
		break;
	}
	return true;
	
}

uint32_t TOWNS_MEMORY::read_data8w(uint32_t addr, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	uint32_t banknum = addr >> 12;
	maddr = read_bank_adrs_cx[banknum];
	if(maddr != NULL)  {
		// Memory found.
		if(wait != NULL) *wait = mem_wait_val;
		uint8_t *p = &maddr[addr & 0x00000fff];
		return (uint32_t)(*p);
	} else {
		daddr = device_bank_adrs_cx[banknum];
		if(daddr != NULL) {
			// Device chained.
			return daddr->read_data8w(addr, wait);
		} else {
			uint32_t _mask;
			uint32_t _offset;
			DEVICE* readfn;
			DEVICE* writefn;
			uint8_t* readp;
			uint8_t* writep;
			if(bank_check(addr, &_mask, &_offset, (void**)(&readfn), (void**)(&writefn), (void**)(&readp), (void**)(&writep))) {
				if(readp != NULL) {
					if(wait != NULL) *wait = mem_wait_val;
					return (uint32_t)(*readp);
				} else if(readfn != NULL) {
					return readfn->read_data8w((addr & _mask) + _offset, wait);
				}
				// Function or memory don't exist this bank.
			} else {
				// Bank not registered.
				if((addr >= 0x000cff80) && (addr <= 0x000cffff)) {
					bool _hit;
					uint32_t val;
					val = read_mmio(addr, wait, &_hit);
					if(!_hit) {
						///
					} else {
						return val;
					}
				}
			}
		}
		// Neither memory nor device nor bank.
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
		// Memory found.
		if(wait != NULL) *wait = mem_wait_val;
		uint16_t *p = (uint16_t*)(&maddr[addr & 0x00000ffe]);
#if defined(__LITTLE_ENDIAN__)		
		return (uint32_t)(*p);
#else
		pair16_t d, n;
		d.w = *p;
		n.l  = d.h;
		n.h  = d.l;
		return (uint32_t)(n.w);
#endif
	} else {
		daddr = device_bank_adrs_cx[banknum];
		if(daddr != NULL) {
			// Device chained.
			return daddr->read_data16w(addr, wait);
		} else {
			uint32_t _mask;
			uint32_t _offset;
			DEVICE* readfn;
			DEVICE* writefn;
			uint16_t* readp;
			uint16_t* writep;
			if(bank_check(addr & 0xfffffffe, &_mask, &_offset, (void**)(&readfn), (void**)(&writefn), (void**)(&readp), (void**)(&writep))) {
				if(readp != NULL) {
					if(wait != NULL) *wait = mem_wait_val;
#if defined(__LITTLE_ENDIAN__)		
					return (uint32_t)(*readp);
#else
					pair16_t d, n;
					d.w = *readp;
					
					
				} else if(readfn != NULL) {
					return readfn->read_data16w((addr & _mask) + _offset, wait);
				}
				// Function or memory don't exist this bank.
			} else {
				// Bank not registered.
				if((addr >= 0x000cff80) && (addr <= 0x000cffff)) {
					bool _hit;
					uint32_t val;
					val = read_mmio(addr, wait, &_hit);
					if(!_hit) {
						///
					} else {
						return val;
					}
				}
			}
		}
		// Neither memory nor device nor bank.
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
		// Memory found.
		if(wait != NULL) *wait = mem_wait_val;
		uint32_t *p = (uint32_t*)(&maddr[addr & 0x00000ffc]);
#if defined(__LITTLE_ENDIAN__)		
		return (uint32_t)(*p);
#else
		pair32_t d, n;
		d.d = (uint32_t)*p;
		n.l  = d.h3;
		n.h  = d.h2;
		n.h2 = d.h;
		n.h3 = d.l;
		return n.d;
#endif
	} else {
		daddr = device_bank_adrs_cx[banknum];
		if(daddr != NULL) {
			// Device chained.
			return daddr->read_data32w(addr, wait);
		} else {
			uint32_t _mask;
			uint32_t _offset;
			DEVICE* readfn;
			DEVICE* writefn;
			uint32_t* readp;
			uint32_t* writep;
			if(bank_check(addr & 0xfffffffc, &_mask, &_offset, (void**)(&readfn), (void**)(&writefn), (void**)(&readp), (void**)(&writep))) {
				if(readp != NULL) {
					if(wait != NULL) *wait = mem_wait_val;
#if defined(__LITTLE_ENDIAN__)		
					return *readp;
#else
					pair32_t d, n;
					d.d = *readp;
					n.h2 = d.l;
					n.h1 = d.h;
					n.h  = d.h2;
					n.l  = d.h3;
					return n.d;
#endif
				} else if(readfn != NULL) {
					return readfn->read_data32w((addr & _mask) + _offset, wait);
				}
				// Function or memory don't exist this bank.
			} else {
				// Bank not registered.
				if((addr >= 0x000cff80) && (addr <= 0x000cffff)) {
					bool _hit;
					uint32_t val;
					val = read_mmio(addr, wait, &_hit);
					if(!_hit) {
						///
					} else {
						return val;
					}
				}
			}
			// Bank not registered.
		}
		// Neither memory nor device nor bank.
	}
	return 0xffffffff;
}

void TOWNS_MEMORY::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	uint32_t banknum = addr >> 12;
	maddr = write_bank_adrs_cx[banknum];
	if(maddr != NULL)  {
		// Memory found.
		if(wait != NULL) *wait = mem_wait_val;
		uint8_t *p = &maddr[addr & 0x00000fff];
		*p = (uint8_t)(data & 0x000000ff);
		return;
	} else {
		daddr = device_bank_adrs_cx[banknum];
		if(daddr != NULL) {
			// Device chained.
			daddr->write_data8w(addr, data, wait);
			return;
		} else {
			uint32_t _mask;
			uint32_t _offset;
			DEVICE* readfn;
			DEVICE* writefn;
			uint8_t* readp;
			uint8_t* writep;
			if(bank_check(addr, &_mask, &_offset, (void**)(&readfn), (void**)(&writefn), (void**)(&readp), (void**)(&writep))) {
				if(writep != NULL) {
					if(wait != NULL) *wait = mem_wait_val;
					*writep = (uint8_t)(data & 0x000000ff);
					return;
				} else if(writefn != NULL) {
					writefn->write_data8w((addr & _mask) + _offset, data, wait);
					return;
				}
				// Function or memory don't exist this bank.
			} else {
				// Bank not registered.
				if((addr >= 0x000cff80) && (addr <= 0x000cffff)) {
					bool _hit;
					write_mmio(addr, data, wait, &_hit);
					if(!_hit) {
						///
					} else {
						return val;
					}
				}
			}
			// Bank not registered.
		}
		// Neither memory nor device nor bank.
	}
	return;
}

void TOWNS_MEMORY::write_data16w(uint32_t addr, uint32_t data, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	uint32_t banknum = addr >> 12;
	maddr = write_bank_adrs_cx[banknum];
	if(maddr != NULL)  {
		// Memory found.
		if(wait != NULL) *wait = mem_wait_val;
#if defined(__LITTLE_ENDIAN__)		
		uint16_t *p = (uint16_t*)(&maddr[addr & 0x00000ffe]); // OK?
		*p = (uint16_t)(data & 0x0000ffff);
#else
		uint8_t *p = (uint8_t*)(&maddr[addr & 0x00000ffe]); // OK?
		pair16_t d;
		d.w = (uint16_t)(data & 0xffff);
		p[0] = d.b.l;
		p[1] = d.b.h;
#endif
		return;
	} else {
		daddr = device_bank_adrs_cx[banknum];
		if(daddr != NULL) {
			// Device chained.
			daddr->write_data16w(addr, data, wait);
			return;
		} else {
			uint32_t _mask;
			uint32_t _offset;
			DEVICE* readfn;
			DEVICE* writefn;
			uint16_t* readp;
			uint16_t* writep;
			if(bank_check(addr & 0xfffffffe, &_mask, &_offset, (void**)(&readfn), (void**)(&writefn), (void**)(&readp), (void**)(&writep))) {
				if(writep != NULL) {
					if(wait != NULL) *wait = mem_wait_val;
#if defined(__LITTLE_ENDIAN__)		
					*writep = (uint16_t)(data & 0x0000ffff);
#else
					uint8_t *p = (uint8_t*)writep;
					pair16_t d;
					d.w = (uint16_t)(data & 0xffff);
					p[0] = d.b.l;
					p[1] = d.b.h;
#endif
					return;
				} else if(writefn != NULL) {
					writefn->write_data16w((addr & _mask) + _offset, data, wait);
					return;
				}
				// Function or memory don't exist this bank.
			} else {
				// Bank not registered.
				if((addr >= 0x000cff80) && (addr <= 0x000cffff)) {
					bool _hit;
					write_mmio(addr, data, wait, &_hit);
					if(!_hit) {
						///
						return; // NOP?Exception?
					} else {
						return;
					}
				}
			}
			// Bank not registered.
		}
		// Neither memory nor device nor bank.
	}
	return;
}

void TOWNS_MEMORY::write_data32w(uint32_t addr, uint32_t data, int *wait)
{
	uint8_t *maddr;
	DEVICE *daddr;
	uint32_t banknum = addr >> 12;
	maddr = write_bank_adrs_cx[banknum];
	if(maddr != NULL)  {
		// Memory found.
		if(wait != NULL) *wait = mem_wait_val;
#if defined(__LITTLE_ENDIAN__)
		uint32_t *p = (uint32_t*)(&maddr[addr & 0x00000ffc]);
		*p = data;
#else
		uint8_t *p = (uint8_t*)(&maddr[addr & 0x00000ffc]);
		pair32_t d;
		d.d = data;
		p[0] = d.b.l;
		p[1] = d.b.h;
		p[2] = d.b.h2;
		p[3] = d.b.h3;
#endif		
		return;
	} else {
		daddr = device_bank_adrs_cx[banknum];
		if(daddr != NULL) {
			// Device chained.
			daddr->write_data32w(addr, data, wait);
			return;
		} else {
			uint32_t _mask;
			uint32_t _offset;
			DEVICE* readfn;
			DEVICE* writefn;
			uint32_t* readp;
			uint32_t* writep;
			if(bank_check(addr & 0xfffffffc, &_mask, &_offset, (void**)(&readfn), (void**)(&writefn), (void**)(&readp), (void**)(&writep))) {
				if(writep != NULL) {
					if(wait != NULL) *wait = mem_wait_val;
#if defined(__LITTLE_ENDIAN__)
					*writep = data;					
#else
					uint8_t *p = (uint8_t*)writep;
					pair32_t d;
					d.d = data;
					p[0] = d.b.l;
					p[1] = d.b.h;
					p[2] = d.b.h2;
					p[3] = d.b.h3;
#endif

					return;
				} else if(writefn != NULL) {
					writefn->write_data32w((addr & _mask) + _offset, data, wait);
					return;
				}
				// Function or memory don't exist this bank.
			} else {
				// Bank not registered.
				if((addr >= 0x000cff80) && (addr <= 0x000cffff)) {
					bool _hit;
					write_mmio(addr, data, wait, &_hit);
					if(!_hit) {
						///
					} else {
						return val;
					}
				}
			}
			// Bank not registered.
		}
		// Neither memory nor device nor bank.
	}
	return;
}



uint32_t TOWNS_MEMORY::read_data8(uint32_t addr)
{
	return read_data8w(addr, NULL);
}

uint32_t TOWNS_MEMORY::read_data16(uint32_t addr)
{
	return read_data16w(addr, NULL);
}

uint32_t TOWNS_MEMORY::read_data32(uint32_t addr)
{
	return read_data32w(addr, NULL);
}

void TOWNS_MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	return write_data8w(addr, data, NULL);
}

void TOWNS_MEMORY::write_data16(uint32_t addr, uint32_t data)
{
	return write_data16w(addr, data, NULL);
}

void TOWNS_MEMORY::write_data32(uint32_t addr, uint32_t data)
{
	return write_data32w(addr, data, NULL);
}

// Address (TOWNS BASIC):
// 0x0020 - 0x0022, 0x0030-0x0031,
// 0x0400 - 0x0404,
// 0x0480 - 0x0484
// Is set extra NMI (0x05c0 - 0x05c2)?
uint32_t TOWNS_MEMORY::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	switch(addr & 0xffff) {
	case 0x0020: // Software reset ETC.
		// reset cause register
		val = ((software_reset) ? 1 : 0) | ((d_cpu->get_shutdown_flag() != 0) ? 2 : 0);
		software_reset = false;
		d_cpu->set_shutdown_flag(0);
		val =  val | 0x7c;
		break;
	case 0x0022:
		// Power register
		val = 0xff;
		break;
	case 0x0030:
		val = (((machine_id & 0x1f) << 3) | (cpu_id & 7));
		// SPEED: bit0/Write
		break;
	case 0x0031:
		val = ((machine_id >> 5) & 0xff);
		break;
	case 0x006c: // Wait register.
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			val = 0x7f;
		}
		break;
	case 0x0400: // Resolution:
		val = 0xfe;
		break;
	case 0x0404: // System Status Reg.
		val = (bankc0_vram) ? 0x7f : 0xff;
		break;
	case 0x0480: // MEMORY BANK REGISTER
		val = val & ((bank0_dict) ? 0xff : 0xfe);
		val = val & ((bankf8_ram) ? 0xff : 0xfd);
		break;
	case 0x0484:
		val = (val & 0xf0 ) | (dict_bank & 0x0f);
		break;
	case 0x05c2:
		val = (extra_nmi_mask) ? 0xf7 : 0xff;
		break;
	case 0x05c2:
		val = (extra_nmi_val) ? 0xff : 0xf7;
		break;
	case 0x05e8:
		// After Towns1F/2F/1H/2H
		{
			switch(machine_id & 0xff00) {
			case 0x0000:
			case 0x0100:
				val = 0xff;
				break;
			case 0x0200:
			case 0x0300:
			case 0x0400:
			case 0x0500:
			case 0x0600:
			case 0x0700:
			case 0x0800:
			case 0x0a00:
				val = ((extram_size >> 20) & 0x1f);
				break;
			case 0x0b00:
			case 0x0c00:
			case 0x0d00:
			case 0x0f00:
				val = ((extram_size >> 20) & 0x7f);
				break;
			default:
				val = 0xff; // ???
				break;
			}
		}
		break;
	   
	default:
		break;
	}
	return val;
}

void TOWNS_MEMORY::write_io8(uint32_t addr, uint32_t data)
{

	switch(addr & 0xffff) {
	case 0x0020: // Software reset ETC.
		// reset cause register
		if((data & 0x80) != 0) {
			nmi_vector_protect = true;
		} else {
			nmi_vector_protect = false;
		}
		if((data & 0x01) != 0) {
			software_reset = true;
		} else {
			software_reset = false;
		}
		if((data & 0x40) != 0) {
			d_cpu->set_shutdown_flag(1);
			emu->power_off();
		}
		if(software_reset) {
			d_cpu->reset();
		}
		break;
	case 0x0022:
		if((data & 0x40) != 0) {
			d_cpu->set_shutdown_flag(1);
			emu->power_off;
		}
		// Power register
		break;
	case 0x006c: // Wait register.
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			register_event(this, EVENT_1US_WAIT, 1.0, false, &event_wait_1us);
			d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		}
		break;
	case 0x0404: // System Status Reg.
		bankc0_vram = ((data & 0x80) != 0);
		break;
	case 0x0480: // MEMORY BANK REGISTER
		bank0_dict = ((data & 0x01) != 0);
		bankf8_ram = ((data & 0x02) != 0);
		break;
	case 0x0484:
		dict_bank = data &  0x0f;
		break;
	default:
		break;
	}
	return;
}

void TOWNS_MEMORY::event_callback(int id, int err)
{
	switch(id) {
	case EVENT_1US_WAIT:
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		}
		break;
	default:
		break;
	}
	
}

uint32_t TOWNS_MEMORY::read_mmio(uint32_t addr, int *wait, bool *hit)
{
	if(hit != NULL) *hit = false;
	if(wait != NULL) *wait = 0; // OK?
	if(addr >= 0x000d0000) return 0xffffffff;
	if(addr <  0x000cff80) return 0xffffffff;
	uint32_t val = 0xff;
	bool found = false;
	switch(addr & 0x7f) {
	case 0x00:
		if(d_vram != NULL) {
			val = d_vram->read_io8(FMTOWNS_VRAM_IO_CURSOR);
			found = true;
		}
		break;
	case 0x01:
		if(d_vram != NULL) {
			val = d_vram->read_io8(FMTOWNS_VRAM_IO_FMR_RAMSELECT);
			found = true;
		}
		break;
	case 0x02:
		if(d_vram != NULL) {
			val = d_vram->read_io8(FMTOWNS_VRAM_IO_FMR_DISPMODE);
			found = true;
		}
		break;
	case 0x03:
		if(d_vram != NULL) {
			val = d_vram->read_io8(FMTOWNS_VRAM_IO_FMR_PAGESEL);
			found = true;
		}
		break;
	case 0x04:
		val = 0x7f; // Reserve.FIRQ
		found = true;
		break;
	case 0x06:
		if(d_vram != NULL) {
			val = d_vram->read_io8(FMTOWNS_VRAM_IO_SYNC_STATUS);
			found = true;
		}
		break;
	//case 0x14:
	//case 0x15:
	case 0x16:
	case 0x17:
		if(d_vram != NULL) {
			val = d_vram->read_io8(FMTOWNS_VRAM_KANJICG + (addr & 3));
			found = true;
		}
		break;
	case 0x18:
		if(d_beep != NULL) {
			d_beep->write_signal(SIG_BEEP_ON, 1, 1);
			found = true;
		}
		break;
	case 0x19:
		val = val & ((ankcg_enabled) ? 0x00 : 0x01);
		found = true;
		break;
	case 0x20:
		val = 0xff;
		val = val & 0x7f;
		found = true;
		break;
	default:
		break;
	}
	if(hit != NULL) *hit = found;
	return (uint32_t)val;
}

void TOWNS_MEMORY::write_mmio(uint32_t addr, uint32_t data, int *wait, bool *hit)
{
	if(hit != NULL) *hit = false;
	if(wait != NULL) *wait = 0; // OK?
	if(addr >= 0x000d0000) return;
	if(addr <  0x000cff80) return;
	bool found = false;
	switch(addr & 0x7f) {
	case 0x00:
		if(d_vram != NULL) {
			d_vram->write_io8(FMTOWNS_VRAM_IO_CURSOR, data);
			found = true;
		}
		break;
	case 0x01:
		if(d_vram != NULL) {
			d_vram->write_io8(FMTOWNS_VRAM_IO_FMR_RAMSELECT, data);
			found = true;
		}
		break;
	case 0x02:
		if(d_vram != NULL) {
			d_vram->write_io8(FMTOWNS_VRAM_IO_FMR_DISPMODE, data);
			found = true;
		}
		break;
	case 0x03:
		if(d_vram != NULL) {
			d_vram->write_io8(FMTOWNS_VRAM_IO_FMR_PAGESEL, data);
			found = true;
		}
		break;
	case 0x04:
		found = true;
		break;
	case 0x06:
		found = true;
		break;
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
		if(d_vram != NULL) {
			d_vram->write_io8(FMTOWNS_VRAM_KANJICG + (addr & 3), data);
			found = true;
		}
		break;
	case 0x18:
		if(d_beep != NULL) {
			d_beep->write_signal(SIG_BEEP_ON, 0, 1);
			found = true;
		}
		break;
	case 0x19:
	    ankcg_enabled = ((data & 1) == 0);
		found = true;
		break;
	case 0x20:
		found = true;
		break;
	default:
		break;
	}
	if(hit != NULL) *hit = found;
	return;
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
	
	for(uint32_t ui = 0x000c0; ui < 0x000c8; ui++) { // $000c0000 - $000effff
			type_bank_adrs_cx[ui] = (TOWNS_MEMORY_FMR_VRAM << 24) | ((ui - 0xc0) << 12);	
	}
	for(uint32_t ui = 0x000c8; ui < 0x000c9; ui++) { // $000c0000 - $000effff
			type_bank_adrs_cx[ui] = (TOWNS_MEMORY_FMR_TEXT << 24) | ((ui - 0xc8) << 12);	
	}
	for(uint32_t ui = 0x000c9; ui < 0x000ca; ui++) { // $000c0000 - $000effff
			type_bank_adrs_cx[ui] = (TOWNS_MEMORY_FMR_VRAM_RESERVE << 24) | ((ui - 0xc8) << 12);	
	}
	for(uint32_t ui = 0x000ca; ui < 0x000cb; ui++) { // $000c0000 - $000effff
			type_bank_adrs_cx[ui] = (TOWNS_MEMORY_SPRITE_ANKCG1 << 24) | ((ui - 0xca) << 12);	
	}
	for(uint32_t ui = 0x000cb; ui < 0x000cc; ui++) { // $000c0000 - $000effff
			type_bank_adrs_cx[ui] = (TOWNS_MEMORY_ANKCG2 << 24) | ((ui - 0xcb) << 12);	
	}
	for(uint32_t ui = 0x000cc; ui < 0x000d0; ui++) { // $000c0000 - $000effff
			type_bank_adrs_cx[ui] = (TOWNS_MEMORY_MMIO_0CC << 24) | ((ui - 0xcc) << 12);	
	}
	for(uint32_t ui = 0x000d0; ui < 0x000d8; ui++) { // $000c0000 - $000effff
			type_bank_adrs_cx[ui] = (TOWNS_MEMORY_DICT_0D0 << 24) | ((ui - 0xd0) << 12);	
	}
	for(uint32_t ui = 0x000d8; ui < 0x000da; ui++) { // $000c0000 - $000effff
			type_bank_adrs_cx[ui] = (TOWNS_MEMORY_CMOS_0D8 << 24) | ((ui - 0xd8) << 12);	
	}
	for(uint32_t ui = 0x000da; ui < 0x000f0; ui++) {  // $00000000 - $000bffff
		read_bank_adrs_cx[ui] = &(ram_0da[ui << 12]);
		write_bank_adrs_cx[ui] = &(ram_0da[ui << 12]);
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
	/*
	for(uint32_t ui = 0x000c0; ui < 0x000ca; ui++) {
		device_bank_adrs_cx[ui] = d_vram;
	}
	for(uint32_t ui = 0x000ca; ui < 0x000cb; ui++) {
		device_bank_adrs_cx[ui] = d_sprite;
	}
	*/
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
	for(uint32_t ui = 0xc2140; ui < 0xc2142; ui++) { 
		device_bank_adrs_cx[ui] = d_cmos;
	}
	for(uint32_t ui = 0xc2200; ui < 0xc2201; ui++) { 
		device_bank_adrs_cx[ui] = d_pcm;
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

	// SYSTEM CODE ROM
	for(uint32_t ui = 0xfffc0; ui < 0x100000; ui++) { 
		read_bank_adrs_cx[ui] = &(rom_system[(ui - 0xfffc0)  << 12]);
	}
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


void TOWNS_MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_EXTNMI) {
		extra_nmi_val = ((data & mask) != 0);
	} else if(id == SIG_CPU_NMI) {
		// Check protect
		d_cpu->write_signal(SIG_CPU_NMI, data, mask);
	} else if(id == SIG_CPU_IRQ) {
		d_cpu->write_signal(SIG_CPU_IRQ, data, mask);
	} else if(id == SIG_CPU_BUSREQ) {
		d_cpu->write_signal(SIG_CPU_BUSREQ, data, mask);
	} else if(id == SIG_I386_A20) {
		d_cpu->write_signal(SIG_I386_A20, data, mask);
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

