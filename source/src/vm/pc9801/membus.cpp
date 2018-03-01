/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
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
		E0000h - E7FFFh: VRAM (I)
		E8000h - FFFFFh: IPL

	HIRESO PC-98XA/XL/XL^2/RL
		00000h - 7FFFFh: RAM
		80000h - BFFFFh: MEMORY WINDOW
		C0000h - DFFFFh: VRAM
		E0000h - E1FFFh: TEXT VRAM
		E2000h - E3FFFh: ATTRIBUTE
		E4000h - E4FFFh: CG WINDOW
		F0000h - FFFFFh: IPL
*/

void MEMBUS::initialize()
{
	MEMORY::initialize();
	
	// RAM
	memset(ram, 0, sizeof(ram));
#if !defined(SUPPORT_HIRESO)
	set_memory_rw(0x000000, 0x09ffff, ram);
#else
	set_memory_rw(0x00000, 0xbffff, ram);
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	if(sizeof(ram) > 0x100000) {
		set_memory_rw(0x100000, sizeof(ram) - 1, ram + 0x100000);
	}
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
	memset(ipl, 0xff, sizeof(ipl));
	if(!read_bios(_T("IPL.ROM"), ipl, sizeof(ipl))) {
		read_bios(_T("BIOS.ROM"), ipl, sizeof(ipl));
	}
#if defined(SUPPORT_ITF_ROM)
	memset(itf, 0xff, sizeof(itf));
	read_bios(_T("ITF.ROM"), itf, sizeof(itf));
#endif
#if defined(_PC9801) || defined(_PC9801E)
	memset(fd_bios_2hd, 0xff, sizeof(fd_bios_2hd));
	read_bios(_T("2HDIF.ROM"), fd_bios_2hd, sizeof(fd_bios_2hd));
	set_memory_r(0xd6000, 0xd6fff, fd_bios_2dd);
	
	memset(fd_bios_2dd, 0xff, sizeof(fd_bios_2dd));
	read_bios(_T("2DDIF.ROM"), fd_bios_2dd, sizeof(fd_bios_2dd));
	set_memory_r(0xd7000, 0xd7fff, fd_bios_2hd);
#endif
#if !defined(SUPPORT_HIRESO)
#if defined(SUPPORT_SASI)
	memset(sasi_bios, 0xff, sizeof(sasi_bios));
	read_bios(_T("SASI.ROM"), sasi_bios, sizeof(sasi_bios));
#endif
	
	memset(sound_bios, 0xff, sizeof(sound_bios));
	if(config.sound_type == 0) {
		d_display->sound_bios_ok = (read_bios(_T("SOUND.ROM"), sound_bios, sizeof(sound_bios)) != 0);
		set_memory_r(0xcc000, 0xcffff, sound_bios);
	} else if(config.sound_type == 2) {
		d_display->sound_bios_ok = (read_bios(_T("MUSIC.ROM"), sound_bios, sizeof(sound_bios)) != 0);
		set_memory_r(0xcc000, 0xcffff, sound_bios);
	} else
#endif
	d_display->sound_bios_ok = false;
}

void MEMBUS::reset()
{
	MEMORY::reset();
	
#if defined(SUPPORT_ITF_ROM)
	unset_memory_r(0x100000 - sizeof(ipl), 0xfffff);
	set_memory_r(0x100000 - sizeof(itf), 0xfffff, itf);
	itf_selected = true;
#else
	set_memory_r(0x100000 - sizeof(ipl), 0xfffff, ipl);
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	dma_access_ctrl = 0x00;
#if !defined(SUPPORT_HIRESO)
	window_80000h = 0x80000;
	window_a0000h = 0xa0000;
#else
	window_80000h = 0x100000;
	window_a0000h = 0x120000;
#endif
#endif
}

void MEMBUS::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
#if defined(SUPPORT_ITF_ROM)
	case 0x043d:
		switch(data & 0xff) {
		case 0x00:
		case 0x10:
		case 0x18:
			unset_memory_r(0x100000 - sizeof(ipl), 0xfffff);
			set_memory_r(0x100000 - sizeof(itf), 0xfffff, itf);
			itf_selected = true;
			break;
		case 0x02:
		case 0x12:
			set_memory_r(0x100000 - sizeof(ipl), 0xfffff, ipl);
			itf_selected = false;
			break;
		}
		break;
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	case 0x0439:
		dma_access_ctrl = data;
		break;
#if !defined(SUPPORT_HIRESO)
	case 0x0461:
#else
	case 0x0091:
#endif
		window_80000h = (data & 0xfe) << 16;
		break;
#if !defined(SUPPORT_HIRESO)
	case 0x0463:
#else
	case 0x0093:
#endif
		window_a0000h = (data & 0xfe) << 16;
		break;
#endif
#if defined(SUPPORT_32BIT_ADDRESS)
	case 0x053d:
#if !defined(SUPPORT_HIRESO)
		if(data & 0x80) {
			set_memory_r(0xcc000, 0xcffff, sound_bios);
		} else {
			unset_memory_r(0xcc000, 0xcffff);
		}
#if defined(SUPPORT_SASI)
		if(data & 0x40) {
			set_memory_r(0xd7000, 0xd7fff, sasi_bios);
		} else {
			unset_memory_r(0xd7000, 0xd7fff);
		}
#endif
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
	switch(addr) {
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	case 0x0439:
		return dma_access_ctrl;
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
#endif
	// dummy for no cases
	default:
		break;
	}
	return 0xff;
}

#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
#if !defined(SUPPORT_HIRESO)
	#define UPPER_MEMORY_ADDR 0xfa0000
#else
	#define UPPER_MEMORY_ADDR 0xfc0000
#endif
uint32_t MEMBUS::read_data8(uint32_t addr)
{
#if defined(SUPPORT_32BIT_ADDRESS)
	if(addr < 0x1000000) {
#endif
		if(addr >= UPPER_MEMORY_ADDR) {
			addr &= 0xfffff;
		}
		if(addr >= 0x80000 && addr < 0xa0000) {
			if((addr = (addr & 0x1ffff) | window_80000h) >= UPPER_MEMORY_ADDR) {
				addr &= 0xfffff;
			}
		} else if(addr >= 0xa0000 && addr < 0xc0000) {
			if((addr = (addr & 0x1ffff) | window_a0000h) >= UPPER_MEMORY_ADDR) {
				addr &= 0xfffff;
			}
		}
		return MEMORY::read_data8(addr);
#if defined(SUPPORT_32BIT_ADDRESS)
	} else if(addr >= 0xfff00000) {
		return this->read_data8(addr & 0xffffff);
	} else {
		return 0xff;
	}
#endif
}

void MEMBUS::write_data8(uint32_t addr, uint32_t data)
{
#if defined(SUPPORT_32BIT_ADDRESS)
	if(addr < 0x1000000) {
#endif
		if(addr >= UPPER_MEMORY_ADDR) {
			addr &= 0xfffff;
		}
		if(addr >= 0x80000 && addr < 0xa0000) {
			if((addr = (addr & 0x1ffff) | window_80000h) >= UPPER_MEMORY_ADDR) {
				addr &= 0xfffff;
			}
		} else if(addr >= 0xa0000 && addr < 0xc0000) {
			if((addr = (addr & 0x1ffff) | window_a0000h) >= UPPER_MEMORY_ADDR) {
				addr &= 0xfffff;
			}
		}
		MEMORY::write_data8(addr, data);
#if defined(SUPPORT_32BIT_ADDRESS)
	} else if(addr >= 0xfff00000) {
		this->write_data8(addr & 0xffffff, data);
	}
#endif
}

uint32_t MEMBUS::read_data16(uint32_t addr)
{
#if defined(SUPPORT_32BIT_ADDRESS)
	if(addr < 0x1000000) {
#endif
		if(addr >= UPPER_MEMORY_ADDR) {
			addr &= 0xfffff;
		}
		if(addr >= 0x80000 && addr < 0xa0000) {
			if((addr = (addr & 0x1ffff) | window_80000h) >= UPPER_MEMORY_ADDR) {
				addr &= 0xfffff;
			}
		} else if(addr >= 0xa0000 && addr < 0xc0000) {
			if((addr = (addr & 0x1ffff) | window_a0000h) >= UPPER_MEMORY_ADDR) {
				addr &= 0xfffff;
			}
		}
		return MEMORY::read_data16(addr);
#if defined(SUPPORT_32BIT_ADDRESS)
	} else if(addr >= 0xfff00000) {
		return this->read_data16(addr & 0xffffff);
	} else {
		return 0xffff;
	}
#endif
}

void MEMBUS::write_data16(uint32_t addr, uint32_t data)
{
#if defined(SUPPORT_32BIT_ADDRESS)
	if(addr < 0x1000000) {
#endif
		if(addr >= UPPER_MEMORY_ADDR) {
			addr &= 0xfffff;
		}
		if(addr >= 0x80000 && addr < 0xa0000) {
			if((addr = (addr & 0x1ffff) | window_80000h) >= UPPER_MEMORY_ADDR) {
				addr &= 0xfffff;
			}
		} else if(addr >= 0xa0000 && addr < 0xc0000) {
			if((addr = (addr & 0x1ffff) | window_a0000h) >= UPPER_MEMORY_ADDR) {
				addr &= 0xfffff;
			}
		}
		MEMORY::write_data16(addr, data);
#if defined(SUPPORT_32BIT_ADDRESS)
	} else if(addr >= 0xfff00000) {
		this->write_data16(addr & 0xffffff, data);
	}
#endif
}

uint32_t MEMBUS::read_data32(uint32_t addr)
{
#if defined(SUPPORT_32BIT_ADDRESS)
	if(addr < 0x1000000) {
#endif
		if(addr >= UPPER_MEMORY_ADDR) {
			addr &= 0xfffff;
		}
		if(addr >= 0x80000 && addr < 0xa0000) {
			if((addr = (addr & 0x1ffff) | window_80000h) >= UPPER_MEMORY_ADDR) {
				addr &= 0xfffff;
			}
		} else if(addr >= 0xa0000 && addr < 0xc0000) {
			if((addr = (addr & 0x1ffff) | window_a0000h) >= UPPER_MEMORY_ADDR) {
				addr &= 0xfffff;
			}
		}
		return MEMORY::read_data32(addr);
#if defined(SUPPORT_32BIT_ADDRESS)
	} else if(addr >= 0xfff00000) {
		return this->read_data32(addr & 0xffffff);
	} else {
		return 0xffffffff;
	}
#endif
}

void MEMBUS::write_data32(uint32_t addr, uint32_t data)
{
#if defined(SUPPORT_32BIT_ADDRESS)
	if(addr < 0x1000000) {
#endif
		if(addr >= UPPER_MEMORY_ADDR) {
			addr &= 0xfffff;
		}
		if(addr >= 0x80000 && addr < 0xa0000) {
			if((addr = (addr & 0x1ffff) | window_80000h) >= UPPER_MEMORY_ADDR) {
				addr &= 0xfffff;
			}
		} else if(addr >= 0xa0000 && addr < 0xc0000) {
			if((addr = (addr & 0x1ffff) | window_a0000h) >= UPPER_MEMORY_ADDR) {
				addr &= 0xfffff;
			}
		}
		MEMORY::write_data32(addr, data);
#if defined(SUPPORT_32BIT_ADDRESS)
	} else if(addr >= 0xfff00000) {
		this->write_data32(addr & 0xffffff, data);
	}
#endif
}

uint32_t MEMBUS::read_dma_data8(uint32_t addr)
{
	if(dma_access_ctrl & 4) {
		addr &= 0xfffff;
	}
	return this->read_data8(addr);
}

void MEMBUS::write_dma_data8(uint32_t addr, uint32_t data)
{
	if(dma_access_ctrl & 4) {
		addr &= 0xfffff;
	}
	this->write_data8(addr, data);
}

uint32_t MEMBUS::read_dma_data16(uint32_t addr)
{
	if(dma_access_ctrl & 4) {
		addr &= 0xfffff;
	}
	return this->read_data16(addr);
}

void MEMBUS::write_dma_data16(uint32_t addr, uint32_t data)
{
	if(dma_access_ctrl & 4) {
		addr &= 0xfffff;
	}
	this->write_data16(addr, data);
}

uint32_t MEMBUS::read_dma_data32(uint32_t addr)
{
	if(dma_access_ctrl & 4) {
		addr &= 0xfffff;
	}
	return this->read_data32(addr);
}

void MEMBUS::write_dma_data32(uint32_t addr, uint32_t data)
{
	if(dma_access_ctrl & 4) {
		addr &= 0xfffff;
	}
	this->write_data32(addr, data);
}
#endif

#define STATE_VERSION	2

void MEMBUS::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
#if defined(SUPPORT_ITF_ROM)
	state_fio->FputBool(itf_selected);
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	state_fio->FputUint8(dma_access_ctrl);
	state_fio->FputInt32(window_80000h);
	state_fio->FputInt32(window_a0000h);
#endif
	MEMORY::save_state(state_fio);
}

bool MEMBUS::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(ram, sizeof(ram), 1);
#if defined(SUPPORT_ITF_ROM)
	itf_selected = state_fio->FgetBool();
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	dma_access_ctrl = state_fio->FgetUint8();
	window_80000h = state_fio->FgetUint32();
	window_a0000h = state_fio->FgetUint32();
#endif
	
	// post process
#if defined(SUPPORT_ITF_ROM)
	if(itf_selected) {
		unset_memory_r(0x100000 - sizeof(ipl), 0xfffff);
		set_memory_r(0x100000 - sizeof(itf), 0xfffff, itf);
	} else {
		set_memory_r(0x100000 - sizeof(ipl), 0xfffff, ipl);
	}
#endif
	return MEMORY::load_state(state_fio);
}
