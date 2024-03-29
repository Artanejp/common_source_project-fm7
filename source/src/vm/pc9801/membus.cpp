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

#if defined(_PC9801RA) || defined(_PC9801RA2) || defined(_PC9801RA21) //defined(UPPER_I386)  && defined(SUPPORT_BIOS_RAM)
#define SUPPORT_SHADOW_RAM
#endif

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
 // Note: Penalty wait values is 4 when not hit accessble.(from upsteam 20221204).
void MEMBUS::initialize()
{
	MEMORY::initialize();

	// RAM
	memset(ram, 0x00, sizeof(ram));
	// VRAM
	gvram_wait_val = 0;
	tvram_wait_val = 0;

	// BIOS
	memset(bios, 0xff, sizeof(bios));
	if(!read_bios(_T("IPL.ROM"), bios, sizeof(bios))) {
		read_bios(_T("BIOS.ROM"), bios, sizeof(bios));
	}
	// Check PnP Bios and disable. From NP2 v0.83.
#if 0
#if !defined(SUPPORT_HIRESO) //&& defined(UPPER_I386)
	for(int ad = 0; ad < 0x10000; ad += 0x10) {
		pair32_t magic;
		magic.b.l  = bios[0x8000 + ad + 0];
		magic.b.h  = bios[0x8000 + ad + 1];
		magic.b.h2 = bios[0x8000 + ad + 2];
		magic.b.h3 = bios[0x8000 + ad + 3];
		if(magic.d == 0x506e5024) {
			out_debug_log(_T("Found PNP BIOS at %05X.Disable this.\n"), ad + 0xf0000);
			bios[0x8000 + ad + 0] = 0x6e;
			bios[0x8000 + ad + 2] = 0x24;
		}
	}
#endif
#endif
#if defined(SUPPORT_BIOS_RAM)
	shadow_ram_selected = true;
#else
	shadow_ram_selected = false;
#endif

#if defined(SUPPORT_ITF_ROM)
	memset(itf, 0xff, sizeof(itf));
	read_bios(_T("ITF.ROM"), itf, sizeof(itf));
//	memcpy(itf, pseudo_itfrom, (sizeof(itf) > sizeof(pseudo_itfrom)) ? sizeof(pseudo_itfrom) : sizeof(itf));

	itf_selected = true;
#endif

	// EXT BIOS
#if defined(_PC9801) || defined(_PC9801E)
	memset(fd_bios_2hd, 0xff, sizeof(fd_bios_2hd));
	if(config.dipswitch &  DIPSWITCH_2HD) {
		read_bios(_T("2HDIF.ROM"), fd_bios_2hd, sizeof(fd_bios_2hd));
	}
//	read_bios(_T("2HDIF.ROM"), fd_bios_2hd, sizeof(fd_bios_2hd));

	memset(fd_bios_2dd, 0xff, sizeof(fd_bios_2dd));
	if(config.dipswitch &  DIPSWITCH_2DD) {
		read_bios(_T("2DDIF.ROM"), fd_bios_2dd, sizeof(fd_bios_2dd));
	}
//	read_bios(_T("2DDIF.ROM"), fd_bios_2dd, sizeof(fd_bios_2dd));

 	set_memory_r(0xd6000, 0xd6fff, fd_bios_2dd);
 	set_memory_r(0xd7000, 0xd7fff, fd_bios_2hd);
#endif
	memset(sound_bios, 0xff, sizeof(sound_bios));
//	memset(sound_bios_ram, 0x00, sizeof(sound_bios_ram));
	sound_bios_selected = false;
	sound_bios_load = false;

//	memset(sound_bios_ram, 0x00, sizeof(sound_bios_ram));
//	sound_bios_ram_selected = false;
	if((config.sound_type == 0) || (config.sound_type == 1)) {
		out_debug_log(_T("Loading Sound BIOS \"SOUND.ROM\" type %d"), config.sound_type);
		sound_bios_load = (read_bios(_T("SOUND.ROM"), sound_bios, sizeof(sound_bios)) != 0);
	} else if((config.sound_type == 2) || (config.sound_type == 3)) {
		out_debug_log(_T("Loading Sound BIOS \"MUSIC.ROM\" type %d"), config.sound_type);
		sound_bios_load = (read_bios(_T("MUSIC.ROM"), sound_bios, sizeof(sound_bios)) != 0);
	}
	if(sound_bios_load) {
		if((config.sound_type & 1) == 0) {
			sound_bios_selected = true;
		}
		out_debug_log(_T("SUCCESS"));
	}
	if(sound_bios_selected) {
		d_display->set_memsw_4(d_display->get_memsw_4() |  8);
	} else {
		d_display->set_memsw_4(d_display->get_memsw_4() & ~8);
	}
#if defined(SUPPORT_SASI_IF)
	sasi_bios_load = false;
	memset(sasi_bios, 0xff, sizeof(sasi_bios));
	memset(sasi_bios_ram, 0x00, sizeof(sasi_bios_ram));
	sasi_bios_load = (read_bios(_T("SASI.ROM"), sasi_bios, sizeof(sasi_bios)) != 0);
	if(!(sasi_bios_load)) {
		memcpy(sasi_bios, pseudo_sasi_bios, sizeof(pseudo_sasi_bios));
	}
	sasi_bios_selected = true;
	sasi_bios_ram_selected = false;
#endif
#if defined(SUPPORT_SCSI_IF)
	memset(scsi_bios, 0xff, sizeof(scsi_bios));
	memset(scsi_bios_ram, 0x00, sizeof(scsi_bios_ram));
	scsi_bios_selected = (read_bios(_T("SCSI.ROM"), scsi_bios, sizeof(scsi_bios)) != 0);
	scsi_bios_ram_selected = false;
#endif
#if defined(SUPPORT_IDE_IF)
	memset(ide_bios, 0xff, sizeof(ide_bios));
	memset(ide_bios_ram, 0x00, sizeof(ide_bios_ram));
	ide_bios_selected = (read_bios(_T("IDE.ROM"), ide_bios, sizeof(ide_bios)) != 0);
	ide_bios_bank = 0;
#endif
//	page08_intram_selected = false;
	page08_intram_selected = true;
	// EMS
#if defined(SUPPORT_NEC_EMS)
	memset(nec_ems, 0, sizeof(nec_ems));
#endif
	// Belows are UGLY HACK from mame.
#if defined(_PC9801RA)
//	read_bios(_T("00000.ROM"), &(ram[0x00000]), 0x8000);
//	read_bios(_T("C0000.ROM"), &(ram[0xC0000]), 0x8000);
//	read_bios(_T("C8000.ROM"), &(ram[0xC8000]), 0x8000);
//	read_bios(_T("D0000.ROM"), &(ram[0xD0000]), 0x8000);
//	read_bios(_T("E8000.ROM"), &(bios_ram[0x00000]), 0x8000);
//	read_bios(_T("F0000.ROM"), &(bios_ram[0x08000]), 0x8000);
//	read_bios(_T("F8000.ROM"), &(bios_ram[0x10000]), 0x8000);
#endif
	last_access_is_interam = false;
	set_wait_rw(0x00000, (uint32_t)(space - 1), 4); // Initialize penalty.

	intram_wait = 0;
	bank08_wait = 0;

	exmem_wait = 0;
	slotmem_wait = 0;
	exboards_wait = 0;
	introm_wait = 0;

	config_intram();
	update_bios();

}

void MEMBUS::reset()
{
	MEMORY::reset();
	config_intram();
	// BIOS/ITF
#if defined(SUPPORT_BIOS_RAM)
	bios_ram_selected = false;
	shadow_ram_selected = true;
//	memcpy(bios_ram, bios, min((int)sizeof(bios), (int)sizeof(bios_ram))); // ;
#else
	shadow_ram_selected = false;
#endif

#if !defined(SUPPORT_HIRESO)
	if((sound_bios_load)  && ((config.sound_type & 1) == 0)){
		using_sound_bios = true;
		//d_display->sound_bios_ok();
		sound_bios_selected = true;
	} else {
		using_sound_bios = false;
		sound_bios_selected = false;
	}
#if defined(USE_SOUND_TYPE)
	if(config.sound_type == (USE_SOUND_TYPE - 1)) {
		sound_bios_selected = false;
		using_sound_bios = false;
	}
#endif
	// Re-Update Sound BIOS
	if(sound_bios_selected) {
		d_display->set_memsw_4(d_display->get_memsw_4() |  8);
	} else {
		d_display->set_memsw_4(d_display->get_memsw_4() & ~8);
	}

	//out_debug_log("SOUND BIOS=%s", (sound_bios_selected) ? "YES" : "NO");
	// EMS
#if defined(SUPPORT_NEC_EMS)
	nec_ems_selected = false;
	update_nec_ems();
	use_ems_as_protected = false; // OK?
	ems_protected_base = 0x00;
#endif
#endif

	// SASI
#if defined(SUPPORT_SASI_IF)
	sasi_bios_selected = true;
//	sasi_bios_selected = false;
	sasi_bios_ram_selected = false; // OK?
#endif
	// SASI
#if defined(SUPPORT_SCSI_IF)
	scsi_bios_selected = false;
	scsi_bios_ram_selected = false; // OK?
#endif
	// ToDo: IDE
#if defined(SUPPORT_IDE_IF)
	ide_bios_bank = 0; // ToDo: BANK
#endif
#if defined(SUPPORT_24BIT_ADDERSS) || defined(SUPPORT_32BIT_ADDRESS)
#if !defined(SUPPORT_HIRESO)
	dma_access_ctrl = 0xfe; // bit2 = 1, bit0 = 0
	dma_access_a20 = false;
//	dma_access_ctrl = 0xff; // bit2 = 1, bit0 = 0
//	dma_access_a20 = true;
	window_80000h = 0x80000;
	window_a0000h = 0xa0000;
#else
	dma_access_ctrl = 0xfb; // bit2 = 0, bit0 = 1
	dma_access_a20 = true;
	window_80000h = 0x100000;
	window_a0000h = 0x120000;
#endif
#endif
	page08_intram_selected = true;

	update_bios();
}

void MEMBUS::write_io8(uint32_t addr, uint32_t data)
{
	//out_debug_log(_T("I/O WRITE $%04x %04x\n"), addr, data);
	switch(addr) {
#if defined(SUPPORT_ITF_ROM)
	case 0x043d:
		switch(data & 0xff) {
	#if defined(SUPPORT_HIRESO)
		case 0x00: // HIRESO
		case 0x18: // H98S
	#endif
		case 0x10: // Normally BIOS, but, MENU ROM will be selected at H98S.
			if(!itf_selected) {
				itf_selected = true;
				update_bios();
			}
			break;
	#if defined(SUPPORT_HIRESO)
		case 0x02:
	#endif
		case 0x12: // BIOS ROM
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
				//update_bios();
			}
#else
//	#if !defined(SUPPORT_HIRESO)
//			unset_memory_rw(0xb0000, 0xbffff);
//			set_memory_mapped_io_rw(0xb0000, 0xbffff, d_display);
//			update_bios();
//	#endif
#endif
			break;
		case 0x22:
#if defined(SUPPORT_NEC_EMS)
			if(!nec_ems_selected) {
				nec_ems_selected = true;
				update_nec_ems();
//				update_bios();
			}
#else
//	#if !defined(SUPPORT_HIRESO)
//			unset_memory_rw(0xb0000, 0xbffff);
//			set_memory_rw(0xb0000, 0xbffff, &(ram[0xb0000]));
//			update_bios();
//	#endif
#endif
			break;
		case 0xc0:
#if defined(SUPPORT_SASI_IF)
			// Changing ROM/RAM may select SASI I/F board.20190321 K.O
			if(sasi_bios_selected) {
				if(sasi_bios_ram_selected) {
					sasi_bios_ram_selected = false;
					update_sasi_bios();
					//update_bios();
				}
			}
#endif
#if defined(SUPPORT_SCSI_IF)
			// Changing ROM/RAM maybe select SCSI I/F board.(Still not tested)20190321 K.O
			//if(scsi_bios_selected) {
				if(scsi_bios_ram_selected) {
					scsi_bios_ram_selected = false;
					update_scsi_bios();
					//update_bios();
				}
			//}
#endif
			break;
		case 0xc2:
#if defined(SUPPORT_SASI_IF)
			// Changing ROM/RAM may select SASI I/F board.20190321 K.O
			if(sasi_bios_selected) {
				if(!sasi_bios_ram_selected) {
					sasi_bios_ram_selected = true;
					update_sasi_bios();
					//update_bios();
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
					//update_bios();
				}
			}
#endif
			break;
		case 0x80:
			if(!(page08_intram_selected)) {
				page08_intram_selected = true;
//#if !defined(_PC9801RA)
				update_bios();
//#endif
			}
			break;
		case 0x82:
			if(page08_intram_selected) {
				page08_intram_selected = false;
//#if !defined(_PC9801RA)
				update_bios();
//#endif
			}
			break;
		}
		break;
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
#if !defined(_PC98XA)
	case 0x0439:
		dma_access_ctrl = data;
		dma_access_a20 = ((data & 0x04) != 0) ? false : true;
		break;
#endif
#if defined(SUPPORT_NEC_EMS)
	case 0x08e9:
		use_ems_as_protected = (data != 0) ? true: false;
		ems_protected_base = (uint32_t)(data & 0x0f) << 20;
		update_bios();
		break;
#endif
#if defined(SUPPORT_HIRESO)
	case 0x0091:
		{
			uint32_t _bak = window_80000h;
#if defined(_PC98XA)
			if(data < 0x10) {
				//update_bios();
				break;
			}
#endif
			// http://www.webtech.co.jp/company/doc/undocumented_mem/io_mem.txt
			window_80000h = (data & 0xfe) << 16;
			if(_bak != window_80000h) {
				update_bios();
			}
		}
		break;
#endif
#if 1
	case 0x0461:
		// http://www.webtech.co.jp/company/doc/undocumented_mem/io_mem.txt
		// ToDo: Cache flush for i486 or later.
		if(data >= 0x08) {
			uint32_t _bak = window_80000h;
			window_80000h = (data & 0xfe) << 16;
			if(_bak != window_80000h) {
				update_bios();
			}
		}
		break;
#endif
#if defined(SUPPORT_HIRESO)
	case 0x0093:
#if defined(_PC98XA)
		if(data < 0x10) {
			break;
		}
#endif
		// http://www.webtech.co.jp/company/doc/undocumented_mem/io_mem.txt
		{
			uint32_t _bak = window_a0000h;
			window_a0000h = (data & 0xfe) << 16;
			if(_bak != window_a0000h) {
				update_bios();
			}
		}
		break;
#endif
#if 1
	case 0x0463:
		// http://www.webtech.co.jp/company/doc/undocumented_mem/io_mem.txt
		// ToDo: Cache flush for i486 or later.
		if(data >= 0x08) {
			uint32_t _bak = window_a0000h;
			window_a0000h = (data & 0xfe) << 16;
			if(_bak != window_a0000h) {
				update_bios();
			}
		}
		break;
#endif
#endif
#if defined(UPPER_I386) // ToDo: Upper type
	case 0x053d:
		{
			bool result = false;
			bool _bak;
/*
  //Note: THIS is disabled due to enable bios at startup.
#if !defined(SUPPORT_HIRESO) // 20190521 K.O
			if(sound_bios_load && (using_sound_bios)) {
				_bak = sound_bios_selected;
				sound_bios_selected = ((data & 0x80) != 0);
				if(_bak != sound_bios_selected) result = true;
				out_debug_log("SOUND BIOS=%s (053Dh)", (sound_bios_selected) ? "YES" : "NO");
			}
#endif
*/
#if defined(SUPPORT_SASI_IF)
			{
				_bak = sasi_bios_selected;
				sasi_bios_selected = ((data & 0x40) != 0);
				if(_bak != sasi_bios_selected) result = true;
			}
#endif
#if defined(SUPPORT_SCSI_IF)
			{
				_bak = scsi_bios_selected;
				scsi_bios_selected = ((data & 0x20) != 0);
				if(_bak != scsi_bios_selected) result = true;
			}
#endif
#if defined(SUPPORT_IDE_IF)
			{
				_bak = ide_bios_selected;
				ide_bios_selected = ((data & 0x10) != 0);
				if(_bak != ide_bios_selected) result = true;
			}
#endif
			{
				_bak = shadow_ram_selected;
				shadow_ram_selected = ((data & 0x04) == 0);
				if(_bak != shadow_ram_selected) result = true;
			}
#if defined(SUPPORT_BIOS_RAM)
			{
				_bak = bios_ram_selected;
				bios_ram_selected = ((data & 0x02) != 0);
				if(_bak != bios_ram_selected) result = true;
			}
#endif
			if(result) update_bios();
		}
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
	case 0x0461: // ToDo: Some VMs enable to read value.
		return window_80000h >> 16;
//		return 0xff;
		break;
#else
	case 0x0091:
		return window_80000h >> 16;
		break;
#endif
#if !defined(SUPPORT_HIRESO)
	case 0x0463: // ToDo: Some VMs enable to read value.
		return window_a0000h >> 16;
//		return 0xff;
		break;
#else
	case 0x0093:
		return window_a0000h >> 16;
		break;
#endif
	case 0x0567:
		return (uint8_t)((sizeof(ram) - 0x100000) >> 17);
#endif
	// dummy for no cases
	default:
		break;
	}
	return 0xff;
}

#if defined(SUPPORT_32BIT_ADDRESS) || defined(SUPPORT_24BIT_ADDRESS)
#if !defined(SUPPORT_HIRESO)
	#define UPPER_MEMORY_24BIT	0x00fa0000
	#define UPPER_MEMORY_32BIT	0xfffa0000
#else
	#define UPPER_MEMORY_24BIT	0x00fc0000
	#define UPPER_MEMORY_32BIT	0xfffc0000
#endif

	// Note: Artane. variant of ePC9801xx changes bank dynamically.
	// Not need to translate addrsss.
	// - 20230323 K.O
inline bool MEMBUS::get_memory_addr(uint32_t *addr)
{
#if 0
	for(;;) {
		if(*addr < 0x80000) {
			return true;
		}
		if(*addr < 0xa0000) {
			__UNLIKELY_IF((*addr = (*addr & 0x1ffff) | window_80000h) >= UPPER_MEMORY_24BIT) {
				*addr &= 0xfffff;
			}
			return true;
		}
		if(*addr < 0xc0000) {
			__UNLIKELY_IF((*addr = (*addr & 0x1ffff) | window_a0000h) >= UPPER_MEMORY_24BIT) {
				*addr &= 0xfffff;
			}
			return true;
		}
		__LIKELY_IF(*addr < UPPER_MEMORY_24BIT) {
			return true;
		}
	#if defined(SUPPORT_32BIT_ADDRESS)
		__UNLIKELY_IF(*addr < 0x1000000 || *addr >= UPPER_MEMORY_32BIT) {
			*addr &= 0xfffff;
		} else {
			return false;
		}
	#else
		*addr &= 0xfffff;
	#endif
	}
#else
#endif
	return false;
}

// 4clk = wait when access memory on expansion board ???

#endif

// Note: Artane. variant of ePC9801xx changes bank dynamically.
// Not need to translate addrsss.
// - 20230323 K.O
uint32_t MEMBUS::read_dma_data8w(uint32_t addr, int *wait)
{
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	if(dma_access_ctrl & 4) {
		addr &= 0x000fffff;
	}
#endif
	return read_data8w(addr, wait);
}

void MEMBUS::write_dma_data8w(uint32_t addr, uint32_t data, int *wait)
{
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	if(dma_access_ctrl & 4) {
		addr &= 0x000fffff;
	}
#endif
	write_data8w(addr, data, wait);
}

uint32_t MEMBUS::read_dma_data16w(uint32_t addr, int *wait)
{
	__LIKELY_IF(!(addr & 1)) {
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
		if(dma_access_ctrl & 4) {
			addr &= 0x000fffff;
		}
#endif
		return read_data16w(addr, wait);
	} else {
		int wait_l = 0, wait_h = 0;
		uint32_t val;
		val  = read_dma_data8w(addr    , &wait_l);
		val |= read_dma_data8w(addr + 1, &wait_h) << 8;
		*wait = wait_l + wait_h;
		return val;
	}
}

void MEMBUS::write_dma_data16w(uint32_t addr, uint32_t data, int *wait)
{
	__LIKELY_IF(!(addr & 1)) {
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
		if(dma_access_ctrl & 4) {
			addr &= 0x000fffff;
		}
#endif
		write_data16w(addr, data, wait);
	} else {
		int wait_l = 0, wait_h = 0;
		write_dma_data8w(addr    , (data     ) & 0xff, &wait_l);
		write_dma_data8w(addr + 1, (data >> 8) & 0xff, &wait_h);
		*wait = wait_l + wait_h;
	}
}


void MEMBUS::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch) {
	case SIG_INTRAM_WAIT:
		intram_wait = (int)(data & 0xff);
		update_bios();
		break;
	case SIG_BANK08_WAIT:
		bank08_wait = (int)(data & 0xff);
		update_bios();
		break;
	case SIG_EXMEM_WAIT:
		exmem_wait = (int)(data & 0xff);
		update_bios();
		break;
	case SIG_SLOTMEM_WAIT:
		slotmem_wait = (int)(data & 0xff);
		update_bios();
		break;
	case SIG_EXBOARDS_WAIT:
		exboards_wait = (int)(data & 0xff);
		update_bios();
		break;
	case SIG_INTROM_WAIT:
		introm_wait = (int)(data & 0xff);
		update_bios();
		break;
	case SIG_GVRAM_WAIT:
		gvram_wait_val = (int)(data & 0xff);
		update_bios();
		break;
	case SIG_TVRAM_WAIT:
		tvram_wait_val = (int)(data & 0xff);
		update_bios();
		break;
	default:
		break;
	}
}


uint32_t MEMBUS::read_signal(int ch)
{
	switch(ch) {
	case SIG_LAST_ACCESS_INTERAM:
		return ((last_access_is_interam) ? 0xffffffff : 0x00000000);
		break;
	}
	return 0;
}

void MEMBUS::config_intram()
{
#if !defined(SUPPORT_HIRESO)
	// ToDo: Internal ROM wait value.
	#if defined(SUPPORT_32BIT_ADDRESS) || defined(SUPPORT_24BIT_ADDRESS)
	set_memory_rw(0x00000, (sizeof(ram) >= 0x80000) ? 0x7ffff :  (sizeof(ram) - 1), ram);
	set_wait_rw(0x00000, (sizeof(ram) >= 0x80000) ? 0x7ffff :  (sizeof(ram) - 1), intram_wait);
//	set_memory_rw(0x00000, (sizeof(ram) >= 0xa0000) ? 0x9ffff :  (sizeof(ram) - 1), ram);
	#else
	set_memory_rw(0x00000, (sizeof(ram) >= 0xc0000) ? 0xbffff :  (sizeof(ram) - 1), ram);
	set_wait_rw(0x00000, (sizeof(ram) >= 0xc0000) ? 0xbffff :  (sizeof(ram) - 1), intram_wait);
	#endif
#else
	set_memory_rw(0x00000, 0xbffff, ram);
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	set_wait_rw(0x00100000, 0x00efffff, 4);
	if(sizeof(ram) > 0x100000) {
		uint32_t _end_addr1 = (sizeof(ram) >= 0xe00000) ? 0xdfffff : (sizeof(ram) - 1);
		set_memory_rw(0x100000, _end_addr1, ram + 0x100000);
		set_wait_rw(0x100000, _end_addr1, exmem_wait);

		uint32_t _begin_addr2 = _end_addr1 + 1;
		unset_memory_rw(_begin_addr2, 0x00efffff);
	} else {
		unset_memory_rw(0x00100000, 0x00efffff);
	}
#endif
}

void MEMBUS::update_bios_mainmem()
{
#if !defined(SUPPORT_HIRESO)
	#if defined(SUPPORT_32BIT_ADDRESS)|| defined(SUPPORT_24BIT_ADDRESS)
//	unset_memory_rw(0x80000, 0xbffff);
	unset_memory_rw(0xa0000, 0xbffff);
	#endif
	set_memory_mapped_io_rw(0xa0000, 0xa4fff, d_display);
	set_memory_mapped_io_rw(0xa8000, 0xbffff, d_display);
	set_wait_rw(0xa0000, 0xa4fff, tvram_wait_val);
	set_wait_rw(0xa5000, 0xa7fff, 4); // Penalty values.
	set_wait_rw(0xa8000, 0xbffff, tvram_wait_val);
#else
//	unset_memory_rw(0xc0000, 0xe4fff);
//	set_wait_rw(0xc0000, 0xe4fff, 4); // Penalty values.
	set_memory_mapped_io_rw(0xc0000, 0xe4fff, d_display);
	set_wait_rw(0xc0000, 0xe4fff, gvram_wait_val); // OK?
	#if defined(SUPPORT_BIOS_RAM) && defined(SUPPORT_32BIT_ADDRESS)
	if(shadow_ram_selected) {
		set_memory_rw(0xc0000, 0xe7fff, &(ram[0xc0000])); // OK?
		set_wait_rw(0xc0000, 0xe7fff, intram_wait);
	}
	#endif
#endif
}

void MEMBUS::update_bios_extra_boards()
{
#if !defined(SUPPORT_HIRESO)
	{
		unset_memory_rw(0xc0000, 0xe7fff);
		set_wait_rw(0xc0000, 0xe7fff, 4);
		//#endif
		#if defined(_PC9801) || defined(_PC9801E)
		set_memory_r(0xd6000, 0xd6fff, fd_bios_2dd);
		set_memory_r(0xd7000, 0xd7fff, fd_bios_2hd);
		set_wait_r(0xd7000, 0xd7fff, exboards_wait); // OK?
		#endif
		#if defined(SUPPORT_16_COLORS)
		set_memory_mapped_io_rw(0xe0000, 0xe7fff, d_display);
		set_wait_rw(0xe0000, 0xe7fff, gvram_wait_val); // OK?
		#endif
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
	}
#else
	// ToDo: For HIRESO VMs
#endif
}

void MEMBUS::update_bios_ipl_and_itf()
{
#if defined(SUPPORT_ITF_ROM)
	if(itf_selected) {
//		unset_memory_w(0x00100000 - sizeof(bios), 0x000fffff);
		set_memory_r(0x00100000 - sizeof(itf), 0x000fffff, itf);
		set_wait_r(0x00100000 - sizeof(itf), 0x000fffff, introm_wait); // OK?
	} else
#endif
	{
#if defined(SUPPORT_BIOS_RAM)
		if(bios_ram_selected) {
			set_memory_rw(0x100000 - sizeof(bios), 0xfffff, ram + 0x100000 - sizeof(bios));
			set_wait_rw(0x00100000 - sizeof(bios), 0x000fffff, intram_wait); // OK?
		} else
#endif
		{
			set_memory_r(0x00100000 - sizeof(bios), 0x000fffff, bios);
			unset_memory_w(0x00100000 - sizeof(bios), 0x000fffff);
			set_wait_r(0x00100000 - sizeof(bios), 0x000fffff, introm_wait); // OK?
			set_wait_w(0x00100000 - sizeof(bios), 0x000fffff, 4); // OK?
		}
	}
//	set_wait_rw(0x00100000 - sizeof(bios), 0xfffff, introm_wait);
}

void MEMBUS::update_bios_window(uint32_t window_addr, uint32_t begin)
{
#if defined(SUPPORT_32BIT_ADDRESS) || defined(SUPPORT_24BIT_ADDRESS)
	uint32_t end = begin + 0x1ffff;
	if(sizeof(ram) > 0x10000) {
		set_wait_rw(0x00100000, (sizeof(ram) >= 0x00e00000) ? 0x00dfffff : (sizeof(ram) - 1), exmem_wait);
	}
	if((page08_intram_selected) /*&& (shadow_ram_selected)*/){
		set_wait_rw(begin, end, bank08_wait);
		if((window_addr == 0x80000) || (window_addr >= 0x100000)) {
			if((window_addr + 0x1ffff) < sizeof(ram)) {
				set_memory_rw(begin, end, &(ram[window_addr]));
				set_wait_rw(begin, end, intram_wait);
			} else {
				unset_memory_rw(begin, end);
				set_wait_rw(begin, end, 4);
			}
		} else if(window_addr == 0xa0000) {
			copy_table_rw(begin, 0xa0000, 0xbffff);
		} else if(window_addr == 0xc0000) {
		#if defined(SUPPORT_SHADOW_RAM)
			if(shadow_ram_selected) {
				set_memory_rw(begin, end, &(ram[window_addr]));
				set_wait_rw(begin, end, intram_wait);
			} else {
				copy_table_rw(begin, 0xc0000, 0xdffff);
			}
		#else
			copy_table_rw(begin, 0xc0000, 0xdffff);
		#endif
		} else if(window_addr == 0xe0000) {
			uint32_t head_address;
	#if !defined(SUPPORT_HIRESO)
			head_address = begin + 0x8000;
	#else
			head_address = begin + 0x10000;
	#endif
	#if defined(SUPPORT_SHADOW_RAM)
			if(shadow_ram_selected) {
				set_memory_rw(begin, head_address - 1, &(ram[window_addr]));
				set_memory_rw(head_address, end, ram + 0x100000 - sizeof(bios));
				set_wait_rw(begin, end, intram_wait);
			} else
	#endif
	#if defined(SUPPORT_BIOS_RAM)
			if(bios_ram_selected) {
				unset_memory_rw(begin, head_address - 1);
				set_memory_rw(head_address, end, ram + 0x100000 - sizeof(bios)); // Q? Will appear BIOS/ITF ROM? 20190730 K.O
				set_wait_rw(begin, end, intram_wait);
			} else
		#endif
			{
				unset_memory_rw(begin, end);
				set_wait_rw(begin, end, 4);
#if defined(SUPPORT_ITF_ROM)
				if(itf_selected) {
					set_memory_r(end - sizeof(itf) + 1, end , &(itf[0x00000]));
					set_wait_rw(end - sizeof(itf) + 1, end, introm_wait);
				} else
			#endif
				{
					set_memory_r(head_address, end, &(bios[0x00000]));
					set_wait_r(head_address, end, 4);
				}
			}
		} else {  // less than 0x80000h
			//set_memory_rw(begin, end, &(ram[begin]));
			unset_memory_rw(begin, end);
			set_wait_rw(begin, end, 4);
		}
	} else {
		// Internal RAM is not selected.
		// ToDo: Hi reso
		set_wait_rw(begin, end, bank08_wait);
		if(window_addr < 0x80000) {
			unset_memory_rw(begin, end);
			//set_memory_rw(0x80000, 0x9ffff, &(ram[window_addr]));
		} else if(window_addr == 0x80000) {
			// ToDo: External BUS
			unset_memory_rw(begin, end);
		} else if(window_addr == 0xa0000) {
			copy_table_rw(begin, 0xa0000, 0xbffff); // DISPLAY
		} else if(window_addr == 0xc0000) {
		#if defined(SUPPORT_SHADOW_RAM)
			if(shadow_ram_selected) {
				set_memory_rw(begin, end, &(ram[window_addr]));
			} else
		#endif
			{
				copy_table_rw(begin, 0xc0000, 0xdffff); // BOARD
			}
		} else if(window_addr == 0xe0000) {
		#if defined(SUPPORT_SHADOW_RAM)
			if(shadow_ram_selected) {
				set_memory_rw(begin, end, &(ram[window_addr]));
				//copy_table_rw(0x00080000, 0xe8000, 0xfffff); // BIOS
			} else
		#endif
			{
				copy_table_rw(begin, 0xe0000, 0xfffff); // BIOS
			}
		} else { // Upper 100000h
			//unset_memory_rw(0x00080000, 0x0009ffff);
			if((window_addr + 0x1ffff) < sizeof(ram)) {
				set_memory_rw(begin, end, &(ram[window_addr]));
			} else {
				unset_memory_rw(begin, end);
			}
		}
	}
#endif
}
void MEMBUS::update_bios()
{
	update_bios_mainmem();
#if !defined(SUPPORT_HIRESO)
	update_bios_extra_boards();
#else // HIRESO
#endif
	update_bios_ipl_and_itf();

#if defined(SUPPORT_32BIT_ADDRESS) || defined(SUPPORT_24BIT_ADDRESS)
	#if defined(SUPPORT_NEC_EMS)
	// Is It right?
	if((use_ems_as_protected) && ((ems_protected_base & 0xf00000) != 0)) {
		set_memory_rw(ems_protected_base, ems_protected_base + ((sizeof(nec_ems) >= 0x10000) ? 0xffff : (sizeof(nec_ems) - 1)), &(nec_ems[0]));
		set_wait_rw(ems_protected_base, ems_protected_base + ((sizeof(nec_ems) >= 0x10000) ? 0xffff : (sizeof(nec_ems) - 1)), exboards_wait);
		if(sizeof(nec_ems) < 0x10000) {
			unset_memory_rw(ems_protected_base + sizeof(nec_ems), ems_protected_base + 0xffff);
			set_wait_rw(ems_protected_base + sizeof(nec_ems), ems_protected_base + 0xffff, exboards_wait);
		}
	} else {
		if(sizeof(ram) > 0x100000) {
			set_memory_rw(0x100000, (sizeof(ram) >= 0xe00000) ? 0xdfffff : (sizeof(ram) - 1), ram + 0x100000);
			set_wait_rw(0x100000, (sizeof(ram) >= 0xe00000) ? 0xdfffff : (sizeof(ram) - 1), exmem_wait);
			unset_memory_rw((sizeof(ram) >= 0x00e00000) ? 0x00e00000 : sizeof(ram), 0x00efffff);
			set_wait_rw((sizeof(ram) >= 0x00e00000) ? 0x00e00000 : sizeof(ram), 0x00efffff, 4);
		} else {
			unset_memory_rw(0x00100000, 0x00efffff);
			set_wait_rw(0x00100000, 0x00efffff, 4);
		}
	}
	#endif
	if(sizeof(ram) > 0x10000) {
		set_wait_rw(0x00100000, (sizeof(ram) >= 0x00e00000) ? 0x00dfffff : (sizeof(ram) - 1), exmem_wait);
	}
#endif
	if(shadow_ram_selected) {
//		set_wait_rw(0xa0000, 0xbffff, bank08_wait);
		set_wait_rw(0xa0000, 0xbffff, intram_wait);
	} else {
		set_wait_rw(0xa0000, 0xbffff, intram_wait);
	}
#if defined(SUPPORT_32BIT_ADDRESS) || defined(SUPPORT_24BIT_ADDRESS)
	update_bios_window(window_80000h, 0x80000);
	update_bios_window(window_a0000h, 0xa0000);
#endif
#if 0
	/*if((page08_intram_selected) )*/{
		if((window_a0000h == 0xc0000)) {
	#if defined(_PC9801RA21) //defined(UPPER_I386) && defined(SUPPORT_BIOS_RAM)
			if(shadow_ram_selected) {
				//copy_table_rw(0xa0000, 0xc0000, 0xdffff);
				set_memory_rw(0xa0000, 0xbffff, &(ram[window_a0000h]));
			} else {
				copy_table_rw(0xa0000, 0xc0000, 0xdffff);
			}
	#else
			copy_table_rw(0xa0000, 0xc0000, 0xdffff);
	#endif
		} else 	if(window_a0000h == 0xe0000) {
	#if defined(_PC9801RA) || defined(_PC9801RA2) || defined(_PC9801RA21) //defined(UPPER_I386) && defined(SUPPORT_BIOS_RAM)
			if(shadow_ram_selected) {
		#if !defined(SUPPORT_HIRESO)
				//copy_table_rw(0xa0000, 0xe0000, 0xe7fff);
				set_memory_rw(0xa0000, 0xa7fff, &(ram[window_a0000h]));
			#if defined(SUPPORT_BIOS_RAM)
				set_memory_rw(0xa8000, 0xbffff, ram + 0x100000 - sizeof(bios));
			#endif
		#else
				//copy_table_rw(0xa0000, 0xe0000, 0xeffff);
				set_memory_rw(0xa0000, 0xaffff, &(ram[window_a0000h]));
			#if defined(SUPPORT_BIOS_RAM)
				set_memory_rw(0xb0000, 0xbffff, ram + 0x100000 - sizeof(bios));
			#endif
		#endif
			} else {
				copy_table_rw(0xa0000, 0xe0000, 0xfffff);
			}
	#else
			copy_table_rw(0xa0000, 0xe0000, 0xfffff);
	#endif
		} else if((window_a0000h >= 0x80000) && ((window_a0000h + 0x1ffff) < sizeof(ram)) && !((window_a0000h >= 0xa0000) && (window_a0000h <= 0xfffff))) {
			set_memory_rw(0xa0000, 0xbffff, &(ram[window_a0000h]));
		} else {
			if(window_a0000h >= 0x80000) {
				copy_table_rw(0x000a0000, window_a0000h, window_a0000h + 0x1ffff);
			} else {
				unset_memory_rw(0xa0000, 0xbffff);
			}

		}
	} //else {
	//	// NOOP
	//}
#endif
#if defined(SUPPORT_32BIT_ADDRESS) || defined(SUPPORT_24BIT_ADDRESS)
	#if !defined(SUPPORT_HIRESO)
	if((window_80000h >= 0xa0000) && (window_80000h <= 0xeffff)) {
		d_display->write_signal(SIG_DISPLAY98_SET_PAGE_80, window_80000h, 0xffffffff);
	}
	if((window_a0000h >= 0xa0000) && (window_a0000h <= 0xeffff)) {
		d_display->write_signal(SIG_DISPLAY98_SET_PAGE_A0, window_a0000h, 0xffffffff);
	}
	#else
	// ToDo
	#endif
#endif
	// ToDo: PC9821
#if defined(SUPPORT_32BIT_ADDRESS)
	unset_memory_rw(0x00f00000, (UPPER_MEMORY_32BIT & 0x00ffffff) - 1);
	#if !defined(SUPPORT_HIRESO)
	copy_table_rw(0x00ee8000, 0x000e8000, 0x000fffff);
	copy_table_rw(0x00fe8000, 0x000e8000, 0x000fffff);
	#endif
	copy_table_rw(UPPER_MEMORY_32BIT, (UPPER_MEMORY_32BIT & 0x000fffff), 0x000fffff);
	copy_table_rw((UPPER_MEMORY_32BIT & 0x00ffffff), (UPPER_MEMORY_32BIT & 0x000fffff), 0x000fffff);
#elif defined(SUPPORT_24BIT_ADDRESS)
	unset_memory_rw(0x00f00000, UPPER_MEMORY_24BIT - 1);
	copy_table_rw(UPPER_MEMORY_24BIT, UPPER_MEMORY_24BIT & 0x000fffff, 0x000fffff);
	#if !defined(SUPPORT_HIRESO)
	copy_table_rw(0x00ee8000, 0x000e8000, 0x000fffff);
	copy_table_rw(0x00fe8000, 0x000e8000, 0x000fffff);
	#endif
#endif
}


void MEMBUS::update_sound_bios()
{
	if((sound_bios_selected) && (sound_bios_load) && (using_sound_bios)){
//		if(sound_bios_selected) {
//			set_memory_rw(0xcc000, 0xcffff, sound_bios_ram);
//		} else {
			set_memory_r(0xcc000, 0xcffff, sound_bios);
			unset_memory_w(0xcc000, 0xcffff);
			set_wait_r(0xcc000, 0xcffff, exboards_wait);
			set_wait_w(0xcc000, 0xcffff, 4);
//		}
	} else {
		set_memory_r(0xcc000, 0xcffff, sound_bios);
		unset_memory_w(0xcc000, 0xcffff);
		set_wait_r(0xcc000, 0xcffff, exboards_wait);
		set_wait_w(0xcc000, 0xcffff, 4);
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
			set_wait_rw(0xd7000, 0xd7fff, exboards_wait);
		} else {
			set_memory_r(0xd7000, 0xd7fff, sasi_bios);
			unset_memory_w(0xd7000, 0xd7fff);
			set_wait_r(0xd7000, 0xd7fff, exboards_wait);
			set_wait_w(0xd7000, 0xd7fff, 4);
		}
	} else {
		unset_memory_rw(0xd7000, 0xd7fff);
		set_wait_rw(0xd7000, 0xd7fff, 4);
	}

}
#endif

#if defined(SUPPORT_SCSI_IF)
void MEMBUS::update_scsi_bios()
{
	if(scsi_bios_selected) {
		if(scsi_bios_ram_selected) {
			set_memory_rw(0xdc000, 0xdcfff, scsi_bios_ram);
			set_wait_rw(0xdc000, 0xdcfff, exboards_wait);
		} else {
			set_memory_r(0xdc000, 0xdcfff, scsi_bios);
			unset_memory_w(0xdc000, 0xdcfff);
			set_wait_r(0xdc000, 0xdcfff, exboards_wait);
			set_wait_w(0xdc000, 0xdcfff, 4);
		}
	} else {
		unset_memory_rw(0xdc000, 0xdcfff);
		set_wait_rw(0xdc000, 0xdcfff, 4);
	}
}
#endif

#if defined(SUPPORT_IDE_IF)
void MEMBUS::update_ide_bios()
{
	if(ide_bios_selected) {
		set_memory_r(0xd8000, 0xd9fff, &(ide_bios[ide_bios_bank * 0x2000]));
		unset_memory_w(0xd8000, 0xd9fff);
		set_wait_r(0xd8000, 0xd9fff, exboards_wait);
		set_wait_w(0xd8000, 0xdbfff, 4);
		set_memory_rw(0xda000, 0xdbfff, ide_bios_ram);
		set_wait_rw(0xda000, 0xdbfff, exboards_wait);
//		}
	} else {
		unset_memory_rw(0xd8000, 0xdbfff);
		set_wait_rw(0xd8000, 0xdbfff, 4);
	}

}
#endif

#if defined(SUPPORT_NEC_EMS)
void MEMBUS::update_nec_ems()
{
	if(nec_ems_selected) {
		unset_memory_rw(0xb0000, 0xbffff);
		set_memory_rw(0xb0000, 0xbffff, nec_ems);
		set_wait_rw(0xb0000, 0xbffff, slotmem_wait);
	} else {
		unset_memory_rw(0xb0000, 0xbffff);
		set_memory_mapped_io_rw(0xb0000, 0xbffff, d_display);
		set_wait_rw(0xb0000, 0xbffff, gvram_wait_val);
	}
}
#endif


#define STATE_VERSION	14

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
	state_fio->StateValue(bios_ram_selected);
 #endif
 #if defined(SUPPORT_ITF_ROM)
	state_fio->StateValue(itf_selected);
 #endif
 #if !defined(SUPPORT_HIRESO)
//	state_fio->StateArray(sound_bios_ram, sizeof(sound_bios_ram), 1);
	state_fio->StateValue(sound_bios_selected);
	state_fio->StateValue(sound_bios_load);
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
	state_fio->StateValue(ide_bios_bank);
	state_fio->StateArray(ide_bios_ram, sizeof(ide_bios_ram), 1);
	state_fio->StateValue(ide_bios_selected);
 #endif
 #if defined(SUPPORT_NEC_EMS)
	state_fio->StateArray(nec_ems, sizeof(nec_ems), 1);
	state_fio->StateValue(nec_ems_selected);
	state_fio->StateValue(use_ems_as_protected);
	state_fio->StateValue(ems_protected_base);
 #endif
 #endif
 #if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	state_fio->StateValue(dma_access_ctrl);
	state_fio->StateValue(window_80000h);
	state_fio->StateValue(window_a0000h);
 #endif
	state_fio->StateValue(page08_intram_selected);
	state_fio->StateValue(shadow_ram_selected);
	state_fio->StateValue(last_access_is_interam);

	state_fio->StateValue(intram_wait);
	state_fio->StateValue(bank08_wait);
	state_fio->StateValue(exmem_wait);
	state_fio->StateValue(slotmem_wait);
	state_fio->StateValue(exboards_wait);
	state_fio->StateValue(introm_wait);
	state_fio->StateValue(gvram_wait_val);
	state_fio->StateValue(tvram_wait_val);

	if(!MEMORY::process_state(state_fio, loading)) {
 		return false;
 	}

 	// post process
	if(loading) {
		config_intram();
		update_bios();
#if !defined(SUPPORT_HIRESO)
		using_sound_bios = ((config.sound_type & 1) == 0) ? true : false;
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
		dma_access_a20 = ((dma_access_ctrl & 0x04) != 0) ? false : true;
#endif
	}
 	return true;
}

}
