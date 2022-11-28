/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : tanam
	Date   : 2013.07.15-

	[ memory ]
*/

#include "memory.h"
#include "timer.h"

#define RAM		(MEMORY_BASE + RAM_BASE)
#define BASICROM	(MEMORY_BASE + BASICROM_BASE)
#define EXTROM		(MEMORY_BASE + EXTROM_BASE)
#define CGROM1		(MEMORY_BASE + CGROM1_BASE)
#define EmptyRAM	(MEMORY_BASE + EmptyRAM_BASE)

void MEMORY::initialize()
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("BASICROM.60")), FILEIO_READ_BINARY)) {
		fio->Fread(BASICROM, 0x4000, 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("CGROM60.60")), FILEIO_READ_BINARY)) {
		fio->Fread(CGROM1, 0x1000, 1);
		fio->Fclose();
	}
	delete fio;
	
	register_vline_event(this);
}

void MEMORY::reset()
{
	if (!inserted) {
		EXTROM1 = RAM + 0x4000;
		EXTROM2 = RAM + 0x6000;
		FILEIO* fio = new FILEIO();
		if (fio->Fopen(create_local_path(_T("EXTROM.60")), FILEIO_READ_BINARY)) {
			fio->Fread(EXTROM, 0x4000, 1);
			fio->Fclose();
			EXTROM1 = EXTROM;
			EXTROM2 = EXTROM + 0x2000;
			inserted = true;
		}
		delete fio;
	}
	memset(RAM ,0,0x10000);
	memset(EmptyRAM, 0, 0x2000);
	CGROM = CGROM1;
	VRAM = RAM;
	CGSW93 = CRTKILL = 0;
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	if (addr >= 0x4000 && addr < 0x8000 && !inserted) {
		RAM[addr] = data;
	} else if (addr >= 0x8000 && addr <=0xFFFF) {
		RAM[addr] = data;
	}
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	if (addr >= 0x0000 && addr < 0x4000) {
		return(BASICROM[addr]);
	} else if (addr >= 0x4000 && addr < 0x6000) {
		return(EXTROM1[addr-0x4000]);
	} else if (addr >= 0x6000 && addr < 0x8000) {
		if (CGSW93==0) {
			return(EXTROM2[addr-0x6000]);
		} else {
			return(CGROM1[addr-0x6000]);
		}
	} else {
		return(RAM[addr]);
	}
}

void MEMORY::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	*wait = addr < 0x8000 ? 1 : 0;
	write_data8(addr, data);
}

uint32_t MEMORY::read_data8w(uint32_t addr, int *wait)
{
	*wait = addr < 0x8000 ? 1 : 0;
	return read_data8(addr);
}

uint32_t MEMORY::fetch_op(uint32_t addr, int *wait)
{
	*wait = 1;
	return read_data8(addr);
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	uint16_t port=(addr & 0x00ff);
	uint8_t Value=data;
	return;
}

void MEMORY::write_io8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = (addr & 0xf0) == 0xa0 ? 1 : 0;
	write_io8(addr, data);
}

uint32_t MEMORY::read_io8w(uint32_t addr, int* wait)
{
	*wait = (addr & 0xf0) == 0xa0 ? 1 : 0;
	return read_io8(addr);
}

#define EVENT_HBLANK	1

void MEMORY::event_vline(int v, int clock)
{
	{
		if (!CRTKILL) {
			if (v < 192) {
				d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
				register_event_by_clock(this, EVENT_HBLANK, (uint64_t)((double)CPU_CLOCKS / FRAMES_PER_SEC / LINES_PER_FRAME * 296 / 455), false, NULL);
			}
		}
	}
}

void MEMORY::event_callback(int event_id, int err)
{
	if(event_id == EVENT_HBLANK) {
		d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
	}
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_PIO_PORT_C) {
		if(data & 4) {
			CGSW93=0;
		} else {
			CGSW93=1;
		}
		CRTKILL = (data & 2) ? 0 : 1;
		if (CRTKILL) {
			d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
		}
	}
}

void MEMORY::open_cart(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(EXTROM, 0x4000, 1);
		fio->Fclose();
		EXTROM1 = EXTROM;
		EXTROM2 = EXTROM + 0x2000;
		inserted = true;
	} else {
		EXTROM1 = RAM + 0x4000;
		EXTROM2 = RAM + 0x6000;
		inserted = false;
	}
	delete fio;
}

void MEMORY::close_cart()
{
	EXTROM1 = RAM + 0x4000;
	EXTROM2 = RAM + 0x6000;
	inserted = false;
}

#define STATE_VERSION	1

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(RAM, RAM_SIZE, 1);
	if(loading) {
		CGROM = MEMORY_BASE + state_fio->FgetInt32_LE();
		EXTROM1 = MEMORY_BASE + state_fio->FgetInt32_LE();
		EXTROM2 = MEMORY_BASE + state_fio->FgetInt32_LE();
		VRAM = MEMORY_BASE + state_fio->FgetInt32_LE();
	} else {
		state_fio->FputInt32_LE((int)(CGROM - MEMORY_BASE));
		state_fio->FputInt32_LE((int)(EXTROM1 - MEMORY_BASE));
		state_fio->FputInt32_LE((int)(EXTROM2 - MEMORY_BASE));
		state_fio->FputInt32_LE((int)(VRAM - MEMORY_BASE));
	}
	state_fio->StateValue(CGSW93);
	state_fio->StateValue(inserted);
	return true;
}
