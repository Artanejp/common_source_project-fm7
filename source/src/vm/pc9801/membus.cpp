/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2017.06.22-

	[ memory bus ]
*/

#include "membus.h"

void MEMBUS::initialize()
{
	MEMORY::initialize();
	
	memset(ipl, 0xff, sizeof(ipl));
	if(!read_bios(_T("IPL.ROM"), ipl, sizeof(ipl))) {
		read_bios(_T("BIOS.ROM"), ipl, sizeof(ipl));
	}
#if defined(SUPPORT_ITF_ROM)
	memset(itf, 0xff, sizeof(itf));
	read_bios(_T("ITF.ROM"), itf, sizeof(itf));
#endif
	
}

void MEMBUS::reset()
{
	MEMORY::reset();
	
#if defined(SUPPORT_ITF_ROM)
	set_memory_r(0xf8000, 0xfffff, itf);
	itf_selected = true;
#else
	set_memory_r(0x100000 - sizeof(ipl), 0xfffff, ipl);
#endif
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	dma_access_ctrl = 0x00;
	window_80000h = 0x80000;
	window_a0000h = 0xa0000;
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
			set_memory_r(0xf8000, 0xfffff, itf);
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
uint32_t MEMBUS::read_data8(uint32_t addr)
{
#if defined(SUPPORT_32BIT_ADDRESS)
	if(addr < 0x1000000) {
#endif
		if(addr >= 0xf00000) {
			addr &= 0xfffff;
		}
		if(addr >= 0x80000 && addr < 0xa0000) {
			if(window_80000h != 0x80000) {
				return this->read_data8((addr & 0x1ffff) | window_80000h);
			}
		} else if(addr >= 0xa0000 && addr < 0xc0000) {
			if(window_a0000h != 0xa0000) {
				return this->read_data8((addr & 0x1ffff) | window_a0000h);
			}
		}
		return MEMORY::read_data8(addr);
#if defined(SUPPORT_32BIT_ADDRESS)
	} else if(addr >= 0xfff00000) {
		return this->read_data8(addr & 0xfffff);
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
		if(addr >= 0xf00000) {
			addr &= 0xfffff;
		}
		if(addr >= 0x80000 && addr < 0xa0000) {
			if(window_80000h != 0x80000) {
				this->write_data8((addr & 0x1ffff) | window_80000h, data);
				return;
			}
		} else if(addr >= 0xa0000 && addr < 0xc0000) {
			if(window_a0000h != 0xa0000) {
				this->write_data8((addr & 0x1ffff) | window_a0000h, data);
				return;
			}
		}
		MEMORY::write_data8(addr, data);
#if defined(SUPPORT_32BIT_ADDRESS)
	} else if(addr >= 0xfff00000) {
		this->write_data8(addr & 0xfffff, data);
	}
#endif
}

uint32_t MEMBUS::read_data16(uint32_t addr)
{
#if defined(SUPPORT_32BIT_ADDRESS)
	if(addr < 0x1000000) {
#endif
		if(addr >= 0xf00000) {
			addr &= 0xfffff;
		}
		if(addr >= 0x80000 && addr < 0xa0000) {
			if(window_80000h != 0x80000) {
				return this->read_data16((addr & 0x1ffff) | window_80000h);
			}
		} else if(addr >= 0xa0000 && addr < 0xc0000) {
			if(window_a0000h != 0xa0000) {
				return this->read_data16((addr & 0x1ffff) | window_a0000h);
			}
		}
		return MEMORY::read_data16(addr);
#if defined(SUPPORT_32BIT_ADDRESS)
	} else if(addr >= 0xfff00000) {
		return this->read_data16(addr & 0xfffff);
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
		if(addr >= 0xf00000) {
			addr &= 0xfffff;
		}
		if(addr >= 0x80000 && addr < 0xa0000) {
			if(window_80000h != 0x80000) {
				this->write_data16((addr & 0x1ffff) | window_80000h, data);
				return;
			}
		} else if(addr >= 0xa0000 && addr < 0xc0000) {
			if(window_a0000h != 0xa0000) {
				this->write_data16((addr & 0x1ffff) | window_a0000h, data);
				return;
			}
		}
		MEMORY::write_data16(addr, data);
#if defined(SUPPORT_32BIT_ADDRESS)
	} else if(addr >= 0xfff00000) {
		this->write_data16(addr & 0xfffff, data);
	}
#endif
}

uint32_t MEMBUS::read_data32(uint32_t addr)
{
#if defined(SUPPORT_32BIT_ADDRESS)
	if(addr < 0x1000000) {
#endif
		if(addr >= 0xf00000) {
			addr &= 0xfffff;
		}
		if(addr >= 0x80000 && addr < 0xa0000) {
			if(window_80000h != 0x80000) {
				return this->read_data32((addr & 0x1ffff) | window_80000h);
			}
		} else if(addr >= 0xa0000 && addr < 0xc0000) {
			if(window_a0000h != 0xa0000) {
				return this->read_data32((addr & 0x1ffff) | window_a0000h);
			}
		}
		return MEMORY::read_data32(addr);
#if defined(SUPPORT_32BIT_ADDRESS)
	} else if(addr >= 0xfff00000) {
		return this->read_data32(addr & 0xfffff);
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
		if(addr >= 0xf00000) {
			addr &= 0xfffff;
		}
		if(addr >= 0x80000 && addr < 0xa0000) {
			if(window_80000h != 0x80000) {
				this->write_data32((addr & 0x1ffff) | window_80000h, data);
				return;
			}
		} else if(addr >= 0xa0000 && addr < 0xc0000) {
			if(window_a0000h != 0xa0000) {
				this->write_data32((addr & 0x1ffff) | window_a0000h, data);
				return;
			}
		}
		MEMORY::write_data32(addr, data);
#if defined(SUPPORT_32BIT_ADDRESS)
	} else if(addr >= 0xfff00000) {
		this->write_data32(addr & 0xfffff, data);
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

#define STATE_VERSION	1

void MEMBUS::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
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
		set_memory_r(0x0f8000, 0x0fffff, itf);
	} else {
		set_memory_r(0x100000 - sizeof(ipl), 0x0fffff, ipl);
	}
#endif
	return MEMORY::load_state(state_fio);
}
