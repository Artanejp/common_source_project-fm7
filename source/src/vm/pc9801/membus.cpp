/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2017.06.22-

	[ memory bus ]
*/

#include "membus.h"
#include "display.h"

#ifdef _MSC_VER
	// Microsoft Visual C++
	#pragma warning( disable : 4065 )
#endif

/*
	NORMAL PC-9801
		00000h - 9FFFFh: RAM
		A0000h - A1FFFh: TEXT VRAM
		A2000h - A3FFFh: ATTRIBUTE
		A4000h - A4FFFh: CG WINDOW
		A8000h - BFFFFh: VRAM (BRG)
		C0000h - DFFFFh: EXT BIOS
			CC000h - CFFFFh: SOUND BIOS
			D6000h - D6FFFh: 2DD FDD BIOS
			D7000h - D7FFFh: 2HD FDD BIOS
			D7000h - D7FFFh: SASI BIOS
			D8000h - DBFFFh: IDE BIOS
			DC000h - DCFFFh: SCSI BIOS
		E0000h - E7FFFh: VRAM (I)
		E8000h - FFFFFh: BIOS

	HIRESO PC-98XA/XL/XL^2/RL
		00000h - 7FFFFh: RAM
		80000h - BFFFFh: MEMORY WINDOW
		C0000h - DFFFFh: VRAM
		E0000h - E1FFFh: TEXT VRAM
		E2000h - E3FFFh: ATTRIBUTE
		E4000h - E4FFFh: CG WINDOW
		F0000h - FFFFFh: BIOS
*/

namespace PC9801 {

// This code from NP2.cbus/sasibios.res
static const uint8_t pseudo_sasi_bios[] = {
			0xcb,0x90,0x90,0xcb,0x90,0x90,0xcb,0x90,0x90,0x55,0xaa,0x02,
			0xeb,0x22,0x90,0xeb,0x23,0x90,0xcb,0x90,0x90,0xeb,0x54,0x90,
			0xeb,0x33,0x90,0xcb,0x90,0x90,0xcb,0x90,0x90,0xcb,0x90,0x90,
			0xcb,0x90,0x90,0xcb,0x90,0x90,0xcb,0x90,0x90,0xcb,0x90,0x90,
			0xc6,0x07,0xd9,0xcb,0x8c,0xc8,0xc7,0x06,0x44,0x00,0x9d,0x00,
			0xa3,0x46,0x00,0x88,0x26,0xb0,0x04,0x88,0x26,0xb8,0x04,0xb8,
			0x00,0x03,0xcd,0x1b,0xcb,0xfc,0x8c,0xca,0x8e,0xda,0xb9,0x08,
			0x00,0xbe,0xcf,0x00,0xba,0xef,0x07,0xfa,0xac,0xee,0xe2,0xfc,
			0xfb,0x58,0x5b,0x59,0x5a,0x5d,0x07,0x5f,0x5e,0x1f,0xcf,0x3c,
			0x0a,0x74,0x05,0x3c,0x0b,0x74,0x01,0xcb,0x2c,0x09,0x84,0x06,
			0x5d,0x05,0x74,0x20,0xfe,0xc8,0xb4,0x06,0xb9,0xc0,0x1f,0x8e,
			0xc1,0x31,0xed,0xbb,0x00,0x04,0x31,0xc9,0x31,0xd2,0xcd,0x1b,
			0x72,0x0a,0x0c,0x80,0xa2,0x84,0x05,0x9a,0x00,0x00,0xc0,0x1f,
			0xcb,0x50,0xe4,0x82,0x24,0xfd,0x3c,0xad,0x74,0x06,0x24,0xf9,
			0x3c,0xa1,0x75,0x0f,0x1e,0x31,0xc0,0x8e,0xd8,0x80,0x0e,0x5f,
			0x05,0x01,0xb0,0xc0,0xe6,0x82,0x1f,0xb0,0x20,0xe6,0x08,0xb0,
			0x0b,0xe6,0x08,0xe4,0x08,0x84,0xc0,0x75,0x04,0xb0,0x20,0xe6,
			0x00,0x58,0xcf,0x73,0x61,0x73,0x69,0x62,0x69,0x6f,0x73,
};

	
void MEMBUS::initialize()
{
	MEMORY::initialize();
	
	// RAM
	memset(ram, 0x00, sizeof(ram));
#if !defined(SUPPORT_HIRESO)
	set_memory_rw(0x00000, 0x9ffff, ram);
#else
	set_memory_rw(0x00000, 0xbffff, ram);
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	if(sizeof(ram) > 0x100000) {
		set_memory_rw(0x100000, sizeof(ram) - 1, ram + 0x100000);
	}
#endif
#if defined(SUPPORT_32BIT_ADDRESS)
	is_shadow_bank_80000h = false;
	is_shadow_bank_a0000h = false;
	memset(shadow_bank_i386_80000h, 0x00, sizeof(shadow_bank_i386_80000h));
#endif
	// VRAM
#if !defined(SUPPORT_HIRESO)
	set_memory_mapped_io_rw(0xa0000, 0xa4fff, d_display);
	set_memory_mapped_io_rw(0xa8000, 0xbffff, d_display);
#if defined(SUPPORT_16_COLORS)
	set_memory_mapped_io_rw(0xe0000, 0xe7fff, d_display);
#endif
#else
	set_memory_mapped_io_rw(0xc0000, 0xe4fff, d_display);
#endif
	
	// BIOS
	memset(bios, 0xff, sizeof(bios));
	if(!read_bios(_T("IPL.ROM"), bios, sizeof(bios))) {
		read_bios(_T("BIOS.ROM"), bios, sizeof(bios));
	}
#if defined(SUPPORT_BIOS_RAM)
	memset(bios_ram, 0x00, sizeof(bios_ram));
	#if defined(SUPPORT_32BIT_ADDRESS) || defined(_PC9801RA)
	memset(shadow_ram, 0x00, sizeof(shadow_ram));
	#endif
	shadow_ram_selected = false;
#endif
#if defined(SUPPORT_ITF_ROM)
	memset(itf, 0xff, sizeof(itf));
	read_bios(_T("ITF.ROM"), itf, sizeof(itf));
#endif
	
#if !defined(SUPPORT_HIRESO)
	// EXT BIOS
#if defined(_PC9801) || defined(_PC9801E)
	memset(fd_bios_2hd, 0xff, sizeof(fd_bios_2hd));
	read_bios(_T("2HDIF.ROM"), fd_bios_2hd, sizeof(fd_bios_2hd));
	set_memory_r(0xd6000, 0xd6fff, fd_bios_2dd);
	
	memset(fd_bios_2dd, 0xff, sizeof(fd_bios_2dd));
	read_bios(_T("2DDIF.ROM"), fd_bios_2dd, sizeof(fd_bios_2dd));
	set_memory_r(0xd7000, 0xd7fff, fd_bios_2hd);
#endif
	memset(sound_bios, 0xff, sizeof(sound_bios));
//	memset(sound_bios_ram, 0x00, sizeof(sound_bios_ram));
	sound_bios_selected = false;
//	sound_bios_ram_selected = false;
	if(config.sound_type == 0) {
		sound_bios_selected = (read_bios(_T("SOUND.ROM"), sound_bios, sizeof(sound_bios)) != 0);
	} else if(config.sound_type == 2) {
		sound_bios_selected = (read_bios(_T("MUSIC.ROM"), sound_bios, sizeof(sound_bios)) != 0);
	}
	if(sound_bios_selected) {
		d_display->sound_bios_ok();
	}
	update_sound_bios();
#if defined(SUPPORT_SASI_IF)
	sasi_bios_load = false;
	memset(sasi_bios, 0xff, sizeof(sasi_bios));
	memset(sasi_bios_ram, 0x00, sizeof(sasi_bios_ram));
	sasi_bios_selected = (read_bios(_T("SASI.ROM"), sasi_bios, sizeof(sasi_bios)) != 0);
	if(sasi_bios_selected) {
		sasi_bios_load = true;
	} else {
		memcpy(sasi_bios, pseudo_sasi_bios, sizeof(pseudo_sasi_bios));
		sasi_bios_selected = true;
	}

	sasi_bios_ram_selected = false;
	update_sasi_bios();
#endif
#if defined(SUPPORT_SCSI_IF)
	memset(scsi_bios, 0xff, sizeof(scsi_bios));
	memset(scsi_bios_ram, 0x00, sizeof(scsi_bios_ram));
	scsi_bios_selected = (read_bios(_T("SCSI.ROM"), scsi_bios, sizeof(scsi_bios)) != 0);
	scsi_bios_ram_selected = false;
	update_scsi_bios();
#endif
#if defined(SUPPORT_IDE_IF)
	memset(ide_bios, 0xff, sizeof(ide_bios));
//	memset(ide_bios_ram, 0x00, sizeof(ide_bios_ram));
	ide_bios_selected = (read_bios(_T("IDE.ROM"), ide_bios, sizeof(ide_bios)) != 0);
//	ide_bios_ram_selected = false;
	update_ide_bios();
#endif
	page08_intram_selected = true;	
	// EMS
#if defined(SUPPORT_NEC_EMS)
	memset(nec_ems, 0, sizeof(nec_ems));
#endif
#endif
}

void MEMBUS::reset()
{
	MEMORY::reset();
	
	// BIOS/ITF
#if defined(SUPPORT_BIOS_RAM)
	bios_ram_selected = false;
	shadow_ram_selected = true;
#endif
#if defined(SUPPORT_ITF_ROM)
	itf_selected = true;
#endif
	update_bios();
	
#if !defined(SUPPORT_HIRESO)
	// EMS
#if defined(SUPPORT_NEC_EMS)
	nec_ems_selected = false;
	update_nec_ems();
#endif
#endif

	// SASI
#if defined(SUPPORT_SASI_IF)
	sasi_bios_selected = false;
	sasi_bios_ram_selected = false; // OK?
#endif
	// SASI
#if defined(SUPPORT_SCSI_IF)
	scsi_bios_selected = false;
	scsi_bios_ram_selected = false; // OK?
#endif
	// ToDo: IDE
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
#if !defined(SUPPORT_HIRESO)
	dma_access_ctrl = 0xfe; // bit2 = 1, bit0 = 0
	window_80000h = 0x80000;
	window_a0000h = 0xa0000;
#else
	dma_access_ctrl = 0xfb; // bit2 = 0, bit0 = 1
	window_80000h = 0x100000;
	window_a0000h = 0x120000;
#endif
#endif
#if defined(SUPPORT_32BIT_ADDRESS)
	is_shadow_bank_80000h = false;
	is_shadow_bank_a0000h = false;
#endif
	page08_intram_selected = true;	
}

void MEMBUS::write_io8(uint32_t addr, uint32_t data)
{
	//out_debug_log(_T("I/O WRITE $%04x %04x\n"), addr, data);
	switch(addr) {
#if defined(SUPPORT_ITF_ROM)
	case 0x043d:
		switch(data & 0xff) {
	#if defined(SUPPORT_HIRESO)
		case 0x00:
		case 0x18:
	#endif
		case 0x10:
			if(!itf_selected) {
				itf_selected = true;
				update_bios();
			}
			break;
	#if defined(SUPPORT_HIRESO)
		case 0x02:
	#endif
		case 0x12:
			if(itf_selected) {
				itf_selected = false;
				update_bios();
			}
			break;
		}
		break;
#endif
#if !defined(SUPPORT_HIRESO)
	case 0x043f:
		switch(data & 0xff) {
		case 0x20:
#if defined(SUPPORT_NEC_EMS)
			if(nec_ems_selected) {
				nec_ems_selected = false;
				update_nec_ems();
			}
#endif
			break;
		case 0x22:
#if defined(SUPPORT_NEC_EMS)
			if(!nec_ems_selected) {
				nec_ems_selected = true;
				update_nec_ems();
			}
#endif
			break;
		case 0xc0:
#if defined(SUPPORT_SASI_IF)
			// Changing ROM/RAM may select SASI I/F board.20190321 K.O
			if(sasi_bios_selected) {
				if(sasi_bios_ram_selected) {
					sasi_bios_ram_selected = false;
					update_sasi_bios();
				}
			}
#endif
#if defined(SUPPORT_SCSI_IF)
			// Changing ROM/RAM maybe select SCSI I/F board.(Still not tested)20190321 K.O 
			if(scsi_bios_selected) {
				if(scsi_bios_ram_selected) {
					scsi_bios_ram_selected = false;
					update_scsi_bios();
				}
			}
#endif
			break;
		case 0xc2:
#if defined(SUPPORT_SASI_IF)
			// Changing ROM/RAM may select SASI I/F board.20190321 K.O
			if(sasi_bios_selected) {
				if(!sasi_bios_ram_selected) {
					sasi_bios_ram_selected = true;
					update_sasi_bios();
				}
			}
#endif
			break;
		case 0xc4:
#if defined(SUPPORT_SCSI_IF)
			// Changing ROM/RAM maybe select SCSI I/F board.(Still not tested)20190321 K.O 
			if(scsi_bios_selected) {
				if(!scsi_bios_ram_selected) {
					scsi_bios_ram_selected = true;
					update_scsi_bios();
				}
			}
#endif
			break;
		case 0x80:
			page08_intram_selected = true;
			break;
		case 0x82:
			page08_intram_selected = false;
			break;
		}
		break;
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
#if !defined(_PC98XA)
	case 0x0439:
		dma_access_ctrl = data;
		break;
#endif
#if defined(SUPPORT_HIRESO)
	case 0x0091:
#if defined(SUPPORT_32BIT_ADDRESS)
		is_shadow_bank_80000h = ((data & 0xfe) == 0x08) ? true : false;
#endif
#if defined(_PC98XA)
		if(data < 0x10) {
			break;
		}
#endif
		// http://www.webtech.co.jp/company/doc/undocumented_mem/io_mem.txt
#if defined(SUPPORT_32BIT_ADDRESS)
		//if(!is_shadow_bank_80000h) {
			window_80000h = (data & 0xfe) << 16;
		//}
#else
		window_80000h = (data & 0xfe) << 16;
#endif
		break;
#endif
	case 0x0461:
		// http://www.webtech.co.jp/company/doc/undocumented_mem/io_mem.txt
		// ToDo: Cache flush for i486 or later.
#if defined(SUPPORT_32BIT_ADDRESS)
		is_shadow_bank_80000h = ((data & 0xfe) == 0x08) ? true : false;
		//if(!is_shadow_bank_80000h) {
			window_80000h = (data & 0xfe) << 16;
		//}
#else
		window_80000h = (data & 0xfe) << 16;
#endif
		break;
#if defined(SUPPORT_HIRESO)
	case 0x0093:
#if defined(SUPPORT_32BIT_ADDRESS)
		is_shadow_bank_a0000h = ((data & 0xfe) == 0x0a) ? true : false;
#endif
#if defined(_PC98XA)
		if(data < 0x10) {
			break;
		}
#endif
		// http://www.webtech.co.jp/company/doc/undocumented_mem/io_mem.txt
#if defined(SUPPORT_32BIT_ADDRESS)
		is_shadow_bank_a0000h = ((data & 0xfe) == 0x0a) ? true : false;
		//if(!is_shadow_bank_a0000h) {
			window_a0000h = (data & 0xfe) << 16;
		//}
#else
		window_a0000h = (data & 0xfe) << 16;
#endif
		break;
#endif
	case 0x0463:
		// http://www.webtech.co.jp/company/doc/undocumented_mem/io_mem.txt
		// ToDo: Cache flush for i486 or later.
#if defined(SUPPORT_32BIT_ADDRESS)
		is_shadow_bank_a0000h = ((data & 0xfe) == 0x0a) ? true : false;
		//if(!is_shadow_bank_a0000h) {
			window_a0000h = (data & 0xfe) << 16;
		//}
#else
		window_a0000h = (data & 0xfe) << 16;
#endif
		break;
#endif
#if defined(SUPPORT_32BIT_ADDRESS)
	case 0x053d:
#if !defined(SUPPORT_HIRESO)
		if(sound_bios_selected != ((data & 0x80) != 0)) {
			sound_bios_selected = ((data & 0x80) != 0);
			update_sound_bios();
		}
#if defined(SUPPORT_SASI_IF)
		if(sasi_bios_selected != ((data & 0x40) != 0)) {
			sasi_bios_selected = ((data & 0x40) != 0);
			update_sasi_bios();
		}
#endif
#if defined(SUPPORT_SCSI_IF)
		if(scsi_bios_selected != ((data & 0x20) != 0)) {
			scsi_bios_selected = ((data & 0x20) != 0);
			update_scsi_bios();
		}
#endif
#if defined(SUPPORT_IDE_IF)
		if(ide_bios_selected != ((data & 0x10) != 0)) {
			ide_bios_selected = ((data & 0x10) != 0);
			update_ide_bios();
		}
#endif
#endif
#if defined(SUPPORT_BIOS_RAM)
		if(shadow_ram_selected != ((data & 0x04) == 0)) {
			shadow_ram_selected = ((data & 0x04) == 0);
			update_bios();
		}
		if(bios_ram_selected != ((data & 0x02) != 0)) {
			bios_ram_selected = ((data & 0x02) != 0);
			update_bios();
		}
#endif
		break;
#endif
	// dummy for no cases
	default:
		break;
	}
}

uint32_t MEMBUS::read_io8(uint32_t addr)
{
	//out_debug_log(_T("I/O READ $%04x\n"), addr);
	switch(addr) {
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
#if !defined(_PC98XA)
	case 0x0439:
		return dma_access_ctrl;
#endif
#if !defined(SUPPORT_HIRESO)
	case 0x0461:
#else
	case 0x0091:
#endif
		return window_80000h >> 16;
#if !defined(SUPPORT_HIRESO)
	case 0x0463:
#else
	case 0x0093:
#endif
		return window_a0000h >> 16;
	case 0x0567:
		return (uint8_t)(sizeof(ram) >> 17);
#endif
	// dummy for no cases
	default:
		break;
	}
	return 0xff;
}

#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
#if !defined(SUPPORT_HIRESO)
	#define UPPER_MEMORY_24BIT	0x00fa0000
	#define UPPER_MEMORY_32BIT	0xfffa0000
#else
	#define UPPER_MEMORY_24BIT	0x00fc0000
	#define UPPER_MEMORY_32BIT	0xfffc0000
#endif

uint32_t MEMBUS::read_data8(uint32_t addr)
{
	if(addr < 0x80000) {
		return MEMORY::read_data8(addr);
	} else if(addr < 0xa0000) {
#if defined(SUPPORT_32BIT_ADDRESS)
		if(is_shadow_bank_80000h) {
			return (uint32_t)shadow_bank_i386_80000h[addr & 0x1ffff];
		}
#endif
		// ToDo: Correctness extra ram emulation.
		if(!page08_intram_selected) {
			return 0xff;
		}
		addr = (addr & 0x1ffff) | window_80000h;
	} else if(addr < 0xc0000) {
#if defined(SUPPORT_32BIT_ADDRESS)
		if(is_shadow_bank_a0000h) {
			return (uint32_t)shadow_bank_i386_80000h[(addr & 0x1ffff) + 0x20000];
		}
#endif
		addr = (addr & 0x1ffff) | window_a0000h;
	}
	if(addr < UPPER_MEMORY_24BIT) {
		return MEMORY::read_data8(addr);
#if defined(SUPPORT_24BIT_ADDRESS)
	} else {
#else
	} else if(addr < 0x1000000 || addr >= UPPER_MEMORY_32BIT) {
#endif
		return MEMORY::read_data8(addr & 0xfffff);
	}
	return 0xff;
}

void MEMBUS::write_data8(uint32_t addr, uint32_t data)
{
	if(addr < 0x80000) {
		MEMORY::write_data8(addr, data);
		return;
	} else if(addr < 0xa0000) {
#if defined(SUPPORT_32BIT_ADDRESS)
		if(is_shadow_bank_80000h) {
			shadow_bank_i386_80000h[addr & 0x1ffff] = data;
			return;
		}
#endif
		// ToDo: Correctness extra ram emulation.
		if(!page08_intram_selected) {
			return;
		}
		addr = (addr & 0x1ffff) | window_80000h;
	} else if(addr < 0xc0000) {
#if defined(SUPPORT_32BIT_ADDRESS)
		if(is_shadow_bank_a0000h) {
			shadow_bank_i386_80000h[(addr & 0x1ffff) + 0x20000] = data;
			return;
		}
#endif
		addr = (addr & 0x1ffff) | window_a0000h;
	}
	if(addr < UPPER_MEMORY_24BIT) {
		MEMORY::write_data8(addr, data);
#if defined(SUPPORT_24BIT_ADDRESS)
	} else {
#else
	} else if(addr < 0x1000000 || addr >= UPPER_MEMORY_32BIT) {
#endif
		MEMORY::write_data8(addr & 0xfffff, data);
	}
}

uint32_t MEMBUS::read_dma_data8(uint32_t addr)
{
	if(dma_access_ctrl & 4) {
		addr &= 0x000fffff;
	}
	return MEMBUS::read_data8(addr);
}

void MEMBUS::write_dma_data8(uint32_t addr, uint32_t data)
{
	if(dma_access_ctrl & 4) {
		addr &= 0x000fffff;
	}
	MEMBUS::write_data8(addr, data);
}
#endif

void MEMBUS::update_bios()
{
#if defined(SUPPORT_BIOS_RAM)
	//#if defined(_PC9801RA)
	//unset_memory_rw(0xe0000, 0xe7fff);
	//if(shadow_ram_selected) {
	//	set_memory_rw(0xe0000, 0xe7fff, shadow_ram);
	//} else {
	//	#if defined(SUPPORT_16_COLORS)
	//	set_memory_mapped_io_rw(0xe0000, 0xe7fff, d_display);
	//	#endif
	//}
	#if defined(SUPPORT_32BIT_ADDRESS)
	unset_memory_rw(0xc0000, 0xe7fff);
	if(shadow_ram_selected) {
		set_memory_rw(0xc0000, 0xe7fff, shadow_ram);
	} else {
		#if defined(SUPPORT_HIRESO)
		set_memory_mapped_io_rw(0xe0000, 0xe4fff, d_display);
		#else		
		#if defined(SUPPORT_16_COLORS)
		set_memory_mapped_io_rw(0xe0000, 0xe7fff, d_display);
		#endif
		update_sound_bios();
		#endif
		#if defined(SUPPORT_SASI_IF)
		update_sasi_bios();
		#endif
		#if defined(SUPPORT_IDE_IF)
		update_ide_bios();
		#endif
	}
	
	#endif
#endif
	unset_memory_rw(0x100000 - sizeof(bios), 0xfffff);
#if defined(SUPPORT_ITF_ROM)
	if(itf_selected) {
		set_memory_r(0x100000 - sizeof(itf), 0xfffff, itf);
	} else {
#endif
#if defined(SUPPORT_BIOS_RAM)

		if(bios_ram_selected) {
			set_memory_rw(0x100000 - sizeof(bios_ram), 0xfffff, bios_ram);
		} else {
#endif
			set_memory_r(0x100000 - sizeof(bios), 0xfffff, bios);
#if defined(SUPPORT_BIOS_RAM)
			set_memory_w(0x100000 - sizeof(bios_ram), 0xfffff, bios_ram);
		}
#endif
#if defined(SUPPORT_ITF_ROM)
	}
#endif
}

#if !defined(SUPPORT_HIRESO)
void MEMBUS::update_sound_bios()
{
	if(sound_bios_selected) {
//		if(sound_bios_selected) {
//			set_memory_r(0xcc000, 0xcffff, sound_bios_ram);
//		} else {
			set_memory_r(0xcc000, 0xcffff, sound_bios);
			unset_memory_w(0xcc000, 0xcffff);
//		}
	} else {
		unset_memory_rw(0xcc000, 0xcffff);
	}
}

#if defined(SUPPORT_SASI_IF)
void MEMBUS::update_sasi_bios()
{
	//out_debug_log(_T("SASI BIOS SELECTED: %s RAM=%s\n"), (sasi_bios_selected) ? _T("YES") : _T("NO"),
	//			  (sasi_bios_ram_selected) ? _T("YES") : _T("NO"));
	if(sasi_bios_selected) {
		if(sasi_bios_ram_selected) {
			set_memory_rw(0xd7000, 0xd7fff, sasi_bios_ram);
		} else {
			set_memory_r(0xd7000, 0xd7fff, sasi_bios);
			unset_memory_w(0xd7000, 0xd7fff);
		}
	} else {
		unset_memory_rw(0xd7000, 0xd7fff);
	}
}
#endif

#if defined(SUPPORT_SCSI_IF)
void MEMBUS::update_scsi_bios()
{
	if(scsi_bios_selected) {
		if(scsi_bios_ram_selected) {
			set_memory_rw(0xdc000, 0xdcfff, scsi_bios_ram);
		} else {
			set_memory_r(0xdc000, 0xdcfff, scsi_bios);
			unset_memory_w(0xdc000, 0xdcfff);
		}
	} else {
		unset_memory_rw(0xdc000, 0xdcfff);
	}
}
#endif

#if defined(SUPPORT_IDE_IF)
void MEMBUS::update_ide_bios()
{
	if(ide_bios_selected) {
//		if(ide_bios_selected) {
//			set_memory_r(0xd8000, 0xdbfff, ide_bios_ram);
//		} else {
			set_memory_r(0xd8000, 0xdbfff, ide_bios);
			unset_memory_w(0xd8000, 0xdbfff);
//		}
	} else {
		unset_memory_rw(0xd8000, 0xdbfff);
	}
}
#endif

#if defined(SUPPORT_NEC_EMS)
void MEMBUS::update_nec_ems()
{
	if(nec_ems_selected) {
		unset_memory_rw(0xb0000, 0xbffff);
		set_memory_rw(0xb0000, 0xbffff, nec_ems);
	} else {
		unset_memory_rw(0xb0000, 0xbffff);
		set_memory_mapped_io_rw(0xb0000, 0xbffff, d_display);
	}
}
#endif
#endif

#define STATE_VERSION	6

bool MEMBUS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateArray(ram, sizeof(ram), 1);
 #if defined(SUPPORT_BIOS_RAM)
	state_fio->StateArray(bios_ram, sizeof(bios_ram), 1);
	state_fio->StateValue(bios_ram_selected);
 #endif
 #if defined(SUPPORT_ITF_ROM)
	state_fio->StateValue(itf_selected);
 #endif
 #if !defined(SUPPORT_HIRESO)
//	state_fio->StateArray(sound_bios_ram, sizeof(sound_bios_ram), 1);
	state_fio->StateValue(sound_bios_selected);
//	state_fio->StateValue(sound_bios_ram_selected);
 #if defined(SUPPORT_SASI_IF)
	state_fio->StateArray(sasi_bios_ram, sizeof(sasi_bios_ram), 1);
	state_fio->StateValue(sasi_bios_selected);
	state_fio->StateValue(sasi_bios_ram_selected);
	state_fio->StateValue(sasi_bios_load);
 #endif
 #if defined(SUPPORT_SCSI_IF)
	state_fio->StateArray(scsi_bios_ram, sizeof(scsi_bios_ram), 1);
	state_fio->StateValue(scsi_bios_selected);
	state_fio->StateValue(scsi_bios_ram_selected);
 #endif
 #if defined(SUPPORT_IDE_IF)
-//	state_fio->StateBool(ide_bios_ram_selected);
//	state_fio->StateArray(ide_bios_ram, sizeof(ide_bios_ram), 1);
	state_fio->StateValue(ide_bios_selected);
//	state_fio->StateValue(ide_bios_ram_selected);
 #endif
 #if defined(SUPPORT_NEC_EMS)
	state_fio->StateArray(nec_ems, sizeof(nec_ems), 1);
	state_fio->StateValue(nec_ems_selected);
 #endif
 #endif
 #if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	state_fio->StateValue(dma_access_ctrl);
	state_fio->StateValue(window_80000h);
	state_fio->StateValue(window_a0000h);
 #endif
 #if defined(SUPPORT_32BIT_ADDRESS)
	state_fio->StateValue(is_shadow_bank_80000h);
	state_fio->StateValue(is_shadow_bank_a0000h);
	state_fio->StateArray(shadow_bank_i386_80000h, sizeof(shadow_bank_i386_80000h), 1);
 #endif
	state_fio->StateValue(page08_intram_selected);
 #if defined(SUPPORT_BIOS_RAM)
	#if defined(SUPPORT_32BIT_ADDRESS) || defined(_PC9801RA)
	state_fio->StateValue(shadow_ram_selected);
	state_fio->StateArray(shadow_ram, sizeof(shadow_ram), 1);
	#endif
 #endif
	if(!MEMORY::process_state(state_fio, loading)) {
 		return false;
 	}
 	
 	// post process
	if(loading) {
		update_bios();
#if !defined(SUPPORT_HIRESO)
		update_sound_bios();
#if defined(SUPPORT_SASI_IF)
		update_sasi_bios();
#endif
#if defined(SUPPORT_SCSI_IF)
		update_scsi_bios();
#endif
#if defined(SUPPORT_IDE_IF)
		update_ide_bios();
#endif
#if defined(SUPPORT_EMS)
		update_nec_ems();
#endif
#endif
	}
 	return true;
}

}
